//
// Created by Gegel85 on 24/07/2019.
//

#include <string>
#include "PokemonTypes.hpp"

std::string typeToString(PokemonTypes type)
{
	switch (type) {
	case TYPE_NORMAL:
		return "Normal";
	case TYPE_FIGHTING:
		return "Fighting";
	case TYPE_FLYING:
		return "Fly";
	case TYPE_POISON:
		return "Poison";
	case TYPE_GROUND:
		return "Ground";
	case TYPE_ROCK:
		return "Rock";
	case TYPE_BUG:
		return "Bug";
	case TYPE_GHOST:
		return "Ghost";
	case TYPE_FIRE:
		return "Fire";
	case TYPE_WATER:
		return "Water";
	case TYPE_GRASS:
		return "Grass";
	case TYPE_ELECTRIC:
		return "Electric";
	case TYPE_PSYCHIC:
		return "Psy";
	case TYPE_ICE:
		return "Ice";
	case TYPE_DRAGON:
		return "Dragon";
	default:
		return "???";
	}
}