#include <ladspam-0/jack/synth.h>

namespace ladspam
{
	namespace jack
	{
		extern "C"
		{
			int jack_process(jack_nframes_t nframes, void* arg)
			{
				return ((synth*)arg)->process(nframes);
			}
		}
		
		synth::synth(const std::string &client_name, unsigned control_period) :
			m_jack_client(jack_client_open(client_name.c_str(), JackUseExactName, 0)),
			m_control_period(control_period),
			m_plugin_counter(0),
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
		
			if (0 != jack_get_buffer_size(m_jack_client) % m_control_period)
			{
				throw std::runtime_error("control period must be a divider of buffer size");
			}
			jack_activate(m_jack_client);
		}
	}
}