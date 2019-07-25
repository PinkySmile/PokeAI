#include <iostream>
#include <csignal>
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
	handler.addPokemonToTeam(
		"",
		100,
		/* Pokemon::pokemonList[Pokemon::Mewtwo] */
		Pokemon::PokemonBase{ 0x00, "TestPkmn", 0, 0, 32718, 0, 32718, TYPE_NORMAL, TYPE_ELECTRIC, 0, 0 },
		std::vector<Pokemon::Move>{
			Pokemon::availableMoves[Pokemon::Splash],
			Pokemon::availableMoves[Pokemon::Aurora_Beam]
		}
	);
	handler.connect(argv[1], port);
	signal(SIGINT, nullptr);

	Pokemon::displayPacket(values);
	return 0;
}
