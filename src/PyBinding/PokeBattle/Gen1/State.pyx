# distutils: language = c++

from libcpp cimport bool
from libcpp.vector cimport vector
from cython cimport cast
from cython.operator cimport dereference
from ._Pokemon cimport Pokemon as __Pokemon, pythonLoggerLambda, evalLogger
from ._State cimport BattleAction as __BattleAction, BattleActionToString as __BattleActionToString, PlayerState as __PlayerState, BattleState as __BattleState, pythonCallbackLambda, pythonCallbackLambdaVoid, evalCallback, evalCallbackVoid, EmptyAction as __EmptyAction, Attack1 as __Attack1, Attack2 as __Attack2, Attack3 as __Attack3, Attack4 as __Attack4, Switch1 as __Switch1, Switch2 as __Switch2, Switch3 as __Switch3, Switch4 as __Switch4, Switch5 as __Switch5, Switch6 as __Switch6, NoAction as __NoAction, StruggleMove as __StruggleMove, Run as __Run, DESYNC_MISS, DESYNC_THROW, DESYNC_INVERT, DESYNC_IGNORE
from .Pokemon cimport Pokemon as PyPokemon
from .RandomGenerator cimport RandomGenerator as PyRandomGenerator

from .Pokemon import Pokemon
from .RandomGenerator import RandomGenerator

cpdef enum DesyncPolicy:
	Miss = DESYNC_MISS
	Throw = DESYNC_THROW
	Invert = DESYNC_INVERT
	Ignore = DESYNC_IGNORE


cpdef enum BattleAction:
	EmptyAction = __EmptyAction
	Attack1 = __Attack1
	Attack2 = __Attack2
	Attack3 = __Attack3
	Attack4 = __Attack4
	Switch1 = __Switch1
	Switch2 = __Switch2
	Switch3 = __Switch3
	Switch4 = __Switch4
	Switch5 = __Switch5
	Switch6 = __Switch6
	NoAction = __NoAction
	StruggleMove = __StruggleMove
	Run = __Run

def battle_action_to_string(s):
	return __BattleActionToString(s).decode('ASCII')


cdef class PlayerState:
	cdef __PlayerState *__instance

	@property
	def name(self):
		o = self.__instance.name
		return o.decode('ASCII')
	@name.setter
	def name(self, v):
		self.__instance.name = v.encode('ASCII')

	@property
	def last_action(self):
		return BattleAction(self.__instance.lastAction)
	@last_action.setter
	def last_action(self, v):
		self.__instance.lastAction = cast(__BattleAction, v)

	@property
	def next_action(self):
		return BattleAction(self.__instance.nextAction)
	@next_action.setter
	def next_action(self, v):
		self.__instance.nextAction = cast(__BattleAction, v)

	@property
	def pokemon_on_field_index(self):
		return self.__instance.pokemonOnField
	@pokemon_on_field_index.setter
	def pokemon_on_field_index(self, v):
		self.__instance.pokemonOnField = v

	@property
	def pokemon_on_field(self):
		p = <PyPokemon>Pokemon()
		p.__instance = &self.__instance.team[self.__instance.pokemonOnField]
		return p

	@property
	def team(self):
		result = []
		for pkmn in range(self.__instance.team.size()):
			p = <PyPokemon>Pokemon()
			p.__instance = &self.__instance.team[pkmn]
			result.append(p)
		return result
	@team.setter
	def team(self, v):
		cdef vector[__Pokemon] l
		for pkmn in v:
			l.push_back(dereference((<PyPokemon>pkmn).__instance))
		self.__instance.team = l

	def is_pkmn_discovered(self, size_t i):
		if 0 <= i < 6:
			return self.__instance.discovered[i].first
		return False
	def set_pkmn_discovered(self, size_t i, bool v):
		if 0 <= i < 6:
			self.__instance.discovered[i].first = v

	def is_pkmn_move_discovered(self, size_t pkmn, size_t move):
		if 0 <= pkmn < 6 and 0 <= move < 4:
			return self.__instance.discovered[pkmn].first and self.__instance.discovered[pkmn].second[move]
		return False
	def set_pkmn_move_discovered(self, size_t pkmn, size_t move, bool v):
		if 0 <= pkmn < 6 and 0 <= move < 4:
			self.__instance.discovered[pkmn].second[move] = v


cdef class BattleState:
	def __cinit__(self):
		self.__instance = NULL
		self.__allocd = False

	def __dealloc__(self):
		if self.__allocd:
			del self.__instance

	def __init__(self, alloc=True):
		if alloc:
			self.__instance = new __BattleState()
			self.__allocd = True
		self.__battleLoggerPy = None
		self.__onTurnStartPy = None
		self.__onBattleEndPy = None
		self.__onBattleStartPy = None

	def copy(self):
		cdef __BattleState *obj = new __BattleState(dereference(self.__instance))
		b = BattleState()
		del b.__instance
		b.__instance = obj
		return b

	@property
	def me(self):
		p = PlayerState()
		p.__instance = &self.__instance.me
		return p

	@property
	def op(self):
		p = PlayerState()
		p.__instance = &self.__instance.op
		return p

	@property
	def rng(self):
		p = <PyRandomGenerator>RandomGenerator(False)
		p.__instance = &self.__instance.rng
		return p

	@property
	def desync(self):
		return DesyncPolicy(self.__instance.desync)
	@desync.setter
	def desync(self, c):
		self.__instance.desync = c

	@property
	def logger(self):
		#return self.battleLoggerPy
		raise NotImplementedError("Not implemented")
	@logger.setter
	def logger(self, c):
		if not callable(c):
			raise TypeError("Requires callable")
		self.__battleLoggerPy = c
		self.__instance.battleLogger = pythonLoggerLambda(<void *>self.__battleLoggerPy, &evalLogger)

	@property
	def on_turn_start(self):
		#return self.__instance.onTurnStart
		raise NotImplementedError("Not implemented")
	@on_turn_start.setter
	def on_turn_start(self, c):
		if not callable(c):
			raise TypeError("Requires callable")
		self.__onTurnStartPy = c
		self.__instance.onTurnStart = pythonCallbackLambda(<void*>self.__onTurnStartPy, &evalCallback)

	@property
	def on_battle_end(self):
		#return self.__instance.onBattleEnd
		raise NotImplementedError("Not implemented")
	@on_battle_end.setter
	def on_battle_end(self, c):
		if not callable(c):
			raise TypeError("Requires callable")
		self.__onBattleEndPy = c
		self.__instance.onBattleEnd = pythonCallbackLambdaVoid(<void*>self.__onBattleEndPy, &evalCallbackVoid)

	@property
	def on_battle_start(self):
		#return self.__instance.onBattleStart
		raise NotImplementedError("Not implemented")
	@on_battle_start.setter
	def on_battle_start(self, c):
		if not callable(c):
			raise TypeError("Requires callable")
		self.__onBattleStartPy = c
		self.__instance.onBattleStart = pythonCallbackLambdaVoid(<void*>self.__onBattleStartPy, &evalCallbackVoid)
