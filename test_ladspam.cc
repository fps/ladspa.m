#include <ladspam-0/jack/synth.h>
#include <ladspam-0/jack/instrument.h>

#include <unistd.h>
#include <iostream>
#include <string>

int main()
{
#if 0
	ladspam::jack::synth m1("m1", 8);
	m1.append_plugin("/usr/lib/ladspa/flanger_1191.so", "flanger");
	m1.append_plugin("/usr/lib/ladspa/amp.so", "amp_mono");

	m1.set_port_value(0, 2, 99);
	m1.set_port_value(0, 3, 0.9);
	
	m1.connect(0, 5, 1, 0);
	
	ladspam::jack::synth m2("m2", 8);
	m2.insert_plugin(0, "/usr/lib/ladspa/flanger_1191.so", "flanger");
	m2.insert_plugin(0, "/usr/lib/ladspa/amp.so", "amp_mono");

	m1.disconnect(0, 5, 1, 0);

	m1.insert_plugin(0, "/usr/lib/ladspa/flanger_1191.so", "flanger");
	m1.insert_plugin(m1.number_of_plugins(), "/usr/lib/ladspa/amp.so", "amp_mono");
#endif

#if 0
	ladspam::jack::instrument i1("i1", 8, 2);

	i1.append_plugin("/usr/lib/ladspa/dahdsr_fexp.so", "dahdsr_fexp");
	i1.append_plugin("/usr/lib/ladspa/product_1668.so", "product_iaia_oa");
	i1.append_plugin("/usr/lib/ladspa/sawtooth_1641.so", "sawtooth_fa_oa");

	i1.append_plugin("/usr/lib/ladspa/dahdsr_fexp.so", "dahdsr_fexp");
	i1.append_plugin("/usr/lib/ladspa/product_1668.so", "product_iaia_oa");
	i1.append_plugin("/usr/lib/ladspa/sawtooth_1641.so", "sawtooth_fa_oa");

	// Set the parameters for the envelope of voice 1
	i1.set_port_value(0, 4, 0.2);
	i1.set_port_value(0, 5, 0.2);
	i1.set_port_value(0, 6, 0.2);
	i1.set_port_value(0, 7, 0.2);
	
	// Set the parameters for the envelope of voice 2
	i1.set_port_value(3, 4, 0.2);
	i1.set_port_value(3, 5, 0.2);
	i1.set_port_value(3, 6, 0.2);
	i1.set_port_value(3, 7, 0.2);
#endif

	ladspam::synth synth(48000, 8);
	
	synth.append_plugin("/usr/lib/ladspa/dahdsr_fexp.so", "dahdsr_fexp");
	synth.append_plugin("/usr/lib/ladspa/product_1668.so", "product_iaia_oa");
	synth.append_plugin("/usr/lib/ladspa/sawtooth_1641.so", "sawtooth_fa_oa");

	synth.append_plugin("/usr/lib/ladspa/dahdsr_fexp.so", "dahdsr_fexp");
	synth.append_plugin("/usr/lib/ladspa/product_1668.so", "product_iaia_oa");
	synth.append_plugin("/usr/lib/ladspa/sawtooth_1641.so", "sawtooth_fa_oa");

	// Set the parameters for the envelope of voice 1
	synth.set_port_value(0, 4, 0.2);
	synth.set_port_value(0, 5, 0.2);
	synth.set_port_value(0, 6, 0.2);
	synth.set_port_value(0, 7, 0.2);
	
	// Set the parameters for the envelope of voice 2
	synth.set_port_value(3, 4, 0.2);
	synth.set_port_value(3, 5, 0.2);
	synth.set_port_value(3, 6, 0.2);
	synth.set_port_value(3, 7, 0.2);

	synth.connect(0, 8, 1, 0);
	synth.connect(0, 8, 1, 0);
	synth.disconnect(0, 8, 1, 0);
	synth.disconnect(0, 8, 1, 0);
	
	for (unsigned chunk = 0; chunk < 1000; ++chunk)
	{
		synth.process();
	}
	
#if 0
	std::string s;
	std::cin >> s;
#endif
	
}
