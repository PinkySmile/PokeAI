//
// Created by Gegel85 on 21/08/2019.
//

#include <string>
#include <future>
#include "commandLine.hpp"
#include "Pokemon.hpp"
#include "BgbHandler.hpp"
#include "GameHandle.hpp"

#ifndef _WIN32
#include <sys/select.h>
typedef fd_set FD_SET;
#endif

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
	if (val == "Run")
		return PokemonGen1::Run;
	std::cout << "Invalid action " << val << std::endl;
	return PokemonGen1::NoAction;
}

bool handleCommand(const std::string &command, PokemonGen1::GameHandle &game, PokemonGen1::BattleAction &action)
{
	std::string arg;

	if (command == "action") {
		std::cin >> arg;
		action = getAction(arg);
		if (action)
			std::cout << "Your next battle action will be " << arg << std::endl;
	} else if (command == "teamSize") {
		std::cin >> arg;
		try {
			game.setTeamSize(std::stoul(arg));
			std::cout << "Changed team size to " << arg << " pokemon(s)"<< std::endl;
		} catch (std::exception &e) {
			std::cout << e.what() << std::endl;
		}
	} else if (command == "setPkmn") {
		std::cin >> arg;
		try {
			unsigned index = std::stoul(arg);

			std::cin >> arg;
			unsigned id = std::stoul(arg);

			std::cin >> arg;
			unsigned level = std::stoul(arg);

			std::cin >> arg;
			unsigned nb = std::stoul(arg);
			std::vector<PokemonGen1::Move> moves;
			for (unsigned i = 0; i < nb; i++) {
				std::cin >> arg;
				unsigned n = std::stoul(arg);
				moves.push_back(PokemonGen1::availableMoves.at(n));
			}

			std::string name;
			std::getline(std::cin, name);

			while (!name.empty() && name[0] == ' ')
				name = name.substr(1);

			game.changePokemon(
				index,
				name,
				level,
				PokemonGen1::pokemonList.at(id),
				moves
			);
			std::cout << "Changed pokemon " << index << " to " << game.getPokemonTeam()[index].dump() << std::endl;
		} catch (std::exception &e) {
			std::cout << e.what() << std::endl;
		}
	} else if (command == "ready") {
		game.setReady(true);
		std::cout << "You are now ready" << std::endl;
	} else if (command == "connect") {
		unsigned short port;

		std::cout << "Ip: ";
		std::cin >> arg;
		std::cout << "Port: ";
		std::cin >> port;
		std::cout << "Connecting to " << arg << ":" << port << std::endl;
		try {
			game.connect(arg, port);
		} catch (std::exception &e) {
			std::cout << "Error connecting to " << arg << ":" << port << ": " << e.what() << std::endl;
		}
	} else if (command == "disconnect") {
		std::cout << "Disconnecting..." << std::endl;
		game.disconnect();
	} else if (command == "team") {
		std::cout << "You have " << game.getPokemonTeam().size() << " pokemon(s) in your team" << std::endl;
		for (const auto &pkmn : game.getPokemonTeam())
			std::cout << pkmn.dump() << std::endl;
	} else if (command == "quit") {
		std::cout << "Quitting..." << std::endl;
		game.disconnect();
		return false;
	} else if (command == "help") {
		std::cout << "Here is a list of commands:" << std::endl;
		std::cout << "quit" << std::endl;
		std::cout << "team" << std::endl;
		std::cout << "disconnect" << std::endl;
		std::cout << "connect <ip> <port>" << std::endl;
		std::cout << "teamSize <newSize>" << std::endl;
		std::cout << "setPkmn <index> <id> <level> <nbrOfMove> [<move1> <move2> ...] <nickname>" << std::endl;
		std::cout << "teamSize <newSize>" << std::endl;
		std::cout << "action Attack1|Attack2|Attack3|Attack4|Switch1|Switch2|Switch3|Switch4|Switch5|Switch6|Run|Struggle" << std::endl;
		std::cout << "moves" << std::endl;
		std::cout << "pokemons" << std::endl;
		std::cout << "move <move_name>" << std::endl;
		std::cout << "pokemon <pokemon_name>" << std::endl;
		std::cout << "state" << std::endl;
	} else if (command == "moves") {
		for (auto &move : PokemonGen1::availableMoves)
			std::cout << static_cast<int>(move.getID()) << ": " << move.getName() << std::endl;
	} else if (command == "pokemons") {
		for (auto &pkmn : PokemonGen1::pokemonList)
			std::cout << static_cast<int>(pkmn.id) << ": " << pkmn.name << std::endl;
	} else if (command == "move") {
		std::string name;

		std::getline(std::cin, name);
		while (name[0] == ' ')
			name.erase(name.begin());
		while (!name.empty() && std::isspace(*name.end()))
			name.pop_back();
		std::cout << "Searching move '" << name << "'" << std::endl;
		for (auto &move : PokemonGen1::availableMoves)
			if (move.getName().find(name) != std::string::npos)
				std::cout << static_cast<int>(move.getID()) << ": " << move.getName() << std::endl;
	} else if (command == "pokemon") {
		std::string name;

		std::getline(std::cin, name);
		while (!name.empty() && std::isspace(name[0]))
			name.erase(name.begin());
		while (!name.empty() && std::isspace(*name.end()))
			name.pop_back();
		std::cout << "Searching pokemon '" << name << "'" << std::endl;
		for (auto &pkmn : PokemonGen1::pokemonList)
			if (pkmn.name.find(name) != std::string::npos)
				std::cout << static_cast<int>(pkmn.id) << ": " << pkmn.name << std::endl;
	} else if (command == "state") {
		auto &state = game.getBattleState();

		std::cout << "Me" << std::endl;
		for (unsigned i = 0; i < 6; i++) {
			if (i < state.team.size())
				std::cout << state.team[i].dump();
			else
				std::cout << "--";
			if (state.pokemonOnField == i)
				std::cout << " (Active)";
			std::cout << std::endl;
		}
		std::cout << "Opponent (" << state.opponentName << ")" << std::endl;
		for (unsigned i = 0; i < 6; i++) {
			if (i < state.opponentTeam.size())
				std::cout << state.opponentTeam[i].dump();
			else
				std::cout << "--";
			if (state.opponentPokemonOnField == i)
				std::cout << " (Active)";
			std::cout << std::endl;
		}
	} else
		std::cout << "Invalid command \"" << command << "\"" << std::endl;
	return true;
}

std::string getAnswer()
{
	std::string answer;

	std::cout << "> ";
	if (std::cin.eof()) {
		std::cout << "quit" << std::endl;
		return "quit";
	}
	std::cin >> answer;
	if (std::cin.eof() && answer.empty()) {
		std::cout << "quit" << std::endl;
		return "quit";
	}
	return answer;
}

void commandLine(const std::string &trainerName)
{
	bool loop = true;
	std::future<std::string> future;
	PokemonGen1::BattleAction nextAction = PokemonGen1::NoAction;
	PokemonGen1::GameHandle handler(
		[](const ByteHandle &byteHandle, const LoopHandle &loopHandler, const std::string &ip, unsigned short port)
		{
			return new BGBHandler(byteHandle, byteHandle, loopHandler, ip, port, getenv("MAX_DEBUG"));
		},
		[&nextAction](PokemonGen1::GameHandle &handle) {
			PokemonGen1::BattleAction action = nextAction;
			const PokemonGen1::BattleState &state = handle.getBattleState();

			if (!state.opponentTeam[state.opponentPokemonOnField].getHealth())
				return static_cast<PokemonGen1::BattleAction>(PokemonGen1::Switch1 + state.pokemonOnField);

			if (action)
				nextAction = PokemonGen1::NoAction;
			return action;
		},
		trainerName,
		nullptr,
		false,
		getenv("MIN_DEBUG")
	);

	handler.addPokemonToTeam(
		"",
		100,
		PokemonGen1::pokemonList[PokemonGen1::Rhydon],
		std::vector<PokemonGen1::Move>{
			PokemonGen1::availableMoves[PokemonGen1::Tackle],
			PokemonGen1::availableMoves[PokemonGen1::Tail_Whip]
		}
	);
	std::cout << "Type help for help" << std::endl;
	handler.setReady(false);
	while (loop) {
		future = std::async(getAnswer);
		do {
			if (handler.getStage() == PokemonGen1::BATTLE)
				handler.setReady(false);
		} while (future.wait_for(std::chrono::seconds(1)) != std::future_status::ready);

		loop = handleCommand(future.get(), handler, nextAction);
	}
}