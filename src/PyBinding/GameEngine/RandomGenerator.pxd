# cython: language_level=3

from libcpp.vector cimport vector

from StatsChange cimport StatsChange
from StatusChange cimport StatusChange

cdef extern from "../../GameEngine/RandomGenerator.hpp" namespace "PokemonGen1":
	cdef cppclass RandomGenerator:
		RandomGenerator();
		void makeRandomList(unsigned int size)
		void setList(const vector[unsigned char] &list)
		const vector[unsigned char] &getList() const
		unsigned getIndex() const
		void setIndex(unsigned index)
		void reset()
		unsigned char operator()()