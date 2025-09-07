# distutils: language = c++

from ._StatsChange cimport StatsChange as __StatsChange, statToString, statToLittleString, STATS_ATK, STATS_DEF, STATS_SPD, STATS_SPE, STATS_EVD, STATS_ACC

cpdef enum StatsChange:
	Atk = STATS_ATK
	Def = STATS_DEF
	Spd = STATS_SPD
	Spe = STATS_SPE
	Evd = STATS_EVD
	Acc = STATS_ACC

def stat_to_string(s):
	return statToString(s).decode('ASCII')

def stat_to_little_string(s):
	return statToLittleString(s).decode('ASCII')