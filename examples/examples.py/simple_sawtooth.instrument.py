from ladspam1_pb2_util import *

number_of_voices = 3

instrument = Instrument()
instrument.number_of_voices = number_of_voices

synth = instrument.synth

voice_outs = []

for n in range(number_of_voices):
	env = add_plugin(synth, 'dahdsr_fexp')
	set_port_value(synth, env, 2, 0.0)
	set_port_value(synth, env, 3, 0.0)
	set_port_value(synth, env, 4, 0.0)
	set_port_value(synth, env, 5, 0.8)
	set_port_value(synth, env, 6, 0.3)
	set_port_value(synth, env, 7, 0.2)
	
	osc = add_plugin(synth, 'sawtooth_fa_oa')

	prod = add_plugin(synth, 'product_iaia_oa')

	make_connection(synth, env, 8, prod, 0)
	make_connection(synth, osc, 1, prod, 1)
	
	make_voice_connection(instrument, n, TRIGGER, env, 1)
	make_voice_connection(instrument, n, GATE, env, 0)
	make_voice_connection(instrument, n, FREQUENCY, osc, 0)

	delay = add_plugin(synth, 'delay_5s')
	set_port_value(synth, delay, 0, (1 + n) * 0.250)
	set_port_value(synth, delay, 1, 0.25)

	make_connection(synth, prod, 2, delay, 2)

	voice_outs.append(delay)

sum = add_plugin(synth, 'sum_iaia_oa')

expose_port(synth, sum, 2, "output0")
# expose_port(synth, sum, 2)

for n in range(number_of_voices):
	make_connection(synth, voice_outs[n], 3, sum, 0)
	pass

dump_instrument(instrument)

