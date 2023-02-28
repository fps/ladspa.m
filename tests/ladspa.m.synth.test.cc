#include <ladspa.m1/synth.h>

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
  try {
		namespace ladspam = ladspam1;
	
		const unsigned buffer_size = 8;
		const unsigned sample_rate = 1000;
		const unsigned int seconds = 2;
		
		ladspam::synth synth(sample_rate, buffer_size);
		
		synth.append_plugin("plugins/ladspa/ladspa.m.osc.plugins.so", "ladspa.m.sine.osc");
	
		synth.set_port_value(0, 0, 1);
		synth.set_port_value(0, 1, 0);
		synth.set_port_value(0, 2, 0);
		
		// synth.connect(2, 1, 4, 1);
		
		std::cout << "Generating approximately " << seconds << " seconds of audio at a samplerate of " << sample_rate << "..." << std::endl;
		
		{
			boost::timer::auto_cpu_timer t;
			
			for (unsigned chunk = 0; chunk < (seconds * sample_rate) / buffer_size; ++chunk)
			{
				synth.process(buffer_size);
				for (unsigned frame = 0; frame < buffer_size; ++frame)
				{
					std::cout << frame << ": \t" << (*synth.get_buffer(0, 3))[frame] << std::endl;
				}
			}
		}
		
		std::cout << "Done. " << std::endl;
	} catch (std::runtime_error &e) {
		std::cout << "Error: " << e.what() << std::endl;
	}
}
