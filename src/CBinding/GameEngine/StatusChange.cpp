//
// Created by PinkySmile on 07/08/2025.
//

#include <cstring>
#include "StatusChange.h"
#include "GameEngine/StatusChange.hpp"

const char *PokemonGen1_statusToString(PokemonGen1_StatusChange status)
{
	switch (status) {
	case PokemonGen1_STATUS_NONE:
		return "OK";
	case PokemonGen1_STATUS_PARALYZED:
		return "paralyzed";
	case PokemonGen1_STATUS_BURNED:
		return "burned";
	case PokemonGen1_STATUS_FROZEN:
		return "frozen";
	case PokemonGen1_STATUS_POISONED:
		return "poisoned";
	case PokemonGen1_STATUS_BADLY_POISONED:
		return "badly poisoned";
	case PokemonGen1_STATUS_ASLEEP_FOR_1_TURN:
	case PokemonGen1_STATUS_ASLEEP_FOR_2_TURN:
	case PokemonGen1_STATUS_ASLEEP_FOR_3_TURN:
	case PokemonGen1_STATUS_ASLEEP_FOR_4_TURN:
	case PokemonGen1_STATUS_ASLEEP_FOR_5_TURN:
	case PokemonGen1_STATUS_ASLEEP_FOR_6_TURN:
	case PokemonGen1_STATUS_ASLEEP_FOR_7_TURN:
		return "asleep";
	case PokemonGen1_STATUS_CONFUSED:
	case PokemonGen1_STATUS_CONFUSED_FOR_1_TURN:
	case PokemonGen1_STATUS_CONFUSED_FOR_2_TURN:
	case PokemonGen1_STATUS_CONFUSED_FOR_3_TURN:
	case PokemonGen1_STATUS_CONFUSED_FOR_4_TURN:
		return "confused";
	case PokemonGen1_STATUS_LEECHED:
		return "leeched by seeds";
	case PokemonGen1_STATUS_KO:
		return "KO";
	case PokemonGen1_STATUS_FLINCHED:
		return "flinched";
	default:
		return "???";
	}
}