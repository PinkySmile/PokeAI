//
// Created by PinkySmile on 30/08/2019.
//

#ifndef POKEAI_GUI_HPP
#define POKEAI_GUI_HPP

#include <string>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "GameEngine/BattleHandler.hpp"

struct BattleResources {
	sf::Music				loop;
	sf::Music				start;
	sf::Font				font;
	sf::Texture				categories[3];
	sf::Texture				balls[4];
	sf::Texture				pokemonsBack[256];
	sf::Texture				pokemonsFront[256];
	sf::Texture				trainer[2][2];
	sf::Texture				boxes[4];
	sf::Texture				arrows[2];
	sf::Texture				hpOverlay;
	sf::Texture				levelSprite;
	sf::Texture				choicesHUD;
	sf::Texture				attackHUD;
	sf::Texture				waitingHUD;
	sf::SoundBuffer				hitSounds[3];
	sf::SoundBuffer				trainerLand;
	sf::SoundBuffer				battleCries[256];
	sf::Sound                               crySound{this->battleCries[0]};
	std::map<std::string, sf::Texture>	types;
};

std::string strToUpper(std::string str);
void gui(const std::string &trainerName);
void battle(sf::RenderWindow &window, PokemonGen1::BattleHandler &game, BattleResources &resources, std::vector<std::string> &log, std::pair<unsigned char, unsigned char> ai, bool updateManually);

#endif //POKEAI_GUI_HPP
