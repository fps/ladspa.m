#include <ladspam-0/m_jack.h>

#include <unistd.h>
#include <iostream>
#include <string>

int main()
{
	ladspam::m_jack m1("m1", 8);
	m1.append_plugin("/usr/lib/ladspa/flanger_1191.so", "flanger");
	m1.append_plugin("/usr/lib/ladspa/amp.so", "amp_mono");

	m1.set_port_value(0, 2, 99);
	m1.set_port_value(0, 3, 0.9);
	
	ladspam::m_jack m2("m2", 8);
	m2.insert_plugin(0, "/usr/lib/ladspa/flanger_1191.so", "flanger");
	m2.insert_plugin(0, "/usr/lib/ladspa/amp.so", "amp_mono");

	usleep(1000000);

	m1.insert_plugin(0, "/usr/lib/ladspa/flanger_1191.so", "flanger");
	m1.insert_plugin(m1.number_of_plugins(), "/usr/lib/ladspa/amp.so", "amp_mono");

	std::string s;
	std::cin >> s;
}
