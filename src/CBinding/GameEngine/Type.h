//
// Created by PinkySmile on 07/08/2025.
//

#ifndef POKEAI_TYPE_H
#define POKEAI_TYPE_H


#include "c_cpp.h"

typedef enum PokemonGen1_Type {
	PokemonGen1_TYPE_NORMAL,
	PokemonGen1_TYPE_FIGHTING,
	PokemonGen1_TYPE_FLYING,
	PokemonGen1_TYPE_POISON,
	PokemonGen1_TYPE_GROUND,
	PokemonGen1_TYPE_ROCK,
	PokemonGen1_TYPE_INVALID,
	PokemonGen1_TYPE_BUG,
	PokemonGen1_TYPE_GHOST,
	PokemonGen1_TYPE_0x09,
	PokemonGen1_TYPE_0x0A,
	PokemonGen1_TYPE_0x0B,
	PokemonGen1_TYPE_0x0C,
	PokemonGen1_TYPE_0x0D,
	PokemonGen1_TYPE_0x0E,
	PokemonGen1_TYPE_0x0F,
	PokemonGen1_TYPE_0x10,
	PokemonGen1_TYPE_0x11,
	PokemonGen1_TYPE_0x12,
	PokemonGen1_TYPE_0x13,
	PokemonGen1_TYPE_FIRE,
	PokemonGen1_TYPE_WATER,
	PokemonGen1_TYPE_GRASS,
	PokemonGen1_TYPE_ELECTRIC,
	PokemonGen1_TYPE_PSYCHIC,
	PokemonGen1_TYPE_ICE,
	PokemonGen1_TYPE_DRAGON,
	PokemonGen1_TYPE_NEUTRAL_PHYSICAL = 0x100,
	PokemonGen1_TYPE_NEUTRAL_SPECIAL = 0x101
} PokemonGen1_Type;

#define MAX_TYPE_SIZE 9
#define SHORT_TYPE_SIZE 4

GEN1API void PokemonGen1_typeToString(PokemonGen1_Type type, char *buffer);
GEN1API void PokemonGen1_typeToStringShort(PokemonGen1_Type type, char *buffer);
GEN1API double PokemonGen1_getAttackDamageMultiplier(PokemonGen1_Type attackType, PokemonGen1_Type target);
GEN1API double PokemonGen1_getAttackDamageMultiplierDual(PokemonGen1_Type attackType, PokemonGen1_Type target1, PokemonGen1_Type target2);


#endif //POKEAI_TYPE_H
