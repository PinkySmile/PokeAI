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
		return (*this)(0, UINT32_MAX);
	}

	unsigned PokemonRandomGenerator::operator()(unsigned int start, unsigned int end)
	{
		std::uniform_int_distribution dist{start, end};

		return dist.operator()(this->_random);
	}

	unsigned PokemonRandomGenerator::operator()(unsigned int end)
	{
		return (*this)(0, end);
	}
}