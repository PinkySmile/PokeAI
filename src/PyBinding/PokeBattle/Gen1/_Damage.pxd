# cython: language_level=3

from libcpp cimport bool

cdef extern from "<GameEngine/Damage.hpp>":
	struct DamageResult:
		bool critical
		unsigned damage
		bool affect
		bool isVeryEffective
		bool isNotVeryEffective
		DamageResult()
		DamageResult(const DamageResult &)
