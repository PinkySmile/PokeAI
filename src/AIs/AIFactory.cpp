//
// Created by PinkySmile on 22/06/25.
//

#include "AIFactory.hpp"
#include "AIHeuristic.hpp"

namespace PokemonGen1
{
	AI *AIFactory::create(unsigned id)
	{
		if (id == 0)
			return nullptr;
		if (id == 1)
			return new AIHeuristic();
		return nullptr;
	}

	std::vector<std::string> AIFactory::getList()
	{
		return {
			"No AI",
			"Heuristic AI"
		};
	}
}