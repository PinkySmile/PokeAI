//
// Created by PinkySmile on 06/08/2025.
//

#ifndef POKEAI_STATE_H
#define POKEAI_STATE_H


#include <stdbool.h>

typedef enum PokemonGen1_BattleAction {
	PokemonGen1_NoAction,
	PokemonGen1_Attack1 = 0x60,
	PokemonGen1_Attack2,
	PokemonGen1_Attack3,
	PokemonGen1_Attack4,
	PokemonGen1_Switch1,
	PokemonGen1_Switch2,
	PokemonGen1_Switch3,
	PokemonGen1_Switch4,
	PokemonGen1_Switch5,
	PokemonGen1_Switch6,
	PokemonGen1_StruggleMove = 0x6E,
	PokemonGen1_Run,
} PokemonGen1_BattleAction;

typedef bool PokemonGen1_MovesDiscovered[4];
typedef struct {
	bool pokemon;
	PokemonGen1_MovesDiscovered moves;
} PokemonGen1_PkmnDiscovered;
typedef void (*PokemonGen1_BattleLogger)(const char *message);


#endif //POKEAI_STATE_H
