# distutils: language = c++

from cython cimport cast
from libcpp.pair cimport pair
from ._Type cimport Type as __Type, typeToString, typeToStringShort, getAttackDamageMultiplier, TYPE_NORMAL, TYPE_FIGHTING, TYPE_FLYING, TYPE_POISON, TYPE_GROUND, TYPE_ROCK, TYPE_INVALID, TYPE_BUG, TYPE_GHOST, TYPE_FIRE, TYPE_WATER, TYPE_GRASS, TYPE_ELECTRIC, TYPE_PSYCHIC, TYPE_ICE, TYPE_DRAGON, TYPE_NEUTRAL_PHYSICAL, TYPE_NEUTRAL_SPECIAL

cpdef enum Type:
	Normal = TYPE_NORMAL
	Fighting = TYPE_FIGHTING
	Flying = TYPE_FLYING
	Poison = TYPE_POISON
	Ground = TYPE_GROUND
	Rock = TYPE_ROCK
	Invalid = TYPE_INVALID
	Bug = TYPE_BUG
	Ghost = TYPE_GHOST
	Fire = TYPE_FIRE
	Water = TYPE_WATER
	Grass = TYPE_GRASS
	Electric = TYPE_ELECTRIC
	Psychic = TYPE_PSYCHIC
	Ice = TYPE_ICE
	Dragon = TYPE_DRAGON
	Neutral_Physical = TYPE_NEUTRAL_PHYSICAL
	Neutral_Special = TYPE_NEUTRAL_SPECIAL

def type_to_string(type: Type):
	return typeToString(type).decode('ASCII')

def type_to_string_short(type: Type):
	return typeToStringShort(type).decode('ASCII')

def get_attack_damage_multiplier(attack: Type, defender: Type|int|tuple[Type|int,Type|int]):
	cdef pair[__Type, __Type] p

	if isinstance(defender, tuple):
		p.first = defender[0]
		p.second = defender[1]
		return getAttackDamageMultiplier(cast(__Type, attack), p)
	return getAttackDamageMultiplier(cast(__Type, attack), cast(__Type, defender))
