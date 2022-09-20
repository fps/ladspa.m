from ladspam1_pb2_util import *

number_of_voices = 3

instrument = Instrument()

instrument.number_of_voices = number_of_voices

synth = instrument.synth

voice_outs = []

for voice in range(0, number_of_voices):
	# Envelope for the frequency
	freq_env = add_plugin(synth, 'ladspa.m.exp.env')
	set_port_value(synth, freq_env, 0, 0.002)

	# Hook it up to the voice ports
	# make_voice_connection(instrument, voice, GATE, freq_env, 0)
	make_voice_connection(instrument, voice, TRIGGER, freq_env, 1)
	
	# Scale up the envelope output to the required frequency sweep
	freq_env_prod = add_plugin(synth, 'product_iaia_oa')
	set_port_value(synth, freq_env_prod, 0, 10000)
	make_connection(synth, freq_env, 2, freq_env_prod, 1)


	# We sum the base frequency with the scaled envelope output
	freq_env_sum = add_plugin(synth, 'sum_iaia_oa')
	
	make_connection(synth, freq_env_prod, 2, freq_env_sum, 0)
	make_voice_connection(instrument, voice, FREQUENCY, freq_env_sum, 1)


	osc = add_plugin(synth, 'ladspa.m.sine.osc')
	## set_port_value(synth, osc, 1, 1)
	
	make_connection(synth, freq_env_sum, 2, osc, 0)
	
	# make_connection(synth, freq_env_sum, 2, osc, 0)
	make_voice_connection(instrument, voice, FREQUENCY, osc, 0)
	 
	# Envelope for the amplitude
	amp_env = add_plugin(synth, 'ladspa.m.exp.env')
	set_port_value(synth, amp_env, 0, 0.05)

	# Hook it up to the voice ports
	# make_voice_connection(instrument, voice, GATE, amp_env, 0)
	make_voice_connection(instrument, voice, TRIGGER, amp_env, 1)
	
	# A product to scale the oscillator output with the envelope
	amp_env_prod = add_plugin(synth, 'product_iaia_oa')

	make_connection(synth, amp_env, 2, amp_env_prod, 0)
	make_connection(synth, osc, 3, amp_env_prod, 1)
	
	voice_outs.append(amp_env_prod)
	
sum = add_plugin(synth, 'sum_iaia_oa')
set_port_value(synth, sum, 1, 0)

for voice in range(number_of_voices):
	make_connection(synth, voice_outs[voice], 2, sum, 0)

expose_port(synth, sum, 2, "output")
# expose_port(synth, sum, 2)

dump_instrument(instrument)
