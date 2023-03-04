PREFIX ?= /usr/local
CXXFLAGS ?= -O2 -g -pedantic -Wall -Werror -std=c++11 -Ibuild/include ${ADDITIONAL_CXXFLAGS} `pkg-config ladspamm1 --cflags`  -lprotobuf -ljack
LDFLAGS ?= `pkg-config ladspamm1 --libs` -lboost_timer -Lbuild/lib
VALGRIND_FLAGS ?= --leak-check=full  --show-leak-kinds=all

PROTO_HEADERS = proto/ladspam1.pb.h
PROTO_SOURCES = proto/ladspam1.pb.cc
PROTO_FILES  = ${PROTO_HEADERS} ${PROTO_SOURCES}

JACK_CLIENT_DEPS = ${PROTO_FILES}

all: library jack-clients plugins

jack-clients: build/bin/ladspa.m1.jack.synth build/bin/ladspa.m1.jack.instrument 

library: build/lib/libladspa.m1.so

test: tests/ladspa.m.synth.test plugins.ladspa
	LADSPA_PATH="" valgrind ${VALGRIND_FLAGS} ./tests/ladspa.m.synth.test

build/bin/ladspa.m1.jack.%: ${PROTO_FILES} build/lib/libladspa.m1.so ladspam-jack0/%.cc jack-tools/%.cc makefile
	g++ ${CXXFLAGS} -o $@ ladspam-jack0/synth.cc ${LDFLAGS} build/lib/libladspa.m1.so proto/ladspam1.pb.cc jack-tools/synth.cc

build/lib/libladspa.m1.so: build ${PROTO_FILES} ladspam-jack0/synth.cc ladspam-jack0/instrument.cc
	g++ ${CXXFLAGS} -shared -o $@ ladspam-jack0/synth.cc ladspam-jack0/instrument.cc ${PROTO_SOURCES}

build/share/ladspa.m1/ladspam_pb2.py: build
	protoc -I proto --python_out=proto ladspam1.proto

proto/ladspam1.pb.h: build
	protoc -I proto --cpp_out=proto ladspam1.proto
	install proto/ladspam1.pb.h build/include/ladspa.m1

proto/ladspam1.pb.cc: proto/ladspam1.pb.h
	mv -f build/include/ladspa.m1/ladspam1.pb.cc proto

plugins: plugins.ladspa plugins.lv2 plugins.dssi

plugins.lv2:

plugins.ladspa: build/lib/ladspa/ladspa.m.env.plugins.so build/lib/ladspa/ladspa.m.osc.plugins.so

plugins.dssi: build/lib/dssi/ladspa.m.dssi.instrument.so

build/lib/ladspa/%.so: plugins/ladspa/%.cc
	g++ ${CXXFLAGS} -shared -o $@ -shared $<

build/lib/dssi/%.so: plugins/dssi/%.cc
	g++ ${CXXFLAGS} -shared -o $@ -shared $<

.PHONY: install 
build: ladspa.m1/*.h proto/ladspam1.proto
	install -d build/include/ladspa.m1
	install ladspa.m1/*.h build/include/ladspa.m1
	install -d build/share/ladspa.m1
	install -d build/lib/ladspa
	install -d build/lib/dssi
	install -d build/lib/lv2
	install -d build/bin
	touch build

install:
	install -d ${PREFIX}/include/ladspamm1
	install ladspamm1/* ${PREFIX}/include/ladspamm1/

