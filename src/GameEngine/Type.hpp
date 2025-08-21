//
// Created by PinkySmile on 14/07/2019.
//

#ifndef POKEAI_TYPE_HPP
#define POKEAI_TYPE_HPP


namespace PokemonGen1
{
	/*
	** From Rhydon
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Tables.cs#L7
	*/
	enum Type {
		TYPE_NORMAL,
		TYPE_FIGHTING,
		TYPE_FLYING,
		TYPE_POISON,
		TYPE_GROUND,
		TYPE_ROCK,
		TYPE_BIRD,
		TYPE_BUG,
		TYPE_GHOST,
		TYPE_FIRE = 20,
		TYPE_WATER,
		TYPE_GRASS,
		TYPE_ELECTRIC,
		TYPE_PSYCHIC,
		TYPE_ICE,
		TYPE_DRAGON,
		TYPE_INVALID,
		TYPE_NEUTRAL_PHYSICAL = 0x100,
		TYPE_NEUTRAL_SPECIAL = 0x101
	};

	std::string typeToString(Type);
	std::string typeToStringShort(Type);
	double getAttackDamageMultiplier(Type attackType, Type target);
	double getAttackDamageMultiplier(Type attackType, std::pair<Type, Type> target);
}

#endif //POKEAI_TYPE_HPP
