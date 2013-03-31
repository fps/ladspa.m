#include <ladspam-0/m_jack.h>

#include <unistd.h>
#include <iostream>
#include <string>

int main()
{
	ladspam::m_jack m1("m1", 8);
	m1.insert_plugin(0, "/usr/lib/ladspa/flanger_1191.so", "flanger");
#if 0
	m1.insert_plugin(0, "/usr/lib/ladspa/amp.so", "amp_mono");

	ladspam::m_jack m2("m2");
	m2.insert_plugin(0, "/usr/lib/ladspa/flanger_1191.so", "flanger");
	m2.insert_plugin(0, "/usr/lib/ladspa/amp.so", "amp_mono");

	usleep(1000000);

	m1.insert_plugin(0, "/usr/lib/ladspa/flanger_1191.so", "flanger");
	m1.insert_plugin(m1.number_of_plugins(), "/usr/lib/ladspa/amp.so", "amp_mono");
#endif
	std::string s;
	std::cin >> s;
}
