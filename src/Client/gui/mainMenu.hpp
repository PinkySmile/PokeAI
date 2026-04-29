//
// Created by PinkySmile on 22/04/2026.
//

#ifndef POKEAI_MAINMENU_HPP
#define POKEAI_MAINMENU_HPP

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "gui.hpp"
#include "GameEngine/Gen1/Pokemon.hpp"
#include "GameEngine/Gen1/BattleHandler.hpp"
#include "Emulator/EmulatorGameHandle.hpp"

void mainMenu(
	sf::RenderWindow &window,
	std::unique_ptr<PokemonGen1::EmulatorGameHandle> &emulator,
	PokemonGen1::BattleHandler &game,
	BattleResources &resources,
	std::pair<unsigned char, unsigned char> &ai,
	bool &ready
);

void makeMainMenuGUI(
	sf::RenderWindow &window,
	tgui::Gui &gui,
	std::unique_ptr<PokemonGen1::EmulatorGameHandle> &emulator,
	PokemonGen1::BattleHandler &game,
	BattleResources &resources,
	std::pair<unsigned char, unsigned char> &aisSelected,
	bool &side,
	bool &ready
);

void populatePokemonPanel(
	sf::RenderWindow &window,
	tgui::Gui &gui,
	std::unique_ptr<PokemonGen1::EmulatorGameHandle> &emulator,
	PokemonGen1::BattleHandler &game,
	BattleResources &resources,
	tgui::Panel::Ptr panel,
	unsigned index,
	std::vector<PokemonGen1::Pokemon> &team,
	std::pair<unsigned char, unsigned char> &aisSelected,
	bool &side,
	bool &ready
);

#endif //POKEAI_MAINMENU_HPP
