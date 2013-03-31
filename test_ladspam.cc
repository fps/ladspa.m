#include <ladspam-0/jack/synth.h>
#include <ladspam-0/jack/instrument.h>

#include <unistd.h>
#include <iostream>
#include <string>

int main()
{
	ladspam::jack::synth m1("m1", 8);
	m1.append_plugin("/usr/lib/ladspa/flanger_1191.so", "flanger");
	m1.append_plugin("/usr/lib/ladspa/amp.so", "amp_mono");

	m1.set_port_value(0, 2, 99);
	m1.set_port_value(0, 3, 0.9);
	
	m1.connect(0, 5, 1, 0);
	
	ladspam::jack::synth m2("m2", 8);
	m2.insert_plugin(0, "/usr/lib/ladspa/flanger_1191.so", "flanger");
	m2.insert_plugin(0, "/usr/lib/ladspa/amp.so", "amp_mono");

	ladspam::jack::instrument i1("i1", 8, 3);
	i1.append_plugin("/usr/lib/ladspa/flanger_1191.so", "flanger");
	i1.append_plugin("/usr/lib/ladspa/amp.so", "amp_mono");
	i1.connect(0, 5, 1, 0);

	usleep(10000000);

	m1.disconnect(0, 5, 1, 0);

	m1.insert_plugin(0, "/usr/lib/ladspa/flanger_1191.so", "flanger");
	m1.insert_plugin(m1.number_of_plugins(), "/usr/lib/ladspa/amp.so", "amp_mono");

	std::string s;
	std::cin >> s;
}
