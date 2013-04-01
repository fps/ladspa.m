#ifndef LADSPAM_M_INCLUDED_HH
#define LADSPAM_M_INCLUDED_HH

#include <string>
#include <vector>

#include <ladspamm-0/library.h>
#include <ladspamm-0/plugin_instance.h>

#include <boost/optional.hpp>

namespace ladspam
{

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
		synth(unsigned sample_rate, unsigned control_period) :
			m_control_period(control_period),
			m_plugin_counter(0),
			m_sample_rate(sample_rate)
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
			plugin_ptr the_plugin(new plugin(load_ladspa_plugin(library, label), m_plugin_counter++, m_control_period));
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
			m_plugins[plugin_index]->m_port_values[port_index] = value;
			
			return true;
		}
		
		//! number_of_frames must be smaller than control_period
		virtual int process(unsigned number_of_frames)
		{
			return 0;
		}
		
	protected:
		unsigned m_control_period;
		
		unsigned m_plugin_counter;
		
		unsigned m_sample_rate;
		
		struct plugin
		{
			typedef std::vector<float> buffer;

			typedef boost::shared_ptr<buffer> buffer_ptr;
			
			typedef boost::optional<buffer_ptr> connection;
			
			ladspamm::plugin_instance_ptr m_plugin_instance;
			
			std::vector<buffer_ptr> m_port_buffers;
			
			std::vector<float> m_port_values;
			
			std::vector<connection> m_connections;

			plugin
			(
				ladspamm::plugin_instance_ptr plugin_instance,
				unsigned plugin_counter,
				unsigned control_period
			) :
				m_plugin_instance(plugin_instance)
			{
				ladspamm::plugin_ptr plugin = m_plugin_instance->the_plugin;
				
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
					m_connections.push_back(connection());
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

