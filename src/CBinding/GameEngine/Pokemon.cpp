//
// Created by PinkySmile on 06/08/2025.
//

#include <cstring>
#include "Pokemon.h"
#include "GameEngine/Pokemon.hpp"

PokemonGen1_Pokemon_Base *PokemonGen1_getPokemonBase(size_t index)
{
	auto it = PokemonGen1::pokemonList.find(index);

	if (it == PokemonGen1::pokemonList.end())
		return nullptr;

	auto obj = new PokemonGen1_Pokemon_Base();

	obj->id = it->second.id;
	obj->dexId = it->second.dexId;
	obj->name = it->second.name.c_str();
	obj->HP = it->second.HP;
	obj->ATK = it->second.ATK;
	obj->DEF = it->second.DEF;
	obj->SPD = it->second.SPD;
	obj->SPE = it->second.SPE;
	obj->typeA = static_cast<PokemonGen1_Type>(it->second.typeA);
	obj->typeB = static_cast<PokemonGen1_Type>(it->second.typeB);
	obj->catchRate = it->second.catchRate;
	obj->baseXpYield = it->second.baseXpYield;
	obj->movePoolSize = it->second.movePool.size();
	obj->movePool = new PokemonGen1_AvailableMove[obj->movePoolSize];
	memcpy(&obj->statsAtLevel, it->second.statsAtLevel, sizeof(it->second.statsAtLevel));

	auto moveIt = it->second.movePool.begin();

	for (size_t i = 0; i < it->second.movePool.size(); i++) {
		obj->movePool[i] = static_cast<PokemonGen1_AvailableMove>(*moveIt);
		moveIt++;
	}
	return obj;
}

void PokemonGen1_Pokemon_Base_destroy(PokemonGen1_Pokemon_Base *object)
{
	delete[] object->movePool;
	delete object;
}