# cython: language_level=3

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from Pokemon cimport Pokemon
from RandomGenerator cimport RandomGenerator

cdef extern from "../../GameEngine/Team.hpp" namespace "PokemonGen1":
	ctypedef pair[string, vector[Pokemon]] Trainer

	cpdef Trainer loadTrainer(const vector[unsigned char] &data, RandomGenerator &rng, const Pokemon.Logger &logger)
	cpdef vector[unsigned char] saveTrainer(const Trainer &trainer)