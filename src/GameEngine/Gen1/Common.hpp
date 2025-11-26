//
// Created by PinkySmile on 26/11/2025.
//

#ifndef POKEAI_COMMON_HPP
#define POKEAI_COMMON_HPP


#include <array>
#include <functional>
#include "../Event.hpp"

namespace PokemonGen1
{
	typedef std::array<bool, 4> MovesDiscovered;
	typedef std::pair<bool, MovesDiscovered> PkmnDiscovered;
	typedef std::function<void (const PkmnCommon::Event &event)> BattleLogger;
}


#endif //POKEAI_COMMON_HPP
