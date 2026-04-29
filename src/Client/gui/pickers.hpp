//
// Created by PinkySmile on 22/04/2026.
//

#ifndef POKEAI_PICKERS_HPP
#define POKEAI_PICKERS_HPP

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include "gui.hpp"
#include "GameEngine/Gen1/Pokemon.hpp"
#include "GameEngine/Gen1/BattleHandler.hpp"
#include "Emulator/EmulatorGameHandle.hpp"

void openChangeMoveBox(
	tgui::Gui &gui,
	BattleResources &resources,
	PokemonGen1::Pokemon &pkmn,
	unsigned moveIndex,
	tgui::Button::Ptr moveButton
);

void openChangePkmnBox(
	tgui::Gui &gui,
	PokemonGen1::BattleHandler &game,
	std::unique_ptr<PokemonGen1::EmulatorGameHandle> &emulator,
	BattleResources &resources,
	unsigned index,
	PokemonGen1::Pokemon &pkmn,
	sf::RenderWindow &window,
	tgui::Panel::Ptr pkmnPan,
	std::pair<unsigned char, unsigned char> &aisSelected,
	bool &side,
	bool &ready
);

void invalidatePkmnPickerCache();

#endif //POKEAI_PICKERS_HPP
