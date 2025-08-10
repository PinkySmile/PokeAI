//
// Created by PinkySmile on 06/08/2025.
//

#ifndef POKEAI_RANDOMGENERATOR_H
#define POKEAI_RANDOMGENERATOR_H


#include <stddef.h>
#include "c_cpp.h"

#ifdef BUILD_LIB
#include "GameEngine/RandomGenerator.hpp"
#endif

TYPEDEF(RandomGenerator, RandomGenerator);

GEN1API PokemonGen1_RandomGenerator *PokemonGen1_RandomGenerator_create();
GEN1API void PokemonGen1_RandomGenerator_destroy(PokemonGen1_RandomGenerator *This);
GEN1API void PokemonGen1_RandomGenerator_makeRandomList(PokemonGen1_RandomGenerator *This, unsigned int size);
GEN1API void PokemonGen1_RandomGenerator_setList(PokemonGen1_RandomGenerator *This, const unsigned char *buffer, size_t size);
GEN1API const unsigned char *PokemonGen1_RandomGenerator_getList(const PokemonGen1_RandomGenerator *This, size_t *size);
GEN1API unsigned PokemonGen1_RandomGenerator_getIndex(const PokemonGen1_RandomGenerator *This);
GEN1API void PokemonGen1_RandomGenerator_setIndex(PokemonGen1_RandomGenerator *This, unsigned index);
GEN1API void PokemonGen1_RandomGenerator_reset(PokemonGen1_RandomGenerator *This);
GEN1API unsigned char PokemonGen1_RandomGenerator_call(PokemonGen1_RandomGenerator *This);


#endif //POKEAI_RANDOMGENERATOR_H
