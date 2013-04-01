#include <ladspam-0/jack/synth.h>
#include <ladspam-0/jack/instrument.h>

#include <unistd.h>
#include <iostream>
#include <string>

#include <boost/timer/timer.hpp>

int main()
{
	ladspam::synth synth(48000, 8);
	
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
	
	for (unsigned chunk = 0; chunk < (1024/8); ++chunk)
	{
		synth.process();
		
		for (unsigned frame = 0; frame < 8; ++frame)
		{
			std::cout << (*synth.get_buffer(4, 2))[frame] << std::endl;
		}
	}
}
