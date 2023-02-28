#include <ladspam-jack-0/instrument.h>
#include <ladspam1.pb.h>

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cout << "Missing filename parameter" << std::endl;
		return EXIT_FAILURE;
	}
	
	ladspam_proto1::Instrument instrument_pb;
	std::ifstream input_file(argv[1], std::ios::in | std::ios::binary);
	
	if (false == input_file.good())
	{
		std::cout << "Failed to open input stream" << std::endl;
		return EXIT_FAILURE;
	}
		
	
	if (false == instrument_pb.ParseFromIstream(&input_file))
	{
		std::cout << "Failed to parse instrument definition file" << std::endl;
		return EXIT_FAILURE;
	}
	
	ladspam_jack::instrument instrument("synth", instrument_pb, 32);
	
	std::cout << "type anything and press enter to quit..." << std::endl;
	std::string line;
	std::cin >> line;
}

