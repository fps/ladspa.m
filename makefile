PREFIX ?= /usr/local
CXXFLAGS ?= -O2 -g -pedantic -Wall -Werror -std=c++11 -Ibuild/include ${ADDITIONAL_CXXFLAGS} `pkg-config ladspamm1 --cflags`  -lprotobuf -ljack
LDFLAGS ?= `pkg-config ladspamm1 --libs` -lboost_timer -Lbuild/lib
VALGRIND_FLAGS ?= --leak-check=full  --show-leak-kinds=all

all: build/bin/ladspa.m1.jack.synth build/bin/ladspa.m1.jack.instrument

test: tests/ladspa.m.synth.test plugins.ladspa
	LADSPA_PATH="" valgrind ${VALGRIND_FLAGS} ./tests/ladspa.m.synth.test

build/bin/ladspa.m1.jack.synth: build build/include/ladspa.m1/ladspam1.pb.h build/lib/libladspa.m1.so ladspam-jack0/synth.cc proto/ladspam.pb.cc jack-tools/synth.cc
	g++ ${CXXFLAGS} -o $@ ladspam-jack0/synth.cc ${LDFLAGS} build/lib/libladspa.m1.so proto/ladspam.pb.cc jack-tools/synth.cc

build/bin/ladspa.m1.jack.instrument: build build/include/ladspa.m1/ladspam1.pb.h build/lib/libladspa.m1.so ladspam-jack0/instrument.cc proto/ladspam.pb.cc jack-tools/instrument.cc
	g++ ${CXXFLAGS} -o $@ ladspam-jack0/instrument.cc ${LDFLAGS} build/lib/libladspa.m1.so proto/ladspam.pb.cc jack-tools/instrument.cc

build/lib/libladspa.m1.so: ladspam-jack0/synth.cc ladspam-jack0/instrument.cc
	g++ ${CXXFLAGS} -shared -o $@ ladspam-jack0/synth.cc ladspam-jack0/instrument.cc

build/share/ladspa.m1/ladspam_pb2.py: build
	protoc -I proto --python_out=proto ladspam1.proto

build/include/ladspa.m1/ladspam1.pb.h: build
	protoc -I proto --cpp_out=build/include/ladspa.m1 ladspam1.proto

plugins: plugins.ladspa plugins.lv2

plugins.lv2:

plugins.ladspa: plugins/ladspa/ladspa.m.dssi.instrument.so plugins/ladspa/ladspa.m.env.plugins.so plugins/ladspa/ladspa.m.osc.plugins.so

%.so: %.cc
	g++ ${CXXFLAGS} -shared -o $@ -shared $<

.PHONY: install 
build: ladspa.m1/*.h proto/ladspam1.proto
	install -d build/include/ladspa.m1
	install ladspa.m1/*.h build/include/ladspa.m1
	install -d build/share/ladspa.m1
	install -d build/lib/ladspa
	install -d build/lib/lv2
	install -d build/bin

install:
	install -d ${PREFIX}/include/ladspamm1
	install ladspamm1/* ${PREFIX}/include/ladspamm1/

