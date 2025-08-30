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
	enum BattleAction : unsigned char {
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

	std::string BattleActionToString(BattleAction action);

	typedef std::array<bool, 4> MovesDiscovered;
	typedef std::pair<bool, MovesDiscovered> PkmnDiscovered;
	typedef std::function<void (const std::string &message)> BattleLogger;

	enum DesyncStrategy {
		DESYNC_MISS,  // Make moves miss when they would desync
		DESYNC_THROW, // Throw an exception when a move would desync
		DESYNC_INVERT,// Invert calculations to replicate the opponent's PoV and stay in sync
		DESYNC_IGNORE // Apply normal calculations even when it would result in a desync
	};

	struct PlayerState {
		std::string name;
		BattleAction lastAction = NoAction;
		BattleAction nextAction = NoAction;
		unsigned char pokemonOnField = 0;
		std::vector<Pokemon> team;
		std::array<PkmnDiscovered, 6> discovered;

		nlohmann::json serialize();
		void deserialize(const nlohmann::json &json, BattleState &state);
	};

	struct BattleState {
		PlayerState me;
		PlayerState op;
		RandomGenerator rng;
		DesyncStrategy desync = DESYNC_INVERT;
		unsigned short lastDamage = 0;
		BattleLogger battleLogger = [](const std::string &){};
		std::function<bool ()> onTurnStart;
		std::function<void ()> onBattleEnd;
		std::function<void ()> onBattleStart;

		nlohmann::json serialize();
		void deserialize(const nlohmann::json &json);
	};

#ifdef __PYX_EXTERN_C
	inline std::function<bool ()> pythonCallbackLambda(void *python_function, std::function<bool (void *)> eval)
	{
		return [=]() { return eval(python_function); };
	}

	inline std::function<void ()> pythonCallbackLambdaVoid(void *python_function, std::function<void (void *)> eval)
	{
		return [=]() { return eval(python_function); };
	}
#endif
}


#endif //POKEAI_STATE_HPP
