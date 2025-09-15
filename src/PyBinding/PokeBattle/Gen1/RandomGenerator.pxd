# cython: language_level=3

from libcpp cimport bool
from ._RandomGenerator cimport RandomGenerator as __RandomGenerator

cdef class RandomGenerator:
	cdef __RandomGenerator *__instance
	cdef bool __allocd