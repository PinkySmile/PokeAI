//
// Created by Gegel85 on 14/07/2019.
//

#ifndef POKEAI_STATUSCHANGE_HPP
#define POKEAI_STATUSCHANGE_HPP


#include <vector>
#include <string>

namespace PokemonGen1
{
	enum StatusChange {
		STATUS_NONE,
		STATUS_ASLEEP_FOR_1_TURN,
		STATUS_ASLEEP_FOR_2_TURN,
		STATUS_ASLEEP_FOR_3_TURN,
		STATUS_ASLEEP_FOR_4_TURN,
		STATUS_ASLEEP_FOR_5_TURN,
		STATUS_ASLEEP_FOR_6_TURN,
		STATUS_ASLEEP_FOR_7_TURN,
		STATUS_ASLEEP                  = 0b111,
		STATUS_POISONED                = 1U << 3U,
		STATUS_BURNED                  = 1U << 4U,
		STATUS_FROZEN                  = 1U << 5U,
		STATUS_PARALYZED               = 1U << 6U,
		STATUS_KO                      = 1U << 7U, //From here, it is not handled by the game
		STATUS_BADLY_POISONED          = (1U << 8U) | STATUS_POISONED,
		STATUS_ANY_NON_VOLATILE_STATUS = 0b11111111U,
		STATUS_LEECHED                 = 1U << 9U,
		STATUS_CONFUSED_FOR_1_TURN     = 1U << 10U,
		STATUS_CONFUSED_FOR_2_TURN     = 1U << 11U,
		STATUS_CONFUSED_FOR_3_TURN     = STATUS_CONFUSED_FOR_1_TURN | STATUS_CONFUSED_FOR_2_TURN,
		STATUS_CONFUSED_FOR_4_TURN     = 1U << 12U,
		STATUS_CONFUSED                = STATUS_CONFUSED_FOR_3_TURN | STATUS_CONFUSED_FOR_4_TURN,
		STATUS_FLINCHED                = 1U << 13U,
	};

	std::string statusToString(StatusChange stat);
}

#endif //POKEAI_STATUSCHANGE_HPP
