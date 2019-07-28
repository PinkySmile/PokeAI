#include <iostream>
#include <csignal>
#include "Pokemon.hpp"
#include "BgbHandler.hpp"
#include "GameHandle.hpp"

void sigHandler(int);

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
		return EXIT_FAILURE;
	}

	PokemonGen1::GameHandle handler([](const ByteHandle &byteHandle, const LoopHandle &loopHandler, const std::string &ip, unsigned short port)
	{
		return new BGBHandler(byteHandle, byteHandle, loopHandler, ip, port);
	},
	[](PokemonGen1::GameHandle &) {
		std::vector<PokemonGen1::BattleAction> actions = {
			PokemonGen1::Attack1,
			PokemonGen1::Attack2,
			PokemonGen1::Attack3,
			PokemonGen1::Attack4
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
		PokemonGen1::pokemonList[PokemonGen1::Mewtwo],
		std::vector<PokemonGen1::Move>{
			PokemonGen1::availableMoves[PokemonGen1::Splash],
			PokemonGen1::availableMoves[PokemonGen1::Aurora_Beam]
		}
	);
	handler.connect(argv[1], port);
	signal(SIGINT, nullptr);
	return 0;
}
