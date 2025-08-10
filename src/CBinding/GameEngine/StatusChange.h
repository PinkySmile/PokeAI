//
// Created by PinkySmile on 07/08/2025.
//

#ifndef POKEAI_STATUSCHANGE_H
#define POKEAI_STATUSCHANGE_H


#include "c_cpp.h"

typedef enum PokemonGen1_StatusChange {
	PokemonGen1_STATUS_NONE,
	PokemonGen1_STATUS_ASLEEP_FOR_1_TURN,
	PokemonGen1_STATUS_ASLEEP_FOR_2_TURN,
	PokemonGen1_STATUS_ASLEEP_FOR_3_TURN,
	PokemonGen1_STATUS_ASLEEP_FOR_4_TURN,
	PokemonGen1_STATUS_ASLEEP_FOR_5_TURN,
	PokemonGen1_STATUS_ASLEEP_FOR_6_TURN,
	PokemonGen1_STATUS_ASLEEP_FOR_7_TURN,
	PokemonGen1_STATUS_ASLEEP                  = 0b111,
	PokemonGen1_STATUS_POISONED                = 1U << 3U,
	PokemonGen1_STATUS_BURNED                  = 1U << 4U,
	PokemonGen1_STATUS_FROZEN                  = 1U << 5U,
	PokemonGen1_STATUS_PARALYZED               = 1U << 6U,
	PokemonGen1_STATUS_KO                      = 1U << 7U, //From here, it is not handled by the game
	PokemonGen1_STATUS_BADLY_POISONED          = (1U << 8U) | PokemonGen1_STATUS_POISONED,
	PokemonGen1_STATUS_ANY_NON_VOLATILE_STATUS = 0b11111111U,
	PokemonGen1_STATUS_LEECHED                 = 1U << 9U,
	PokemonGen1_STATUS_CONFUSED_FOR_1_TURN     = 0b001U << 10U,
	PokemonGen1_STATUS_CONFUSED_FOR_2_TURN     = 0b010U << 10U,
	PokemonGen1_STATUS_CONFUSED_FOR_3_TURN     = 0b011U << 10U,
	PokemonGen1_STATUS_CONFUSED_FOR_4_TURN     = 0b100U << 10U,
	PokemonGen1_STATUS_CONFUSED                = 0b111U << 10U,
	PokemonGen1_STATUS_FLINCHED                = 1U << 13U,
} PokemonGen1_StatusChange;

GEN1API const char *PokemonGen1_statusToString(PokemonGen1_StatusChange status);


#endif //POKEAI_STATUSCHANGE_H
