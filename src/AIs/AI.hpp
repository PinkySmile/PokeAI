//
// Created by andgel on 08/09/2020
//

#ifndef POKEAI_AI_HPP
#define POKEAI_AI_HPP

#include "../GameEngine/GameHandle.hpp"

namespace PokemonGen1
{
	class AI {
	public:
		virtual BattleAction getNextMove() = 0;
	};
}

#endif //POKEAI_AI_HPP
