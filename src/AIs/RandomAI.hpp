//
// Created by PinkySmile on 01/07/25.
//

#ifndef POKEAI_RANDOMAI_HPP
#define POKEAI_RANDOMAI_HPP


#include <random>
#include "AI.hpp"

namespace PokemonGen1
{
	class RandomAI : public AI {
	private:
		std::random_device _rand;

	public:
		~RandomAI() override = default;
		BattleAction getNextMove(const BattleState &state, bool b) override;
	};
}

#endif //POKEAI_RANDOMAI_HPP
