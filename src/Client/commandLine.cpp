//
// Created by PinkySmile on 21/08/2019.
//

#include <algorithm>
#include <chrono>
#include <cstring>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "GameEngine/Gen1/BattleHandler.hpp"
#include "GameEngine/Gen1/Pokemon.hpp"
#include "GameEngine/Gen1/State.hpp"
#include "Emulator/BgbHandler.hpp"
#include "Emulator/EmulatorGameHandle.hpp"

using namespace PokemonGen1;

static const std::unordered_map<std::string, BattleAction> actionNames{
	{ "Attack1",  Attack1 },
	{ "Attack2",  Attack2 },
	{ "Attack3",  Attack3 },
	{ "Attack4",  Attack4 },
	{ "Switch1",  Switch1 },
	{ "Switch2",  Switch2 },
	{ "Switch3",  Switch3 },
	{ "Switch4",  Switch4 },
	{ "Switch5",  Switch5 },
	{ "Switch6",  Switch6 },
	{ "Struggle", StruggleMove },
	{ "Run",      Run },
};

// Strip every non-alphanumeric character and upper-case the rest, so that
// user input like "Tail Whip" or "tail-whip" compares equal to "TAILWHIP".
static std::string normalizeName(std::string str)
{
	str.erase(
		std::remove_if(str.begin(), str.end(), [](char c) { return !std::isalnum(static_cast<unsigned char>(c)); }),
		str.end()
	);
	std::transform(str.begin(), str.end(), str.begin(), [](char c) {
		return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
	});
	return str;
}

static std::string trim(std::string str)
{
	while (!str.empty() && std::isspace(static_cast<unsigned char>(str.front())))
		str.erase(str.begin());
	while (!str.empty() && std::isspace(static_cast<unsigned char>(str.back())))
		str.pop_back();
	return str;
}

static BattleAction getAction(const std::string &val)
{
	auto it = actionNames.find(val);

	if (it != actionNames.end())
		return it->second;
	std::cout << "Invalid action " << val << std::endl;
	return EmptyAction;
}

static unsigned findPokemonId(const std::string &input)
{
	try {
		unsigned long parsed = std::stoul(input);

		if (parsed <= 255 && pokemonList.count(static_cast<unsigned char>(parsed)))
			return static_cast<unsigned>(parsed);
	} catch (...) {}

	std::string target = normalizeName(input);

	for (const auto &[id, pkmn] : pokemonList)
		if (normalizeName(pkmn.name) == target)
			return id;
	return 256;
}

static unsigned findMoveId(const std::string &input)
{
	try {
		unsigned long parsed = std::stoul(input);

		if (parsed < availableMoves.size())
			return static_cast<unsigned>(parsed);
	} catch (...) {}

	std::string target = normalizeName(input);

	for (const auto &move : availableMoves)
		if (normalizeName(move.getName()) == target)
			return move.getID();
	return availableMoves.size();
}

static Pokemon getPkmnFromCin(BattleState &state, bool enemy)
{
	std::string arg;

	std::cin >> arg;
	unsigned id = findPokemonId(arg);
	if (id > 255)
		throw std::out_of_range("Invalid pokemon specie. Type `pokemons` for a list of available species.");

	std::cin >> arg;
	unsigned char level = static_cast<unsigned char>(std::stoul(arg));

	std::cin >> arg;
	unsigned nb = std::stoul(arg);
	std::vector<Move> moves;

	moves.reserve(nb);
	for (unsigned i = 0; i < nb; i++) {
		std::cin >> arg;

		unsigned moveId = findMoveId(arg);

		if (moveId >= availableMoves.size())
			throw std::out_of_range("Invalid move. Type `moves` for a list of available moves.");
		moves.push_back(availableMoves.at(moveId));
	}

	std::getline(std::cin, arg);
	arg = trim(arg);
	return { state, arg, level, pokemonList.at(id), moves, enemy };
}

static void printHelp()
{
	std::cout <<
		"Here is a list of commands:\n"
		"  quit\n"
		"  team\n"
		"  state\n"
		"  ready\n"
		"  disconnect\n"
		"  connect <ip> <port>\n"
		"  addPkmn <id> <level> <nbrOfMove> [<move1> <move2> ...] <nickname>\n"
		"  setPkmn <index> <id> <level> <nbrOfMove> [<move1> <move2> ...] <nickname>\n"
		"  action Attack1|Attack2|Attack3|Attack4|Switch1|Switch2|Switch3|Switch4|Switch5|Switch6|Run|Struggle\n"
		"  moves\n"
		"  pokemons\n"
		"  move <move_name>\n"
		"  pokemon <pokemon_name>\n"
		<< std::flush;
}

static void printState(BattleHandler &game)
{
	const auto &state = game.getBattleState();

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
}

static bool handleCommand(const std::string &command, std::unique_ptr<EmulatorGameHandle> &emulator, BattleHandler &game, BattleState &state)
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
		try {
			state.me.team.push_back(getPkmnFromCin(state, false));
			std::cout << "Added " << state.me.team.back().dump() << " to the team." << std::endl;
		} catch (std::exception &e) {
			std::cout << e.what() << std::endl;
		}
	} else if (command == "setPkmn") {
		try {
			std::cin >> arg;

			unsigned index = std::stoul(arg);

			state.me.team.at(index) = getPkmnFromCin(state, false);
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
				[arg, port](const ByteHandle &byteHandle, const LoopHandle &loopHandle) {
					return new BGBHandler(byteHandle, byteHandle, loopHandle, arg, port, getenv("MAX_DEBUG"));
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
		printHelp();
	} else if (command == "moves") {
		for (const auto &move : availableMoves)
			std::cout << static_cast<int>(move.getID()) << ": " << move.getName() << std::endl;
	} else if (command == "pokemons") {
		for (const auto &[id, pkmn] : pokemonList)
			std::cout << static_cast<int>(pkmn.id) << ": " << pkmn.name << std::endl;
	} else if (command == "move") {
		std::string name;

		std::getline(std::cin, name);
		name = trim(name);
		std::cout << "Searching move '" << name << "'" << std::endl;
		for (const auto &move : availableMoves)
			if (move.getName().find(name) != std::string::npos)
				std::cout << static_cast<int>(move.getID()) << ": " << move.getName() << std::endl;
	} else if (command == "pokemon") {
		std::string name;

		std::getline(std::cin, name);
		name = trim(name);
		std::cout << "Searching pokemon '" << name << "'" << std::endl;
		for (const auto &[id, pkmn] : pokemonList)
			if (pkmn.name.find(name) != std::string::npos)
				std::cout << static_cast<int>(pkmn.id) << ": " << pkmn.name << std::endl;
	} else if (command == "state") {
		printState(game);
	} else {
		std::cout << "Invalid command \"" << command << "\"" << std::endl;
	}
	return true;
}

static std::string getAnswer()
{
	std::string answer;

	std::cout << "> " << std::flush;
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

static void commandLine(const std::string &trainerName)
{
	bool loop = true;
	std::future<std::string> future;
	std::unique_ptr<EmulatorGameHandle> emulator;
	BattleHandler battle{false, getenv("MIN_DEBUG") != nullptr};
	auto &state = battle.getBattleState();

	state.rng.makeRandomList(9);
	state.battleLogger = [](const PkmnCommon::Event &event) {
		if (auto text = std::get_if<PkmnCommon::TextEvent>(&event))
			std::cout << text->message << std::endl;
	};
	state.me.name = trainerName;
	state.me.team.emplace_back(
		state, "", 100,
		pokemonList.at(Rhydon),
		std::vector<Move>{
			availableMoves[Tackle],
			availableMoves[Tail_Whip],
		}
	);
	std::cout << "Type help for help" << std::endl;
	while (loop) {
		future = std::async(std::launch::async, getAnswer);
		do {
			if (emulator && emulator->getStage() == EmulatorGameHandle::BATTLE)
				emulator->setReady(false);
		} while (future.wait_for(std::chrono::seconds(1)) != std::future_status::ready);
		loop = handleCommand(future.get(), emulator, battle, state);
	}
}

int main(int argc, char **argv)
{
	if (argc > 1 && std::strcmp(argv[1], "-h") == 0) {
		std::cerr << "Usage: " << argv[0] << " [<trainerName>]" << std::endl;
		return EXIT_FAILURE;
	}
	commandLine(argc > 1 ? argv[1] : "PokeAI");
	return EXIT_SUCCESS;
}
