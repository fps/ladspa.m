PREFIX ?= /usr/local
CXXFLAGS ?= -O2 -g -pedantic -Wall -Werror -std=c++11 -I. ${ADDITIONAL_CXXFLAGS} `pkg-config ladspamm1 --cflags` 
LDFLAGS ?= `pkg-config ladspamm1 --libs` -lboost_timer
VALGRIND_FLAGS ?= --leak-check=full  --show-leak-kinds=all

all: jack-tools

test: tests/ladspa.m.synth.test plugins.ladspa
	LADSPA_PATH="" valgrind ${VALGRIND_FLAGS} ./tests/ladspa.m.synth.test

jack-tools: jack-tools/synth

plugins: plugins.ladspa plugins.lv2

plugins.lv2:

plugins.ladspa: plugins/ladspa/ladspa.m.dssi.instrument.so  plugins/ladspa/ladspa.m.env.plugins.so  plugins/ladspa/ladspa.m.osc.plugins.so

%.so: %.cc
	g++ ${CXXFLAGS} -shared -o $@ -shared $<

install:
	install -d ${PREFIX}/include/ladspamm1
	install ladspamm1/* ${PREFIX}/include/ladspamm1/

