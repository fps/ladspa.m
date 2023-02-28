.PHONY: all install clean

PREFIX ?= /usr/local

all: ladspa.m.jack.instrument ladspa.m.jack.synth

CXXFLAGS ?= -O3 -march=native -mfpmath=sse -DNDEBUG -Wall
#CXXFLAGS ?= -O0 -g -march=native -mfpmath=sse -DNDEBUG
CXXFLAGS += `pkg-config ladspa.m-1 ladspamm-1 ladspa.m.proto-1 jack --cflags`
LDFLAGS += `pkg-config ladspa.m-1 ladspamm-1 ladspa.m.proto-1 jack --libs`

ladspa.m.jack.instrument: instrument.cc ladspam-jack-0/instrument.h ladspam-jack-0/synth.h ladspam-jack-0/synth.cc
	g++ $(CXXFLAGS) -o ladspa.m.jack.instrument instrument.cc -I . ladspam-jack-0/instrument.cc ladspam-jack-0/synth.cc $(LDFLAGS) 
ladspa.m.jack.synth: synth.cc ladspam-jack-0/instrument.h ladspam-jack-0/synth.h ladspam-jack-0/synth.cc
	g++ $(CXXFLAGS) -o ladspa.m.jack.synth synth.cc -I . ladspam-jack-0/synth.cc $(LDFLAGS) 
 
install: all
	install -d $(PREFIX)/bin
	install ladspa.m.jack.instrument $(PREFIX)/bin
	install ladspa.m.jack.synth $(PREFIX)/bin

clean:
	rm -f ladspa.m.jack.instrument  ladspa.m.jack.synth
