//
// Created by Gegel85 on 14/07/2019.
//

#ifndef POKEAI_MOVE_HPP
#define POKEAI_MOVE_HPP


#include <string>
#include <functional>
#include "PokemonTypes.hpp"
#include "StatusChange.hpp"
#include "StatsChange.hpp"


#define DEFAULT_MOVE(id) Move{id, "Move "#id, TYPE_INVALID, STATUS, 0, 0, 0, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, nullptr, "This move is invalid and will cause desync when used"}
#define NO_STATS_CHANGE {}, {}
#define DEFAULT_HITS {1, 1}
#define ONE_RUN DEFAULT_HITS, ""
#define TWO_TO_FIVE_HITS {2, 5}
#define DEFAULT_CRIT_CHANCE (1)
#define NO_STATUS_CHANGE {STATUS_NONE, 0}
#define NO_LOADING false, ""
#define NEED_LOADING(msg) true, msg

#define NO_CALLBACK nullptr, ""
#define NOT_IMPLEMENTED nullptr, "Not implemented"

//Miss callbacks
#define GLITCH_HYPER_BEAM [](Pokemon &, Pokemon &target, const std::function<void(const std::string &msg)> &){\
	target.setRecharging(false);\
	return true;\
}, "Removes the opponent recharge state and will make the target use it's move once more"

#define SUICIDE_MISS [](Pokemon &owner, Pokemon &, const std::function<void(const std::string &msg)> &){\
	owner.takeDamage(owner.getHealth());\
	return true;\
}, "Kills user"

#define TAKE_1DAMAGE [](Pokemon &owner, Pokemon &, const std::function<void(const std::string &msg)> &logger){\
	owner.takeDamage(1);\
	logger(owner.getName() + " crashes!");\
	return true;\
}, "Take 1 damage"


//Hit callbacks
#define OHKO_DESC "Kills in one hit if the user's speed is higher than the foe's"
#define ONE_HIT_KO_HANDLE [](Pokemon &owner, Pokemon &target, unsigned, bool, const std::function<void(const std::string &msg)> &logger){\
	if (owner.getSpeed() >= target.getSpeed()) {\
                target.takeDamage(target.getHealth());\
                logger("One-hit KO!");\
                return true;\
        }\
        return false;\
}, OHKO_DESC

#define QU_RECOIL_DESC "Take a quarter of the damage dealt as recoil"
#define TAKE_QUARTER_MOVE_DAMAGE [](Pokemon &owner, Pokemon &, unsigned damages, bool, const std::function<void(const std::string &msg)> &logger){\
	owner.takeDamage(damages / 4);\
	logger(owner.getName() + "'s hits with recoil!");\
	return true;\
}, QU_RECOIL_DESC

#define TRANSFORM_DESC "Transform the user into the foe, copying stats, types and sprite"
#define TRANSFORM [](Pokemon &owner, Pokemon &target, unsigned, bool, const std::function<void(const std::string &msg)> &logger){\
	owner.transform(target);\
	logger(owner.getName() + " transformed into " + target.getSpeciesName());\
	return true;\
}, TRANSFORM_DESC

#define TAKE_HALF_MOVE_DAMAGE_DESC "Take half dealt damage as recoil"
#define TAKE_HALF_MOVE_DAMAGE [](Pokemon &owner, Pokemon &, unsigned damages, bool, const std::function<void(const std::string &msg)> &logger){\
	owner.takeDamage(damages / 2);\
	logger(owner.getName() + "'s hits with recoil!");\
	return true;\
}, TAKE_HALF_MOVE_DAMAGE_DESC

#define WRAP_TARGET_DESC "Set the foe in the wrapped state for all the move duration"
#define WRAP_TARGET [](Pokemon &, Pokemon &target, unsigned, bool last, const std::function<void(const std::string &msg)> &){\
	target.setWrapped(!last);\
	return true;\
}, WRAP_TARGET_DESC

#define CONFUSE_ON_LAST_DESC "Confuse the user on last run"
#define CONFUSE_ON_LAST [](Pokemon &owner, Pokemon &, unsigned, bool last, const std::function<void(const std::string &msg)> &){\
	if (last)\
		owner.addStatus(STATUS_CONFUSED);\
	return true;\
}, CONFUSE_ON_LAST_DESC

#define DEAL_20_DAMAGE_DESC "Deal 20 damages"
#define DEAL_20_DAMAGE [](Pokemon &, Pokemon &target, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	target.takeDamage(20);\
	return true;\
}, DEAL_20_DAMAGE_DESC

#define DEAL_40_DAMAGE_DESC "Deal 40 damages"
#define DEAL_40_DAMAGE [](Pokemon &, Pokemon &target, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	target.takeDamage(40);\
	return true;\
}, DEAL_40_DAMAGE_DESC

#define DEAL_LVL_AS_DAMAGE_DESC "Deal the user's level as raw damages"
#define DEAL_LVL_AS_DAMAGE [](Pokemon &owner, Pokemon &target, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	if (!owner.getLevel())\
		return true;\
	target.takeDamage(owner.getLevel());\
	return true;\
}, DEAL_LVL_AS_DAMAGE_DESC

#define DEAL_0_5_TO_1_5_LEVEL_DAMAGE_DESC "Deal between 0.5 and 1.5 of the user's level as damage"
#define DEAL_0_5_TO_1_5_LEVEL_DAMAGE [](Pokemon &owner, Pokemon &target, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	if (!owner.getLevel())\
		return true;\
	target.takeDamage(owner.getLevel() * (100 * owner.getRandomGenerator()() / 255. + 50));\
	return true;\
}, DEAL_0_5_TO_1_5_LEVEL_DAMAGE_DESC

#define ABSORB_HALF_DAMAGE_DESC "Absorb half dealt damages"
#define ABSORB_HALF_DAMAGE [](Pokemon &owner, Pokemon &, unsigned damages, bool, const std::function<void(const std::string &msg)> &){\
	owner.takeDamage(-(damages / 2));\
	return true;\
}, ABSORB_HALF_DAMAGE_DESC

#define HEAL_HALF_HEALTH_DESC "Heal half max HP"
#define HEAL_HALF_HEALTH [](Pokemon &owner, Pokemon &, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	owner.takeDamage(-(owner.getMaxHealth() / 2));\
	return true;\
}, HEAL_HALF_HEALTH_DESC

#define HEAL_ALL_HEALTH_AND_SLEEP_DESC "Heal all lost HP and sleep for 2 turns"
#define HEAL_ALL_HEALTH_AND_SLEEP [](Pokemon &owner, Pokemon &, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	owner.takeDamage(-owner.getMaxHealth());\
	owner.setStatus(STATUS_ASLEEP, 2);\
	return true;\
}, HEAL_ALL_HEALTH_AND_SLEEP_DESC

#define CANCEL_STATS_CHANGE_DESC "Resets all stats, status and crit chance multiplier changes"
#define CANCEL_STATS_CHANGE [](Pokemon &owner, Pokemon &target, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	owner.resetStatsChanges();\
	owner.setStatus(STATUS_NONE);\
	owner.setGlobalCritRatio(1);\
	target.resetStatsChanges();\
	target.setStatus(STATUS_NONE);\
	target.setGlobalCritRatio(1);\
	return true;\
}, CANCEL_STATS_CHANGE_DESC

#define SET_USER_CRIT_RATIO_TO_1_QUARTER_DESC "User has 4 times less chance to crit"
#define SET_USER_CRIT_RATIO_TO_1_QUARTER [](Pokemon &owner, Pokemon &, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	owner.setGlobalCritRatio(0.25);\
	return true;\
}, SET_USER_CRIT_RATIO_TO_1_QUARTER_DESC

#define STORE_DAMAGES_DESC "Store damages"
#define STORE_DAMAGES [](Pokemon &owner, Pokemon &target, unsigned, bool last, const std::function<void(const std::string &msg)> &){\
	if (last)\
		target.takeDamage(owner.getDamagesStored() * 2);\
	owner.storeDamages(!last);\
	return true;\
}, STORE_DAMAGES_DESC

#define USE_RANDOM_MOVE_DESC "Use a randomly chosen move"
#define USE_RANDOM_MOVE [](Pokemon &owner, Pokemon &target, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	unsigned index;\
\
	do {\
		index = owner.getRandomGenerator()();\
	} while (!index || index >= 0xA5);\
	owner.useMove(availableMoves[index], target);\
	return true;\
}, USE_RANDOM_MOVE_DESC

#define USE_LAST_FOE_MOVE_DESC "Use last foe's used move"
#define USE_LAST_FOE_MOVE [](Pokemon &owner, Pokemon &target, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	owner.useMove(target.getLastUsedMove(), target);\
	return true;\
}, USE_LAST_FOE_MOVE_DESC

#define SUICIDE_DESC "Kill user"
#define SUICIDE [](Pokemon &owner, Pokemon &, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	owner.takeDamage(owner.getHealth());\
	return true;\
}, SUICIDE_DESC

#define CONVERSION_DESC "Copy foe's types"
#define CONVERSION [](Pokemon &owner, Pokemon &target, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	owner.setTypes(target.getTypes());\
	return true;\
}, CONVERSION_DESC

#define DEAL_HALF_HP_DAMAGE_DESC "Deal half foe's HP"
#define DEAL_HALF_HP_DAMAGE [](Pokemon &, Pokemon &target, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	target.takeDamage(target.getHealth() / 2);\
	return true;\
}, DEAL_HALF_HP_DAMAGE_DESC

#define MIMIC_MOVE_DESC "Use last foe's used move"
#define MIMIC_MOVE [](Pokemon &owner, Pokemon &target, unsigned, bool, const std::function<void(const std::string &msg)> &){\
	owner.useMove(target.getLastUsedMove(), target);\
	return true;\
}, MIMIC_MOVE_DESC

namespace PokemonGen1
{
	class Pokemon;

	enum MoveCategory {
		PHYSICAL,
		SPECIAL,
		STATUS,
	};

	class Move {
	public:
		struct StatusChangeProb {
			StatusChange	status;
			double		prob;
		};
		struct StatsChangeProb {
			StatsChange	stat;
			char		nb;
			double		prob;
		};

	private:
		std::function<bool (Pokemon &owner, Pokemon &target, unsigned damage, bool lastRun, const std::function<void(const std::string &msg)> &)> _hitCallback;
		std::function<bool (Pokemon &owner, Pokemon &target, const std::function<void(const std::string &msg)> &logger)> _missCallback;
		double _critChance;
		std::string _loadingMsg;
		std::string _keepGoingMsg;
		std::string _name;
		PokemonTypes _type;
		MoveCategory _category;
		unsigned int _power;
		unsigned char _id;
		unsigned char _pp;
		unsigned char _ppup;
		unsigned char _maxpp;
		unsigned char _nbHit;
		unsigned char _accuracy;
		std::pair<unsigned, unsigned> _nbRuns;
		std::pair<unsigned, unsigned> _nbHits;
		StatusChangeProb _statusChange;
		std::vector<StatsChangeProb> _ownerChange;
		std::vector<StatsChangeProb> _foeChange;
		char _priority;
		bool _needLoading;
		bool _invulnerableDuringLoading;
		bool _needRecharge;
		std::string _hitCallBackDescription;
		std::string _missCallBackDescription;
		std::string _fullDescription;

	public:
		Move(
			unsigned char id,
			const std::string &name,
			PokemonTypes type,
			MoveCategory category,
			unsigned int power,
			unsigned char accuracy,
			unsigned char maxpp,
			StatusChangeProb statusChange = NO_STATUS_CHANGE,
			std::vector<StatsChangeProb> ownerChange = {},
			std::vector<StatsChangeProb> foeChange = {},
			std::pair<unsigned, unsigned> nbHits = DEFAULT_HITS,
			std::pair<unsigned, unsigned> nbRuns = DEFAULT_HITS,
			const std::string &keepGoingMsg = "",
			char priority = 0,
			double critChance = DEFAULT_CRIT_CHANCE,
			bool needLoading = false,
			const std::string &loadingMsg = "",
			bool invulnerableDuringLoading = false,
			bool needRecharge = false,
			const std::function<bool(Pokemon &owner, Pokemon &target, unsigned damage, bool lastRun, const std::function<void(const std::string &msg)> &logger)> &&hitCallback = nullptr,
			const std::string &hitCallBackDescription = "",
			const std::function<bool(Pokemon &owner, Pokemon &target, const std::function<void(const std::string &msg)> &logger)> &&missCallback = nullptr,
			const std::string &missCallBackDescription = ""
		);
		Move(const Move &);

		Move &operator=(const Move &);

		const std::pair<unsigned int, unsigned int> &getNbRuns() const;
		const std::pair<unsigned int, unsigned int> &getNbHits() const;
		const StatusChangeProb &getStatusChange() const;
		const std::vector<StatsChangeProb> &getOwnerChange() const;
		const std::vector<StatsChangeProb> &getFoeChange() const;
		bool needsLoading() const;
		bool isInvulnerableDuringLoading() const;
		bool needsRecharge() const;
		const std::string &getHitCallBackDescription() const;
		const std::string &getMissCallBackDescription() const;
		double getCritChance() const;
		unsigned char getAccuracy() const;
		MoveCategory getCategory() const;
		bool makesInvulnerable() const;
		unsigned char getMaxPP() const;
		unsigned char getPower() const;
		unsigned char getPPUp() const;
		PokemonTypes getType() const;
		unsigned char getPP() const;
		unsigned char getID() const;
		std::string getName() const;
		const std::string &getDescription() const;
		char getPriority() const;
		bool isFinished() const;

		void glitch();
		void setPP(unsigned char pp);
		void setPPUp(unsigned char nb);

		bool attack(Pokemon &owner, Pokemon &target, const std::function<void(const std::string &msg)> &logger);
	};

	extern const std::vector<Move> availableMoves;

	/*
	** From Rhydon
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/PK1.cs#L532
	*/

	enum MoveType
	{
		None = 0x00,
		Pound = 0x01,
		Karate_Chop = 0x02,
		Doubleslap = 0x03,
		Comet_Punch = 0x04,
		Mega_Punch = 0x05,
		Pay_Day = 0x06,
		Fire_Punch = 0x07,
		Ice_Punch = 0x08,
		Thunderpunch = 0x09,
		Scratch = 0x0a,
		Vicegrip = 0x0b,
		Guillotine = 0x0c,
		Razor_Wind = 0x0d,
		Swords_Dance = 0x0e,
		Cut = 0x0f,
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
		Jump_Kick = 0x1a,
		Rolling_Kick = 0x1b,
		Sand_Attack = 0x1c,
		Headbutt = 0x1d,
		Horn_Attack = 0x1e,
		Fury_Attack = 0x1f,
		Horn_Drill = 0x20,
		Tackle = 0x21,
		Body_Slam = 0x22,
		Wrap = 0x23,
		Take_down = 0x24,
		Thrash = 0x25,
		Double_Edge = 0x26,
		Tail_Whip = 0x27,
		Poison_Sting = 0x28,
		Twineedle = 0x29,
		Pin_Missile = 0x2a,
		Leer = 0x2b,
		Bite = 0x2c,
		Growl = 0x2d,
		Roar = 0x2e,
		Sing = 0x2f,
		Supersonic = 0x30,
		Sonicboom = 0x31,
		Disable = 0x32,
		Acid = 0x33,
		Ember = 0x34,
		Flamethrower = 0x35,
		Mist = 0x36,
		Water_Gun = 0x37,
		Hydro_Pump = 0x38,
		Surf = 0x39,
		Ice_Beam = 0x3a,
		Blizzard = 0x3b,
		Psybeam = 0x3c,
		Bubblebeam = 0x3d,
		Aurora_Beam = 0x3e,
		Hyper_Beam = 0x3f,
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
		Growth = 0x4a,
		Razor_Leaf = 0x4b,
		Solarbeam = 0x4c,
		Poisonpowder = 0x4d,
		Stun_Spore = 0x4e,
		Sleep_Powder = 0x4f,
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
		Fissure = 0x5a,
		Dig = 0x5b,
		Toxic = 0x5c,
		Confusion = 0x5d,
		Psychic_M = 0x5e,
		Hypnosis = 0x5f,
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
		Harden = 0x6a,
		Minimize = 0x6b,
		Smokescreen = 0x6c,
		Confuse_Ray = 0x6d,
		Withdraw = 0x6e,
		Defense_Curl = 0x6f,
		Barrier = 0x70,
		Light_Screen = 0x71,
		Haze = 0x72,
		Reflect = 0x73,
		Focus_Energy = 0x74,
		Bide = 0x75,
		Metronome = 0x76,
		Mirror_Move = 0x77,
		Selfdestruct = 0x78,
		Egg_Bomb = 0x79,
		Lick = 0x7a,
		Smog = 0x7b,
		Sludge = 0x7c,
		Bone_Club = 0x7d,
		Fire_Blast = 0x7e,
		Waterfall = 0x7f,
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
		Dream_Eater = 0x8a,
		Poison_Gas = 0x8b,
		Barrage = 0x8c,
		Leech_Life = 0x8d,
		Lovely_Kiss = 0x8e,
		Sky_Attack = 0x8f,
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
		Fury_Swipes = 0x9a,
		Bonemerang = 0x9b,
		Rest = 0x9c,
		Rock_Slide = 0x9d,
		Hyper_Fang = 0x9e,
		Sharpen = 0x9f,
		Conversion = 0xa0,
		Tri_Attack = 0xa1,
		Super_Fang = 0xa2,
		Slash = 0xa3,
		Substitute = 0xa4,
		Struggle = 0xa5
	};
}


#endif //POKEAI_MOVE_HPP
