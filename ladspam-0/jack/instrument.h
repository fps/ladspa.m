#ifndef LADSPAM_M_I_INCLUDED_HH
#define LADSPAM_M_I_INCLUDED_HH

#include <jack/midiport.h>

#include <ladspam-0/jack/synth.h>

namespace ladspam
{

struct m;

struct m;
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
				for (unsigned voice_index = 0; voice_index < number_of_voices; ++voice_index)
				{
					m_voices.push_back
					(
						boost::shared_ptr<voice>
						(
							new voice(m_jack_client, voice_index)
						)
					);
				}
				
				m_midi_in_jack_port = jack_port_register
				(
					m_jack_client, 
					"midi", 
					JACK_DEFAULT_MIDI_TYPE, 
					JackPortIsInput, 
					0
				);
			}

			virtual ~instrument()
			{
				
			}
			
			float note_frequency(unsigned int note) 
			{
				return (2.0 * 440.0 / 32.0) * pow(2, (((jack_default_audio_sample_t)note - 9.0) / 12.0));
			}

			unsigned oldest_voice()
			{
				jack_nframes_t minimum_start_frame = m_voices[0]->m_start_frame;
				unsigned oldest_index = 0;
				
				for (unsigned voice_index = 1; voice_index < m_voices.size(); ++voice_index)
				{
					if (m_voices[voice_index]->m_start_frame < minimum_start_frame)
					{
						oldest_index = voice_index;
						minimum_start_frame = m_voices[voice_index]->m_start_frame;
					}
				}
				
				return oldest_index;
			}
			
			int voice_playing_note(unsigned note)
			{
				for (unsigned voice_index = 1; voice_index < m_voices.size(); ++voice_index)
				{
					if (m_voices[voice_index]->m_note == note && m_voices[voice_index]->m_gate > 0)
					{
						return voice_index;
					}
				}
				
				// UGLY
				return -1;
			}
			
			virtual int process(jack_nframes_t nframes)
			{
				for (unsigned voice_index = 0; voice_index < m_voices.size(); ++voice_index)
				{
					float *trigger_out_buffer = 
						(float*)jack_port_get_buffer(m_voices[voice_index]->m_jack_ports[0], nframes);
						
					float *gate_out_buffer = 
						(float*)jack_port_get_buffer(m_voices[voice_index]->m_jack_ports[1], nframes);
						
					float *freq_out_buffer = 
						(float*)jack_port_get_buffer(m_voices[voice_index]->m_jack_ports[2], nframes);
						
					float *velocity_out_buffer = 
						(float*)jack_port_get_buffer(m_voices[voice_index]->m_jack_ports[3], nframes);
						
					std::fill(gate_out_buffer, gate_out_buffer + nframes, 0);
					std::fill(trigger_out_buffer, trigger_out_buffer + nframes, 0);
					std::fill(freq_out_buffer, freq_out_buffer + nframes, 0);
					std::fill(velocity_out_buffer, velocity_out_buffer + nframes, 0);
				}

				void *midi_in_buffer = jack_port_get_buffer(m_midi_in_jack_port, nframes);
				
				jack_midi_event_t midi_in_event;

				jack_nframes_t event_count = jack_midi_get_event_count(midi_in_buffer);
				jack_nframes_t event_index = 0;
				if (event_count > 0) {
					//cout << "ev" << endl;
					jack_midi_event_get(&midi_in_event, midi_in_buffer, event_index);
				}
				
				for (jack_nframes_t frame = 0, max = nframes; frame < max; ++frame) 
				{
					while (event_index < event_count && midi_in_event.time == frame) 
					{
						//cout << "event" << endl;
						
						if( ((*(midi_in_event.buffer) & 0xf0)) == 0x90 ) 	
						{
							/** note on */
							
							unsigned note = *(midi_in_event.buffer + 1);
							float velocity = *(midi_in_event.buffer + 2) / 128.0;
							
							int voice_index = oldest_voice();

							m_voices[voice_index]->m_note = note;
							m_voices[voice_index]->m_gate = 1.0;
							m_voices[voice_index]->m_start_frame = jack_last_frame_time(m_jack_client) + frame;
							m_voices[voice_index]->m_on_velocity = velocity;
							
							float *trigger_out_buffer = (float*)jack_port_get_buffer(m_voices[voice_index]->m_jack_ports[0], nframes);
							
							trigger_out_buffer[frame] = 1.0;
						}
						
						if( ((*(midi_in_event.buffer)) & 0xf0) == 0x80 ) 
						{
							/** note off */

							unsigned note = *(midi_in_event.buffer + 1);
							
							int  voice_index = voice_playing_note(note);
							
							if (-1 != voice_index)
							{
								m_voices[voice_index]->m_gate = 0;
							}
						}
						
						++event_index;
						/**
						* TODO: Check return value for ENODATA
						*/
						jack_midi_event_get(&midi_in_event, midi_in_buffer, event_index);
					}

					for (unsigned voice_index = 0; voice_index < m_voices.size(); ++voice_index)
					{
						float *gate_out_buffer = 
							(float*)jack_port_get_buffer(m_voices[voice_index]->m_jack_ports[1], nframes);
							
						float *freq_out_buffer = 
							(float*)jack_port_get_buffer(m_voices[voice_index]->m_jack_ports[2], nframes);
							
						float *velocity_out_buffer = 
							(float*)jack_port_get_buffer(m_voices[voice_index]->m_jack_ports[3], nframes);
							
						gate_out_buffer[frame] = m_voices[voice_index]->m_gate;
						velocity_out_buffer[frame] = m_voices[voice_index]->m_on_velocity;
						freq_out_buffer[frame] = note_frequency(m_voices[voice_index]->m_note);
					}
				}
		
				ladspam::jack::synth::process(nframes);
				
				return 0;
			}
			
			protected:
				jack_port_t *m_midi_in_jack_port;
				
				struct voice
				{
					float m_gate;
					unsigned m_note;
					unsigned m_on_velocity;
					unsigned m_off_velocity;
					jack_nframes_t m_start_frame;
					std::vector<jack_port_t *> m_jack_ports;
					jack_client_t *m_jack_client;
					
					voice(jack_client_t *jack_client, unsigned voice_index) :
						m_gate(0.0),
						m_jack_client(jack_client)
					{
						std::stringstream trigger_stream;
						trigger_stream << "voice-" << voice_index << "-trigger";
						
						jack_port_t *trigger_port = jack_port_register
						(
							m_jack_client, 
							trigger_stream.str().c_str(),
							JACK_DEFAULT_AUDIO_TYPE,
							JackPortIsOutput,
							0
						);
						
						m_jack_ports.push_back(trigger_port);

						std::stringstream gate_stream;
						gate_stream << "voice-" << voice_index << "-gate";
						
						jack_port_t *gate_port = jack_port_register
						(
							m_jack_client, 
							gate_stream.str().c_str(),
							JACK_DEFAULT_AUDIO_TYPE,
							JackPortIsOutput,
							0
						);
				
						m_jack_ports.push_back(gate_port);

						std::stringstream frequency_stream;
						frequency_stream << "voice-" << voice_index << "-frequency";
						
						jack_port_t *frequency_port = jack_port_register
						(
							m_jack_client, 
							frequency_stream.str().c_str(),
							JACK_DEFAULT_AUDIO_TYPE,
							JackPortIsOutput,
							0
						);
						
						m_jack_ports.push_back(frequency_port);

						std::stringstream velocity_stream;
						velocity_stream << "voice-" << voice_index << "-velocity";
						
						jack_port_t *velocity_port = jack_port_register
						(
							m_jack_client, 
							velocity_stream.str().c_str(),
							JACK_DEFAULT_AUDIO_TYPE,
							JackPortIsOutput,
							0
						);
						
						m_jack_ports.push_back(velocity_port);

					}
					
					~voice()
					{
						for (unsigned port_index = 0; port_index < m_jack_ports.size(); ++port_index)
						{
							jack_port_unregister(m_jack_client, m_jack_ports[port_index]);
						}
					}
				};

				std::vector<boost::shared_ptr<voice> > m_voices;
		};
	}
}

#endif
