//
// Created by PinkySmile on 15/07/2019.
//

#ifndef POKEAI_RANDOMGENERATOR_HPP
#define POKEAI_RANDOMGENERATOR_HPP


#include <random>

namespace PokemonGen1
{
	class RandomGenerator {
	private:
		std::mt19937 _random;
		unsigned _currentIndex = 0;
		std::vector<unsigned char> _numbersBase;
		std::vector<unsigned char> _numbers;

	public:
		RandomGenerator();

		void makeRandomList(unsigned int size);
		void setList(const std::vector<unsigned char> &list);
		const std::vector<unsigned char> &getList() const;
		unsigned getIndex() const;
		void setIndex(unsigned index);
		void reset();
		unsigned char operator()();
	};
}


#endif //POKEAI_RANDOMGENERATOR_HPP
