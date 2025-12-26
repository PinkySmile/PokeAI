# distutils: language = c++

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool
from cython.operator cimport dereference

from ._Gen1Renderer cimport Gen1Renderer as __Gen1Renderer
from ._IRenderer cimport Vector2u, fromGen1
from ._Event cimport Event, dictToEvent
from .State cimport BattleState

cdef class Gen1Renderer :
	cdef __Gen1Renderer *__instance
	cdef vector[unsigned char] __buffer

	def __cinit__(self):
		self.__instance = NULL

	def __dealloc__(self):
		del self.__instance

	def __init__(self, variant, bool hasColors):
		if isinstance(variant, str):
			variant = variant.encode('utf-8')
		self.__instance = new __Gen1Renderer(variant, hasColors)
		x, y = self.size
		self.__buffer.resize(x * y * 4)

	def update(self):
		return self.__instance.update()

	def reset(self):
		return self.__instance.reset()

	def consume_event(self, dict event):
		return self.__instance.consumeEvent(dictToEvent(event))

	def previous_turn(self):
		return self.__instance.previousTurn()

	def next_turn(self):
		return self.__instance.nextTurn()

	@property
	def size(self):
		cdef Vector2u s = self.__instance.getSize()
		return (s.x, s.y)

	@property
	def turn(self):
		return self.__instance.getTurn()
	@turn.setter
	def turn(self, t):
		self.__instance.goToTurn(t)

	@property
	def animation_ended(self) -> bool:
		return self.__instance.hasAnimationEnded()

	def render_pic(self, buffer=None) -> bytes:
		self.__instance.renderBuff(self.__buffer.data())
		return bytes(self.__buffer)

	@property
	def sound_disabled(self):
		return self.__instance.soundDisabled
	@sound_disabled.setter
	def sound_disabled(self, v):
		self.__instance.soundDisabled = v

	@property
	def display_turn(self):
		return self.__instance.displayTurn
	@display_turn.setter
	def display_turn(self, v):
		self.__instance.displayTurn = v

	@property
	def music_disabled(self):
		return self.__instance.musicDisabled
	@music_disabled.setter
	def music_disabled(self, v):
		self.__instance.musicDisabled = v

	@property
	def waiting(self):
		return self.__instance.waiting
	@waiting.setter
	def waiting(self, v):
		self.__instance.waiting = v

	@property
	def state(self):
		raise NotImplementedError("Not implemented")
	@state.setter
	def state(self, v):
		if isinstance(v, BattleState):
			self.__instance.state = fromGen1(dereference((<BattleState>v).__instance))
		else:
			raise RuntimeError("Invalid argument")

	# size_t renderBuff(unsigned char *buffer)