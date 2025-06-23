//
// Created by PinkySmile on 22/06/25.
//

#ifndef POKEAI_AIFACTORY_HPP
#define POKEAI_AIFACTORY_HPP


#include "AI.hpp"

namespace PokemonGen1
{
	class AIFactory {
	public:
		static AI *create(unsigned id);
	};
}


#endif //POKEAI_AIFACTORY_HPP
