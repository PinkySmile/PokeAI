//
// Created by Gegel85 on 14/07/2019.
//

#ifndef POKEAI_STATSCHANGE_HPP
#define POKEAI_STATSCHANGE_HPP


#include <string>

namespace PokemonGen1
{
	class UpgradableStats;

	enum StatsChange {
		STATS_ATK,
		STATS_DEF,
		STATS_SPD,
		STATS_SPE,
		STATS_ESQ,
		STATS_ACC,
	};

	std::string statToString(StatsChange stat);
	std::string statToLittleString(StatsChange stat);
	unsigned char getStat(const class UpgradableStats &stats, StatsChange stat);
}


#endif //POKEAI_STATSCHANGE_HPP
