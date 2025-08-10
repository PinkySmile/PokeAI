//
// Created by PinkySmile on 23/08/2020
//

#include "Pokemon.hpp"
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
		case STATS_EVD:
			return "EVADE";
		case STATS_ACC:
			return "ACCURACY";
		default:
			return "???";
		}
	}

	std::string statToLittleString(StatsChange stat)
	{
		switch (stat) {
		case STATS_ATK:
			return "ATK";
		case STATS_DEF:
			return "DEF";
		case STATS_SPD:
			return "SPD";
		case STATS_SPE:
			return "SPE";
		case STATS_EVD:
			return "EVD";
		case STATS_ACC:
			return "ACC";
		default:
			return "???";
		}
	}
}