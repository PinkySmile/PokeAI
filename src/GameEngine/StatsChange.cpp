//
// Created by andgel on 23/08/2020
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
		case STATS_ESQ:
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
		case STATS_ESQ:
			return "EVD";
		case STATS_ACC:
			return "ACC";
		default:
			return "???";
		}
	}

	unsigned char getStat(const UpgradableStats &stats, StatsChange stat)
	{
		switch (stat) {
		case STATS_ATK:
			return stats.ATK;
		case STATS_DEF:
			return stats.DEF;
		case STATS_SPD:
			return stats.SPD;
		case STATS_SPE:
			return stats.SPE;
		case STATS_ESQ:
			return stats.ESQ;
		case STATS_ACC:
			return stats.PRE;
		default:
			return 0;
		}
	}
}