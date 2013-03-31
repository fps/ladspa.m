#ifndef LADSPAM_M_I_INCLUDED_HH
#define LADSPAM_M_I_INCLUDED_HH

#include <ladspam-0/jack/synth.h>

namespace ladspam
{
	namespace jack 
	{
		struct instrument : ladspam::jack::synth
		{
			instrument
			(
				const std::string &client_name,
				unsigned control_period,
				unsigned number_of_voices
			) :
				ladspam::jack::synth(client_name, control_period)
			{
				
			}

			virtual ~instrument()
			{
				
			}
			
			virtual int process(jack_nframes_t nframes)
			{
				ladspam::jack::synth::process(nframes);
				
				return 0;
			}
			
			protected:
				jack_port_t *m_midi_in_port;
				
				std::vector<jack_port_t *> m_midi_cc_ports;
				
				std::vector<std::vector<jack_port_t *> > m_voice_ports;
		};
	}
}

#endif
