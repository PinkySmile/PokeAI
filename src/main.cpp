#include <iostream>
#include <csignal>
#include "Pokemon.hpp"
#include "BgbHandler.hpp"
#include "GameHandle.hpp"

#ifndef _WIN32
#include <sys/select.h>
typedef fd_set FD_SET;
#endif

#include <unistd.h>

PokemonGen1::BattleAction getAction(const std::string &val)
{
	if (val == "Attack1")
		return PokemonGen1::Attack1;
	if (val == "Attack2")
		return PokemonGen1::Attack2;
	if (val == "Attack3")
		return PokemonGen1::Attack3;
	if (val == "Attack4")
		return PokemonGen1::Attack4;
	if (val == "Switch1")
		return PokemonGen1::Switch1;
	if (val == "Switch2")
		return PokemonGen1::Switch2;
	if (val == "Switch3")
		return PokemonGen1::Switch3;
	if (val == "Switch4")
		return PokemonGen1::Switch4;
	if (val == "Switch5")
		return PokemonGen1::Switch5;
	if (val == "Switch6")
		return PokemonGen1::Switch6;
	if (val == "Struggle")
		return PokemonGen1::StruggleMove;
	if (val == "Run" || std::cin.eof())
		return PokemonGen1::Run;
	if (val != "None")
		std::cout << "Invalid action " << val << std::endl;
	return PokemonGen1::NoAction;

}

void handleCommand(const std::string &command, PokemonGen1::GameHandle &game, PokemonGen1::BattleAction &action)
{
	std::string arg;

	if (command == "setbattle") {
		std::cin >> arg;
		action = getAction(arg);
		std::cout << "Your next battle action will be " << arg << std::endl;
	} else
		std::cout << "Invalid command \"" << command << "\"" << std::endl;;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
		return EXIT_FAILURE;
	}

	PokemonGen1::BattleAction nextAction = PokemonGen1::NoAction;

	PokemonGen1::GameHandle handler([](const ByteHandle &byteHandle, const LoopHandle &loopHandler, const std::string &ip, unsigned short port)
	{
		return new BGBHandler(byteHandle, byteHandle, loopHandler, ip, port);
	},
	[&nextAction](PokemonGen1::GameHandle &) {
		PokemonGen1::BattleAction action = nextAction;

		if (action)
			nextAction = PokemonGen1::NoAction;
		return action;
	});

	unsigned long port = std::stoul(argv[2]);

	if (port > 65535)
		throw std::out_of_range("Too big value");

	handler.addPokemonToTeam(
		"",
		100,
		PokemonGen1::pokemonList[0xEB],
		std::vector<PokemonGen1::Move>{
			PokemonGen1::availableMoves[PokemonGen1::Thunder],
			PokemonGen1::availableMoves[PokemonGen1::Earthquake]
		}
	);
	handler.connect(argv[1], port);
	while (handler.isConnected()) {
		std::string str;

		std::cin >> str;
		handleCommand(str, handler, nextAction);
	}

	return EXIT_SUCCESS;
}
