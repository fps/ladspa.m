PREFIX ?= /usr/local

INSTALL ?= install
SED ?= sed

INCLUDE_PATH = $(PREFIX)/include/ladspam-0
PKGCONFIG_DIR ?= $(PREFIX)/lib/pkgconfig

.PHONY: install all clean

all: libladspam-0.so ladspam-0-test

libladspam-0.so: ladspam-0/m_jack.cc ladspam-0/m_jack.h
	g++ -O3 -I . -fPIC -shared -o libladspam-0.so ladspam-0/m_jack.cc `pkg-config jack ladspamm-0 --cflags --libs`

install: all
	$(INSTALL) -d $(PKGCONFIG_DIR)
	$(INSTALL) ladspam-0.pc $(PKGCONFIG_DIR)/
	$(SED) -i -e s@PREFIX@$(PREFIX)@g $(PKGCONFIG_DIR)/ladspam-0.pc 
	$(INSTALL) -d $(INCLUDE_PATH)
	$(INSTALL) ladspam-0/*.h $(INCLUDE_PATH)
	$(INSTALL) libladspam-0.so $(PREFIX)/lib/

ladspam-0-test: test_ladspam.cc libladspam-0.so
	g++ -I .  -ansi -Wall -g -O0 -o ladspam-0-test  test_ladspam.cc -L . -lladspam-0 -Wl,-rpath,.

docs:
	doxygen

clean:
	rm -f ladspam-0-test libladspam-0.so
	
