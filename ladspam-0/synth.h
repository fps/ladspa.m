#ifndef LADSPAM_M_INCLUDED_HH
#define LADSPAM_M_INCLUDED_HH

#include <string>
#include <vector>

#include <ladspamm-0/library.h>
#include <ladspamm-0/plugin_instance.h>

#include <boost/optional.hpp>

#include <cassert>

/**
	\mainpage ladspa.m
	
	\include README.md
*/

/**
	@brief All classes are in this namespace
*/
namespace ladspam
{
	/**
	 * @brief A very low level class to create a graph of LADSPA plugins.
	 * 
	 * NOTE: Most of the functions have asserts to find errors in client programs (mostly checking the bounds of indices). Use the NDEBUG preprocessor macro to compile without the assertions, once you have tested your client code thoroughly.  
	 * 
	 * NOTE: This class is threadsafe in the sense that two different threads can each own a DIFFERENT synth instance. It is NOT safe to call functions on a single synth instance from different threads.
	 * 
	 * A synthesis graph is defined by a list of loaded plugins (see insert_plugin() and append_plugin()). The connections are made with the connect() functions.
	 * 
	 * The order oif the added plugins matters. The plugins' run() functions are called in precisely the order in which they are in the plugins list.
	 * 
	 * You are free to connect control to audio ports and vice versa. Note though that control port values are updated only at the start of the process() call.
	 * 
	 * Take a look at the example code in test_synth.cc.
	 */
	struct synth 
	{
		/**
		 * @brief A buffer is just an array of floats.
		 */
		typedef std::vector<float> buffer;

		/**
		 * @brief Utility typedef to ease typing
		 */
		typedef boost::shared_ptr<buffer> buffer_ptr;

		/**
		 * @brief Create a synth with a given sample rate and buffer size.
		 * 
		 * The buffer_size determines the maximum number of frames process() can be called with lateron.
		 * 
		 * The sample_rate is used to instantiate plugins with that sample rate.
		 */
		synth(unsigned sample_rate, unsigned buffer_size) :
			m_buffer_size(buffer_size),
			m_sample_rate(sample_rate)
		{
			
		}
		
		/**
		 * @brief The sample rate given at construction time.
		 */
		inline unsigned sample_rate()
		{
			return m_sample_rate;
		}
		
		/**
		 * @brief The buffer size given at construction time.
		 */
		inline unsigned buffer_size()
		{
			return m_buffer_size;
		}
		
		/**
		 * @brief The number of plugins in the plugins list.
		 */
		inline unsigned number_of_plugins() const
		{
			return m_plugins.size();
		}

		/**
		 * @brief Removes the plugin from the plugins list.
		 * 
		 * Connections to other plugins are automatically cleared.
		*/
		inline void remove_plugin(unsigned index)
		{
			assert(index < number_of_plugins());
			
			m_plugins.erase(m_plugins.begin() + index);
		}

		/**
		 * @brief Inserts a plugin into the plugins list at the specified position.
		 * 
		 * Existing connections are not affected by possible reorderings of plugins in the plugin list.
		 */
		inline void insert_plugin
		(
			unsigned index, 
			const std::string &library, 
			const std::string& label
		)
		{
			assert(index <= number_of_plugins());
			
			plugin_ptr the_plugin(new plugin(load_ladspa_plugin(library, label), m_buffer_size));
			m_plugins.insert(m_plugins.begin() + index, the_plugin);
		}
		
		/**
		 * @brief Appends a plugin to the end of the plugins list.
		 */
		inline void append_plugin
		(
			const std::string &library, 
			const std::string& label
		)
		{
			insert_plugin(number_of_plugins(), library, label);	
		}
		
		/**
		 * @brief Returns the plugin instance of the loaded plugin at the position index in the plugins list.
		 */
		inline ladspamm::plugin_instance_ptr get_plugin(unsigned index)
		{
			assert(index < number_of_plugins());
			
			return m_plugins[index]->m_plugin_instance;
		}

		/**
		 * @brief Connect a plugin's sink port to a (external) buffer.
		 * 
		 * This function can be used to pass signals into a synthesis graph from the outside.
		 * 
		 * The sink_plugin_index has the prefix "sink" to denote that you should use an index of an input port. To get the n'th input port of a plugin, use the sink_port_index() function.
		 */
		inline void connect
		(
			unsigned sink_plugin_index,
			unsigned sink_port_index,
			buffer_ptr buffer
		)
		{
			assert(sink_plugin_index < number_of_plugins());
			assert(sink_port_index < m_plugins[sink_plugin_index]->m_number_of_ports);
			
			m_plugins[sink_plugin_index]->m_connections[sink_port_index].push_back(buffer);
		}

		/**
		 * @brief Connect a source port to a sink port. 
		 */
		inline void connect
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index
		)
		{
			assert(sink_plugin_index < number_of_plugins());
			assert(source_plugin_index < number_of_plugins());
			assert(sink_port_index < m_plugins[sink_plugin_index]->m_number_of_ports);
			assert(source_port_index < m_plugins[source_plugin_index]->m_number_of_ports);

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

		/**
		 * @brief A utility function to lookup the (absolute) port index of the index'th input port.
		 */
		unsigned sink_port_index(unsigned plugin_index, unsigned sink_port_index)
		{
			assert(plugin_index < number_of_plugins());
			assert(sink_port_index < m_plugins[plugin_index]->m_number_of_input_ports);
			
			return m_plugins[plugin_index]->m_input_port_indices[sink_port_index];
		}
		
		/**
		 * @brief A utility function to lookup the (absolute) port index of the index'th output port.
		 */
		unsigned source_port_index(unsigned plugin_index, unsigned source_port_index)
		{
			assert(plugin_index < number_of_plugins());
			assert(source_port_index < m_plugins[plugin_index]->m_number_of_output_ports);
			
			return m_plugins[plugin_index]->m_output_port_indices[source_port_index];
		}

		inline void disconnect
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index
		)
		{
			assert(sink_plugin_index < number_of_plugins());
			assert(source_plugin_index < number_of_plugins());
			assert(sink_port_index < m_plugins[sink_plugin_index]->m_number_of_ports);
			assert(source_port_index < m_plugins[sink_plugin_index]->m_number_of_ports);

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

		/**
		 * @brief Set a port's value that is used when a port is not connected.
		 */
		inline void set_port_value
		(
			unsigned plugin_index,
			unsigned port_index,
			float value
		)
		{
			assert(plugin_index < number_of_plugins());
			assert(port_index < m_plugins[plugin_index]->m_number_of_ports);

			m_plugins[plugin_index]->m_port_values[port_index] = value;
		}
		
		/**
		 * @brief Returns the buffer of a plugin.
		 * 
		 * This function can be used to get hold of a buffer of a plugin. This way
		 * it is possible to get audio out of the synthesis graph.
		 */
		inline buffer_ptr get_buffer
		(
			unsigned plugin_index,
			unsigned port_index
		)
		{
			assert(plugin_index < number_of_plugins());
			assert(port_index < m_plugins[plugin_index]->m_number_of_ports);

			return m_plugins[plugin_index]->m_port_buffers[port_index];
		}
		
		/**
		 * @brief Execute the plugins in the plugins list in the given order.
		 */
		inline void process(unsigned number_of_frames)
		{
			assert(number_of_frames <= m_buffer_size);
			
			for 
			(
				unsigned plugin_index = 0, plugin_index_max = m_plugins.size(); 
				plugin_index < plugin_index_max; 
				++plugin_index
			)
			{
				plugin &the_plugin = *(m_plugins[plugin_index]);
				ladspamm::plugin_instance &plugin_instance = *the_plugin.m_plugin_instance;
				ladspamm::plugin &the_ladspamm_plugin = *plugin_instance.the_plugin;
				
				for 
				(
					unsigned port_index = 0, port_index_max = the_plugin.m_port_buffers.size(); 
					port_index < port_index_max; 
					++port_index
				)
				{
					buffer &the_buffer = *the_plugin.m_port_buffers[port_index];
					
					std::vector<buffer_ptr> &the_port_connections = the_plugin.m_connections[port_index];
					
					unsigned number_of_connections = the_plugin.m_connections[port_index].size();
					if (the_ladspamm_plugin.port_is_input(port_index))
					{
						if (number_of_connections == 0)
						{
							if (true == the_ladspamm_plugin.port_is_control(port_index))
							{
								the_buffer[0] = the_plugin.m_port_values[port_index];
							}
							else
							{
								std::fill
								(
									the_buffer.begin(),
									the_buffer.begin() + number_of_frames,
									the_plugin.m_port_values[port_index]
								);
							}
						}
						else
						{
							for (unsigned connection_index = 0; connection_index < number_of_connections; ++connection_index)
							{
								const buffer &the_connected_buffer 
									= *the_port_connections[connection_index];
										
								if (0 == connection_index)
								{
									std::copy
									(
										the_connected_buffer.begin(),
										the_connected_buffer.begin() + number_of_frames,
										the_buffer.begin()
									);
								}
								else
								{
									std::transform
									(
										the_connected_buffer.begin(),
										the_connected_buffer.begin() + number_of_frames,
										the_buffer.begin(),
										the_buffer.begin(),
										std::plus<float>()
									);
								}
							}
						}
					}
					else
					{
						// Do nothing for outputs
					}
				}
				
				plugin_instance.run(m_buffer_size);
			}
		}
		
		
	protected:
		unsigned m_buffer_size;
				
		unsigned m_sample_rate;
	
		/**
		 * @brief A utility function for internal use.
		 */
		inline int find_connection_index
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index
		)
		{
			assert(sink_plugin_index < number_of_plugins());
			assert(source_plugin_index < number_of_plugins());
			assert(sink_port_index < m_plugins[sink_plugin_index]->m_number_of_ports);
			assert(source_port_index < m_plugins[source_plugin_index]->m_number_of_ports);

			plugin_ptr sink_plugin = m_plugins[sink_plugin_index];
			
			plugin_ptr source_plugin = m_plugins[source_plugin_index];
			
			for (unsigned port_index = 0; port_index < sink_plugin->m_port_buffers.size(); ++port_index)
			{
				std::vector<buffer_ptr> connections = sink_plugin->m_connections[sink_port_index];
				for (unsigned connection_index = 0; connection_index < connections.size(); ++connection_index)
				{
					if (connections[connection_index] == source_plugin->m_port_buffers[source_port_index])
					{
						return connection_index;
					}
				}
			}
			
			//UGLY!
			return -1;
		}

		struct plugin;
		
		/**
		 * @brief A utility typedef to ease typing.
		 */
		typedef boost::shared_ptr<plugin> plugin_ptr;
		
		/**
		 * @brief The list of loaded plugins
		 */
		std::vector<plugin_ptr> m_plugins;
		
		/**
		 * @brief Loads a LADSPA plugin with the given label from the given library.
		 */
		inline ladspamm::plugin_instance_ptr load_ladspa_plugin(std::string library, std::string label)
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
			
			assert(false);
		}
		
		/**
		 * @brief A type to represent a loaded plugin. 
		 */
		struct plugin
		{
			
			ladspamm::plugin_instance_ptr m_plugin_instance;
			
			std::vector<buffer_ptr> m_port_buffers;
			
			std::vector<float> m_port_values;
			
			std::vector<std::vector<buffer_ptr> > m_connections;

			unsigned m_number_of_input_ports;
			unsigned m_number_of_output_ports;
			unsigned m_number_of_ports;
			
			std::vector<unsigned> m_input_port_indices;
			std::vector<unsigned> m_output_port_indices;
			
			plugin
			(
				ladspamm::plugin_instance_ptr plugin_instance,
				unsigned buffer_size
			) :
				m_plugin_instance(plugin_instance),
				m_number_of_input_ports(0),
				m_number_of_output_ports(0),
				m_number_of_ports(0)
			{
				ladspamm::plugin_ptr plugin = m_plugin_instance->the_plugin;
				
				m_port_values.resize(plugin->port_count());

				for (unsigned port_index = 0; port_index < plugin->port_count(); ++port_index)
				{
					buffer_ptr port_buffer(new buffer);
					port_buffer->resize(buffer_size);

					if (plugin->port_is_input(port_index))
					{
						m_port_values[port_index] = m_plugin_instance->port_default_guessed(port_index);
						m_input_port_indices.push_back(port_index);
						++m_number_of_input_ports;
					}
					else
					{ 
						m_port_values[port_index] = 0;
						m_output_port_indices.push_back(port_index);
						++m_number_of_output_ports;
					}
					
					m_port_buffers.push_back(port_buffer);
					m_connections.push_back(std::vector<buffer_ptr>());
					
					m_plugin_instance->connect_port(port_index, &((*m_port_buffers[port_index])[0]));
					
					++m_number_of_ports;
				}
				
				m_plugin_instance->activate();
			}
			
			~plugin()
			{
				m_plugin_instance->deactivate();
			}
		};
	};
	
	/**
	 * @brief A utility typedef to ease the typing load.
	 */
	typedef boost::shared_ptr<synth> synth_ptr;
}

#endif

