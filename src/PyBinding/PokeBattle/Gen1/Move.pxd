# cython: language_level=3

from libcpp cimport bool
from ._Move cimport Move as __Move

cdef class Move:
	cdef __Move *__instance
	cdef const __Move *__cinstance
	cdef bool __allocd