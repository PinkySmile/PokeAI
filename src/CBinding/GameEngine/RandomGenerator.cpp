//
// Created by PinkySmile on 06/08/2025.
//

#include <cstring>
#include "RandomGenerator.h"
#include "GameEngine/RandomGenerator.hpp"

PokemonGen1_RandomGenerator *PokemonGen1_RandomGenerator_create()
{
	return new PokemonGen1::RandomGenerator();
}

void PokemonGen1_RandomGenerator_destroy(PokemonGen1_RandomGenerator *This)
{
	delete This;
}

void PokemonGen1_RandomGenerator_makeRandomList(PokemonGen1_RandomGenerator *This, unsigned int size)
{
	This->makeRandomList(size);
}

void PokemonGen1_RandomGenerator_setList(PokemonGen1_RandomGenerator *This, const unsigned char *buffer, size_t size)
{
	This->setList({buffer, buffer + size});
}

const unsigned char *PokemonGen1_RandomGenerator_getList(const PokemonGen1_RandomGenerator *This, size_t *size)
{
	const auto &list = This->getList();

	if (size)
		*size = list.size();
	return list.data();
}

unsigned PokemonGen1_RandomGenerator_getIndex(const PokemonGen1_RandomGenerator *This)
{
	return This->getIndex();
}

void PokemonGen1_RandomGenerator_setIndex(PokemonGen1_RandomGenerator *This, unsigned index)
{
	This->setIndex(index);
}

void PokemonGen1_RandomGenerator_reset(PokemonGen1_RandomGenerator *This)
{
	This->reset();
}

unsigned char PokemonGen1_RandomGenerator_call(PokemonGen1_RandomGenerator *This)
{
	return (*This)();
}
