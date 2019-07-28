//
// Created by Gegel85 on 15/07/2019.
//

#include "PokemonRandomGenerator.hpp"

namespace PokemonGen1
{
	PokemonRandomGenerator::PokemonRandomGenerator(unsigned int seed) :
		_random(seed),
		_seed(seed)
	{
	}

	unsigned PokemonRandomGenerator::getSeed()
	{
		return this->_seed;
	}
	
	unsigned PokemonRandomGenerator::operator()()
	{
		return this->_random();
	}

	unsigned PokemonRandomGenerator::operator()(unsigned int start, unsigned int end)
	{
		unsigned randomNumber = (*this)();

		randomNumber = (randomNumber * static_cast<float>(end - start) / UINT32_MAX) + start;
		if (randomNumber == end)
			return end - 1;
		return randomNumber;
	}

	unsigned PokemonRandomGenerator::operator()(unsigned int end)
	{
		return (*this)(0, end);
	}
}