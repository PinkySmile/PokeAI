# cython: language_level=3

from libcpp cimport bool
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from ._Pokemon cimport Pokemon
from ._State cimport BattleState

cdef extern from "<GameEngine/Gen1/Team.hpp>" namespace "PokemonGen1":
	ctypedef pair[string, vector[Pokemon]] Trainer

	cpdef Trainer loadTrainer(const vector[unsigned char] &data, BattleState &logger, bool isEnemy)
	cpdef vector[unsigned char] saveTrainer(const Trainer &trainer)