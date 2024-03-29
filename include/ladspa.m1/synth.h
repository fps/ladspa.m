#ifndef LADSPAM_M_INCLUDED_HH
#define LADSPAM_M_INCLUDED_HH

#include <string>
#include <vector>
#include <stdexcept>

#include <ladspamm1/world.h>
#include <ladspamm1/library.h>
#include <ladspamm1/plugin_instance.h>

#include <boost/optional.hpp>

/**
	\mainpage ladspa.m-1
	
	\include README.md
*/

/**
	@brief All classes are in this namespace
*/
namespace ladspam1
{
	/**
	 * @brief A very low level class to create a graph of LADSPA plugins.
	 * 
	 * NOTE: Some of the functions might throw std::runtime_error objects.
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
		 * @brief The ladspamm world that helps us find plugins
		 */
		ladspamm1::world m_ladspam_world;
		
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
		 * @brief Find a library that contains the given plugin label. 
		 */
		std::string find_plugin_library(const std::string &label)
		{
			for (unsigned library_index = 0; library_index < m_ladspam_world.libraries.size(); ++library_index)
			{
				for (unsigned plugin_index = 0; plugin_index < m_ladspam_world.libraries[library_index]->plugins.size(); ++plugin_index)
				{
					if (m_ladspam_world.libraries[library_index]->plugins[plugin_index]->label() == label)
					{
						return m_ladspam_world.libraries[library_index]->the_dl->filename;
					}
				}
			}
			
			throw std::runtime_error("find_plugin_library(): No library containing the plugin with label " + label + " found");
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
			return (unsigned)m_plugins.size();
		}

		/**
		 * @brief Removes the plugin from the plugins list.
		 * 
		 * Connections to other plugins are automatically cleared.
		*/
		inline void remove_plugin(unsigned index)
		{
			if (false == (index < number_of_plugins()))
			{
				throw std::runtime_error("remove_plugin(): index out of bounds");
			}
			
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
			if (false == (index <= number_of_plugins()))
			{
				throw std::runtime_error("insert_plugin(): index out of bounds");
			}
			
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
		inline ladspamm1::plugin_instance_ptr get_plugin(unsigned index)
		{
			if (false == (index < number_of_plugins()))
			{
				throw std::runtime_error("get_plugin(): index out of bounds");
			}
			
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
			if (false == (sink_plugin_index < number_of_plugins()))
			{
				throw std::runtime_error("connect(): sink_plugin_index out of bounds");
			}
			
			if (false == (sink_port_index < m_plugins[sink_plugin_index]->m_number_of_ports))
			{
				throw std::runtime_error("connect(): sink_port_index out of bounds");
			}
			
			m_plugins[sink_plugin_index]->m_connections[sink_port_index].push_back(buffer);
		}

		/**
		 * @brief Connect a source port to a sink port. 
		 * 
		 * Note the port indices are absolute indices. I.e. you refer to the index'th port of the plugin. To get an absolute index of a sink or source port take a look at the source_port_index() and sink_port_index() functions.
		 */
		inline void connect
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index
		)
		{
			if (false == (sink_plugin_index < number_of_plugins()))
			{
				throw std::runtime_error("connect(): sink_plugin_index out of bounds");
			}
			
			if (false == (source_plugin_index < number_of_plugins()))
			{
				throw std::runtime_error("connect(): source_plugin_index out of bounds");
			}
			
			if (false == (sink_port_index < m_plugins[sink_plugin_index]->m_number_of_ports))
			{
				throw std::runtime_error("connect(): sink_port_index out of bounds");
			}
			
			if (false == (source_port_index < m_plugins[source_plugin_index]->m_number_of_ports))
			{
				throw std::runtime_error("connect(): source_port_index out of bounds");
			}

			int connection_index = find_connection_index
			(
				source_plugin_index, 
				source_port_index, 
				sink_plugin_index, 
				sink_port_index
			);
			
			if (-1 != connection_index)
			{
				// std::cout << "Ignored - Ports already connected" << std::endl;
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
			if (false == (plugin_index < number_of_plugins()))
			{
				throw std::runtime_error("sink_port_index(): plugin_index out of bounds");
			}
			
			if (false == (sink_port_index < m_plugins[plugin_index]->m_number_of_input_ports))
			{
				throw std::runtime_error("sink_port_index(): sink_port_index out of bounds");
			}
			
			return m_plugins[plugin_index]->m_input_port_indices[sink_port_index];
		}
		
		/**
		 * @brief A utility function to lookup the (absolute) port index of the index'th output port.
		 */
		unsigned source_port_index(unsigned plugin_index, unsigned source_port_index)
		{
			if (false == (plugin_index < number_of_plugins()))
			{
				throw std::runtime_error("source_port_index: plugin_index out of bounds");
			}
			
			if (false == (source_port_index < m_plugins[plugin_index]->m_number_of_output_ports))
			{
				throw std::runtime_error("source_port_index(): source_port_index out of bounds");
			}
			
			return m_plugins[plugin_index]->m_output_port_indices[source_port_index];
		}

		/**
		 * @brief Disconnect a pair of ports
		 * 
		 * Note the port indices are absolute indices. I.e. you refer to the index'th port of the plugin. To get an absolute index of a sink or source port take a look at the source_port_index() and sink_port_index() functions.
		 */
		inline void disconnect
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index
		)
		{
			if (false == (sink_plugin_index < number_of_plugins()))
			{
				throw std::runtime_error("get_plugin(): sink_plugin_index out of bounds");
			}
			
			if (false == (source_plugin_index < number_of_plugins()))
			{
				throw std::runtime_error("get_plugin(): source_plugin_index out of bounds");
			}
			
			if (false == (sink_port_index < m_plugins[sink_plugin_index]->m_number_of_ports))
			{
				throw std::runtime_error("get_plugin(): sink_port_index out of bounds");
			}
			
			if (false == (source_port_index < m_plugins[sink_plugin_index]->m_number_of_ports))
			{
				throw std::runtime_error("get_plugin(): source_port_index out of bounds");
			}

			int connection_index = find_connection_index
			(
				source_plugin_index, 
				source_port_index, 
				sink_plugin_index, 
				sink_port_index
			);
			
			if (-1 == connection_index)
			{
				// std::cout << "Ignored - Ports not connected" << std::endl;
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
			if (false == (plugin_index < number_of_plugins()))
			{
				throw std::runtime_error("get_plugin(): plugin_index out of bounds");
			}
			
			if (false == (port_index < m_plugins[plugin_index]->m_number_of_ports))
			{
				throw std::runtime_error("get_plugin(): port_index out of bounds");
			}

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
			if (false == (plugin_index < number_of_plugins()))
			{
				throw std::runtime_error("get_plugin(): plugin_index out of bounds");
			}
			
			if (false == (port_index < m_plugins[plugin_index]->m_number_of_ports))
			{
				throw std::runtime_error("get_plugin(): port_index out of bounds");
			}

			return m_plugins[plugin_index]->m_port_buffers[port_index];
		}
		
		/**
		 * @brief Execute the plugins in the plugins list in the given order.
		 */
		inline void process(const unsigned number_of_frames)
		{
			if (false == (number_of_frames <= m_buffer_size))
			{
				throw std::runtime_error("process(): number_of_frames > m_buffer_size");
			}
			
			/*
			 * Process all plugins in their order
			 */
			for 
			(
				size_t plugin_index = 0, plugin_index_max = m_plugins.size(); 
				plugin_index < plugin_index_max; 
				++plugin_index
			)
			{
				plugin &the_plugin = *(m_plugins[plugin_index]);
				
				ladspamm1::plugin_instance &plugin_instance = *the_plugin.m_plugin_instance;
				
				ladspamm1::plugin &the_ladspamm_plugin = *plugin_instance.the_plugin;

				/*
				 * Handle port input values (connections or constants)
				 */
				for 
				(
					size_t port_index = 0, port_index_max = the_plugin.m_port_buffers.size(); 
					port_index < port_index_max; 
					++port_index
				)
				{
					buffer &the_buffer = *the_plugin.m_port_buffers[port_index];
					
					const std::vector<buffer_ptr> &the_port_connections = the_plugin.m_connections[port_index];
					
					const size_t number_of_connections = the_plugin.m_connections[port_index].size();
					
					/*
					 * We only care about input ports.
					 */
					if (the_ladspamm_plugin.port_is_input((unsigned)port_index))
					{
						/*
						 * If there are no connections, fill with the constant port value
						 */
						if (number_of_connections == 0)
						{
							if (true == the_ladspamm_plugin.port_is_control((unsigned)port_index))
							{
								// std::cout << "control" << std::endl;
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
						/*
						 * In case of connections, copy and add the inputs of the connected
						 * ports
						 */
						else
						{
							for 
							(
								unsigned connection_index = 0; 
								connection_index < number_of_connections; 
								++connection_index
							)
							{
								const buffer &the_connected_buffer 
									= *the_port_connections[connection_index];
								
								/*
								 * Treat the first connection different, since there's
								 * no need to add.
								 */
								if (0 == connection_index)
								{
									std::copy
									(
										the_connected_buffer.begin(),
										the_connected_buffer.begin() + number_of_frames,
										the_buffer.begin()
									);
								}
								/*
								 * Add buffer values of connected port to
								 * input buffer
								 */
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
				
				plugin_instance.run(number_of_frames);
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
			if (false == (sink_plugin_index < number_of_plugins()))
			{
				throw std::runtime_error("get_plugin(): sink_plugin_index out of bounds");
			}
			
			if (false == (source_plugin_index < number_of_plugins()))
			{
				throw std::runtime_error("get_plugin(): source_plugin_index out of bounds");
			}
			
			if (false == (sink_port_index < m_plugins[sink_plugin_index]->m_number_of_ports))
			{
				throw std::runtime_error("get_plugin(): sink_port_index out of bounds");
			}
			
			if (false == (source_port_index < m_plugins[source_plugin_index]->m_number_of_ports))
			{
				throw std::runtime_error("get_plugin(): source_port_index out of bounds");
			}

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
		inline ladspamm1::plugin_instance_ptr load_ladspa_plugin(std::string library, std::string label)
		{
			ladspamm1::library_ptr lib(new ladspamm1::library(library));
			
			for (unsigned index = 0; index < lib->plugins.size(); ++index)
			{
				if (lib->plugins[index]->label() == label)
				{
					ladspamm1::plugin_instance_ptr instance
					(
						new ladspamm1::plugin_instance
						(
							lib->plugins[index], 
							m_sample_rate
						)
					);
					
					instance->activate();
					return instance;
				}
			}
			
			throw std::runtime_error("Plugin with label " + label + " not found in the library " + library);
		}
		
		/**
		 * @brief A type to represent a loaded plugin. 
		 */
		struct plugin
		{
			
			ladspamm1::plugin_instance_ptr m_plugin_instance;
			
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
				ladspamm1::plugin_instance_ptr plugin_instance,
				unsigned buffer_size
			) :
				m_plugin_instance(plugin_instance),
				m_number_of_input_ports(0),
				m_number_of_output_ports(0),
				m_number_of_ports(0)
			{
				ladspamm1::plugin_ptr plugin = m_plugin_instance->the_plugin;
				
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

