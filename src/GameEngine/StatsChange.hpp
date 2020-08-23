//
// Created by Gegel85 on 14/07/2019.
//

#ifndef POKEAI_STATSCHANGE_HPP
#define POKEAI_STATSCHANGE_HPP


#include <string>

namespace PokemonGen1
{
	enum StatsChange {
		STATS_ATK,
		STATS_DEF,
		STATS_SPD,
		STATS_SPE,
		STATS_ESQ,
		STATS_PRE,
	};

	std::string statToString(StatsChange stat);
}


#endif //POKEAI_STATSCHANGE_HPP
