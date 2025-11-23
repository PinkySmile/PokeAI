//
// Created by PinkySmile on 22/11/2025.
//

#ifndef POKEAI_EVENT_HPP
#define POKEAI_EVENT_HPP


#include <variant>
#include <string>

namespace Pokemon {
	enum EventType {
		EVENTTYPE_TEXT,
		EVENTTYPE_MOVE,
		EVENTTYPE_ANIM,
		EVENTTYPE_EXTRAANIM,
		EVENTTYPE_HEALTH,
		EVENTTYPE_SWITCH,
		EVENTTYPE_DEATH,
		EVENTTYPE_HIT,
	};

	struct TextEvent {
		std::string message;
	};

	struct MoveEvent {
		unsigned moveId;
		bool player;
	};

	enum SystemAnim {
		SYSANIM_ASLEEP,
		SYSANIM_BRUN,
		SYSANIM_POISON,
		SYSANIM_BAD_POISON,
		SYSANIM_CONFUSED,
		SYSANIM_CONFUSED_HIT,
		SYSANIM_LEACHED,
		SYSANIM_SUB_BREAK
	};

	struct AnimEvent {
		unsigned animId;
		bool player;
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
