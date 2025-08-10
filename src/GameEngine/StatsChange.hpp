//
// Created by PinkySmile on 14/07/2019.
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
		STATS_EVD,
		STATS_ACC,
	};

	std::string statToString(StatsChange stat);
	std::string statToLittleString(StatsChange stat);
}


#endif //POKEAI_STATSCHANGE_HPP
