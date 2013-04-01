#ifndef LADSPAM_M_INCLUDED_HH
#define LADSPAM_M_INCLUDED_HH

#include <string>
#include <vector>

#include <ladspamm-0/library.h>
#include <ladspamm-0/plugin_instance.h>

#include <boost/optional.hpp>
#include <boost/concept_check.hpp>

namespace ladspam
{

struct m;

struct m;
	/*
		The one unique interface.. Subclasses for
		special implementations might offer yet more
		derived types, e.g. ladspam::jack::instrument.
		
		Subclasses for special implementations also 
		offer unique ways to expose some or all unconnected
		plugin ports to the "outside", e.g. ladspam::jack::synth.
	*/
	struct synth 
	{
		typedef std::vector<float> buffer;

		typedef boost::shared_ptr<buffer> buffer_ptr;

		synth(unsigned sample_rate, unsigned control_period) :
			m_control_period(control_period),
			m_sample_rate(sample_rate),
			m_frame_in_chunk(0)
		{
			
		}
		
		virtual ~synth()
		{

		}

		virtual unsigned number_of_plugins() const
		{
			return m_plugins.size();
		}

		/*
			The parameter index must be in the
			range 0 <= index < number_of_plugins().
		*/
		virtual void remove_plugin(unsigned index)
		{
			m_plugins.erase(m_plugins.begin() + index);
		}

		/*
			The parameter index must be in the
			range 0 <= index < number_of_plugins().
		*/
		virtual void insert_plugin
		(
			unsigned index, 
			const std::string &library, 
			const std::string& label
		)
		{
			plugin_ptr the_plugin(new plugin(load_ladspa_plugin(library, label), m_control_period));
			m_plugins.insert(m_plugins.begin() + index, the_plugin);
		}
		
		virtual void append_plugin
		(
			const std::string &library, 
			const std::string& label
		)
		{
			insert_plugin(number_of_plugins(), library, label);	
		}
		
		void connect
		(
			unsigned sink_plugin_index,
			unsigned sink_port_index,
			buffer_ptr buffer
		)
		{
			
		}

		inline int find_connection_index
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index
		)
		{
			plugin_ptr sink_plugin = m_plugins[sink_plugin_index];
			
			plugin_ptr source_plugin = m_plugins[source_plugin_index];
			
			for (unsigned port_index = 0; port_index < sink_plugin->m_port_buffers.size(); ++port_index)
			{
				std::vector<buffer_ptr> connections = sink_plugin->m_connections[sink_port_index];
				for (unsigned connection_index = 0; connection_index < connections.size(); ++connection_index)
				{
					if (connections[connection_index] == source_plugin->m_port_buffers[source_port_index])
					{
						connections.erase(connections.begin() + connection_index);
						return connection_index;
					}
				}
			}
			
			//UGLY!
			return -1;
		}
		
		virtual void connect
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index
		)
		{
			int connection_index = find_connection_index
			(
				source_plugin_index, 
				source_port_index, 
				sink_plugin_index, 
				sink_port_index
			);
			
			if (-1 != connection_index)
			{
				std::cout << "Ignored - Ports already connected" << std::endl;
				return;
			}
			
			plugin_ptr sink_plugin = m_plugins[sink_plugin_index];
			
			plugin_ptr source_plugin = m_plugins[source_plugin_index];
			
			m_plugins[sink_plugin_index]->m_connections[sink_port_index].push_back
			(
				m_plugins[source_plugin_index]->m_port_buffers[source_port_index]
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
			int connection_index = find_connection_index
			(
				source_plugin_index, 
				source_port_index, 
				sink_plugin_index, 
				sink_port_index
			);
			
			if (-1 == connection_index)
			{
				std::cout << "Ignored - Ports not connected" << std::endl;
				return;
			}
			
			plugin_ptr sink_plugin = m_plugins[sink_plugin_index];
			
			sink_plugin->m_connections[sink_port_index].erase
			(
				sink_plugin->m_connections[sink_port_index].begin() + connection_index
			);
		}

		virtual bool set_port_value
		(
			unsigned plugin_index,
			unsigned port_index,
			float value
		)
		{
			m_plugins[plugin_index]->m_port_values[port_index] = value;
			
			return true;
		}
		
		buffer_ptr get_buffer
		(
			unsigned plugin_index,
			unsigned port_index
		)
		{
			return m_plugins[plugin_index]->m_port_buffers[port_index];
		}
		
		virtual void process()
		{
			for (unsigned plugin_index = 0; plugin_index < m_plugins.size(); ++plugin_index)
			{
				
			}
		}
		
		
	protected:
		unsigned m_control_period;
				
		unsigned m_sample_rate;
		
		unsigned m_frame_in_chunk;
		
		struct plugin
		{
			
			ladspamm::plugin_instance_ptr m_plugin_instance;
			
			std::vector<buffer_ptr> m_port_buffers;
			
			std::vector<float> m_port_values;
			
			std::vector<std::vector<buffer_ptr> > m_connections;

			plugin
			(
				ladspamm::plugin_instance_ptr plugin_instance,
				unsigned control_period
			) :
				m_plugin_instance(plugin_instance)
			{
				ladspamm::plugin_ptr plugin = m_plugin_instance->the_plugin;
				
				m_port_values.resize(plugin->port_count());

				for (unsigned port_index = 0; port_index < plugin->port_count(); ++port_index)
				{
					buffer_ptr port_buffer(new buffer);
					port_buffer->resize(control_period);

					if (plugin->port_is_input(port_index))
					{
						m_port_values[port_index] = m_plugin_instance->port_default_guessed(port_index);
					}
					else
					{ 
						m_port_values[port_index] = 0;
					}
					
					m_port_buffers.push_back(port_buffer);
					m_connections.push_back(std::vector<buffer_ptr>());
				}
				
				m_plugin_instance->activate();
			}
		};
		
		typedef boost::shared_ptr<plugin> plugin_ptr;
		
		std::vector<plugin_ptr> m_plugins;
		
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
							m_sample_rate
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

