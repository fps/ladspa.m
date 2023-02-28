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
start_band = number_of_bands / 4
end_band = 4 * number_of_bands / 4


for band in range(start_band, end_band):
	center_note = band * notes_per_band
	center_note_plus_one = (band + 1) * notes_per_band
	#center_freq = frequency_by_samplerate(note_frequency(center_note), samplerate)
	center_note_freq = note_frequency(center_note)
	center_note_plus_one_freq = note_frequency(center_note_plus_one)
	bandwidth = 1 * (center_note_plus_one_freq - center_note_freq)

	bandpass = add_plugin(synth, 'bandpass_iir')
	set_port_value(synth, bandpass, 0, center_note_freq)
	set_port_value(synth, bandpass, 1, bandwidth)
	set_port_value(synth, bandpass, 2, 3)

	# set_port_value(synth, bandpass, 1, bandwidth)

	make_connection(synth, input_gain, 2, bandpass, 3)

	distortion = add_plugin(synth, 'invada_mono_tube_module_0_1')
	make_connection(synth, bandpass, 4, distortion, 4)
	set_port_value(synth, distortion, 0, 0.0)
	set_port_value(synth, distortion, 3, 50)
	
	bands.append(distortion)
        
output_gain = add_plugin(synth, 'amp_stereo')
set_port_value(synth, output_gain, 0, 1)
expose_port(synth, output_gain, 2, "output0")
expose_port(synth, output_gain, 4, "output1")

index = 0
for band in bands:
	if index % 2 == 0:
		make_connection(synth, band, 5, output_gain, 1)
	else:
		make_connection(synth, band, 5, output_gain, 3)
	index += 1
        


dump_instrument(synth)

