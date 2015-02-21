# ladspa.m

A modular synthesis library for LADSPA plugins. 

# Requirements

* ladspamm-1 - https://github.com/fps/ladspamm
* cmake 2.8

# Building

There is nothing to build except the little test program since ladspa.m is a header only library. To build the test program type

	mkdir bld
	cmake ..
	make 
	make install

There is also a debian package generator. If you want to use that:

	mkdir bld
	cmake ..
	make package
	dpkg -i ladspa.m-1-Linux.deb

# How to use it?

Check out the test program 

https://github.com/fps/ladspa.m/blob/master/test_synth.cc

Note that the ladspa.m does not provide any means to discover plugins on the system. For that use the ladspamm library

https://github.com/fps/ladspamm

# Python bindings

You can also check out the python bindings for ladspa.m

https://github.com/fps/ladspa.m.swig

which allows to setup and execute synthesis graphs from within python for increased flexibility.

# Protobuf file format

There is a library for defining a file format for synth graphs:

https://github.com/fps/ladspa.m.proto

# Jack synth and instrument hosts

There is also a library for loading said files into a jack host:

https://github.com/fps/ladspa.m.jack


# License 

LGPL v2.0 or later (see the LICENSE file)

# Author

Florian Paul Schmidt (mista.tapas@gmx.net)
