PREFIX ?= /usr/local

INSTALL ?= install
SED ?= sed

INCLUDE_PATH = $(PREFIX)/include/ladspam-0
PKGCONFIG_DIR ?= $(PREFIX)/lib/pkgconfig

CXXFLAGS ?= -O3 -march=native -DNDEBUG  -mfpmath=sse -Wall

.PHONY: install all clean docs

all: ladspam-0-test-synth ladspam-0-plugins.so

LADSPAM_HEADERS = ladspam-0/synth.h 

install: all
	$(INSTALL) -d $(PKGCONFIG_DIR)
	$(INSTALL) ladspam-0.pc $(PKGCONFIG_DIR)/
	$(SED) -i -e s@PREFIX@$(PREFIX)@g $(PKGCONFIG_DIR)/ladspam-0.pc 
	$(INSTALL) -d $(INCLUDE_PATH)
	$(INSTALL) $(LADSPAM_HEADERS) $(INCLUDE_PATH)

ladspam-0-test-synth: test_synth.cc ladspam-0/synth.h
	g++ $(CXXFLAGS) -I .  -ansi -Wall -o ladspam-0-test-synth  test_synth.cc  `pkg-config ladspamm-0 --cflags --libs` -lboost_system -lboost_timer

ladspam-0-plugins.so: ladspam-plugins.cc
	g++ $(CXXFLAGS) -std=c++11 -shared -fPIC -o ladspam-0-plugins.so ladspam-plugins.cc

docs:
	doxygen

clean:
	rm -f ladspam-0-test-synth
	
