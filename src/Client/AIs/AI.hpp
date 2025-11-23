//
// Created by PinkySmile on 08/09/2020
//

#ifndef POKEAI_AI_HPP
#define POKEAI_AI_HPP

#include "GameEngine/Gen1/BattleHandler.hpp"

namespace PokemonGen1
{
	class AI {
	public:
		virtual ~AI() = default;
		virtual BattleAction getNextMove(const BattleState &, bool) = 0;
	};
}

#endif //POKEAI_AI_HPP
