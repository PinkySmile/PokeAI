//
// Created by PinkySmile on 30/08/2019.
//

#ifndef POKEAI_GUI_HPP
#define POKEAI_GUI_HPP

#include <map>
#include <string>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "GameEngine/Gen1/BattleHandler.hpp"
#include "Renderers/IRenderer.hpp"

struct BattleResources {
	sf::Texture				categories[3];
	sf::SoundBuffer				emptyCry;
	sf::Sound				crySound{this->emptyCry};
	std::map<std::string, sf::Texture>	types;
	PkmnRenderer::IRenderer			*renderer = nullptr;
};

std::string strToUpper(std::string str);
void gui(const std::string &trainerName);
void battle(sf::RenderWindow &window, PokemonGen1::BattleHandler &game, PkmnRenderer::IRenderer &renderer, std::pair<unsigned char, unsigned char> ai, bool updateManually);

#endif //POKEAI_GUI_HPP
