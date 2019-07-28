//
// Created by Gegel85 on 15/07/2019.
//

#ifndef POKEAI_POKEMONRANDOMGENERATOR_HPP
#define POKEAI_POKEMONRANDOMGENERATOR_HPP


#include <random>

namespace PokemonGen1
{
	class PokemonRandomGenerator {
	private:
		std::mt19937	_random;
		unsigned	_seed;

	public:
		PokemonRandomGenerator(unsigned int seed);

		unsigned getSeed();
		unsigned operator()();
		unsigned operator()(unsigned int end);
		unsigned operator()(unsigned int start, unsigned int end);
	};
}


#endif //POKEAI_POKEMONRANDOMGENERATOR_HPP
