//
// Created by PinkySmile on 20/11/2025.
//

#include <SFML/Graphics.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include "Gen1Renderer.hpp"
#include "GameEngine/Gen1/BattleHandler.hpp"
#include "nlohmann/json.hpp"
#include "GameEngine/Gen1/Team.hpp"

PokemonGen1::BattleAction basic_opponent(PokemonGen1::PlayerState &me, PokemonGen1::PlayerState &op)
{
	auto &pkmn = me.team[me.pokemonOnField];

	if (pkmn.getHealth() == 0)
		return static_cast<PokemonGen1::BattleAction>(PokemonGen1::Switch1 + me.pokemonOnField + 1);
	if (op.team[op.pokemonOnField].getHealth() == 0)
		return PokemonGen1::NoAction;
	if (pkmn.isWrapped())
		return PokemonGen1::NoAction;

	auto &moveSet = pkmn.getMoveSet();

	for (size_t i = 0; i < moveSet.size(); i++) {
		auto &move = moveSet[i];

		if (move.getID() != 0 && move.getPP() != 0 && i + 1 != pkmn.getMoveDisabled())
			return static_cast<PokemonGen1::BattleAction>(PokemonGen1::Attack1 + i);
	}
	return PokemonGen1::StruggleMove;
}

int main(int argc, char **argv)
{
	std::string version;
	bool colors = false;
	std::string replay;
	unsigned turn = 0;
	bool argsDisabled = false;

	for (int index = 1; index < argc && replay.empty(); index++) {
		if (!argsDisabled && argv[index][0] == '-') {
			if (strcmp(argv[index], "--") == 0) {
				argsDisabled = true;
				continue;
			} else if (strcmp(argv[index], "-c") == 0) {
				colors = true;
				continue;
			} else if (strcmp(argv[index], "-r") == 0) {
				version = "r";
				continue;
			} else if (strcmp(argv[index], "-g") == 0) {
				version = "rg";
				continue;
			} else if (strcmp(argv[index], "-t") == 0) {
				index++;
				if (index > argc) {
					puts("Expected argument for option -t");
					return 1;
				}
				turn = std::stoul(argv[index]);
				continue;
			} else {
				printf("Unknown option %s\n", argv[index]);
				return 1;
			}
		}
		replay = argv[index];
	}

	PokemonGen1::BattleHandler handler{false, false};
	auto &state = handler.getBattleState();
	Gen1Renderer renderer{version};

	state.battleLogger = [&renderer](const PkmnCommon::Event &event){
		if (auto text = std::get_if<PkmnCommon::TextEvent>(&event))
			puts(text->message.c_str());
		renderer.consumeEvent(event);
	};
	if (replay.empty()) {
		state.me.name = "Player 1";
		state.op.name = "Player 2";
		state.rng.makeRandomList(9);

		std::vector<unsigned> valid;

		for (auto &base : PokemonGen1::pokemonList) {
			if (base.second.name == "MISSINGNO.")
				continue;
			valid.emplace_back(base.first);
		}

		std::random_device dev;
		std::uniform_int_distribution<size_t> dist{0, valid.size() - 1};

		for (int i = 0; i < 6; i++)
			state.me.team.emplace_back(
				state,
				"Pkmn" + std::to_string(i),
				100,
				PokemonGen1::pokemonList.at(valid.at(dist(dev))),
				std::vector<PokemonGen1::Move>{
					PokemonGen1::availableMoves[PokemonGen1::Metronome],
					PokemonGen1::availableMoves[PokemonGen1::Metronome],
					PokemonGen1::availableMoves[PokemonGen1::Metronome],
					PokemonGen1::availableMoves[PokemonGen1::Metronome]
				},
				false
			);
		for (int i = 0; i < 6; i++)
			state.op.team.emplace_back(
				state,
				"Pkmn" + std::to_string(i),
				100,
				PokemonGen1::pokemonList.at(valid.at(dist(dev))),
				std::vector<PokemonGen1::Move>{
					PokemonGen1::availableMoves[PokemonGen1::Metronome],
					PokemonGen1::availableMoves[PokemonGen1::Metronome],
					PokemonGen1::availableMoves[PokemonGen1::Metronome],
					PokemonGen1::availableMoves[PokemonGen1::Metronome]
				},
				true
			);
	} else
		handler.loadReplay(replay);
	handler.start();
	renderer.state = fromGen1(state);

	try {
		while (!handler.isFinished()) {
			if (replay.empty()) {
				state.me.nextAction = basic_opponent(state.me, state.op);
				state.op.nextAction = basic_opponent(state.op, state.me);
			}
			handler.tick();
		}
	} catch (std::exception &e) {
		std::cerr << "Error during battle: " << e.what() << std::endl;
	}

	auto size = renderer.getSize();
	sf::RenderWindow win{sf::VideoMode{{size.x * 4, size.y * 4}}, state.me.name + " vs " + state.op.name};
	sf::View view;

	view.setCenter({size.x / 2.f, size.y / 2.f});
	view.setSize(sf::Vector2f(size));
	win.setFramerateLimit(60);
	win.setView(view);
	renderer.reset();
	renderer.goToTurn(turn);

	bool paused = false;
	bool ok = false;

	try {
		while (win.isOpen()) {
			while (auto event = win.pollEvent()) {
				if (event->is<sf::Event::Closed>())
					win.close();
				if (auto key = event->getIf<sf::Event::KeyPressed>()) {
					if (key->code == sf::Keyboard::Key::Space)
						paused = !paused;
					if (key->code == sf::Keyboard::Key::Enter)
						ok = true;
					if (key->code == sf::Keyboard::Key::Right)
						renderer.nextTurn();
					if (key->code == sf::Keyboard::Key::Left)
						renderer.previousTurn();
					if (key->code == sf::Keyboard::Key::Home)
						renderer.goToTurn(0);
					if (key->code == sf::Keyboard::Key::End)
						renderer.goToTurn(-1);
				}
				renderer.consumeEvent(*event);
			}
			if (!paused || ok)
				renderer.update();
			ok = false;
			renderer.render(win);
			win.display();
		}
		if (replay.empty())
			handler.saveReplay("result.replay");
	} catch (...) {
		handler.saveReplay("error.replay");
		throw;
	}
}
