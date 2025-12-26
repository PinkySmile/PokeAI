# distutils: language = c++

from libcpp.string cimport string
from libcpp.vector cimport vector

from ._Gen1Renderer cimport Gen1Renderer as __Gen1Renderer
from ._Event cimport Event

cdef class Gen1Renderer:
	cdef __Gen1Renderer *__instance

	def __cinit__(self):
		self.__instance = NULL

	def __dealloc__(self):
		del self.__instance

	def __init__(self, variant, bool hasColors):
		if isinstance(variant, str):
			variant = variant.encode('utf-8')
		self.__instance = new __Gen1Renderer(variant, hasColors)

	def update(self):
		return self.__instance.update()

	def reset(self):
		return self.__instance.reset()

	#def consume_event(self, dict event):
	#	return self.__instance.consumeEvent(event)

	def previous_turn(self):
		return self.__instance.previousTurn()

	def next_turn(self):
		return self.__instance.nextTurn()

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
		return bytes(self.__instance.renderVec())

	# size_t renderBuff(unsigned char *buffer)