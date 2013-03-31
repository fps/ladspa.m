#ifndef LADSPAM_M_I_INCLUDED_HH
#define LADSPAM_M_I_INCLUDED_HH

#include <ladspam-0/m_jack.h>

namespace ladspam{
	/*
		i for instrument...
	*/
	struct i_jack : m_jack
	{
		i_jack
		(
			const std::string &client_name,
			unsigned control_period,
			unsigned number_of_voices
		) :
			m_jack(client_name, control_period),
			m_voice_ports(number_of_voices)
		{
			
		}
		
		virtual ~i_jack()
		{
			
		}
		
		virtual unsigned number_of_plugins() const
		{
			return m_plugins.size() / m_voice_ports.size();
		}

		virtual void remove_plugin(unsigned index)
		{
			for (unsigned voice_index = 0; voice_index < m_voice_ports.size(); ++voice_index)
			{
				m_jack::remove_plugin(m_voice_ports.size() * index);
			}
		}

		virtual void insert_plugin
		(
			unsigned index, 
			const std::string &library, 
			const std::string& label
		)
		{
			for (unsigned voice_index = 0; voice_index < m_voice_ports.size(); ++voice_index)
			{
				m_jack::insert_plugin(m_voice_ports.size() * index + voice_index, library, label);
			}
		}
		
		virtual void connect
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index
		)
		{
			for (unsigned voice_index = 0; voice_index < m_voice_ports.size(); ++voice_index)
			{
				m_jack::connect
				(
					m_voice_ports.size() * source_plugin_index + voice_index,
					source_port_index,
					m_voice_ports.size() * sink_plugin_index + voice_index,
					sink_port_index
				);
			}
		}

		virtual void disconnect
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index
		)
		{
			for (unsigned voice_index = 0; voice_index < m_voice_ports.size(); ++voice_index)
			{
				m_jack::disconnect
				(
					m_voice_ports.size() * source_plugin_index + voice_index,
					source_port_index,
					m_voice_ports.size() * sink_plugin_index + voice_index,
					sink_port_index
				);
			}
		}

		virtual bool set_port_value
		(
			unsigned plugin_index,
			unsigned port_index,
			float value
		)
		{
			return true;
		}

		protected:
			jack_port_t *m_midi_in_port;
			
			std::vector<jack_port_t *> m_midi_cc_ports;
			
			std::vector<std::vector<jack_port_t *> > m_voice_ports;
	};
}

#endif
