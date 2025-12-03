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

	enum MoveIndex {
		None = 0x00,
		Pound = 0x01,
		Karate_Chop = 0x02,
		Doubleslap = 0x03,
		Comet_Punch = 0x04,
		Mega_Punch = 0x05,
		Pay_Day = 0x06,
		Fire_Punch = 0x07,
		Ice_Punch = 0x08,
		Thunder_Punch = 0x09,
		Scratch = 0x0A,
		Vise_Grip = 0x0B,
		Guillotine = 0x0C,
		Razor_Wind = 0x0D,
		Swords_Dance = 0x0E,
		Cut = 0x0F,
		Gust = 0x10,
		Wing_Attack = 0x11,
		Whirlwind = 0x12,
		Fly = 0x13,
		Bind = 0x14,
		Slam = 0x15,
		Vine_Whip = 0x16,
		Stomp = 0x17,
		Double_Kick = 0x18,
		Mega_Kick = 0x19,
		Jump_Kick = 0x1A,
		Rolling_Kick = 0x1B,
		Sand_Attack = 0x1C,
		Headbutt = 0x1D,
		Horn_Attack = 0x1E,
		Fury_Attack = 0x1F,
		Horn_Drill = 0x20,
		Tackle = 0x21,
		Body_Slam = 0x22,
		Wrap = 0x23,
		Take_Down = 0x24,
		Thrash = 0x25,
		Double_Edge = 0x26,
		Tail_Whip = 0x27,
		Poison_Sting = 0x28,
		Twineedle = 0x29,
		Pin_Missile = 0x2A,
		Leer = 0x2B,
		Bite = 0x2C,
		Growl = 0x2D,
		Roar = 0x2E,
		Sing = 0x2F,
		Supersonic = 0x30,
		Sonic_Boom = 0x31,
		Disable = 0x32,
		Acid = 0x33,
		Ember = 0x34,
		Flamethrower = 0x35,
		Mist = 0x36,
		Water_Gun = 0x37,
		Hydro_Pump = 0x38,
		Surf = 0x39,
		Ice_Beam = 0x3A,
		Blizzard = 0x3B,
		Psybeam = 0x3C,
		Bubblebeam = 0x3D,
		Aurora_Beam = 0x3E,
		Hyper_Beam = 0x3F,
		Peck = 0x40,
		Drill_Peck = 0x41,
		Submission = 0x42,
		Low_Kick = 0x43,
		Counter = 0x44,
		Seismic_Toss = 0x45,
		Strength = 0x46,
		Absorb = 0x47,
		Mega_Drain = 0x48,
		Leech_Seed = 0x49,
		Growth = 0x4A,
		Razor_Leaf = 0x4B,
		Solarbeam = 0x4C,
		Poisonpowder = 0x4D,
		Stun_Spore = 0x4E,
		Sleep_Powder = 0x4F,
		Petal_Dance = 0x50,
		String_Shot = 0x51,
		Dragon_Rage = 0x52,
		Fire_Spin = 0x53,
		Thundershock = 0x54,
		Thunderbolt = 0x55,
		Thunder_Wave = 0x56,
		Thunder = 0x57,
		Rock_Throw = 0x58,
		Earthquake = 0x59,
		Fissure = 0x5A,
		Dig = 0x5B,
		Toxic = 0x5C,
		Confusion = 0x5D,
		Psychic_M = 0x5E,
		Hypnosis = 0x5F,
		Meditate = 0x60,
		Agility = 0x61,
		Quick_Attack = 0x62,
		Rage = 0x63,
		Teleport = 0x64,
		Night_Shade = 0x65,
		Mimic = 0x66,
		Screech = 0x67,
		Double_Team = 0x68,
		Recover = 0x69,
		Harden = 0x6A,
		Minimize = 0x6B,
		Smokescreen = 0x6C,
		Confuse_Ray = 0x6D,
		Withdraw = 0x6E,
		Defense_Curl = 0x6F,
		Barrier = 0x70,
		Light_Screen = 0x71,
		Haze = 0x72,
		Reflect = 0x73,
		Focus_Energy = 0x74,
		Bide = 0x75,
		Metronome = 0x76,
		Mirror_Move = 0x77,
		Self_Destruct = 0x78,
		Egg_Bomb = 0x79,
		Lick = 0x7A,
		Smog = 0x7B,
		Sludge = 0x7C,
		Bone_Club = 0x7D,
		Fire_Blast = 0x7E,
		Waterfall = 0x7F,
		Clamp = 0x80,
		Swift = 0x81,
		Skull_Bash = 0x82,
		Spike_Cannon = 0x83,
		Constrict = 0x84,
		Amnesia = 0x85,
		Kinesis = 0x86,
		Softboiled = 0x87,
		Hi_Jump_Kick = 0x88,
		Glare = 0x89,
		Dream_Eater = 0x8A,
		Poison_Gas = 0x8B,
		Barrage = 0x8C,
		Leech_Life = 0x8D,
		Lovely_Kiss = 0x8E,
		Sky_Attack = 0x8F,
		Transform = 0x90,
		Bubble = 0x91,
		Dizzy_Punch = 0x92,
		Spore = 0x93,
		Flash = 0x94,
		Psywave = 0x95,
		Splash = 0x96,
		Acid_Armor = 0x97,
		Crabhammer = 0x98,
		Explosion = 0x99,
		Fury_Swipes = 0x9A,
		Bonemerang = 0x9B,
		Rest = 0x9C,
		Rock_Slide = 0x9D,
		Hyper_Fang = 0x9E,
		Sharpen = 0x9F,
		Conversion = 0xA0,
		Tri_Attack = 0xA1,
		Super_Fang = 0xA2,
		Slash = 0xA3,
		Substitute = 0xA4,
		Struggle = 0xA5
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

		SYSANIM_ATK_DECREASE_BIG, // Attack (10)
		SYSANIM_ATK_DECREASE,
		SYSANIM_ATK_INCREASE,
		SYSANIM_ATK_INCREASE_BIG,
		SYSANIM_DEF_DECREASE_BIG, // Defense (14)
		SYSANIM_DEF_DECREASE,
		SYSANIM_DEF_INCREASE,
		SYSANIM_DEF_INCREASE_BIG,
		SYSANIM_SPA_DECREASE_BIG, // Special Attack (18)
		SYSANIM_SPA_DECREASE,
		SYSANIM_SPA_INCREASE,
		SYSANIM_SPA_INCREASE_BIG,
		SYSANIM_SPD_DECREASE_BIG, // Special Defense (22)
		SYSANIM_SPD_DECREASE,
		SYSANIM_SPD_INCREASE,
		SYSANIM_SPD_INCREASE_BIG,
		SYSANIM_SPE_DECREASE_BIG, // Speed (26)
		SYSANIM_SPE_DECREASE,
		SYSANIM_SPE_INCREASE,
		SYSANIM_SPE_INCREASE_BIG,
		SYSANIM_ACC_DECREASE_BIG, // Accuracy (30)
		SYSANIM_ACC_DECREASE,
		SYSANIM_ACC_INCREASE,
		SYSANIM_ACC_INCREASE_BIG,
		SYSANIM_EVD_DECREASE_BIG, // Evade (34)
		SYSANIM_EVD_DECREASE,
		SYSANIM_EVD_INCREASE,
		SYSANIM_EVD_INCREASE_BIG,

		SYSANIM_RECHARGE, // (38)
		SYSANIM_SUB_BREAK,
		SYSANIM_WAKE_UP,
		SYSANIM_BACK_TO_SENSE,
		SYSANIM_THAWED,

		SYSANIM_NOW_ASLEEP, // (43)
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
		bool p1Ran;
		bool p2Ran;
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
