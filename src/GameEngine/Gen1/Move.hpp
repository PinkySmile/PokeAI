//
// Created by PinkySmile on 14/07/2019.
//

#ifndef POKEAI_MOVE_HPP
#define POKEAI_MOVE_HPP


#include <string>
#include <algorithm>
#include <functional>
#include "Type.hpp"
#include "StatusChange.hpp"
#include "StatsChange.hpp"
#include "Damage.hpp"
#include "Common.hpp"


#define NO_STATUS_CHANGE {STATUS_NONE, 0}
#define NO_STATS_CHANGE {}, {}
#define DEFAULT_HITS {1, 1}
#define DEFAULT_CRIT_CHANCE (1)

#define WRAP_TARGET_DESC "Set the foe in the wrapped state for all the move duration"

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
			StatusChange status;
			unsigned char cmpVal;
		};
		struct StatsChangeProb {
			StatsChange   stat;
			char          nb;
			unsigned char cmpVal;
		};

		typedef std::function<bool (unsigned id, Pokemon &owner, Pokemon &target, unsigned damage, bool lastRun, const BattleLogger &logger)> HitCallback;
		typedef std::function<bool (unsigned id, Pokemon &owner, Pokemon &target,                  bool lastRun, const BattleLogger &logger)> MissCallback;

	private:
		HitCallback _canHitCallback;
		HitCallback _hitCallback;
		MissCallback _missCallback;
		unsigned _critChance = 1;
		std::string _loadingMsg;
		std::string _keepGoingMsg;
		std::string _name;
		Type _type;
		MoveCategory _category;
		unsigned char _power;
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
		bool _skipAccuracyCheck;
		bool _needLoading;
		bool _invulnerableDuringLoading;
		bool _needRecharge;
		std::string _hitCallBackDescription;
		std::string _missCallBackDescription;
		std::string _fullDescription;

	public:
		bool wasReplaced = false;

		Move(
			unsigned char id,
			const std::string &name,
			Type type,
			MoveCategory category,
			unsigned char power,
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
			const HitCallback &&canHitCallback = nullptr,
			const HitCallback &&hitCallback = nullptr,
			const std::string &hitCallBackDescription = "",
			const MissCallback &&missCallback = nullptr,
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
		unsigned int getPower() const;
		unsigned char getPPUp() const;
		Type getType() const;
		unsigned char getPP() const;
		unsigned char getID() const;
		std::string getName() const;
		const std::string &getDescription() const;
		char getPriority() const;
		bool isFinished() const;
		unsigned char getHitsLeft() const;
		bool skipAccuracyCheck() const;

		void glitch();
		void setPP(unsigned char pp);
		void setPPUp(unsigned char nb);
		void setHitsLeft(unsigned char nb);
		void reset();

		bool attack(Pokemon &owner, Pokemon &target, const BattleLogger &logger);
	};

	extern const std::array<Move, 256> availableMoves;

	/*
	** From Rhydon
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/PK1.cs#L532
	*/

	enum AvailableMove
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
}


#endif //POKEAI_MOVE_HPP
