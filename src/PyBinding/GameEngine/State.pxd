# cython: language_level=3

from libcpp cimport bool
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.functional cimport function

from Pokemon cimport Pokemon
from RandomGenerator cimport RandomGenerator
from StatsChange cimport StatsChange
from StatusChange cimport StatusChange

cdef extern from "../../GameEngine/State.hpp" namespace "PokemonGen1":
	ctypedef enum BattleAction:
		NoAction,
		Attack1,
		Attack2,
		Attack3,
		Attack4,
		Switch1,
		Switch2,
		Switch3,
		Switch4,
		Switch5,
		Switch6,
		StruggleMove,
		Run

	cdef string BattleActionToString(BattleAction action)

	ctypedef bool MovesDiscovered[4]
	ctypedef pair[bool, MovesDiscovered] PkmnDiscovered

	cdef struct PlayerState:
		string name
		BattleAction lastAction
		BattleAction nextAction
		unsigned char pokemonOnField
		vector[Pokemon] team
		PkmnDiscovered discovered[6]

	cdef cppclass BattleState:
		PlayerState me
		PlayerState op
		RandomGenerator rng
		function[void(const string &)] battleLogger
		function[unsigned char ()] onTurnStart
		function[void ()] onBattleEnd
		function[void ()] onBattleStart

	cdef function[unsigned char ()] pythonCallbackLambda(void *python_function, bool (*eval)(void *))
	cdef function[void ()] pythonCallbackLambdaVoid(void *python_function, void (*eval)(void *))

cdef inline bool evalCallback(void *func_p) noexcept:
	# recover Python function object from void* argument and call it
	return (<object> func_p)()

cdef inline void evalCallbackVoid(void *func_p) noexcept:
	# recover Python function object from void* argument and call it
	(<object> func_p)()