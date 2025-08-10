# cython: language_level=3

from libcpp cimport bool
from libcpp.string cimport string
from State cimport BattleState

cdef extern from "../../GameEngine/BattleHandler.hpp" namespace "PokemonGen1":
	cdef cppclass BattleHandler:
		BattleHandler(bool viewSwapped, bool logMessages);

		void logBattle(const string &message)
		BattleState &getBattleState()
		bool tick()
		bool isFinished()
		void reset()