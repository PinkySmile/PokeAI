//
// Created by PinkySmile on 21/06/25.
//

#ifndef POKEAI_TEAM_HPP
#define POKEAI_TEAM_HPP


#include <vector>
#include "Pokemon.hpp"

namespace PokemonGen1
{
	typedef std::pair<std::string, std::vector<Pokemon>> Trainer;

	Trainer loadTrainer(const std::vector<unsigned char> &data, RandomGenerator &rng, const Pokemon::Logger &logger);
	std::vector<unsigned char> saveTrainer(const Trainer &trainer);
}


#endif //POKEAI_TEAM_HPP
