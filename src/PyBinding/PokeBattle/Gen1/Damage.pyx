# distutils: language = c++

from libcpp cimport bool
from ._Damage cimport DamageResult as __DamageResult

cdef class DamageResult:
	cdef __DamageResult __instance

	def __init__(self, bool critical = False, unsigned damage = 0, bool affect = True, bool isVeryEffective = False, bool isNotVeryEffective = False):
		self.__instance.critical = critical
		self.__instance.damage = damage
		self.__instance.affect = affect
		self.__instance.isVeryEffective = isVeryEffective
		self.__instance.isNotVeryEffective = isNotVeryEffective

	@property
	def critical(self):
		return self.__instance.critical
	@critical.setter
	def critical(self, value):
		self.__instance.critical = value

	@property
	def damage(self):
		return self.__instance.damage
	@damage.setter
	def damage(self, value):
		self.__instance.damage = value

	@property
	def affect(self):
		return self.__instance.affect
	@affect.setter
	def affect(self, value):
		self.__instance.affect = value

	@property
	def is_very_effective(self):
		return self.__instance.isVeryEffective
	@is_very_effective.setter
	def is_very_effective(self, value):
		self.__instance.isVeryEffective = value

	@property
	def is_not_very_effective(self):
		return self.__instance.isNotVeryEffective
	@is_not_very_effective.setter
	def is_not_very_effective(self, value):
		self.__instance.isNotVeryEffective = value
