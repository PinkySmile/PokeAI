//
// Created by Gegel85 on 21/07/2019.
//

#include "StatusChange.hpp"

std::string statusToString(StatusChange status)
{
	switch (status) {
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
	case STATUS_ASLEEP:
		return "asleep";
	case STATUS_CONFUSED:
		return "confused";
	case STATUS_LEECHED:
		return "leeched by seeds";
	default:
		return "???";
	}
}