from ladspam1_pb2_util import *

synth = Synth()

input_gain = add_plugin(synth, 'amp_mono')
set_port_value(synth, input_gain, 0, 1)

expose_port(synth, input_gain, 1, "input0")

samplerate = 48000.0

def note_frequency(note):
	return pow(2.0, (note - 69.0)/12.0) * 440.0

def frequency_by_samplerate(freq, samplerate):
	return freq / samplerate

notes_per_band = 4

bands = []

number_of_bands = 128/notes_per_band
start_band = number_of_bands / 5
start_band = 0
#end_band = 4 * number_of_bands / 5
end_band = number_of_bands


for band in range(start_band, end_band):
	center_note = band * notes_per_band
	center_note_plus_one = (band + 1) * notes_per_band

	center_note_freq = note_frequency(center_note)
	center_note_plus_one_freq = note_frequency(center_note_plus_one)

	bandwidth = 2 * (center_note_plus_one_freq - center_note_freq)

	bandpass = add_plugin(synth, 'bandpass_iir')
	set_port_value(synth, bandpass, 0, center_note_freq)
	set_port_value(synth, bandpass, 1, bandwidth)
	set_port_value(synth, bandpass, 2, 1)

	make_connection(synth, input_gain, 2, bandpass, 3)

	distortion = add_plugin(synth, 'invada_mono_tube_module_0_1')
	make_connection(synth, bandpass, 4, distortion, 4)
	set_port_value(synth, distortion, 0, 6.0)
	set_port_value(synth, distortion, 3, 100)
	
	bandpass2 = add_plugin(synth, 'bandpass_iir')
	set_port_value(synth, bandpass2, 0, center_note_freq)
	set_port_value(synth, bandpass2, 1, 1 * bandwidth)
	set_port_value(synth, bandpass2, 2, 1)
	make_connection(synth, distortion, 5, bandpass2, 3)

	autopan = add_plugin(synth, 'tap_autopan')
	set_port_value(synth, autopan, 0, 1.0 + (2.1111111 / number_of_bands) * band)
	set_port_value(synth, autopan, 1, 100)
	make_connection(synth, bandpass2, 4, autopan, 3)
	make_connection(synth, bandpass2, 4, autopan, 4)
	
	bands.append(autopan)
        
output_gain = add_plugin(synth, 'amp_stereo')
set_port_value(synth, output_gain, 0, 0.8)
expose_port(synth, output_gain, 2, "output0")
expose_port(synth, output_gain, 4, "output1")

for band in bands:
	make_connection(synth, band, 5, output_gain, 1)
	make_connection(synth, band, 6, output_gain, 3)
        


dump_instrument(synth)

