//
// Created by Gegel85 on 14/07/2019.
//

#ifndef POKEAI_POKEMONTYPES_HPP
#define POKEAI_POKEMONTYPES_HPP


/*
** From Rhydon
** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Tables.cs#L7
*/

enum PokemonTypes {
	TYPE_NORMAL,
	TYPE_FIGHTING,
	TYPE_FLYING,
	TYPE_POISON,
	TYPE_GROUND,
	TYPE_ROCK,
	TYPE_INVALID,
	TYPE_BUG,
	TYPE_GHOST,
	TYPE_0x09,
	TYPE_0x0A,
	TYPE_0x0B,
	TYPE_0x0C,
	TYPE_0x0D,
	TYPE_0x0E,
	TYPE_0x0F,
	TYPE_0x10,
	TYPE_0x11,
	TYPE_0x12,
	TYPE_0x13,
	TYPE_FIRE,
	TYPE_WATER,
	TYPE_GRASS,
	TYPE_ELECTRIC,
	TYPE_PSYCHIC,
	TYPE_ICE,
	TYPE_DRAGON,
	TYPE_NEUTRAL_PHYSICAL = 0x100,
	TYPE_NEUTRAL_SPECIAL = 0x101
};

std::string typeToString(PokemonTypes);
double getAttackDamageMultiplier(PokemonTypes attackType, PokemonTypes target);
double getAttackDamageMultiplier(PokemonTypes attackType, std::pair<PokemonTypes, PokemonTypes> target);

#endif //POKEAI_POKEMONTYPES_HPP
