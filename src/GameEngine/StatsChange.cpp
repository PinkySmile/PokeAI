//
// Created by andgel on 23/08/2020
//

#include "StatsChange.hpp"

namespace PokemonGen1
{
	std::string statToString(StatsChange stat)
	{
		switch (stat) {
		case STATS_ATK:
			return "ATTACK";
		case STATS_DEF:
			return "DEFENSE";
		case STATS_SPD:
			return "SPEED";
		case STATS_SPE:
			return "SPECIAL";
		case STATS_ESQ:
			return "EVADE";
		case STATS_ACC:
			return "ACCURACY";
		default:
			return "???";
		}
	}
}