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
	};

	struct ExtraAnimEvent {
		unsigned moveId;
		unsigned index;
		bool player;
	};

	struct HealthModEvent {
		int newHealth;
		bool player;
	};

	struct SwitchEvent {
		unsigned newPkmnId;
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

	typedef std::variant<TextEvent, MoveEvent, AnimEvent, ExtraAnimEvent, HealthModEvent, SwitchEvent, DeathEvent, HitEvent, GameStartEvent, GameEndEvent> Event;
}


#endif //POKEAI_EVENT_HPP
