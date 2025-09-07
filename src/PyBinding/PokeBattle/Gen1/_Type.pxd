# cython: language_level=3

from libcpp.string cimport string
from libcpp.pair cimport pair

cdef extern from "<GameEngine/Type.hpp>" namespace "PokemonGen1":
	ctypedef enum Type:
		TYPE_NORMAL,
		TYPE_FIGHTING,
		TYPE_FLYING,
		TYPE_POISON,
		TYPE_GROUND,
		TYPE_ROCK,
		TYPE_INVALID,
		TYPE_BUG,
		TYPE_GHOST,
		TYPE_0x09,
		TYPE_0x0A,
		TYPE_0x0B,
		TYPE_0x0C,
		TYPE_0x0D,
		TYPE_0x0E,
		TYPE_0x0F,
		TYPE_0x10,
		TYPE_0x11,
		TYPE_0x12,
		TYPE_0x13,
		TYPE_FIRE,
		TYPE_WATER,
		TYPE_GRASS,
		TYPE_ELECTRIC,
		TYPE_PSYCHIC,
		TYPE_ICE,
		TYPE_DRAGON,
		TYPE_NEUTRAL_PHYSICAL,
		TYPE_NEUTRAL_SPECIAL

	cdef string typeToString(Type);
	cdef string typeToStringShort(Type);
	cdef double getAttackDamageMultiplier(Type attackType, Type target);
	cdef double getAttackDamageMultiplier(Type attackType, pair[Type, Type] target);