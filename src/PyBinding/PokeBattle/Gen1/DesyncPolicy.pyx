# distutils: language = c++

from ._State cimport DESYNC_MISS, DESYNC_THROW, DESYNC_INVERT, DESYNC_IGNORE


cpdef enum DesyncPolicy:
	Miss = DESYNC_MISS
	Throw = DESYNC_THROW
	Invert = DESYNC_INVERT
	Ignore = DESYNC_IGNORE