#ifndef LADSPAM_INCLUDED_HH
#define LADSPAM_INCLUDED_HH

#include <boost/shared_ptr.hpp>
#include <utility>

#include <unistd.h>
#include <jack/jack.h>

#include <ladspamm-0/plugin.h>
#include <ladspamm-0/library.h>
#include <ladspamm-0/plugin_instance.h>

#include <ladspam-0/m.h>
#include <ladspam-0/ringbuffer.h>

#include <stdexcept>

namespace ladspam
{
	extern "C"
	{
		int jack_process(jack_nframes_t nframes, void* arg);
	}

	/* 
		m like modular...
	*/
	struct m_jack : m 
	{
		m_jack(const std::string &client_name) :
			m_jack_client(jack_client_open(client_name.c_str(), JackUseExactName, 0)),
			m_active(true),
			m_port_value_commands(1024),
			m_activation_commands(1024),
			m_activation_acknowledgements(1024)
		{
			if (NULL == m_jack_client)
			{
				throw std::runtime_error("Failed to open jack client");
			}
			
			if (0 != jack_set_process_callback(m_jack_client, jack_process, this))
			{
				throw std::runtime_error("Failed to set jack process callback");
			}
		
			jack_activate(m_jack_client);
		}

		virtual ~m_jack()
		{
			jack_deactivate(m_jack_client);
			jack_client_close(m_jack_client);
		}

		virtual unsigned number_of_plugins() const
		{
			return m_plugins.size();
		}

		virtual void remove_plugin(unsigned index)
		{

		}

		virtual void insert_plugin
		(
			unsigned index, 
			const std::string &library, 
			const std::string& label
		)
		{
			plugin the_plugin;

			the_plugin.m_plugin_instance = load_ladspa_plugin(library, label);
			
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

		}

		virtual void disconnect
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index
		)
		{

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

		void process_active(jack_nframes_t nframes)
		{
			
		}
		
		int process(jack_nframes_t nframes)
		{
			if (true == m_active)
			{
				while (true == m_port_value_commands.can_read())
				{
					set_port_value_command command = m_port_value_commands.read();
					m_plugins[command.m_plugin_index].m_port_values[command.m_port_index] = command.m_value;
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
				std::vector<jack_port_t *> m_jack_ports;
				std::vector<float> m_port_values;
			};

			/*
				The protocol to update the plugins is the
				following:
				
				1] set_active(false);
				
				2] update the plugins, register ports. etc..
				
				3] set_active(true)
			*/
			std::vector<plugin> m_plugins;

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

#endif

