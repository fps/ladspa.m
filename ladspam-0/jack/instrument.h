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

			unsigned oldest_voice(unsigned frame)
			{
				jack_nframes_t minimum_age = frame + jack_last_frame_time(m_jack_client) - m_voices[0]->m_start_frame;
				unsigned oldest_index = 0;
				
				for (unsigned voice_index = 1; voice_index < m_voices.size(); ++voice_index)
				{
					jack_nframes_t age = frame + jack_last_frame_time(m_jack_client) - m_voices[voice_index]->m_start_frame;
					if (age > minimum_age)
					{
						oldest_index = voice_index;
						minimum_age = age;
					}
				}
				
				return oldest_index;
			}
			
			int voice_playing_note(unsigned note)
			{
				for (unsigned voice_index = 0; voice_index < m_voices.size(); ++voice_index)
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
				// Reset all trigger ports to 0. The rest will be set anyways later..
				for (unsigned voice_index = 0; voice_index < m_voices.size(); ++voice_index)
				{
					float *buffer = 
						(float*)jack_port_get_buffer(m_voices[voice_index]->m_jack_ports[0], nframes);
						
					std::fill(buffer, buffer + nframes, 0);
				}

				void *midi_in_buffer = jack_port_get_buffer(m_midi_in_jack_port, nframes);
				
				jack_midi_event_t midi_in_event;

				jack_nframes_t event_count = jack_midi_get_event_count(midi_in_buffer);
				jack_nframes_t event_index = 0;
				
				if (event_count > 0) 
				{
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
							
							int oldest_voice_index = oldest_voice(frame);

							m_voices[oldest_voice_index]->m_note = note;
							m_voices[oldest_voice_index]->m_gate = 1.0;
							m_voices[oldest_voice_index]->m_start_frame = jack_last_frame_time(m_jack_client) + frame;
							m_voices[oldest_voice_index]->m_on_velocity = velocity;
							
							float *trigger_out_buffer = (float*)jack_port_get_buffer(m_voices[oldest_voice_index]->m_jack_ports[0], nframes);
							
							trigger_out_buffer[frame] = 1.0;
						}
						
						if( ((*(midi_in_event.buffer)) & 0xf0) == 0x80 ) 
						{
							/** note off */

							unsigned note = *(midi_in_event.buffer + 1);
							
							int voice_index = voice_playing_note(note);
							
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
							
						gate_out_buffer[frame] = m_voices[voice_index]->m_gate;
						
						float *velocity_out_buffer = 
							(float*)jack_port_get_buffer(m_voices[voice_index]->m_jack_ports[2], nframes);
							
						velocity_out_buffer[frame] = m_voices[voice_index]->m_on_velocity / 128.0;
						
						float *freq_out_buffer = 
							(float*)jack_port_get_buffer(m_voices[voice_index]->m_jack_ports[3], nframes);
							
						freq_out_buffer[frame] = note_frequency(m_voices[voice_index]->m_note);
						
						float *scaled_freq_out_buffer = 
							(float*)jack_port_get_buffer(m_voices[voice_index]->m_jack_ports[4], nframes);
							
						scaled_freq_out_buffer[frame] = note_frequency(m_voices[voice_index]->m_note) / (float)jack_get_sample_rate(m_jack_client);
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
						m_note(0),
						m_on_velocity(0),
						m_off_velocity(0),
						m_start_frame(0),
						m_jack_client(jack_client)
					{
						unsigned port_index = 0;
						register_port(voice_index, port_index++, "trigger");
						register_port(voice_index, port_index++, "gate");
						register_port(voice_index, port_index++, "velocity");
						register_port(voice_index, port_index++, "frequency");
						register_port(voice_index, port_index++, "frequency-scaled");
					}
					
					void register_port(unsigned voice_index, unsigned port_index, std::string suffix)
					{
						std::stringstream stream;
						stream << "voice-" << voice_index << "-" << port_index << "-" << suffix;
						
						jack_port_t *port = jack_port_register
						(
							m_jack_client, 
							stream.str().c_str(),
							JACK_DEFAULT_AUDIO_TYPE,
							JackPortIsOutput,
							0
						);
						
						m_jack_ports.push_back(port);
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
