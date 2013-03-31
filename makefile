.PHONY: all

all:
	g++ -Wall test_ladspam.cc -I . `pkg-config jack ladspamm-0 --cflags --libs` ladspam-0/m_jack.cc
