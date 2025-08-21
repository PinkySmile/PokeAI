//
// Created by PinkySmile on 24/07/2019.
//

#include <map>
#include <string>
#include <stdexcept>
#include "Type.hpp"

/*
** From Pokepedia
** https://www.pokepedia.fr/Normal
** https://www.pokepedia.fr/Combat_(type)
** https://www.pokepedia.fr/Vol_(type)
** https://www.pokepedia.fr/Poison_(type)
** https://www.pokepedia.fr/Sol
** https://www.pokepedia.fr/Roche
** https://www.pokepedia.fr/Insecte
** https://www.pokepedia.fr/Spectre_(type)
** https://www.pokepedia.fr/Feu
** https://www.pokepedia.fr/Eau
** https://www.pokepedia.fr/Plante
** https://www.pokepedia.fr/%C3%89lectrik
** https://www.pokepedia.fr/Psy
** https://www.pokepedia.fr/Glace
** https://www.pokepedia.fr/Dragon_(type)
*/

namespace PokemonGen1
{
	const std::map<Type, std::map<Type, double>> typeTable{
		{TYPE_NORMAL,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 1.0},
			{TYPE_FLYING,   1.0},
			{TYPE_POISON,   1.0},
			{TYPE_GROUND,   1.0},
			{TYPE_ROCK,     0.5},
			{TYPE_BUG,      1.0},
			{TYPE_GHOST,    0.0},
			{TYPE_FIRE,     1.0},
			{TYPE_WATER,    1.0},
			{TYPE_GRASS,    1.0},
			{TYPE_ELECTRIC, 1.0},
			{TYPE_PSYCHIC,  1.0},
			{TYPE_ICE,      1.0},
			{TYPE_DRAGON,   1.0},
		}},
		{TYPE_FIGHTING,{
			{TYPE_NORMAL,   2.0},
			{TYPE_FIGHTING, 1.0},
			{TYPE_FLYING,   0.5},
			{TYPE_POISON,   0.5},
			{TYPE_GROUND,   1.0},
			{TYPE_ROCK,     2.0},
			{TYPE_BUG,      0.5},
			{TYPE_GHOST,    0.0},
			{TYPE_FIRE,     1.0},
			{TYPE_WATER,    1.0},
			{TYPE_GRASS,    1.0},
			{TYPE_ELECTRIC, 1.0},
			{TYPE_PSYCHIC,  0.5},
			{TYPE_ICE,      2.0},
			{TYPE_DRAGON,   1.0},
		}},
		{TYPE_FLYING,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 2.0},
			{TYPE_FLYING,   1.0},
			{TYPE_POISON,   1.0},
			{TYPE_GROUND,   1.0},
			{TYPE_ROCK,     0.5},
			{TYPE_BUG,      2.0},
			{TYPE_GHOST,    1.0},
			{TYPE_FIRE,     1.0},
			{TYPE_WATER,    1.0},
			{TYPE_GRASS,    2.0},
			{TYPE_ELECTRIC, 0.5},
			{TYPE_PSYCHIC,  1.0},
			{TYPE_ICE,      1.0},
			{TYPE_DRAGON,   1.0},
		}},
		{TYPE_POISON,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 1.0},
			{TYPE_FLYING,   1.0},
			{TYPE_POISON,   0.5},
			{TYPE_GROUND,   0.5},
			{TYPE_ROCK,     0.5},
			{TYPE_BUG,      2.0},
			{TYPE_GHOST,    0.5},
			{TYPE_FIRE,     1.0},
			{TYPE_WATER,    1.0},
			{TYPE_GRASS,    2.0},
			{TYPE_ELECTRIC, 1.0},
			{TYPE_PSYCHIC,  1.0},
			{TYPE_ICE,      1.0},
			{TYPE_DRAGON,   1.0},
		}},
		{TYPE_GROUND,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 1.0},
			{TYPE_FLYING,   0.0},
			{TYPE_POISON,   2.0},
			{TYPE_GROUND,   1.0},
			{TYPE_ROCK,     2.0},
			{TYPE_BUG,      0.5},
			{TYPE_GHOST,    1.0},
			{TYPE_FIRE,     2.0},
			{TYPE_WATER,    1.0},
			{TYPE_GRASS,    0.5},
			{TYPE_ELECTRIC, 2.0},
			{TYPE_PSYCHIC,  1.0},
			{TYPE_ICE,      1.0},
			{TYPE_DRAGON,   1.0},
		}},
		{TYPE_ROCK,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 0.5},
			{TYPE_FLYING,   2.0},
			{TYPE_POISON,   1.0},
			{TYPE_GROUND,   0.5},
			{TYPE_ROCK,     1.0},
			{TYPE_BUG,      2.0},
			{TYPE_GHOST,    1.0},
			{TYPE_FIRE,     2.0},
			{TYPE_WATER,    1.0},
			{TYPE_GRASS,    1.0},
			{TYPE_ELECTRIC, 1.0},
			{TYPE_PSYCHIC,  1.0},
			{TYPE_ICE,      2.0},
			{TYPE_DRAGON,   1.0},
		}},
		{TYPE_BUG,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 0.5},
			{TYPE_FLYING,   0.5},
			{TYPE_POISON,   2.0},
			{TYPE_GROUND,   1.0},
			{TYPE_ROCK,     1.0},
			{TYPE_BUG,      1.0},
			{TYPE_GHOST,    0.5},
			{TYPE_FIRE,     0.5},
			{TYPE_WATER,    1.0},
			{TYPE_GRASS,    2.0},
			{TYPE_ELECTRIC, 1.0},
			{TYPE_PSYCHIC,  2.0},
			{TYPE_ICE,      1.0},
			{TYPE_DRAGON,   1.0},
		}},
		{TYPE_GHOST,{
			{TYPE_NORMAL,   0.0},
			{TYPE_FIGHTING, 1.0},
			{TYPE_FLYING,   1.0},
			{TYPE_POISON,   1.0},
			{TYPE_GROUND,   1.0},
			{TYPE_ROCK,     1.0},
			{TYPE_BUG,      1.0},
			{TYPE_GHOST,    2.0},
			{TYPE_FIRE,     1.0},
			{TYPE_WATER,    1.0},
			{TYPE_GRASS,    1.0},
			{TYPE_ELECTRIC, 1.0},
			{TYPE_PSYCHIC,  0.0},
			{TYPE_ICE,      1.0},
			{TYPE_DRAGON,   1.0},
		}},
		{TYPE_FIRE,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 1.0},
			{TYPE_FLYING,   1.0},
			{TYPE_POISON,   1.0},
			{TYPE_GROUND,   1.0},
			{TYPE_ROCK,     0.5},
			{TYPE_BUG,      2.0},
			{TYPE_GHOST,    1.0},
			{TYPE_FIRE,     0.5},
			{TYPE_WATER,    0.5},
			{TYPE_GRASS,    2.0},
			{TYPE_ELECTRIC, 1.0},
			{TYPE_PSYCHIC,  1.0},
			{TYPE_ICE,      2.0},
			{TYPE_DRAGON,   0.5},
		}},
		{TYPE_WATER,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 1.0},
			{TYPE_FLYING,   1.0},
			{TYPE_POISON,   1.0},
			{TYPE_GROUND,   2.0},
			{TYPE_ROCK,     2.0},
			{TYPE_BUG,      1.0},
			{TYPE_GHOST,    1.0},
			{TYPE_FIRE,     2.0},
			{TYPE_WATER,    0.5},
			{TYPE_GRASS,    0.5},
			{TYPE_ELECTRIC, 1.0},
			{TYPE_PSYCHIC,  1.0},
			{TYPE_ICE,      1.0},
			{TYPE_DRAGON,   0.5},
		}},
		{TYPE_GRASS,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 1.0},
			{TYPE_FLYING,   0.5},
			{TYPE_POISON,   0.5},
			{TYPE_GROUND,   2.0},
			{TYPE_ROCK,     2.0},
			{TYPE_BUG,      0.5},
			{TYPE_GHOST,    1.0},
			{TYPE_FIRE,     0.5},
			{TYPE_WATER,    2.0},
			{TYPE_GRASS,    0.5},
			{TYPE_ELECTRIC, 1.0},
			{TYPE_PSYCHIC,  1.0},
			{TYPE_ICE,      1.0},
			{TYPE_DRAGON,   0.5},
		}},
		{TYPE_ELECTRIC,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 1.0},
			{TYPE_FLYING,   2.0},
			{TYPE_POISON,   1.0},
			{TYPE_GROUND,   0.0},
			{TYPE_ROCK,     1.0},
			{TYPE_BUG,      1.0},
			{TYPE_GHOST,    1.0},
			{TYPE_FIRE,     1.0},
			{TYPE_WATER,    2.0},
			{TYPE_GRASS,    0.5},
			{TYPE_ELECTRIC, 0.5},
			{TYPE_PSYCHIC,  1.0},
			{TYPE_ICE,      1.0},
			{TYPE_DRAGON,   0.5},
		}},
		{TYPE_PSYCHIC,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 2.0},
			{TYPE_FLYING,   1.0},
			{TYPE_POISON,   2.0},
			{TYPE_GROUND,   1.0},
			{TYPE_ROCK,     1.0},
			{TYPE_BUG,      1.0},
			{TYPE_GHOST,    1.0},
			{TYPE_FIRE,     1.0},
			{TYPE_WATER,    1.0},
			{TYPE_GRASS,    1.0},
			{TYPE_ELECTRIC, 1.0},
			{TYPE_PSYCHIC,  0.5},
			{TYPE_ICE,      1.0},
			{TYPE_DRAGON,   1.0},
		}},
		{TYPE_ICE,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 1.0},
			{TYPE_FLYING,   2.0},
			{TYPE_POISON,   1.0},
			{TYPE_GROUND,   2.0},
			{TYPE_ROCK,     1.0},
			{TYPE_BUG,      1.0},
			{TYPE_GHOST,    1.0},
			{TYPE_FIRE,     1.0},
			{TYPE_WATER,    0.5},
			{TYPE_GRASS,    2.0},
			{TYPE_ELECTRIC, 1.0},
			{TYPE_PSYCHIC,  1.0},
			{TYPE_ICE,      0.5},
			{TYPE_DRAGON,   2.0},
		}},
		{TYPE_DRAGON,{
			{TYPE_NORMAL,   1.0},
			{TYPE_FIGHTING, 1.0},
			{TYPE_FLYING,   1.0},
			{TYPE_POISON,   1.0},
			{TYPE_GROUND,   1.0},
			{TYPE_ROCK,     1.0},
			{TYPE_BUG,      1.0},
			{TYPE_GHOST,    1.0},
			{TYPE_FIRE,     1.0},
			{TYPE_WATER,    1.0},
			{TYPE_GRASS,    1.0},
			{TYPE_ELECTRIC, 1.0},
			{TYPE_PSYCHIC,  1.0},
			{TYPE_ICE,      1.0},
			{TYPE_DRAGON,   2.0},
		}},
	};

	std::string typeToString(Type type)
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
		case TYPE_BIRD:
			return "Bird";
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
			return "Unknown";
		}
	}

	std::string typeToStringShort(Type type)
	{
		switch (type) {
		case TYPE_NORMAL:
			return "Nor";
		case TYPE_FIGHTING:
			return "Fgt";
		case TYPE_FLYING:
			return "Fly";
		case TYPE_POISON:
			return "Psn";
		case TYPE_GROUND:
			return "Gnd";
		case TYPE_ROCK:
			return "Roc";
		case TYPE_BIRD:
			return "Brd";
		case TYPE_BUG:
			return "Bug";
		case TYPE_GHOST:
			return "Gst";
		case TYPE_FIRE:
			return "Fre";
		case TYPE_WATER:
			return "Wtr";
		case TYPE_GRASS:
			return "Grs";
		case TYPE_ELECTRIC:
			return "Elc";
		case TYPE_PSYCHIC:
			return "Psy";
		case TYPE_ICE:
			return "Ice";
		case TYPE_DRAGON:
			return "Dgn";
		default:
			return "???";
		}
	}

	double getAttackDamageMultiplier(Type attackType, Type target)
	{
		try {
			return typeTable.at(attackType).at(target);
		} catch (std::out_of_range &) {
			return 1;
		}
	}

	double getAttackDamageMultiplier(Type attackType, std::pair<Type, Type> target)
	{
		auto result = getAttackDamageMultiplier(attackType, target.first);

		if (target.first != target.second)
			result *= getAttackDamageMultiplier(attackType, target.second);
		return result;
	}
}