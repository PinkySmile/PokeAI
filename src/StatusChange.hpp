//
// Created by Gegel85 on 14/07/2019.
//

#ifndef POKEAI_STATUSCHANGE_HPP
#define POKEAI_STATUSCHANGE_HPP


#include <vector>
#include <string>

enum StatusChange {
	STATUS_NONE		= 0,
	STATUS_PARALYZED	= 1U << 0U,
	STATUS_BURNED		= 1U << 1U,
	STATUS_FROZEN		= 1U << 2U,
	STATUS_POISONED		= 1U << 3U,
	STATUS_BADLY_POISONED	= 1U << 4U,
	STATUS_ASLEEP		= 1U << 5U,
	STATUS_CONFUSED		= 1U << 6U,
	STATUS_LEECHED		= 1U << 7U,
};

std::string statusToString(StatusChange stat);

#endif //POKEAI_STATUSCHANGE_HPP
