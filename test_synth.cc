#include <ladspam-0/synth.h>

#include <unistd.h>
#include <iostream>
#include <string>

#include <boost/timer/timer.hpp>

/**
 * @brief An example program for ladspa.m
 * 
 * This example serves to illustrate how to load a number of LADSPA plugins and connect them. Also it shows how to get access to the output buffers.
 */
int main()
{
	const unsigned buffer_size = 32;
	const unsigned sample_rate = 48000;
	
	ladspam::synth synth(sample_rate, buffer_size);
	
	synth.append_plugin("ladspam-0-plugins.so", "ladspam-sine-osc");

	synth.set_port_value(0, 0, 1);
	
	// synth.connect(2, 1, 4, 1);
	
	std::cout << "Generating 100 seconds of audio at a samplerate of " << sample_rate << "..." << std::endl;
	
	{
		boost::timer::auto_cpu_timer t;
		
		for (unsigned chunk = 0; chunk < (100 * sample_rate) / buffer_size; ++chunk)
		{
			synth.process(buffer_size);
			for (unsigned frame = 0; frame < 8; ++frame)
			{
				std::cout << (*synth.get_buffer(0, 4))[frame] << std::endl;
			}
		}
	}
	
	std::cout << "Done. " << std::endl;
}
