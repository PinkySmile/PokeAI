//
// Created by PinkySmile on 30/08/2019.
//

#include <SFML/Audio.hpp>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include "gui.hpp"
#include "mainMenu.hpp"
#include "Utils.hpp"
#include "Emulator/EmulatorGameHandle.hpp"
#include "Renderers/Gen1Renderer.hpp"

using namespace PokemonGen1;

std::string strToUpper(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

void loadResources(BattleResources &resources)
{
	(void)resources.categories[0].loadFromFile("assets/move_categories/physical.png");
	(void)resources.categories[1].loadFromFile("assets/move_categories/special.png");
	(void)resources.categories[2].loadFromFile("assets/move_categories/status.png");

	for (int i = 0; i <= TYPE_DRAGON; i++) {
		auto typeName = typeToString(static_cast<Type>(i));

		if (resources.types.contains(typeName))
			continue;
		(void)resources.types[typeName].loadFromFile("assets/types/type_" + Utils::toLower(typeName) + ".png");
	}
}

void gui(const std::string &trainerName)
{
	std::unique_ptr<EmulatorGameHandle> emulator;
	std::pair<unsigned char, unsigned char> ais{0, 0};
	BattleHandler battleHandler{false, getenv("MIN_DEBUG") != nullptr};
	auto &state = battleHandler.getBattleState();
	sf::RenderWindow window{sf::VideoMode{{800, 640}}, trainerName};
	BattleResources resources;
	bool ready = false;

	resources.renderer = std::make_unique<PkmnRenderer::Gen1Renderer>("", true);
	state.rng.makeRandomList(9);
	state.battleLogger = [&resources](const PkmnCommon::Event &event){
		if (auto text = std::get_if<PkmnCommon::TextEvent>(&event))
			puts(text->message.c_str());
		resources.renderer->consumeEvent(event);
	};
	loadResources(resources);

	std::vector<Move> defaultMoves{availableMoves[Tackle], availableMoves[Tail_Whip]};
	state.me.team.resize(6, {state, "", 100, pokemonList.at(Rhydon), defaultMoves});
	state.op.team.resize(6, {state, "", 100, pokemonList.at(Rhydon), defaultMoves});

	window.setFramerateLimit(60);
	while (window.isOpen()) {
		bool inBattle =
			battleHandler.playingReplay() ||
			(emulator && emulator->getStage() == EmulatorGameHandle::BATTLE) ||
			(ready && !emulator);

		if (!inBattle) {
			mainMenu(window, emulator, battleHandler, resources, ais, ready);
			continue;
		}
		if (emulator && emulator->getStage() != EmulatorGameHandle::BATTLE) {
			state.me.nextAction = Run;
			state.op.nextAction = Run;
			battleHandler.tick();
		}
		bool escaped = battle(window, battleHandler, *resources.renderer, ais, !emulator);
		ready = false;
		if (escaped && emulator)
			emulator.reset();
	}
}

int main(int argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "-h") == 0) {
		std::cerr << "Usage: " << argv[0] << " [<mode>, <trainerName>]" << std::endl;
		return EXIT_FAILURE;
	}
	gui(argc > 1 ? argv[1] : "PokeAI");
	return EXIT_SUCCESS;
}
