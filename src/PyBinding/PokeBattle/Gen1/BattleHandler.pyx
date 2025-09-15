# distutils: language = c++

from libcpp cimport bool
from .State cimport BattleState as PyBattleState
from ._BattleHandler cimport BattleHandler as __BattleHandler

from .State import BattleState

cdef class BattleHandler:
	cdef __BattleHandler *__instance
	__pyState: BattleState

	def __dealloc__(self):
		del self.__instance

	def __init__(self, bool viewSwapped, bool logMessages):
		self.__instance = new __BattleHandler(viewSwapped, logMessages)
		self.__pyState = None

	def log_battle(self, message: str):
		self.__instance.logBattle(message.encode('ASCII'))

	def tick(self):
		return self.__instance.tick()

	def reset(self):
		self.__instance.reset()

	def save_replay(self, path):
		return self.__instance.saveReplay(path.encode('ASCII'))

	def load_replay(self, path):
		self.__instance.loadReplay(path.encode('ASCII'))

	def save_state(self, path):
		return self.__instance.saveState(path.encode('ASCII'))

	def load_state(self, path):
		self.__instance.loadState(path.encode('ASCII'))

	def stop_replay(self):
		self.__instance.stopReplay()

	@property
	def finished(self):
		return self.__instance.isFinished()

	@property
	def playing_replay(self):
		return self.__instance.playingReplay()

	@property
	def state(self):
		if self.__pyState is None:
			b = <PyBattleState>BattleState(alloc=False)
			b.__instance = &self.__instance.getBattleState()
			self.__pyState = b
		return self.__pyState

	def start(self):
		self.__instance.start()
