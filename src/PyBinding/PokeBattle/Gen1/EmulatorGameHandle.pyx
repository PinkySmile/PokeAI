# distutils: language = c++

from libcpp cimport bool
from libcpp.pair cimport pair
from libcpp.string cimport string
from libcpp.vector cimport vector
from cython cimport cast
from ._EmulatorGameHandle cimport EmulatorGameHandle as __EmulatorGameHandle, Gen1ConnectionStage as __Gen1ConnectionStage, PKMN_CENTER, PINGING_OPPONENT, ROOM_CHOOSE, PING_POKEMON_EXCHANGE, EXCHANGE_POKEMONS, BATTLE


cpdef enum Gen1ConnectionStage:
	PkmnCenter = PKMN_CENTER
	PingingOpponent = PINGING_OPPONENT
	RoomChoose = ROOM_CHOOSE
	PingPokemonExchange = PING_POKEMON_EXCHANGE
	ExchangePokemons = EXCHANGE_POKEMONS
	Battle = BATTLE


cdef class EmulatorGameHandle:
	cdef __EmulatorGameHandle *__instance
	cdef bool __allocd

	def __cinit__(self):
		self.__allocd = False

	def __deallocate__(self):
		if self.__allocd:
			del self.__instance

	def __init__(self):
		pass

	@property
	def ready(self):
		return self.__instance.isReady()
	@ready.setter
	def ready(self, bool ready):
		self.__instance.setReady(ready)

	@property
	def stage(self):
		return Gen1ConnectionStage(self.__instance.getStage())
	@stage.setter
	def stage(self, stage):
		self.__instance.setStage(cast(__Gen1ConnectionStage, stage))

	@property
	def connected(self):
		return self.__instance.isConnected()

	@property
	def battle_sending_progress(self):
		cdef pair[unsigned, unsigned] progress = self.__instance.getBattleSendingProgress()
		return (progress.first, progress.second)

	@staticmethod
	def convert_string(data):
		cdef string s
		cdef vector[unsigned char] d

		if isinstance(data, str):
			s = data.encode('ASCII')
			return __EmulatorGameHandle.convertString(s)
		d = data
		f: bytes = __EmulatorGameHandle.convertString(d)
		return f.decode('utf-8')
