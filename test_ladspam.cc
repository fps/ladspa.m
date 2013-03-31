#include <ladspam-0/m_jack.h>

#include <unistd.h>

int main()
{
	ladspam::m_jack m("m");
	m.insert_plugin(0, "/usr/lib/ladspa/flanger_1191.so", "flanger");
	m.insert_plugin(0, "/usr/lib/ladspa/amp.so", "amp_mono");
#if 0
	
	usleep(1000000);

	m.insert_plugin(0, "/usr/lib/ladspa/flanger_1191.so", "flanger");
	m.insert_plugin(m.number_of_plugins(), "/usr/lib/ladspa/amp.so", "amp_mono");
#endif
	usleep(1000000);
}
