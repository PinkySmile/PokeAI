//
// Created by PinkySmile on 22/06/25.
//

#include "AIFactory.hpp"
#include "AIHeuristic.hpp"

namespace PokemonGen1
{
	AI *AIFactory::create(unsigned id)
	{
		switch (id) {
		case 1:
			return new AIHeuristic();
		default:
			return nullptr;
		}
	}
}