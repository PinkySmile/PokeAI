//
// Created by PinkySmile on 07/08/2025.
//

#include <cstring>
#include "StatsChange.h"
#include "GameEngine/StatsChange.hpp"

const char *PokemonGen1_statToString(PokemonGen1_StatsChange stat, char *buffer)
{
	switch (stat) {
	case PokemonGen1_STATS_ATK:
		return "ATTACK";
	case PokemonGen1_STATS_DEF:
		return "DEFENSE";
	case PokemonGen1_STATS_SPD:
		return "SPEED";
	case PokemonGen1_STATS_SPE:
		return "SPECIAL";
	case PokemonGen1_STATS_EVD:
		return "EVADE";
	case PokemonGen1_STATS_ACC:
		return "ACCURACY";
	default:
		return "???";
	}
}

const char *PokemonGen1_statToLittleString(PokemonGen1_StatsChange stat, char *buffer)
{
	switch (stat) {
	case PokemonGen1_STATS_ATK:
		return "ATK";
	case PokemonGen1_STATS_DEF:
		return "DEF";
	case PokemonGen1_STATS_SPD:
		return "SPD";
	case PokemonGen1_STATS_SPE:
		return "SPE";
	case PokemonGen1_STATS_EVD:
		return "EVD";
	case PokemonGen1_STATS_ACC:
		return "ACC";
	default:
		return "???";
	}
}