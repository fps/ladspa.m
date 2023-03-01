PREFIX ?= /usr/local
CXXFLAGS ?= -O2 -g -pedantic -Wall -Werror -std=c++11 -I. ${ADDITIONAL_CXXFLAGS} `pkg-config ladspamm1 --cflags` 
LDFLAGS ?= `pkg-config ladspamm1 --libs` -lboost_timer
VALGRIND_FLAGS ?= --leak-check=full  --show-leak-kinds=all

all: jack-clients library

test: tests/ladspa.m.synth.test plugins.ladspa
	LADSPA_PATH="" valgrind ${VALGRIND_FLAGS} ./tests/ladspa.m.synth.test

jack-tools: jack-tools/synth

jack-tools/synth: ladspa.m1/ladspam_pb2.py ladspa.m1/ladspam1.pb.h

ladspa.m1/ladspam_pb2.py:
	protoc -I ladspa.m1/ --python_out=ladspa.m1/ ladspam1.proto

ladspa.m1/ladspam1.pb.h:
	protoc -I ladspa.m1/ --cpp_out=ladspa.m1/ ladspam1.proto

plugins: plugins.ladspa plugins.lv2

plugins.lv2:

plugins.ladspa: plugins/ladspa/ladspa.m.dssi.instrument.so plugins/ladspa/ladspa.m.env.plugins.so plugins/ladspa/ladspa.m.osc.plugins.so

%.so: %.cc
	g++ ${CXXFLAGS} -shared -o $@ -shared $<

install:
	install -d ${PREFIX}/include/ladspamm1
	install ladspamm1/* ${PREFIX}/include/ladspamm1/

