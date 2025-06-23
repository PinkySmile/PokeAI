//
// Created by PinkySmile on 21/06/25.
//

#ifndef POKEAI_STATE_HPP
#define POKEAI_STATE_HPP


#include <string>
#include <vector>
#include <nlohmann/json_fwd.hpp>
#include "Pokemon.hpp"

namespace PokemonGen1
{
	enum BattleAction {
		NoAction,
		Attack1 = 0x60,
		Attack2,
		Attack3,
		Attack4,
		Switch1,
		Switch2,
		Switch3,
		Switch4,
		Switch5,
		Switch6,
		StruggleMove = 0x6E,
		Run,
	};

	typedef std::array<bool, 4> MovesDiscovered;
	typedef std::pair<bool, MovesDiscovered> PkmnDiscovered;
	typedef std::function<void (const std::string &message)> BattleLogger;

	struct PlayerState {
		std::string name;
		BattleAction lastAction;
		BattleAction nextAction;
		unsigned char pokemonOnField;
		std::vector<Pokemon> team;
		std::array<PkmnDiscovered, 6> discovered;

		nlohmann::json serialize();
		void deserialize(const nlohmann::json &json, PokemonRandomGenerator &rng, const BattleLogger &logger);
	};

	struct BattleState {
		PlayerState me;
		PlayerState op;
		PokemonRandomGenerator rng;
		BattleLogger battleLogger;
		std::function<bool ()> onTurnStart;
		std::function<void ()> onBattleEnd;
		std::function<void ()> onBattleStart;

		nlohmann::json serialize();
		void deserialize(const nlohmann::json &json);
	};
}


#endif //POKEAI_STATE_HPP
