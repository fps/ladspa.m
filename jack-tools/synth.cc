#include <ladspam-jack-0/synth.h>
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
	
	ladspam_proto1::Synth synth_pb;
	std::ifstream input_file(argv[1], std::ios::in | std::ios::binary);
	
	if (false == input_file.good())
	{
		std::cout << "Failed to open input stream" << std::endl;
		return EXIT_FAILURE;
	}
		
	
	if (false == synth_pb.ParseFromIstream(&input_file))
	{
		std::cout << "Failed to parse synth definition file" << std::endl;
		return EXIT_FAILURE;
	}
	
	ladspam_jack::synth synth("synth", synth_pb, -1);
	
	std::string line;
	std::cin >> line;
}

