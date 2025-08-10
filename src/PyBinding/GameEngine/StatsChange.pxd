# cython: language_level=3

from libcpp.string cimport string

cdef extern from "../../GameEngine/StatsChange.hpp" namespace "PokemonGen1":
	cpdef enum StatsChange:
		STATS_ATK,
		STATS_DEF,
		STATS_SPD,
		STATS_SPE,
		STATS_EVD,
		STATS_ACC

	string statToString(StatsChange stat);
	string statToLittleString(StatsChange stat);