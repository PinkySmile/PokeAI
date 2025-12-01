//
// Created by PinkySmile on 22/11/2025.
//

#ifndef POKEAI_EVENT_HPP
#define POKEAI_EVENT_HPP


#include <variant>
#include <string>

namespace PkmnCommon
{
	struct TextEvent {
		std::string message;
	};

	struct MoveEvent {
		unsigned moveId;
		bool player;
	};

	enum SystemAnim {
		SYSANIM_ASLEEP,
		SYSANIM_BURN,
		SYSANIM_FROZEN,
		SYSANIM_POISON,
		SYSANIM_BAD_POISON,
		SYSANIM_CONFUSED,
		SYSANIM_FLINCHED,
		SYSANIM_PARALYZED,
		SYSANIM_CONFUSED_HIT,
		SYSANIM_LEECHED,

		SYSANIM_ATK_DECREASE_BIG, // Attack
		SYSANIM_ATK_DECREASE,
		SYSANIM_ATK_INCREASE,
		SYSANIM_ATK_INCREASE_BIG,
		SYSANIM_DEF_DECREASE_BIG, // Defense
		SYSANIM_DEF_DECREASE,
		SYSANIM_DEF_INCREASE,
		SYSANIM_DEF_INCREASE_BIG,
		SYSANIM_SPA_DECREASE_BIG, // Special Attack
		SYSANIM_SPA_DECREASE,
		SYSANIM_SPA_INCREASE,
		SYSANIM_SPA_INCREASE_BIG,
		SYSANIM_SPD_DECREASE_BIG, // Special Defense
		SYSANIM_SPD_DECREASE,
		SYSANIM_SPD_INCREASE,
		SYSANIM_SPD_INCREASE_BIG,
		SYSANIM_SPE_DECREASE_BIG, // Speed
		SYSANIM_SPE_DECREASE,
		SYSANIM_SPE_INCREASE,
		SYSANIM_SPE_INCREASE_BIG,
		SYSANIM_ACC_DECREASE_BIG, // Accuracy
		SYSANIM_ACC_DECREASE,
		SYSANIM_ACC_INCREASE,
		SYSANIM_ACC_INCREASE_BIG,
		SYSANIM_EVD_DECREASE_BIG, // Evade
		SYSANIM_EVD_DECREASE,
		SYSANIM_EVD_INCREASE,
		SYSANIM_EVD_INCREASE_BIG,

		SYSANIM_RECHARGE,
		SYSANIM_SUB_BREAK,

		SYSANIM_NOW_ASLEEP,
		SYSANIM_NOW_FROZEN,
		SYSANIM_NOW_BURNED,
		SYSANIM_NOW_POISONED,
		SYSANIM_NOW_PARALYZED,
		SYSANIM_NOW_BADLY_POISONED,
		SYSANIM_NOW_CONFUSED,
	};

	struct AnimEvent {
		unsigned animId;
		bool isGuaranteed;
		bool player;
		bool turn;
	};

	struct ExtraAnimEvent {
		unsigned moveId;
		unsigned index;
		bool player;
	};

	struct HealthModEvent {
		unsigned newHealth;
		bool player;
	};

	struct SwitchEvent {
		unsigned newPkmnId;
		bool player;
	};

	struct WithdrawEvent {
		bool player;
	};

	struct DeathEvent {
		bool player;
	};

	struct HitEvent {
		bool veryEffective;
		bool notVeryEffective;
		bool player;
	};

	struct GameStartEvent {
	};

	struct GameEndEvent {
		bool p1Won;
		bool p2Won;
	};

	struct TurnStartEvent {};

	struct StatusClearedEvent {
		bool player;
	};

	typedef std::variant<
	        TextEvent,
		MoveEvent,
		AnimEvent,
		ExtraAnimEvent,
		HealthModEvent,
		SwitchEvent,
		WithdrawEvent,
		DeathEvent,
		HitEvent,
		StatusClearedEvent,
		TurnStartEvent,
		GameStartEvent,
		GameEndEvent
	> Event;
}


#endif //POKEAI_EVENT_HPP
