#include <iostream>
#include <cstring>
#include "commandLine.hpp"
#include "gui.hpp"

int main(int argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "-h") == 0) {
		std::cerr << "Usage: " << argv[0] << " [<mode>, <trainerName>]" << std::endl;
		return EXIT_FAILURE;
	}

	if (argc == 1 || strcmp(argv[1], "commandLine") == 0)
		commandLine(argc > 2 ? argv[2] : "PokeAI");
	else if (strcmp(argv[1], "gui") == 0)
		gui(argc > 2 ? argv[2] : "PokeAI");
	else
		std::cout << "Invalid mode '" << argv[1] << "'. Valid modes are 'commandLine'" << std::endl;
	return EXIT_SUCCESS;
}
