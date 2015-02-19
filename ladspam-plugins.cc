#include <ladspa.h>
#include <cmath>

static const int LADSPAM_NUMBER_OF_PLUGINS = 3;
static LADSPA_Descriptor ladspam_descriptors[LADSPAM_NUMBER_OF_PLUGINS];

static const int LADSPAM_NUMBER_OF_PORTS = 5;

static const char* ladspam_port_names[] = 
{
    "Frequency",
    "Gate",
    "Trigger",
    "Phase",
    "Out"
};

static LADSPA_PortDescriptor ladspam_port_descriptors[] = 
{
    LADSPA_PORT_AUDIO | LADSPA_PORT_INPUT,
    LADSPA_PORT_AUDIO | LADSPA_PORT_INPUT,
    LADSPA_PORT_AUDIO | LADSPA_PORT_INPUT,
    LADSPA_PORT_AUDIO | LADSPA_PORT_INPUT,
    LADSPA_PORT_AUDIO | LADSPA_PORT_OUTPUT
};

static LADSPA_PortRangeHint ladspam_port_range_hints[] = 
{
    { LADSPA_HINT_SAMPLE_RATE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_440, 0.0, 0.5 },
    { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MAXIMUM, 0.0, 1.0 },
    { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MINIMUM, 0.0, 1.0 },
    { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MINIMUM, 0.0, 1.0 },
    { 0, 0.0, 0.0 }
};

struct ladspam_plugin
{
    float m_phase;

    float *m_ports[LADSPAM_NUMBER_OF_PORTS];

    unsigned long m_sample_rate;

    ladspam_plugin(unsigned long sample_rate) :
        m_phase(0),
        m_sample_rate(sample_rate)
    {

    }
};

LADSPA_Handle ladspam_instantiate(const LADSPA_Descriptor *descriptor, unsigned long sample_rate)
{
    return new ladspam_plugin(sample_rate);    
}

void ladspam_cleanup(LADSPA_Handle instance)
{
    delete (ladspam_plugin*)instance;
}

void ladspam_connect_port(LADSPA_Handle instance, unsigned long port, LADSPA_Data * data_location)
{
    ((ladspam_plugin*)instance)->m_ports[port] = data_location;
}

void ladspam_run_sine(LADSPA_Handle instance, unsigned long sample_count)
{
    ladspam_plugin *i = (ladspam_plugin*)instance;
    for (unsigned long index = 0; index < sample_count; ++index)
    {
        i->m_phase += i->m_ports[0][index] * 2.0 * M_PI / i->m_sample_rate;
        i->m_phase = fmod(i->m_phase, 2.0 * M_PI);
        i->m_ports[4][index] = sin(i->m_phase);
    }
}

void ladspam_run_sawtooth(LADSPA_Handle instance, unsigned long sample_count)
{

}

void ladspam_run_square(LADSPA_Handle instance, unsigned long sample_count)
{

}

const LADSPA_Descriptor* ladspa_descriptor(unsigned long Index)
{
    if (Index >= LADSPAM_NUMBER_OF_PLUGINS) return 0;

    // static LADSPA_Descriptor *d = new LADSPA_Descriptor;
    LADSPA_Descriptor *d = &ladspam_descriptors[Index];

    d->UniqueID = 0;

    d->Properties = LADSPA_PROPERTY_REALTIME | LADSPA_PROPERTY_HARD_RT_CAPABLE;
    d->Maker = "Florian Paul Schmidt";
    d->Copyright = "GPL v2 or later";
    d->PortCount = /* freq phase gate trigger */ 4 + /* out */ 1;    
    
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
            d->Label = "ladspam-sine-osc";
            d->Name = "ladspam sine oscillator";
            d->run = ladspam_run_sine;
            break;
        case 1:
            d->Label = "ladspam-saw-osc";
            d->Name = "ladspam sawtooth oscillator";
            d->run = ladspam_run_sawtooth;
            break;
        case 2:
            d->Label = "ladspam-square-osc";
            d->Name = "ladspam square oscillator";
            d->run = ladspam_run_square;
            break;
    }

    return d;
}

