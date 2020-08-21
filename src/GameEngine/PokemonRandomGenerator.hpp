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
		std::mt19937			_random;
		unsigned			_currentIndex = 0;
		std::vector<unsigned char>	_numbers;

	public:
		PokemonRandomGenerator();

		void makeRandomList(unsigned int size);
		void setList(const std::vector<unsigned char> &list);
		const std::vector<unsigned char> &getList();
		unsigned char operator()();
	};
}


#endif //POKEAI_POKEMONRANDOMGENERATOR_HPP
