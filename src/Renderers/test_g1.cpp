//
// Created by PinkySmile on 20/11/2025.
//

#include <SFML/Graphics.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include "Gen1Renderer.hpp"
#include "GameEngine/Gen1/BattleHandler.hpp"
#include "nlohmann/json.hpp"
#include "GameEngine/Gen1/Team.hpp"

std::random_device dev;

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

/*typedef std::map<PokemonGen1::BattleAction, unsigned> (*AILayer)(PokemonGen1::PlayerState &me, PokemonGen1::PlayerState &op)

std::map<PokemonGen1::BattleAction, unsigned> basic_layer(PokemonGen1::PlayerState &me, PokemonGen1::PlayerState &op)
{
	
}

PokemonGen1::BattleAction layered_ai(PokemonGen1::PlayerState &me, PokemonGen1::PlayerState &op, std::vector<AILayer> layers)
{

}*/

int main(int argc, char **argv)
{
	std::string version;
	bool colors = true;
	bool testMoves = false;
	std::string replay;
	unsigned turn = 0;
	bool argsDisabled = false;

	for (int index = 1; index < argc && replay.empty(); index++) {
		if (!argsDisabled && argv[index][0] == '-') {
			if (strcmp(argv[index], "--") == 0) {
				argsDisabled = true;
				continue;
			} else if (strcmp(argv[index], "-c") == 0) {
				colors = false;
				continue;
			} else if (strcmp(argv[index], "-m") == 0) {
				testMoves = true;
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
	PokemonGen1::BattleState &state = handler.getBattleState();
	PkmnRenderer::Gen1Renderer renderer{version, colors};

	state.battleLogger = [&renderer](const PkmnCommon::Event &event){
		if (auto text = std::get_if<PkmnCommon::TextEvent>(&event))
			puts(text->message.c_str());
		renderer.consumeEvent(event);
	};
	if (testMoves) {
		memset((void*)&renderer.state, 0, sizeof(renderer.state));
		renderer.state.p1.spriteId = PkmnCommon::Pikachu;
		strcpy(renderer.state.p1.team[0].name, "PIKACHU");
		renderer.state.p1.team[0].id = PkmnCommon::Pikachu;
		renderer.state.p1.team[0].hp = 50;
		renderer.state.p1.team[0].maxHp = 75;
		renderer.state.p1.team[0].level = 75;
		renderer.state.p1.active = 0;
		renderer.state.p2.spriteId = PkmnCommon::Clefairy;
		strcpy(renderer.state.p2.team[0].name, "CLEFAIRY");
		renderer.state.p2.team[0].id = PkmnCommon::Clefairy;
		renderer.state.p2.team[0].hp = 50;
		renderer.state.p2.team[0].maxHp = 75;
		renderer.state.p2.team[0].level = 100;
		renderer.state.p2.active = 0;
	} else if (replay.empty()) {
		state.me.name = "Player 1";
		state.op.name = "Player 2";
		state.rng.makeRandomList(9);

		std::vector<unsigned> valid;

		for (auto &base : PokemonGen1::pokemonList) {
			if (base.second.name == "MISSINGNO.")
				continue;
			valid.emplace_back(base.first);
		}

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

	if (testMoves) {
		std::ifstream stream{"assets/gen1/moves/list.json"};
		nlohmann::json json;

		stream >> json;
		for (auto &id : json) {
			renderer.consumeEvent(PkmnCommon::Event(PkmnCommon::TurnStartEvent{}));
			renderer.consumeEvent(PkmnCommon::Event(PkmnCommon::TextEvent{"PIKACHU used " + PokemonGen1::availableMoves[id].getName() + "!"}));
			renderer.consumeEvent(PkmnCommon::Event(PkmnCommon::MoveEvent{id, true, false}));
			if (id == PkmnCommon::Explosion || id == PkmnCommon::Self_Destruct)
				renderer.consumeEvent(PkmnCommon::Event(PkmnCommon::SwitchEvent{0, true}));
			renderer.consumeEvent(PkmnCommon::Event(PkmnCommon::TextEvent{"CLEFAIRY used " + PokemonGen1::availableMoves[id].getName() + "!"}));
			renderer.consumeEvent(PkmnCommon::Event(PkmnCommon::MoveEvent{id, false, false}));
			if (id == PkmnCommon::Explosion || id == PkmnCommon::Self_Destruct)
				renderer.consumeEvent(PkmnCommon::Event(PkmnCommon::SwitchEvent{0, false}));
		}
	} else try {
		handler.start();
		renderer.state = PkmnRenderer::fromGen1(state);
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
	unsigned div = 1;
	unsigned mul = 1;

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
					if (key->code == sf::Keyboard::Key::Up) {
						if (div == 1) {
							mul++;
							win.setFramerateLimit(60 * mul);
						} else {
							div--;
							win.setFramerateLimit(60 / div);
						}
					}
					if (key->code == sf::Keyboard::Key::Down) {
						if (mul == 1) {
							div++;
							win.setFramerateLimit(60 / div);
						} else {
							mul--;
							win.setFramerateLimit(60 * mul);
						}
					}
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
