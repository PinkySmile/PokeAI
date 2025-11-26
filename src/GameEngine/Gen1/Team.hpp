//
// Created by PinkySmile on 21/06/25.
//

#ifndef POKEAI_TEAM_HPP
#define POKEAI_TEAM_HPP


#include <vector>
#include "Pokemon.hpp"

#define TRAINER_DATA_SIZE (11 * 7 + 44 * 6 + 1)

namespace PokemonGen1
{
	typedef std::pair<std::string, std::vector<Pokemon>> Trainer;

	Trainer loadTrainer(const std::vector<unsigned char> &data, BattleState &state);
	std::vector<unsigned char> saveTrainer(const Trainer &trainer);
	void loadScenario(const std::vector<unsigned char> &data, BattleState &state);
}


#endif //POKEAI_TEAM_HPP
