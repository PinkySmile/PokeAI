#include <iostream>
#include "Pokemon.hpp"
#include "BgbHandler.hpp"
#include "PkmnGen1Handle.hpp"

std::vector<unsigned char> values;
void sigHandler(int);

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
		return EXIT_FAILURE;
	}

	Pokemon::PkmnGen1Handle handler([](const ByteHandle &byteHandle, const std::string &ip, unsigned short port)
	{
		return new BGBHandler(byteHandle, byteHandle, ip, port);
	},
	[](Pokemon::PkmnGen1Handle &) {
		std::vector<Pokemon::BattleAction> actions = {
			Pokemon::Attack1,
			Pokemon::Attack2,
			Pokemon::Attack3,
			Pokemon::Attack4
		};

		return actions[0];
	});

	unsigned long port = std::stoul(argv[2]);

	if (port > 65535)
		throw std::out_of_range("Too big value");

	signal(SIGINT, sigHandler);
	handler.addPokemonToTeam("", 55, Pokemon::pokemonList[Pokemon::Mewtwo], std::vector<Pokemon::Move>{
		Pokemon::availableMoves[Pokemon::Struggle],
		Pokemon::availableMoves[Pokemon::Aurora_Beam]
	});
	handler.connect(argv[1], port);
	signal(SIGINT, nullptr);

	for (unsigned int i = 0; i < values.size(); i += 20) {
		for (unsigned j = 0; j < 20 && j + i < values.size(); j++)
			printf("%02X ", values[j + i]);
		for (int j = 0; j < static_cast<int>(i - values.size() + 20); j++)
			printf("   ");
		for (unsigned j = 0; j < 20 && j + i < values.size(); j++)
			printf("%c", isprint(values[j + i]) ? values[j + i] : '.');
		for (int j = 0; j < static_cast<int>(i - values.size() + 20); j++)
			printf(" ");
		printf(" ");
		for (unsigned j = 0; j < 20 && j + i < values.size(); j++)
			printf("%c", isprint(Pokemon::Pkmn1CharToASCIIConversionTable[values[j + i]]) ? Pokemon::Pkmn1CharToASCIIConversionTable[values[j + i]] : '.');
		printf("\n");
	}
	return 0;
}