#ifndef LADSPAM_M_INCLUDED_HH
#define LADSPAM_M_INCLUDED_HH

#include <string>
#include <vector>

namespace ladspam
{
	/*
		The one unique interface.. Subclasses for
		special implementations might offer yet more
		derived types, e.g. ladspam::jack::instrument.
		
		Subclasses for special implementations also 
		offer unique ways to expose some or all unconnected
		plugin ports to the "outside", e.g. ladspam::jack::synth.
	*/
	struct synth 
	{
		virtual ~synth()
		{

		}

		virtual unsigned number_of_plugins() const = 0;

		/*
			The parameter index must be in the
			range 0 <= index < number_of_plugins().
		*/
		virtual void remove_plugin(unsigned index) = 0;

		/*
			The parameter index must be in the
			range 0 <= index < number_of_plugins().
		*/
		virtual void insert_plugin
		(
			unsigned index, 
			const std::string &library, 
			const std::string& label
		) = 0;
		
		virtual void append_plugin
		(
			const std::string &library, 
			const std::string& label
		)
		{
			insert_plugin(number_of_plugins(), library, label);	
		}
		
		
		virtual void connect
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index
		) = 0;

		virtual void disconnect
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index
		) = 0;

		virtual bool set_port_value
		(
			unsigned plugin_index,
			unsigned port_index,
			float value
		) = 0;
	};
}

#endif

