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
	ladspam::synth synth(48000, buffer_size);
	
	synth.append_plugin("/usr/lib/ladspa/sawtooth_1641.so", "sawtooth_fa_oa");
	synth.append_plugin("/usr/lib/ladspa/sawtooth_1641.so", "sawtooth_fa_oa");
	synth.append_plugin("/usr/lib/ladspa/sawtooth_1641.so", "sawtooth_fa_oa");
	synth.append_plugin("/usr/lib/ladspa/product_1668.so", "product_iaia_oa");
	synth.append_plugin("/usr/lib/ladspa/product_1668.so", "product_iaia_oa");

	synth.set_port_value(0, 0, 666);
	
	synth.connect(0, 1, 3, 0);
	synth.connect(1, 1, 3, 1);
	synth.connect(3, 2, 4, 0);
	synth.connect(2, 1, 4, 1);
	
	boost::timer::auto_cpu_timer t;
	
	for (unsigned chunk = 0; chunk < (100 * 48000) / buffer_size; ++chunk)
	{
		synth.process(buffer_size);
#if 0		
		for (unsigned frame = 0; frame < 8; ++frame)
		{
			std::cout << (*synth.get_buffer(4, 2))[frame] << std::endl;
		}
#endif
	}
}
