# distutils: language = c++

from libcpp cimport bool

from cython.operator cimport dereference
from ._Pokemon cimport Pokemon as __Pokemon
from ._Team cimport loadTrainer as __loadTrainer, saveTrainer as __saveTrainer, Trainer
from .Pokemon cimport Pokemon as PyPokemon
from .State cimport BattleState

from .Pokemon import Pokemon

def load_trainer(data, BattleState state, bool enemy):
	trainer = __loadTrainer(data, dereference(state.__instance), enemy)
	result = []
	for index in range(trainer.second.size()):
		p = <PyPokemon>Pokemon()
		p.__instance = new __Pokemon(trainer.second[index])
		result.append(p)
	return (trainer.first.decode('ASCII'), result)

def save_trainer(trainerPy):
	cdef Trainer trainer

	trainer.first = trainerPy[0]
	for pkmn in trainerPy[1]:
		trainer.second.push_back(dereference((<PyPokemon>pkmn).__instance))
	return __saveTrainer(trainer)
