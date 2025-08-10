//Unknown
// Created by PinkySmile on 07/08/2025.
//

#include "Type.h"
#include "GameEngine/Type.hpp"

void PokemonGen1_typeToString(PokemonGen1_Type type, char *buffer)
{
	strcpy(buffer, PokemonGen1::typeToString(type));
}

void PokemonGen1_typeToStringShort(PokemonGen1_Type type, char *buffer)
{
	strcpy(buffer, PokemonGen1::typeToStringShort(type));
}

double PokemonGen1_getAttackDamageMultiplier(PokemonGen1_Type attackType, PokemonGen1_Type target)
{
	return PokemonGen1::getAttackDamageMultiplier(
		static_cast<PokemonGen1::Type>(attackType),
		static_cast<PokemonGen1::Type>(target)
	);
}

double PokemonGen1_getAttackDamageMultiplierDual(PokemonGen1_Type attackType, PokemonGen1_Type target1, PokemonGen1_Type target2)
{
	return PokemonGen1::getAttackDamageMultiplier(
		static_cast<PokemonGen1::Type>(attackType),
		{ static_cast<PokemonGen1::Type>(target1), static_cast<PokemonGen1::Type>(target2) }
	);
}