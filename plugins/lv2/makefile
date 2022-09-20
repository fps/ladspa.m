.PHONY: all install clean

all: bld/ladspa.m.lv2/instrument.so

PREFIX?=/usr/local

CXXFLAGS=-Wall -O3 -march=native
DEPFLAGS= `pkg-config --cflags --libs ladspa.m-1 ladspa.m.proto-1`

bld/ladspa.m.lv2/instrument.so: instrument.cc uris.h
	g++ $(CXXFLAGS) -o bld/ladspa.m.lv2/instrument.so -shared -fPIC instrument.cc ${DEPFLAGS}

install: bld/ladspa.m.lv2/instrument.so
	install -d ${PREFIX}/lib/lv2/ladspa.m.lv2
	install bld/ladspa.m.lv2/* ${PREFIX}/lib/lv2/ladspa.m.lv2

clean:
	rm -f bld/ladspa.m.lv2/instrument.so bld/ladspa.m.lva/synth.so

