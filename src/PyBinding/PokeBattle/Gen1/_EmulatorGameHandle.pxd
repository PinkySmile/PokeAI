# cython: language_level=3

from libcpp cimport bool
from libcpp.pair cimport pair
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.functional cimport function
from ._State cimport BattleState

cdef extern from "<Emulator/EmulatorGameHandle.hpp>" namespace "PokemonGen1":
	#ctypedef function[unsigned char(EmulatorHandle &handler, unsigned char byte)] ByteHandle
	#ctypedef function[void(EmulatorHandle &handler)] LoopHandle

	cdef cppclass EmulatorGameHandle:
		#EmulatorGameHandle(
		#	const EmulatorCreator &emulatorMaker,
		#	BattleState &state,
		#	bool player2,
		#	bool log
		#)

		void setReady(bool ready)
		Gen1ConnectionStage getStage()
		void setStage(Gen1ConnectionStage stage)
		bool isConnected()
		bool isReady()
		pair[unsigned, unsigned] getBattleSendingProgress()

		@staticmethod
		vector[unsigned char] convertString(const string &str)
		@staticmethod
		string convertString(const vector[unsigned char] &str)


cdef extern from "<Emulator/EmulatorGameHandle.hpp>" namespace "PokemonGen1::EmulatorGameHandle":
	ctypedef enum Gen1ConnectionStage:
		PKMN_CENTER,
		PINGING_OPPONENT,
		ROOM_CHOOSE,
		PING_POKEMON_EXCHANGE,
		EXCHANGE_POKEMONS,
		BATTLE,