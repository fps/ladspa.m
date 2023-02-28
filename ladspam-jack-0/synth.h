#ifndef LADSPAM_JACK_SYNTH_HH
#define LADSPAM_JACK_SYNTH_HH

#include <sstream>

#include <ladspa.m-1/synth.h>
#include <ladspam1.pb.h>

#include <jack/jack.h>

namespace ladspam_jack
{
	struct synth
	{
		/**
		 * control_period -1 means take hosts buffer size
		 */
		synth
		(
			const std::string &jack_client_name, 
			const ladspam_proto1::Synth &synth_pb,
			int control_period = -1,
			bool activate_instance = true
		);
		
		~synth()
		{
			if (true == m_activate_instance)
			{
				jack_deactivate(m_jack_client);
				jack_client_close(m_jack_client);
			}
		}
		
		void activate();
		
		virtual void process_chunk(jack_nframes_t nframes, unsigned chunk_index)
		{
			for (unsigned exposed_port_index = 0; exposed_port_index < m_jack_ports.size(); ++exposed_port_index)
			{
				if (jack_port_flags(m_jack_ports[exposed_port_index]) & JackPortIsInput)
				{	
					float *buffer = (float*)jack_port_get_buffer(m_jack_ports[exposed_port_index], nframes);
				
					std::copy
					(
						buffer + chunk_index * m_control_period,
						buffer + (chunk_index + 1) * m_control_period,
						m_exposed_plugin_port_buffers[exposed_port_index]->begin()
					);
				}
			}

			m_synth->process(m_control_period);
			
			for (unsigned exposed_port_index = 0; exposed_port_index < m_jack_ports.size(); ++exposed_port_index)
			{
				if (jack_port_flags(m_jack_ports[exposed_port_index]) & JackPortIsOutput)
				{
					float *buffer = (float*)jack_port_get_buffer(m_jack_ports[exposed_port_index], nframes);
				
					std::copy
					(
						m_exposed_plugin_port_buffers[exposed_port_index]->begin(),
						m_exposed_plugin_port_buffers[exposed_port_index]->end(),
						buffer + chunk_index * m_control_period
					);
				}
			}
		}
		
		virtual int process(jack_nframes_t nframes)
		{
			unsigned number_of_chunks = nframes/m_control_period;
			
			for (unsigned chunk_index = 0; chunk_index < number_of_chunks; ++chunk_index)
			{
				process_chunk(nframes, chunk_index);
			}
			
			return 0;
		}
		protected:
			void expose_ports(const ladspam_proto1::Synth &synth_pb, ladspam1::synth_ptr the_synth)
			{
				for (int port_index = 0; port_index < synth_pb.exposed_ports_size(); ++port_index)
				{
					ladspam_proto1::ExposedPort exposed_port = synth_pb.exposed_ports(port_index);
					ladspam_proto1::Port port = exposed_port.port();
					
					ladspamm1::plugin_ptr the_plugin = the_synth->get_plugin(port.plugin_index())->the_plugin;
					
					unsigned long flags = 0;
					if (the_plugin->port_is_input(port.port_index()))
					{
						flags |= JackPortIsInput;
						
						ladspam1::synth::buffer_ptr buffer(new std::vector<float>);
						
						buffer->resize(m_control_period);
						
						m_exposed_plugin_port_buffers.push_back(buffer);
						
						the_synth->connect(port.plugin_index(), port.port_index(), buffer);
					}
					else
					{
						flags |= JackPortIsOutput;
						
						m_exposed_plugin_port_buffers.push_back(the_synth->get_buffer(port.plugin_index(), port.port_index()));
					}
					
					std::cout << "registering port: " << exposed_port.name() << std::endl;
#if 0
						<< "-" << the_plugin->label()
						<< "-" << the_plugin->port_name(port.port_index());
#endif					
					jack_port_t *jack_port = jack_port_register(m_jack_client, exposed_port.name().c_str(), JACK_DEFAULT_AUDIO_TYPE, flags, 0);

					if (jack_port == 0)
					{
						throw std::runtime_error("failed to create audio port");
					}
					
					m_jack_ports.push_back(jack_port);
				}
			}
			
			ladspam1::synth_ptr build_synth(const ladspam_proto1::Synth& synth_pb, unsigned sample_rate, unsigned control_period)
			{
				ladspam1::synth_ptr the_synth(new ladspam1::synth(sample_rate, control_period));
				
				for (int plugin_index = 0; plugin_index < synth_pb.plugins_size(); ++plugin_index)
				{
					ladspam_proto1::Plugin plugin_pb = synth_pb.plugins(plugin_index);
					
					the_synth->append_plugin
					(
						the_synth->find_plugin_library(plugin_pb.label()), 
						plugin_pb.label()
					);
					
					for (int value_index = 0; value_index < plugin_pb.values_size(); ++value_index)
					{
						ladspam_proto1::Value value = plugin_pb.values(value_index);
						
						the_synth->set_port_value(plugin_index, value.port_index(), value.value());
					}
				}
				
				for (int connection_index = 0; connection_index < synth_pb.connections_size(); ++connection_index)
				{
					ladspam_proto1::Connection connection_pb = synth_pb.connections(connection_index);
					
					the_synth->connect
					(
						connection_pb.source_index(),
						connection_pb.source_port_index(),
						connection_pb.sink_index(),
						connection_pb.sink_port_index()
					);
				}
				
				expose_ports(synth_pb, the_synth);
				
				return the_synth;
			}
		
			bool m_activate_instance;
		
			ladspam1::synth_ptr m_synth;
			
			unsigned m_control_period;
			
			jack_client_t *m_jack_client;
			
			std::vector<jack_port_t *> m_jack_ports;
			std::vector<ladspam1::synth::buffer_ptr> m_exposed_plugin_port_buffers; 
	};
}

#endif
