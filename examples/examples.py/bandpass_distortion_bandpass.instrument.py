from ladspam1_pb2_util import *

number_of_voices = 5

instrument = Instrument()
instrument.number_of_voices = number_of_voices

synth = instrument.synth

voice_outs = []

input_sum = add_plugin(synth, 'sum_iaia_oa')
expose_port(synth, input_sum , 0, "input0")
expose_port(synth, input_sum , 1, "input1")

for n in range(number_of_voices):
	env = add_plugin(synth, 'dahdsr_fexp')
	set_port_value(synth, env, 2, 0.0)
	set_port_value(synth, env, 3, 0.0)
	set_port_value(synth, env, 4, 0.0)
	set_port_value(synth, env, 5, 0.8)
	set_port_value(synth, env, 6, 0.3)
	set_port_value(synth, env, 7, 0.2)
	
	noise =  add_plugin(synth, "noise_source_white")
	set_port_value(synth, noise, 0, 0.6)
	
	bp = add_plugin(synth, 'bandpass_iir')
	set_port_value(synth, bp, 2, 2)
	
	
	make_connection(synth, input_sum, 2, bp, 3)
	make_connection(synth, noise, 1,  bp, 3)

	prod = add_plugin(synth, 'product_iaia_oa')

	make_connection(synth, env, 8, prod, 0)
	make_connection(synth, bp, 4, prod, 1)
	
	make_voice_connection(instrument, n, TRIGGER, env, 1)
	make_voice_connection(instrument, n, GATE, env, 0)
	make_voice_connection(instrument, n, FREQUENCY, bp, 0)


	voice_outs.append(prod)

sum = add_plugin(synth, 'sum_iaia_oa')

expose_port(synth, sum, 2, "output0")
expose_port(synth, sum, 2, "output1")

for n in range(number_of_voices):
	make_connection(synth, voice_outs[n], 2, sum, 0)
	pass

dump_instrument(instrument)

