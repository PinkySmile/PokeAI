//
// Created by Gegel85 on 15/07/2019.
//

#include "PokemonRandomGenerator.hpp"
#include "GameHandle.hpp"

namespace PokemonGen1
{
	PokemonRandomGenerator::PokemonRandomGenerator() :
		_random(time(nullptr))
	{
	}

	void PokemonRandomGenerator::makeRandomList(unsigned int size)
	{
		std::uniform_int_distribution distribution{0, UNAVAILABLE_BYTE - 1};

		this->_currentIndex = 0;
		this->_numbers.clear();
		while (size--)
			this->_numbers.push_back(distribution(this->_random));
	}

	const std::vector<unsigned char>& PokemonRandomGenerator::getList()
	{
		return this->_numbers;
	}

	void PokemonRandomGenerator::setList(const std::vector<unsigned char> &list)
	{
		this->_numbers = list;
		this->_currentIndex = 0;
	}

	unsigned char PokemonRandomGenerator::operator()()
	{
		unsigned char value = this->_numbers[this->_currentIndex];

		this->_currentIndex++;
		this->_currentIndex %= this->_numbers.size();
		return value;
	}
}