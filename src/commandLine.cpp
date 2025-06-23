//
// Created by Gegel85 on 21/08/2019.
//

#include <string>
#include <future>
#include "commandLine.hpp"
#include "GameEngine/Pokemon.hpp"
#include "Networking/BgbHandler.hpp"
#include "GameEngine/BattleHandler.hpp"
#include "GameEngine/EmulatorGameHandle.hpp"

#ifndef _WIN32
#include <sys/select.h>
#include <algorithm>

typedef fd_set FD_SET;
#endif

using namespace PokemonGen1;

BattleAction getAction(const std::string &val)
{
	const static std::unordered_map<std::string, BattleAction> actions{
		{ "Attack1", Attack1 },
		{ "Attack2", Attack2 },
		{ "Attack3", Attack3 },
		{ "Attack4", Attack4 },
		{ "Switch1", Switch1 },
		{ "Switch2", Switch2 },
		{ "Switch3", Switch3 },
		{ "Switch4", Switch4 },
		{ "Switch5", Switch5 },
		{ "Switch6", Switch6 },
		{ "Struggle",StruggleMove },
		{ "Run",     Run }
	};
	auto it = actions.find(val);

	if (it != actions.end())
		return it->second;
	std::cout << "Invalid action " << val << std::endl;
	return NoAction;
}

Pokemon getPkmnFromCin(PokemonRandomGenerator &rng, const BattleLogger &logger, bool enemy)
{
	std::string arg;
	unsigned id = 256;

	std::cin >> arg;
	try {
		id = std::stoul(arg);
	} catch (...) {
		std::transform(arg.begin(), arg.end(), arg.begin(), [](char c) -> char { return std::toupper(c); });
		arg.erase(std::remove_if(arg.begin(), arg.end(), [](char c){ return !std::isalnum(c); }), arg.end());
		for (auto &[_, pkmn] : pokemonList) {
			std::string name = pkmn.name;

			arg.erase(std::remove_if(name.begin(), name.end(), [](char c){ return !std::isalnum(c); }), name.end());
			if (name == arg) {
				id = _;
				break;
			}
		}
	}
	if (id > 255)
		throw std::out_of_range("Invalid pokemon specie. Type `pokemons` for a list of available species.");

	std::cin >> arg;
	unsigned char level = std::stoul(arg);

	std::cin >> arg;
	unsigned nb = std::stoul(arg);
	std::vector<Move> moves;
	for (unsigned i = 0; i < nb; i++) {
		unsigned n;

		std::cin >> arg;
		try {
			n = std::stoul(arg);
		} catch (...) {
			std::transform(arg.begin(), arg.end(), arg.begin(), [](char c) -> char { return std::toupper(c); });
			arg.erase(std::remove_if(arg.begin(), arg.end(), [](char c){ return !std::isalnum(c); }), arg.end());
			for (auto &move : availableMoves) {
				std::string name = move.getName();

				arg.erase(std::remove_if(name.begin(), name.end(), [](char c){ return !std::isalnum(c); }), name.end());
				if (name == arg) {
					id = move.getID();
					break;
				}
			}
		}
		if (id > 256)
			throw std::out_of_range("Invalid move. Type `moves` for a list of available moves.");
		moves.push_back(availableMoves.at(n));
	}

	std::getline(std::cin, arg);
	while (!arg.empty() && arg[0] == ' ')
		arg = arg.substr(1);
	return { rng, logger, arg, level, pokemonList.at(id), moves, enemy };
}

bool handleCommand(const std::string &command, std::unique_ptr<EmulatorGameHandle> &emulator, BattleHandler &game, BattleState &state)
{
	std::string arg;

	if (command == "action") {
		std::cin >> arg;
		state.me.nextAction = getAction(arg);
		if (state.me.nextAction)
			std::cout << "Your next battle action will be " << arg << std::endl;
	} else if (command == "addPkmn") {
		if (state.me.team.size() >= 6) {
			std::cout << "Your team is already full" << std::endl;
			return true;
		}
		std::cin >> arg;
		try {
			state.me.team.push_back(getPkmnFromCin(state.rng, state.battleLogger, false));
			std::cout << "Added " << state.me.team.back().dump() << " to the team."<< std::endl;
		} catch (std::exception &e) {
			std::cout << e.what() << std::endl;
		}
	} else if (command == "setPkmn") {
		try {
			std::cin >> arg;
			unsigned index = std::stoul(arg);

			state.me.team.at(index) = getPkmnFromCin(state.rng, state.battleLogger, false);
			std::cout << "Changed pokemon " << index << " to " << state.me.team.at(index).dump() << std::endl;
		} catch (std::exception &e) {
			std::cout << e.what() << std::endl;
		}
	} else if (command == "ready") {
		if (emulator)
			emulator->setReady(true);
		std::cout << "You are now ready" << std::endl;
	} else if (command == "connect") {
		unsigned short port;

		std::cin >> arg;
		std::cin >> port;
		std::cout << "Connecting to " << arg << ":" << port << std::endl;
		try {
			emulator = std::make_unique<EmulatorGameHandle>(
				[arg, port](const ByteHandle &byteHandle, const LoopHandle &loopHandler) {
					return new BGBHandler(byteHandle, byteHandle, loopHandler, arg, port, getenv("MAX_DEBUG"));
				},
				state,
				false,
				getenv("MIN_DEBUG") != nullptr
			);
		} catch (std::exception &e) {
			std::cout << "Error connecting to " << arg << ":" << port << ": " << e.what() << std::endl;
		}
	} else if (command == "disconnect") {
		std::cout << "Disconnecting..." << std::endl;
		emulator.reset();
	} else if (command == "team") {
		std::cout << "You have " << state.me.team.size() << " pokemon(s) in your team" << std::endl;
		for (const auto &pkmn : state.me.team)
			std::cout << pkmn.dump() << std::endl;
	} else if (command == "quit") {
		std::cout << "Quitting..." << std::endl;
		return false;
	} else if (command == "help") {
		std::cout << "Here is a list of commands:" << std::endl;
		std::cout << "quit" << std::endl;
		std::cout << "team" << std::endl;
		std::cout << "disconnect" << std::endl;
		std::cout << "connect <ip> <port>" << std::endl;
		std::cout << "teamSize <newSize>" << std::endl;
		std::cout << "addPkmn <id> <level> <nbrOfMove> [<move1> <move2> ...] <nickname>" << std::endl;
		std::cout << "setPkmn <index> <id> <level> <nbrOfMove> [<move1> <move2> ...] <nickname>" << std::endl;
		std::cout << "action Attack1|Attack2|Attack3|Attack4|Switch1|Switch2|Switch3|Switch4|Switch5|Switch6|Run|Struggle" << std::endl;
		std::cout << "moves" << std::endl;
		std::cout << "pokemons" << std::endl;
		std::cout << "move <move_name>" << std::endl;
		std::cout << "pokemon <pokemon_name>" << std::endl;
		std::cout << "state" << std::endl;
	} else if (command == "moves") {
		for (auto &move : availableMoves)
			std::cout << static_cast<int>(move.getID()) << ": " << move.getName() << std::endl;
	} else if (command == "pokemons") {
		for (auto &[id, pkmn] : pokemonList)
			std::cout << static_cast<int>(pkmn.id) << ": " << pkmn.name << std::endl;
	} else if (command == "move") {
		std::string name;

		std::getline(std::cin, name);
		while (name[0] == ' ')
			name.erase(name.begin());
		while (!name.empty() && std::isspace(*name.end()))
			name.pop_back();
		std::cout << "Searching move '" << name << "'" << std::endl;
		for (auto &move : availableMoves)
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
		for (auto &[id, pkmn] : pokemonList)
			if (pkmn.name.find(name) != std::string::npos)
				std::cout << static_cast<int>(pkmn.id) << ": " << pkmn.name << std::endl;
	} else if (command == "state") {
		auto &state = game.getBattleState();

		std::cout << "P1 (" << state.me.name << ")" << std::endl;
		for (unsigned i = 0; i < 6; i++) {
			if (i < state.me.team.size())
				std::cout << state.me.team[i].dump();
			else
				std::cout << "--";
			if (state.me.pokemonOnField == i)
				std::cout << " (Active)";
			std::cout << std::endl;
		}

		std::cout << "P2 (" << state.op.name << ")" << std::endl;
		for (unsigned i = 0; i < 6; i++) {
			if (i < state.op.team.size())
				std::cout << state.op.team[i].dump();
			else
				std::cout << "--";
			if (state.op.pokemonOnField == i)
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
	std::unique_ptr<EmulatorGameHandle> emulator;
	BattleHandler battle{false, getenv("MIN_DEBUG") != nullptr};
	auto &state = battle.getBattleState();

	state.rng.makeRandomList(9);
	state.battleLogger = [](const std::string &){};
	state.me.team.emplace_back(
		state.rng, state.battleLogger, "", 100,
		pokemonList.at(Rhydon),
		std::vector<Move>{
			availableMoves[Tackle],
			availableMoves[Tail_Whip]
		}
	);
	std::cout << "Type help for help" << std::endl;
	while (loop) {
		future = std::async(getAnswer);
		do {
			if (emulator && emulator->getStage() == EmulatorGameHandle::BATTLE)
				emulator->setReady(false);
		} while (future.wait_for(std::chrono::seconds(1)) != std::future_status::ready);
		loop = handleCommand(future.get(), emulator, battle, state);
	}
}