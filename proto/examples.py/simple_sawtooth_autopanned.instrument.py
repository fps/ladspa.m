from ladspam1_pb2_util import *
import random

number_of_voices = 5

instrument = Instrument()
instrument.number_of_voices = number_of_voices

synth = instrument.synth

voice_outs = []

depth_prod = add_plugin(synth, 'product_iaia_oa')
set_port_value(synth, depth_prod, 0, 100)

make_control_connection(instrument, 0, 1, depth_prod, 1)

for n in range(number_of_voices):
	env = add_plugin(synth, 'dahdsr_fexp')

	set_port_value(synth, env, 2, 0.0)
	set_port_value(synth, env, 3, 0.0)
	set_port_value(synth, env, 4, 0.0)
	set_port_value(synth, env, 5, 0.8)
	set_port_value(synth, env, 6, 1.3)
	set_port_value(synth, env, 7, 1.2)
	
	osc = add_plugin(synth, 'sawtooth_fa_oa')

	env_prod = add_plugin(synth, 'product_iaia_oa')

	make_connection(synth, env, 8, env_prod, 0)
	make_connection(synth, osc, 1, env_prod, 1)

	make_voice_connection(instrument, n, TRIGGER, env, 1)
	make_voice_connection(instrument, n, GATE, env, 0)
	make_voice_connection(instrument, n, FREQUENCY, osc, 0)

	vel_prod = add_plugin(synth, 'product_iaia_oa')
	make_voice_connection(instrument, n, VELOCITY, vel_prod, 0)
	make_connection(synth, env_prod, 2, vel_prod, 1)

	pan = add_plugin(synth, 'tap_autopan')

	set_port_value(synth, pan, 0, random.uniform(1.5, 2.2))
	set_port_value(synth, pan, 2, 0)

	make_connection(synth, depth_prod, 2, pan, 1)

	make_connection(synth, vel_prod, 2, pan, 3)
	make_connection(synth, vel_prod, 2, pan, 4)
	
	voice_outs.append(pan)


sum1 = add_plugin(synth, 'sum_iaia_oa')
sum2 = add_plugin(synth, 'sum_iaia_oa')

for n in range(number_of_voices):
	make_connection(synth, voice_outs[n], 5, sum1, 0)
	make_connection(synth, voice_outs[n], 6, sum2, 0)
	pass

expose_port(synth, sum1, 2, "output0")
expose_port(synth, sum2, 2, "output1")
	
dump_instrument(instrument)

