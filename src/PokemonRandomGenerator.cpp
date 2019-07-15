//
// Created by Gegel85 on 15/07/2019.
//

#include "PokemonRandomGenerator.hpp"

namespace Pokemon
{
	PokemonRandomGenerator::PokemonRandomGenerator(unsigned int seed) :
		_random(seed)
	{
	}

	unsigned PokemonRandomGenerator::operator()()
	{
		return this->_random();
	}

	unsigned PokemonRandomGenerator::operator()(unsigned int start, unsigned int end)
	{
		unsigned randomNumber = (*this)();

		randomNumber = (randomNumber * static_cast<float>(end - start) / (1U << 31U)) + start;
		if (randomNumber == end)
			return end - 1;
		return randomNumber;
	}

	unsigned PokemonRandomGenerator::operator()(unsigned int end)
	{
		return (*this)(0, end);
	}
}