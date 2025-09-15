# cython: language_level=3

from libcpp cimport bool
from ._Pokemon cimport Pokemon as __Pokemon

cdef class Pokemon:
	cdef __Pokemon *__instance
	cdef bool __allocd
