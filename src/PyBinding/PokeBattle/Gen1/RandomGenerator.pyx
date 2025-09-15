# distutils: language = c++

from libcpp cimport bool
from cython.operator cimport dereference
from ._RandomGenerator cimport RandomGenerator as __RandomGenerator

cdef class RandomGenerator:
	def __cinit__(self):
		self.__instance = NULL
		self.__allocd = False

	def __dealloc__(self):
		if self.__allocd:
			del self.__instance

	def __init__(self, alloc=True):
		if alloc:
			self.__instance = new __RandomGenerator()
			self.__allocd = True

	def __call__(self):
		return dereference(self.__instance)()

	def generate_list(self, unsigned int size):
		self.__instance.makeRandomList(size)

	def reset(self):
		return self.__instance.reset()

	@property
	def list(self):
		return self.__instance.getList()
	@list.setter
	def list(self, list):
		self.__instance.setList(list)

	@property
	def index(self):
		return self.__instance.getIndex()
	@index.setter
	def index(self, unsigned index):
		self.__instance.setIndex(index)
