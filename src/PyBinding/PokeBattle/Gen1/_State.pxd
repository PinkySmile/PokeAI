# cython: language_level=3

from libcpp cimport bool
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.functional cimport function

from ._Pokemon cimport Pokemon
from ._RandomGenerator cimport RandomGenerator

cdef extern from "<GameEngine/State.hpp>" namespace "PokemonGen1":
	ctypedef enum BattleAction:
		EmptyAction,
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
		NoAction,
		StruggleMove,
		Run

	ctypedef enum DesyncPolicy:
		DESYNC_MISS,
		DESYNC_THROW,
		DESYNC_INVERT,
		DESYNC_IGNORE

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
		DesyncPolicy desync
		function[void(const string &)] battleLogger
		function[unsigned char ()] onTurnStart
		function[void ()] onBattleEnd
		function[void ()] onBattleStart
		BattleState()
		BattleState(const BattleState &)
		BattleState &operator=(const BattleState &)

	cdef function[unsigned char ()] pythonCallbackLambda(void *python_function, bool (*e)(void *))
	cdef function[void ()] pythonCallbackLambdaVoid(void *python_function, void (*e)(void *))

cdef inline bool evalCallback(void *func_p) noexcept:
	# recover Python function object from void* argument and call it
	return (<object> func_p)()

cdef inline void evalCallbackVoid(void *func_p) noexcept:
	# recover Python function object from void* argument and call it
	(<object> func_p)()