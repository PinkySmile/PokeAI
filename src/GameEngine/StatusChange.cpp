//
// Created by Gegel85 on 21/07/2019.
//

#include "StatusChange.hpp"

namespace PokemonGen1
{
	std::string statusToString(StatusChange status)
	{
		switch (status) {
		case STATUS_NONE:
			return "OK";
		case STATUS_PARALYZED:
			return "paralyzed";
		case STATUS_BURNED:
			return "burned";
		case STATUS_FROZEN:
			return "frozen";
		case STATUS_POISONED:
			return "poisoned";
		case STATUS_BADLY_POISONED:
			return "badly poisoned";
		case STATUS_ASLEEP_FOR_1_TURN:
		case STATUS_ASLEEP_FOR_2_TURN:
		case STATUS_ASLEEP_FOR_3_TURN:
		case STATUS_ASLEEP_FOR_4_TURN:
		case STATUS_ASLEEP_FOR_5_TURN:
		case STATUS_ASLEEP_FOR_6_TURN:
		case STATUS_ASLEEP_FOR_7_TURN:
			return "asleep";
		case STATUS_CONFUSED_FOR_1_TURN:
		case STATUS_CONFUSED_FOR_2_TURN:
		case STATUS_CONFUSED_FOR_3_TURN:
		case STATUS_CONFUSED_FOR_4_TURN:
			return "confused";
		case STATUS_LEECHED:
			return "leeched by seeds";
		case STATUS_KO:
			return "KO";
		default:
			return "???";
		}
	}
}