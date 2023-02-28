#include <ladspa.h>
#include <dssi.h>

#include <cmath>
#include <vector>
#include <mutex>

#include <ladspa.m1/synth.h>

static DSSI_Descriptor ladspam_dssi_descriptor;

static LADSPA_Descriptor ladspam_ladspa_descriptor;

static const int LADSPAM_NUMBER_OF_AUDIO_CHANNELS = 2;
static const int LADSPAM_NUMBER_OF_CONTROL_PORTS = 8;
static const int LADSPAM_NUMBER_OF_PORTS = LADSPAM_NUMBER_OF_AUDIO_CHANNELS + LADSPAM_NUMBER_OF_CONTROL_PORTS;

static const char * ladspam_port_names[LADSPAM_NUMBER_OF_PORTS] = 
{
	"out_0",
	"out_1",
	"control_0",
	"control_1",
	"control_2",
	"control_3",
	"control_4",
	"control_5",
	"control_6",
	"control_7"
};

static LADSPA_PortDescriptor ladspam_port_descriptors[LADSPAM_NUMBER_OF_PORTS] = 
{
	LADSPA_PORT_AUDIO | LADSPA_PORT_OUTPUT,
	LADSPA_PORT_AUDIO | LADSPA_PORT_OUTPUT,
	LADSPA_PORT_CONTROL | LADSPA_PORT_INPUT,
	LADSPA_PORT_CONTROL | LADSPA_PORT_INPUT,
	LADSPA_PORT_CONTROL | LADSPA_PORT_INPUT,
	LADSPA_PORT_CONTROL | LADSPA_PORT_INPUT,
	LADSPA_PORT_CONTROL | LADSPA_PORT_INPUT,
	LADSPA_PORT_CONTROL | LADSPA_PORT_INPUT,
	LADSPA_PORT_CONTROL | LADSPA_PORT_INPUT,
	LADSPA_PORT_CONTROL | LADSPA_PORT_INPUT
};

static LADSPA_PortRangeHint ladspam_port_range_hints[LADSPAM_NUMBER_OF_PORTS] = 
{
	{ 0, 0.0, 0.0 },
	{ 0, 0.0, 0.0 },
	{ LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE, 0.0, 1.0 },
	{ LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE, 0.0, 1.0 },
	{ LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE, 0.0, 1.0 },
	{ LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE, 0.0, 1.0 },
	{ LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE, 0.0, 1.0 },
	{ LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE, 0.0, 1.0 },
	{ LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE, 0.0, 1.0 },
	{ LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE, 0.0, 1.0 }
};

static const unsigned int ladspam_buffer_size = 64;

struct ladspam_plugin
{
	std::mutex m_mutex;

	ladspam1::synth_ptr m_synth;

	float m_phase;

	float m_last_trigger;

	float *m_ports[LADSPAM_NUMBER_OF_AUDIO_CHANNELS + LADSPAM_NUMBER_OF_CONTROL_PORTS];

	unsigned long m_sample_rate;

	ladspam_plugin(unsigned long sample_rate) :
		m_synth(new ladspam1::synth((unsigned)sample_rate, ladspam_buffer_size)),
		m_phase(0),
		m_last_trigger(0),
		m_sample_rate(sample_rate)
	{

	}
};

static LADSPA_Handle ladspam_instantiate(const LADSPA_Descriptor *descriptor, unsigned long sample_rate)
{
	return new ladspam_plugin(sample_rate);	
}

static void ladspam_cleanup(LADSPA_Handle instance)
{
	delete (ladspam_plugin*)instance;
}

static void ladspam_connect_port(LADSPA_Handle instance, unsigned long port, LADSPA_Data * data_location)
{
	((ladspam_plugin*)instance)->m_ports[port] = data_location;
}

static void ladspam_dssi_run_synth(LADSPA_Handle instance, unsigned long sample_count, snd_seq_event_t *events, unsigned long event_count)
{
	ladspam_plugin *p = (ladspam_plugin*)instance;

	bool locked = p->m_mutex.try_lock();

	if (!locked)
	{
		//! TODO: 
		return;	
	}
	p->m_mutex.unlock();
}

static void ladspam_run(LADSPA_Handle instance, unsigned long sample_count)
{
	ladspam_dssi_run_synth(instance, sample_count, 0, 0);
	
}


const LADSPA_Descriptor* ladspa_descriptor(unsigned long Index)
{
	if (Index != 0) return 0;

	// static LADSPA_Descriptor *d = new LADSPA_Descriptor;
	LADSPA_Descriptor *d = &ladspam_ladspa_descriptor;

	d->UniqueID = 0;

	d->Properties = LADSPA_PROPERTY_REALTIME | LADSPA_PROPERTY_HARD_RT_CAPABLE;
	d->Maker = "Florian Paul Schmidt";
	d->Copyright = "GPL v2 or later";
	d->PortCount = LADSPAM_NUMBER_OF_PORTS;
	
	d->PortDescriptors = ladspam_port_descriptors;
	d->PortRangeHints = ladspam_port_range_hints;

	d->instantiate = ladspam_instantiate;
	d->cleanup = ladspam_cleanup;
	d->connect_port = ladspam_connect_port;

	d->run_adding = 0;
	d->set_run_adding_gain = 0;
	d->activate = 0;
	d->deactivate = 0;
	
	d->PortNames = ladspam_port_names;

	d->ImplementationData = (void *)Index;

	switch(Index)
	{
		case 0:
			d->Label = "ladspa.m.dssi.instrument";
			d->Name = "ladspa.m dssi instrument plugin";
			d->run = ladspam_run;
			break;
		default:
			return 0;
	}

	return d;
}




static char *ladspam_dssi_configure(LADSPA_Handle instance, const char *key, const char *value)
{
	ladspam_plugin *p = (ladspam_plugin*)instance;
	std::lock_guard<std::mutex> lock(p->m_mutex);

	if (std::string("definition") == key)
	{
		p->m_synth = ladspam1::synth_ptr(new ladspam1::synth((unsigned)p->m_sample_rate, ladspam_buffer_size));
	}
	
	return 0;
}


const DSSI_Descriptor *dssi_descriptor(unsigned long index)
{
	if (index != 0) 
	{
		return 0;
	}

	DSSI_Descriptor *d = &ladspam_dssi_descriptor;

	d->DSSI_API_Version = 1;
	d->LADSPA_Plugin = ladspa_descriptor(0);
	d->configure = ladspam_dssi_configure;
	d->get_program = 0;
	d->select_program = 0;
	d->get_midi_controller_for_port = 0;
	d->run_synth = ladspam_dssi_run_synth;
	d->run_synth_adding = 0;
	d->run_multiple_synths = 0;
	d->run_multiple_synths_adding = 0;

	return d;
}

#ifdef LADSPAM_MAIN_CC

#include <iostream>
#include <vector>

int main()
{
	const unsigned long sample_rate = 1000;
	const float frequency = 1.0;
	const unsigned long buffer_size = 2000;
	const LADSPA_Descriptor *d = ladspa_descriptor(0);
	LADSPA_Handle h = d->instantiate(d, sample_rate);
	std::vector<std::vector<float>> buffers;
	for (size_t index = 0; index < LADSPAM_NUMBER_OF_PORTS; ++index)
	{
		buffers.push_back(std::vector<float>(buffer_size,0));
		d->connect_port(h, index, &(buffers[index][0]));
	}
	for (size_t index = 0; index < buffer_size; ++index)
	{
		buffers[0][index] = frequency;
		buffers[1][index] = 0;
		buffers[2][index] = 0;
	}
	d->run(h, buffer_size);
	for (size_t index = 0; index < buffer_size; ++index)
	{
		std::cout << buffers[3][index] << std::endl;
	}
	d->cleanup(h);
}

#endif

