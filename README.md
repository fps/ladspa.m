# ladspa.m

A modular synthesis library for LADSPA plugins. 

# Requirements

* ladspamm-0 - https://github.com/fps/ladspamm

# Building

There is nothing to build except the little test program since ladspa.m is a header only library. To build the test program type

<pre>
make
</pre>

# Installation

Type

<pre>
make install
</pre>

preferrably as superuser:

<pre>
sudo make install
</pre>

This will install the header to /usr/local/include/ladspam-0/ and a pkg-config file to /usr/local/lib/pkgconfig/.

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
