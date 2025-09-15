# cython: language_level=3

from libcpp cimport bool
from ._State cimport BattleState as __BattleState

cdef class BattleState:
	cdef __BattleState *__instance
	cdef bool __allocd
	cdef object __battleLoggerPy
	cdef object __onTurnStartPy
	cdef object __onBattleEndPy
	cdef object __onBattleStartPy