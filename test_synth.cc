#include <ladspam-0/jack/synth.h>
#include <ladspam-0/jack/instrument.h>

#include <unistd.h>
#include <iostream>
#include <string>

#include <boost/timer/timer.hpp>

int main()
{
	ladspam::synth synth(48000, 8);
	
	synth.append_plugin("/usr/lib/ladspa/dahdsr_fexp.so", "dahdsr_fexp");
	synth.append_plugin("/usr/lib/ladspa/product_1668.so", "product_iaia_oa");
	synth.append_plugin("/usr/lib/ladspa/sawtooth_1641.so", "sawtooth_fa_oa");
	synth.append_plugin("/usr/lib/ladspa/sawtooth_1641.so", "sawtooth_fa_oa");

	// Set the parameters for the envelope of voice 1
	synth.set_port_value(0, 4, 0.2);
	synth.set_port_value(0, 5, 0.2);
	synth.set_port_value(0, 6, 0.2);
	synth.set_port_value(0, 7, 0.2);
	
	synth.connect(2, 1, 1, 0);
	synth.connect(3, 1, 1, 1);
	
	boost::timer::auto_cpu_timer t;
	
	for (unsigned chunk = 0; chunk < (256/8); ++chunk)
	{
		synth.process();
		
		for (unsigned frame = 0; frame < 8; ++frame)
		{
			std::cout << (*synth.get_buffer(1, 2))[frame] << std::endl;
		}
	}
}
