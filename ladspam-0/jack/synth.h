#ifndef LADSPAM_M_JACK_INCLUDED_HH
#define LADSPAM_M_JACK_INCLUDED_HH

#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_concepts.hpp>
#include <utility>

#include <unistd.h>
#include <jack/jack.h>

#include <ladspamm-0/plugin.h>
#include <ladspamm-0/library.h>
#include <ladspamm-0/plugin_instance.h>

#include <ladspam-0/synth.h>
#include <ladspam-0/private/ringbuffer.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>

namespace ladspam
{
	namespace jack
	{
		/* 
			m like modular...
		*/
		struct synth : ladspam::synth 
		{
			synth(const std::string &client_name, unsigned control_period);

			virtual ~synth()
			{
				for 
				(
					unsigned plugin_index = 0, plugin_index_max = m_plugins.size(); 
					plugin_index < plugin_index_max; 
					++plugin_index
				)
				{
					remove_plugin(0);
				}
				
				jack_deactivate(m_jack_client);
				jack_client_close(m_jack_client);
			}

			virtual unsigned number_of_plugins() const
			{
				return m_plugins.size();
			}

			virtual void remove_plugin(unsigned index)
			{
				set_active(false);
				
				{
					m_plugins.erase(m_plugins.begin() + index);
				}
				
				set_active(true);
			}

			virtual void insert_plugin
			(
				unsigned index, 
				const std::string &library, 
				const std::string& label
			)
			{
				ladspamm::plugin_instance_ptr instance = load_ladspa_plugin(library, label);
				plugin_ptr the_plugin(new plugin(instance, m_jack_client, m_plugin_counter++, m_control_period));

				
				set_active(false);
				
				{
					m_plugins.insert(m_plugins.begin() + index, the_plugin);
				}
				
				set_active(true);
			}

			virtual void connect
			(
				unsigned source_plugin_index,
				unsigned source_port_index,
				unsigned sink_plugin_index,
				unsigned sink_port_index
			)
			{
				jack_connect
				(
					m_jack_client, 
					jack_port_name(m_plugins[source_plugin_index]->synth_ports[source_port_index]),
					jack_port_name(m_plugins[sink_plugin_index]->synth_ports[sink_port_index])
				);
			}

			virtual void disconnect
			(
				unsigned source_plugin_index,
				unsigned source_port_index,
				unsigned sink_plugin_index,
				unsigned sink_port_index
			)
			{
				jack_disconnect
				(
					m_jack_client, 
					jack_port_name(m_plugins[source_plugin_index]->synth_ports[source_port_index]),
					jack_port_name(m_plugins[sink_plugin_index]->synth_ports[sink_port_index])
				);
			}

			virtual bool set_port_value
			(
				unsigned plugin_index,
				unsigned port_index,
				float value
			)
			{
				set_port_value_command command;

				command.m_plugin_index = plugin_index;
				command.m_port_index = port_index;
				command.m_value = value;
				
				if (false == m_port_value_commands.can_write())
				{
					return false;
				}

				m_port_value_commands.write(command);
				
				return true;
			}

			inline void process_plugin
			(
				jack_nframes_t nframes, 
				unsigned plugin_index, 
				unsigned control_period,
				unsigned number_of_chunks
			)
			{
				plugin_ptr &p = m_plugins[plugin_index];
				
				ladspamm::plugin_instance &instance = *(p->m_plugin_instance);
				
				for (unsigned chunk_index = 0; chunk_index < number_of_chunks; ++chunk_index)
				{
					for 
					(
						unsigned port_index = 0, port_index_max = instance.the_plugin->port_count(); 
						port_index < port_index_max; 
						++port_index
					)
					{
						if (0 == jack_port_connected(p->synth_ports[port_index]))
						{
							instance.connect_port(port_index, &p->m_port_values[port_index][0]);
						}
						else
						{
							instance.connect_port
							(
								port_index, 
								((float*)jack_port_get_buffer(p->synth_ports[port_index], nframes)) + (chunk_index * control_period)
							);
						}
						
					}
					
					instance.run(control_period);
				}
			}
			
			inline void process_active(jack_nframes_t nframes)
			{
				unsigned control_period = std::min(jack_get_buffer_size(m_jack_client), m_control_period);
				unsigned number_of_chunks = jack_get_buffer_size(m_jack_client) / control_period;

				for (unsigned plugin_index = 0; plugin_index < m_plugins.size(); ++plugin_index)
				{
					process_plugin(nframes, plugin_index, control_period, number_of_chunks);
				}
			}
			
			virtual int process(jack_nframes_t nframes)
			{
				if (true == m_active)
				{
					while (true == m_port_value_commands.can_read())
					{
						set_port_value_command command = m_port_value_commands.read();
						
						std::vector<float> &port_values = m_plugins[command.m_plugin_index]->m_port_values[command.m_port_index];
						
						port_values[0] = command.m_value;
					}
					
					process_active(nframes);
				}

				while (true == m_activation_commands.can_read())
				{
					m_active = m_activation_commands.read();
					m_activation_acknowledgements.write(m_active);
				}
				
				return 0;
			}
			
			protected:
				jack_client_t *m_jack_client;

				unsigned m_control_period;
				
				unsigned m_plugin_counter;

				bool m_active;

				struct set_port_value_command
				{
					unsigned m_plugin_index;
					unsigned m_port_index;
					float m_value;
				};

				ringbuffer<set_port_value_command> m_port_value_commands;

				ringbuffer<bool> m_activation_commands;

				ringbuffer<bool> m_activation_acknowledgements;

				struct plugin
				{
					ladspamm::plugin_instance_ptr m_plugin_instance;
					std::vector<jack_port_t *> synth_ports;
					std::vector<std::vector<float> > m_port_values;
					jack_client_t *m_jack_client;
					
					plugin
					(
						ladspamm::plugin_instance_ptr plugin_instance,
						jack_client_t *jack_client,
						unsigned plugin_counter,
						unsigned control_period
					) :
						m_plugin_instance(plugin_instance),
						m_jack_client(jack_client)
					{
						ladspamm::plugin_ptr plugin = m_plugin_instance->the_plugin;
						
						for (unsigned index = 0; index < plugin->port_count(); ++index)
						{
							unsigned long port_flags = 0;

							std::vector<float> port_values;
							port_values.resize(control_period);

							if (plugin->port_is_input(index))
							{
								port_flags |= JackPortIsInput;
								
								port_values[0] = m_plugin_instance->port_default_guessed(index);
							}
							
							if (plugin->port_is_output(index))
							{
								port_flags |= JackPortIsOutput;
							}
							
							m_port_values.push_back(port_values);
							
							std::stringstream port_name_stream;
							
							port_name_stream 
								<< plugin_counter 
								<< "-" << index 
								<< "-" <<  plugin->label() 
								<< "-" << plugin->port_name(index);
							
							jack_port_t *port = jack_port_register
							(
								m_jack_client,
								port_name_stream.str().c_str(),
								JACK_DEFAULT_AUDIO_TYPE,
								port_flags,
								0
							);
							
							if (NULL == port)
							{
								throw std::runtime_error("Failed to register port");
							}
							
							synth_ports.push_back(port);
						}
					}
					
					~plugin()
					{
						m_plugin_instance->deactivate();
						
						for (unsigned port_index = 0; port_index < synth_ports.size(); ++port_index)
						{
							jack_port_unregister(m_jack_client, synth_ports[port_index]);
						}
					}
				};
				
				typedef boost::shared_ptr<plugin> plugin_ptr;

				/*
					The protocol to update the plugins is the
					following:
					
					1] set_active(false);
					
					2] update the plugins, register ports. etc..
					
					3] set_active(true)
				*/
				std::vector<plugin_ptr> m_plugins;

				/*
					Blocks until the process
					thread has acknowledged the change.

					Returns true in case of success,
					false in case of failure.
				*/
				bool set_active(bool active)
				{
					if (false == m_activation_commands.can_write())
					{
						return false;
					}

					m_activation_commands.write(active);

					while (false == m_activation_acknowledgements.can_read())
					{
						usleep(10000);
					}

					m_activation_acknowledgements.read();
					return true;
				}
				
				ladspamm::plugin_instance_ptr load_ladspa_plugin(std::string library, std::string label)
				{
					ladspamm::library_ptr lib(new ladspamm::library(library));
					
					for (unsigned index = 0; index < lib->plugins.size(); ++index)
					{
						if (lib->plugins[index]->label() == label)
						{
							ladspamm::plugin_instance_ptr instance
							(
								new ladspamm::plugin_instance
								(
									lib->plugins[index], 
									jack_get_sample_rate(m_jack_client)
								)
							);
							
							instance->activate();
							return instance;
						}
					}
					
					return ladspamm::plugin_instance_ptr();
				}
		};
	}
}

#endif
