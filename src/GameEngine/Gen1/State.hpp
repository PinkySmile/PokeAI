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
		EmptyAction,
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
		NoAction = 0x6D,
		StruggleMove,
		Run,
	};

	std::string BattleActionToString(BattleAction action);

	typedef std::array<bool, 4> MovesDiscovered;
	typedef std::pair<bool, MovesDiscovered> PkmnDiscovered;
	typedef std::function<void (const std::string &message)> BattleLogger;

	enum DesyncPolicy {  // What to do when a move would desyncs
		DESYNC_MISS,  // Make the move miss
		DESYNC_THROW, // Throw an exception
		DESYNC_INVERT,// Invert calculations (replicate the opponent's PoV)
		DESYNC_IGNORE // Apply normal calculations
	};

	enum BadActionPolicy {    // What to do when an invalid aciton is used
		BADACTION_IGNORE,  // Perform the action anyway; May crash
		BADACTION_STRUGGLE,// Replace the action by StruggleMove
		BADACTION_NOACTION,// Replace the action with NoAction
		BADACTION_FIX,     // Try to find a suitable replacement and do that instead. Throw if no action can be performed.
		BADACTION_THROW    // Throw an exception
	};

	struct PlayerState {
		std::string name;
		BattleAction lastAction = EmptyAction;
		BattleAction nextAction = EmptyAction;
		AvailableMove lastAttack = None;
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
		DesyncPolicy desync = DESYNC_INVERT;
		BadActionPolicy badAction = BADACTION_THROW;
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
