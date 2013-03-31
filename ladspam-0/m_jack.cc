#include <ladspam-0/m_jack.h>

namespace ladspam
{
	extern "C"
	{
		int jack_process(jack_nframes_t nframes, void* arg)
		{
			return ((m_jack*)arg)->process(nframes);
		}
	}
}