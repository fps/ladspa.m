#ifndef LADSPAM_INCLUDED_HH
#define LADSPAM_INCLUDED_HH

#include <boost/shared_ptr.hpp>

#include <ladspam-0/m.h>
#include <ladspam-0/ringbuffer.h>

namespace ladspamm
{
	struct plugin_instance;
}

namespace ladspam
{
	/* 
		m like modular...
	*/
	struct m_jack : m 
	{
		virtual ~m_jack()
		{

		}

		virtual unsigned number_of_plugins() const = 0;

		virtual void remove_plugin(unsigned index) = 0;

		/*
			The parameter index must be in the
			range 0 <= index < number_of_plugins()
		*/
		virtual void insert_plugin
		(
			unsigned index, 
			const std::string &library, 
			const std::string& label
		) = 0;

		virtual void connect
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index,
		) = 0;

		virtual void disconnect
		(
			unsigned source_plugin_index,
			unsigned source_port_index,
			unsigned sink_plugin_index,
			unsigned sink_port_index,
		) = 0;
	};
}

#endif

