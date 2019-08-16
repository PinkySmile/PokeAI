#include <iostream>
#include <csignal>
#include "Pokemon.hpp"
#include "BgbHandler.hpp"
#include "GameHandle.hpp"

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
		return PokemonGen1::Attack1;
	});

	unsigned long port = std::stoul(argv[2]);

	if (port > 65535)
		throw std::out_of_range("Too big value");

	handler.addPokemonToTeam(
		"",
		100,
		PokemonGen1::pokemonList[0x00],
		std::vector<PokemonGen1::Move>{
			PokemonGen1::availableMoves[PokemonGen1::Splash],
			PokemonGen1::availableMoves[PokemonGen1::Earthquake]
		}
	);
	handler.connect(argv[1], port);
	while (handler.isConnected()) {
		unsigned id;
		std::string str;

		std::cin >> str;
		try {
			if (str.substr(0, 2) == "0x")
				id = std::stol(str.substr(2), nullptr, 16);
			else
				id = std::stol(str);
		} catch (std::exception &) {
			continue;
		}
		if (id > 255)
			continue;
		std::cout << "Changing pokemon to " << PokemonGen1::pokemonList[id].name << " (ID: " << id << ")" << std::endl;
		handler.changePokemon(
			0,
			"",
			100,
			PokemonGen1::pokemonList[id],
			std::vector<PokemonGen1::Move>{
				PokemonGen1::availableMoves[PokemonGen1::Splash],
				PokemonGen1::availableMoves[PokemonGen1::Aurora_Beam]
			}
		);
	}

	return EXIT_SUCCESS;
}
