PREFIX ?= /usr/local

INSTALL ?= install
SED ?= sed

INCLUDE_PATH = $(PREFIX)/include/ladspam-0
PKGCONFIG_DIR ?= $(PREFIX)/lib/pkgconfig

OPTIMIZATION_FLAGS = -O3 -march=native -g

.PHONY: install all clean

all: libladspam-0.so ladspam-0-test

LADSPAM_HEADERS = ladspam-0/synth.h 
LADSPAM_PRIVATE_HEADERS = ladspam-0/private/ringbuffer.h
LADSPAM_JACK_HEADERS = ladspam-0/jack/synth.h ladspam-0/jack/instrument.h

LADSPAM_SOURCES = ladspam-0/jack/synth.cc 

libladspam-0.so: $(LADSPAM_HEADERS) $(LADSPAM_SOURCES)
	g++ $(OPTIMIZATION_FLAGS) -I . -fPIC -shared -o libladspam-0.so $(LADSPAM_SOURCES) `pkg-config jack ladspamm-0 --cflags --libs`

install: all
	$(INSTALL) -d $(PKGCONFIG_DIR)
	$(INSTALL) ladspam-0.pc $(PKGCONFIG_DIR)/
	$(SED) -i -e s@PREFIX@$(PREFIX)@g $(PKGCONFIG_DIR)/ladspam-0.pc 
	$(INSTALL) -d $(INCLUDE_PATH)
	$(INSTALL) -d $(INCLUDE_PATH)/jack
	$(INSTALL) -d $(INCLUDE_PATH)/private
	$(INSTALL) $(LADSPAM_HEADERS) $(INCLUDE_PATH)
	$(INSTALL) $(LADSPAM_JACK_HEADERS) $(INCLUDE_PATH)/jack
	$(INSTALL) $(LADSPAM_PRIVATE_HEADERS) $(INCLUDE_PATH)/private
	$(INSTALL) libladspam-0.so $(PREFIX)/lib/

ladspam-0-test: test_ladspam.cc libladspam-0.so
	g++ $(OPTIMIZATION_FLAGS) -I .  -ansi -Wall -o ladspam-0-test  test_ladspam.cc -L . -lladspam-0 -Wl,-rpath,.

docs:
	doxygen

clean:
	rm -f ladspam-0-test libladspam-0.so
	
