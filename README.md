# ladspa.m

A modular synthesis library for LADSPA plugins. 

# Requirements

* ladspamm-0 - https://github.com/fps/ladspamm

# Building

There is nothing to build except the little test program since ladspa.m is a header only library. To build the test program it type

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

# License 

LGPL v2.0 or later (see the LICENSE file)

# Author

Florian Paul Schmidt (mista.tapas@gmx.net)
