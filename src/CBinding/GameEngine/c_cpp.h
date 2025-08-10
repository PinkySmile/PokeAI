//
// Created by PinkySmile on 07/08/2025.
//

#ifndef POKEAI_C_CPP_H
#define POKEAI_C_CPP_H


#ifdef __cplusplus
#define GEN1API extern "C"
#else
#define GEN1API extern
#endif
#ifdef BUILD_LIB
#define TYPEDEF(name1, name2) typedef PokemonGen1::name1 PokemonGen1_##name2
#else
#define TYPEDEF(name1, name2) typedef struct PokemonGen1_##name2 PokemonGen1_##name2
#endif

#endif //POKEAI_C_CPP_H
