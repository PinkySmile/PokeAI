//
// Created by PinkySmile on 07/08/2025.
//

#ifndef POKEAI_STATSCHANGE_H
#define POKEAI_STATSCHANGE_H


#include "c_cpp.h"

typedef enum PokemonGen1_StatsChange {
	PokemonGen1_STATS_ATK,
	PokemonGen1_STATS_DEF,
	PokemonGen1_STATS_SPD,
	PokemonGen1_STATS_SPE,
	PokemonGen1_STATS_EVD,
	PokemonGen1_STATS_ACC,
} PokemonGen1_StatsChange;

GEN1API const char *PokemonGen1_statToString(PokemonGen1_StatsChange stat);
GEN1API const char *PokemonGen1_statToLittleString(PokemonGen1_StatsChange stat);

#endif //POKEAI_STATSCHANGE_H
