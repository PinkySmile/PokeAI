//
// Created by PinkySmile on 15/07/2019.
//

#include "RandomGenerator.hpp"
#include "BattleHandler.hpp"

namespace PokemonGen1
{
	RandomGenerator::RandomGenerator() :
		_random(time(nullptr))
	{
	}

	void RandomGenerator::makeRandomList(unsigned int size)
	{
		std::uniform_int_distribution distribution{0, SYNC_BYTE - 1};

		this->_currentIndex = 0;
		this->_numbers.clear();
		while (size--)
			this->_numbers.push_back(distribution(this->_random));
		if (!this->_numbers.empty() && this->_numbers[0] == 0)
			this->_numbers[0] = 1;
		this->_numbersBase = this->_numbers;
	}

	const std::vector<unsigned char> &RandomGenerator::getList() const
	{
		return this->_numbers;
	}

	void RandomGenerator::setList(const std::vector<unsigned char> &list)
	{
		this->_numbers = list;
		this->_currentIndex = 0;
		if (!this->_numbers.empty() && this->_numbers[0] == 0)
			this->_numbers[0] = 1;
		this->_numbersBase = this->_numbers;
	}

	unsigned char RandomGenerator::operator()()
	{
		unsigned char value = this->_numbers[this->_currentIndex];

		this->_currentIndex++;
		if (this->_numbers.size() == this->_currentIndex) {
			this->_currentIndex = 0;
			for (auto &elem : this->_numbers)
				elem = elem * 5 + 1;
		}
		return value;
	}

	unsigned RandomGenerator::getIndex() const
	{
		return this->_currentIndex;
	}

	void RandomGenerator::setIndex(unsigned int index)
	{
		this->_currentIndex = index;
	}

	void RandomGenerator::reset()
	{
		this->_numbers = this->_numbersBase;
		this->_currentIndex = 0;
	}
}