# cython: language_level=3

from libcpp.vector cimport vector

cdef extern from "../../GameEngine/RandomGenerator.hpp" namespace "PokemonGen1":
	cdef cppclass RandomGenerator:
		RandomGenerator()
		void makeRandomList(unsigned int size)
		void setList(const vector[unsigned char] &l)
		const vector[unsigned char] &getList() const
		unsigned getIndex() const
		void setIndex(unsigned index)
		void reset()
		unsigned char operator()()