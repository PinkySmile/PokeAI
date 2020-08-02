//
// Created by Gegel85 on 14/07/2019.
//

#include "Pokemon.hpp"
#include "Move.hpp"

namespace PokemonGen1
{
	Move::Move(
		unsigned char id,
		const std::string &name,
		PokemonTypes type,
		MoveCategory category,
		unsigned int power,
		unsigned char accuracy,
		unsigned char maxpp,
		StatusChangeProb statusChange,
		std::vector<StatsChangeProb> ownerChange,
		std::vector<StatsChangeProb> foeChange,
		std::pair<unsigned, unsigned> nbHits,
		std::pair<unsigned, unsigned> nbRuns,
		const std::string &keepGoingMsg,
		char priority,
		double critChance,
		bool needLoading,
		const std::string &loadingMsg,
		bool invulnerableDuringLoading,
		bool needRecharge,
		const std::function<bool(Pokemon &owner, Pokemon &target, unsigned damage, bool lastRun, const std::function<void(const std::string &msg)> &logger)> &&hitCallback,
		const std::function<bool(Pokemon &owner, Pokemon &target, const std::function<void(const std::string &msg)> &logger)> &&missCallback
	) :
		_hitCallback(hitCallback),
		_missCallback(missCallback),
		_critChance(critChance),
		_loadingMsg(loadingMsg),
		_keepGoingMsg(keepGoingMsg),
		_name(name),
		_type(type),
		_category(category),
		_power(power),
		_id(id),
		_pp(maxpp),
		_ppup(0),
		_maxpp(maxpp),
		_nbHit(0),
		_accuracy(accuracy),
		_nbRuns(nbRuns),
		_nbHits(nbHits),
		_statusChange(statusChange),
		_ownerChange(ownerChange),
		_foeChange(foeChange),
		_priority(priority),
		_needLoading(needLoading),
		_invulnerableDuringLoading(invulnerableDuringLoading),
		_needRecharge(needRecharge)
	{}


	bool Move::isFinished() const
	{
		return this->_nbHit == 0;
	}

	unsigned char Move::getMaxPP() const
	{
		return this->_maxpp * (5 + this->_ppup) / 5;
	}

	void Move::setPPUp(unsigned char nb)
	{
		this->_ppup = nb;
	}

	unsigned char Move::getPP() const
	{
		return this->_pp;
	}

	void Move::setPP(unsigned char pp)
	{
		this->_pp = pp;
	}

	unsigned char Move::getID() const
	{
		return this->_id;
	}

	unsigned char Move::getPPUp() const
	{
		return this->_ppup;
	}

	std::string Move::getName() const
	{
		return this->_name;
	}

	bool Move::makesInvulnerable() const
	{
		return this->_invulnerableDuringLoading;
	}

	void Move::glitch()
	{
		this->_nbHit = 2;
	}

	bool Move::attack(Pokemon &owner, Pokemon &target, const std::function<void(const std::string &msg)> &logger)
	{
		double multiplier = target.getEvasion() * owner.getAccuracy();
		std::string msg = this->_keepGoingMsg;
		unsigned hits = 0;

		if (!this->_nbHit) {
			if (this->_nbRuns.second == this->_nbRuns.first)
				this->_nbHit = this->_nbRuns.first;
			else if (this->_nbRuns.second - 1 == this->_nbRuns.first)
				this->_nbHit = this->_nbRuns.first + (owner.getRandomGenerator()() & 1U);
			else if (this->_nbRuns.second - 3 == this->_nbRuns.first) {
				this->_nbHit = owner.getRandomGenerator()() & 3U;
				if (this->_nbHit >= 2)
					this->_nbHit = owner.getRandomGenerator()() & 3U;
				this->_nbHit += this->_nbRuns.first;
			}
			if (this->_needLoading) {
				logger(owner.getName() + " " + this->_loadingMsg);
				return true;
			}
			msg = "";
		}

		this->_nbHit--;

		if (!msg.empty())
			logger(owner.getName() + msg);
		else
			logger(owner.getName() + " used " + this->_name);

		if (getAttackDamageMultiplier(this->_type, target.getTypes()) == 0) {
			logger("It doesn't affect " + target.getName());
			if (this->_missCallback)
				this->_missCallback(owner, target, logger);
			return true;
		}

		if (!target.canGetHitBy(this->_id)) {
			if (this->_missCallback)
				this->_missCallback(owner, target, logger);
			return false;
		}

		Pokemon::DamageResult damages{
			.critical = false,
			.damages = 0,
			.affect = true,
			.isVeryEffective = false,
			.isNotVeryEffective = false,
		};

		if (this->_power) {
			bool critical = (target.getRandomGenerator()() < (pokemonList[owner.getID()].SPD / 2 * this->_critChance));

			damages = owner.calcDamage(target, this->_power, this->_type, this->_category, critical);
		}

		if (this->_accuracy <= 100) {
			unsigned random = owner.getRandomGenerator()();

			if ((random / 2.55 >= this->_accuracy * multiplier || random == 255)) {
				this->_nbHit = 0;
				if (this->_missCallback)
					this->_missCallback(owner, target, logger);
				return false;
			} else if (!this->_nbHit)
				owner.setRecharging(this->_needRecharge);
		}

		if (!damages.affect)
			logger("It doesn't affect " + target.getName());

		if (damages.critical)
			logger("Critical hit !");

		if (damages.isNotVeryEffective)
			logger("It's not very effective");

		if (damages.isVeryEffective)
			logger("It's super effective");

		if (this->_nbHits.second == this->_nbHits.first)
			hits = this->_nbHits.first;
		else if (this->_nbHits.second - 1 == this->_nbHits.first)
			hits = this->_nbHits.first + (owner.getRandomGenerator()() & 1U);
		else if (this->_nbHits.second - 3 == this->_nbHits.first) {
			hits = owner.getRandomGenerator()() & 3U;
			if (hits >= 2)
				hits= owner.getRandomGenerator()() & 3U;
			hits += this->_nbRuns.first;
		}

		if (hits > 1)
			logger("Hits " + std::to_string(hits) + " times !");

		while (hits > 0) {
			if (this->_power)
				target.takeDamage(damages.damages);

			if (this->_statusChange.prob && owner.getRandomGenerator()() < this->_statusChange.prob * 256)
				target.addStatus(this->_statusChange.status);

			if (this->_hitCallback)
				return this->_hitCallback(owner, target, damages.damages, this->isFinished(), logger);
			hits--;
		}

		for (const auto &val : this->_foeChange)
			if (owner.getRandomGenerator()() < val.prob * 256)
				target.changeStat(val.stat, val.nb);

		for (const auto &val : this->_ownerChange)
			if (owner.getRandomGenerator()() < val.prob * 256)
				owner.changeStat(val.stat, val.nb);

		return true;
	}

	MoveCategory Move::getCategory() const
	{
		return this->_category;
	}

	unsigned char Move::getAccuracy() const
	{
		return this->_accuracy;
	}

	PokemonTypes Move::getType() const
	{
		return this->_type;
	}

	unsigned char Move::getPower() const
	{
		return this->_power;
	}

	char Move::getPriority() const
	{
		return this->_priority;
	}

	/*
	** From pokemondb, bulbapedia and Rhydon
	** https://pokemondb.net/move/generation/1
	** https://bulbapedia.bulbagarden.net
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Tables.cs#L145
	*/

	//This is only relevant for Gen 1 (Some moves changed)
	const std::vector<Move> availableMoves{
		Move{0x00, "-"           , TYPE_NORMAL  , PHYSICAL,   0,   0, 0},
		Move{0x01, "Pound"       , TYPE_NORMAL  , PHYSICAL,  40, 100, 35},
		Move{0x02, "Karate Chop" , TYPE_NORMAL  , PHYSICAL,  50, 100, 25, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE * 8},
		Move{0x03, "DoubleSlap"  , TYPE_NORMAL  , PHYSICAL,  15,  85, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x04, "Comet Punch" , TYPE_NORMAL  , PHYSICAL,  18,  85, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x05, "Mega Punch"  , TYPE_NORMAL  , PHYSICAL,  80,  85, 20},
		Move{0x06, "Pay Day"     , TYPE_NORMAL  , PHYSICAL,  40, 100, 20},
		Move{0x07, "Fire Punch"  , TYPE_FIRE    , SPECIAL,   75, 100, 15, {STATUS_BURNED, 0.1}},
		Move{0x08, "Ice Punch"   , TYPE_ICE     , SPECIAL,   75, 100, 15, {STATUS_FROZEN, 0.1}},
		Move{0x09, "ThunderPunch", TYPE_ELECTRIC, SPECIAL,   75, 100, 15, {STATUS_PARALYZED, 0.1}},
		Move{0x0A, "Scratch"     , TYPE_NORMAL  , PHYSICAL,  40, 100, 35},
		Move{0x0B, "ViceGrip"    , TYPE_NORMAL  , PHYSICAL,  55, 100, 30},
		Move{0x0C, "Guillotine"  , TYPE_NORMAL  , PHYSICAL,   0,  30,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ONE_HIT_KO_HANDLE},
		Move{0x0D, "Razor Wind"  , TYPE_NORMAL  , PHYSICAL,  80,  75, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NEED_LOADING("whipped up a whirlwind")},
		Move{0x0E, "Swords Dance", TYPE_NORMAL  , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, {{STATS_ATK, 2, 1}}},
		Move{0x0F, "Cut"         , TYPE_NORMAL  , PHYSICAL,  50,  95, 30},
		Move{0x10, "Gust"        , TYPE_NORMAL  , SPECIAL ,  40, 100, 35},
		Move{0x11, "Wing Attack" , TYPE_FLYING  , PHYSICAL,  35, 100, 35},
		Move{0x12, "Whirlwind"   , TYPE_NORMAL  , STATUS  ,   0, 100, 20},
		Move{0x13, "Fly"         , TYPE_FLYING  , PHYSICAL,  90,  95, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NEED_LOADING("flew up high"), true},
		Move{0x14, "Bind"        , TYPE_NORMAL  , PHYSICAL,  15,  75, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, TWO_TO_FIVE_HITS, "'s attack continues!", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, WRAP_TARGET, GLITCH_HYPER_BEAM},
		Move{0x15, "Slam"        , TYPE_NORMAL  , PHYSICAL,  80,  75, 20},
		Move{0x16, "Vine Whip"   , TYPE_GRASS   , PHYSICAL,  35, 100, 25},
		Move{0x17, "Stomp"       , TYPE_NORMAL  , PHYSICAL,  65, 100, 20},
		Move{0x18, "Double Kick" , TYPE_FIGHTING, PHYSICAL,  65, 100, 20},
		Move{0x19, "Mega Kick"   , TYPE_NORMAL  , PHYSICAL, 120,  75,  5},
		Move{0x1A, "Jump Kick"   , TYPE_FIGHTING, PHYSICAL,  70,  95, 25, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, nullptr, TAKE_1DAMAGE},
		Move{0x1B, "Rolling Kick", TYPE_FIGHTING, PHYSICAL,  60,  85, 15},
		Move{0x1C, "Sand-Attack" , TYPE_NORMAL  , STATUS  ,   0, 100, 15},
		Move{0x1D, "Headbutt"    , TYPE_NORMAL  , PHYSICAL,  70, 100, 15},
		Move{0x1E, "Horn Attack" , TYPE_NORMAL  , PHYSICAL,  65, 100, 25},
		Move{0x1F, "Fury Attack" , TYPE_NORMAL  , PHYSICAL,  15,  85, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x20, "Horn Drill"  , TYPE_NORMAL  , PHYSICAL,   0,  30,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ONE_HIT_KO_HANDLE},
		Move{0x21, "Tackle"      , TYPE_NORMAL  , PHYSICAL,  35,  95, 35},
		Move{0x22, "Body Slam"   , TYPE_NORMAL  , PHYSICAL,  85, 100, 15},
		Move{0x23, "Wrap"        , TYPE_NORMAL  , PHYSICAL,  15,  90, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, TWO_TO_FIVE_HITS, "'s attack continues!", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, WRAP_TARGET, GLITCH_HYPER_BEAM},
		Move{0x24, "Take down"   , TYPE_NORMAL  , PHYSICAL,  90,  85, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, TAKE_QUATER_MOVE_DAMAGE},
		Move{0x25, "Thrash"      , TYPE_NORMAL  , PHYSICAL,  90, 100, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, {3, 4}, "'s thrashing about!", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, CONFUSE_ON_LAST},
		Move{0x26, "Double Edge" , TYPE_NORMAL  , PHYSICAL, 100, 100, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, TAKE_QUATER_MOVE_DAMAGE},
		Move{0x27, "Tail Whip"   , TYPE_NORMAL  , STATUS  ,   0, 100, 30, NO_STATUS_CHANGE, {}, {{STATS_DEF, -1, 1}}},
		Move{0x28, "Poison Sting", TYPE_POISON  , PHYSICAL,  15, 100, 35, {STATUS_POISONED, 0.3}},
		Move{0x29, "Twineedle"   , TYPE_BUG     , PHYSICAL,  25, 100, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, {2, 2}},
		Move{0x2A, "Pin Missile" , TYPE_BUG     , PHYSICAL,  14,  85, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x2B, "Leer"        , TYPE_NORMAL  , STATUS  ,   0, 100, 30, NO_STATUS_CHANGE, {}, {{STATS_DEF, -1, 1}}},
		Move{0x2C, "Bite"        , TYPE_NORMAL  , PHYSICAL,  60, 100, 25},
		Move{0x2D, "Growl"       , TYPE_NORMAL  , STATUS  ,   0, 100, 40, NO_STATUS_CHANGE, {}, {{STATS_ATK, -1, 1}}},
		Move{0x2E, "Roar"        , TYPE_NORMAL  , STATUS  ,   0, 100, 20},
		Move{0x2F, "Sing"        , TYPE_NORMAL  , STATUS  ,   0,  55, 15, {STATUS_ASLEEP, 1}},
		Move{0x30, "Supersonic"  , TYPE_NORMAL  , STATUS  ,   0,  55, 20, {STATUS_CONFUSED, 1}},
		Move{0x31, "SonicBoom"   , TYPE_NORMAL  , SPECIAL ,   0,  90, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DEAL_20_DAMAGE},
		Move{0x32, "Disable"     , TYPE_NORMAL  , SPECIAL ,   0,  55, 20}, //TODO: Code this move
		Move{0x33, "Acid"        , TYPE_POISON  , PHYSICAL,  40, 100, 30},
		Move{0x34, "Ember"       , TYPE_FIRE    , SPECIAL ,  40, 100, 25, {STATUS_BURNED, 0.1}},
		Move{0x35, "Flamethrower", TYPE_FIRE    , SPECIAL ,  95, 100, 25, {STATUS_BURNED, 0.1}},
		Move{0x36, "Mist"        , TYPE_NORMAL  , STATUS  ,   0, 255, 30}, //TODO: Code this move
		Move{0x37, "Water Gun"   , TYPE_WATER   , SPECIAL ,  40, 100, 25},
		Move{0x38, "Hydro Pump"  , TYPE_WATER   , SPECIAL , 120,  80,  5},
		Move{0x39, "Surf"        , TYPE_WATER   , SPECIAL ,  90, 100, 15},
		Move{0x3A, "Ice Beam"    , TYPE_ICE     , SPECIAL ,  95, 100, 10, {STATUS_FROZEN, 0.1}},
		Move{0x3B, "Blizzard"    , TYPE_ICE     , SPECIAL , 120,  90,  5, {STATUS_FROZEN, 0.1}},
		Move{0x3C, "Psybeam"     , TYPE_PSYCHIC , SPECIAL ,  65, 100, 20, {STATUS_CONFUSED, 0.1}},
		Move{0x3D, "Bubblebeam"  , TYPE_WATER   , SPECIAL ,  65, 100, 20, NO_STATUS_CHANGE, {}, {{STATS_SPD, -1, 0.1}}},
		Move{0x3E, "Aurora Beam" , TYPE_ICE     , SPECIAL ,  65, 100, 20, NO_STATUS_CHANGE, {}, {{STATS_ATK, -1, 0.1}}},
		Move{0x3F, "Hyper Beam"  , TYPE_NORMAL  , SPECIAL , 150,  90,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, true},
		Move{0x40, "Peck"        , TYPE_FLYING  , PHYSICAL,  35, 100, 35},
		Move{0x41, "Drill Peck"  , TYPE_FLYING  , PHYSICAL,  80, 100, 20},
		Move{0x42, "Submission"  , TYPE_FIGHTING, PHYSICAL,  80,  80, 25, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, TAKE_QUATER_MOVE_DAMAGE},
		Move{0x43, "Low Kick"    , TYPE_FIGHTING, PHYSICAL,  50, 100, 20},
		Move{0x44, "Counter"     , TYPE_FIGHTING, PHYSICAL,  80,  80, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, -5}, //TODO: Code this move
		Move{0x45, "Seismic Toss", TYPE_FIGHTING, PHYSICAL,   0, 100, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DEAL_LVL_AS_DAMAGE},
		Move{0x46, "Strength"    , TYPE_NORMAL  , PHYSICAL,  80, 100, 15},
		Move{0x47, "Absorb"      , TYPE_GRASS   , SPECIAL ,  20, 100, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ABSORB_HALF_DAMAGE},
		Move{0x48, "Mega Drain"  , TYPE_GRASS   , SPECIAL ,  40, 100, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ABSORB_HALF_DAMAGE},
		Move{0x49, "Leech Seed"  , TYPE_GRASS   , STATUS  ,   0,  90, 10, {STATUS_LEECHED, 1}},
		Move{0x4A, "Growth"      , TYPE_NORMAL  , STATUS  ,   0, 255, 40, NO_STATS_CHANGE, {{STATS_ATK, 1, 1}, {STATS_SPE, 1, 1}}},
		Move{0x4B, "Razor Leaf"  , TYPE_GRASS   , PHYSICAL,  55,  95, 25, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE * 8},
		Move{0x4C, "SolarBeam"   , TYPE_GRASS   , SPECIAL , 120, 100, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NEED_LOADING("took in sunlight")},
		Move{0x4D, "PoisonPowder", TYPE_POISON  , STATUS  ,   0,  75, 35, {STATUS_POISONED, 1}},
		Move{0x4E, "Stun Spore"  , TYPE_GRASS   , STATUS  ,   0,  75, 30, {STATUS_PARALYZED, 1}},
		Move{0x4F, "Sleep Powder", TYPE_GRASS   , STATUS  ,   0,  75, 15, {STATUS_ASLEEP, 1}},
		Move{0x50, "Petal Dance" , TYPE_GRASS   , SPECIAL ,  70, 100, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, {3, 4}, "'s thrashing about!", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, CONFUSE_ON_LAST},
		Move{0x51, "String Shot" , TYPE_BUG     , STATUS  ,   0,  90, 40, NO_STATUS_CHANGE, {}, {{STATS_SPD, 1, 1}}},
		Move{0x52, "Dragon Rage" , TYPE_DRAGON  , SPECIAL ,   0, 100, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DEAL_40_DAMAGE},
		Move{0x53, "Fire Spin"   , TYPE_FIRE    , SPECIAL ,  35,  85, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x54, "ThunderShock", TYPE_ELECTRIC, SPECIAL ,  40, 100, 30, {STATUS_PARALYZED, 0.1}},
		Move{0x55, "Thunderbolt" , TYPE_ELECTRIC, SPECIAL ,  90, 100, 15, {STATUS_PARALYZED, 0.1}},
		Move{0x56, "Thunder Wave", TYPE_ELECTRIC, STATUS  ,   0, 100, 30, {STATUS_PARALYZED, 1}},
		Move{0x57, "Thunder"     , TYPE_ELECTRIC, SPECIAL , 120,  70, 10, {STATUS_PARALYZED, 0.1}},
		Move{0x58, "Rock Throw"  , TYPE_ROCK    , PHYSICAL,  50,  65, 15},
		Move{0x59, "Earthquake"  , TYPE_GROUND  , PHYSICAL, 100, 100, 10},
		Move{0x5A, "Fissure"     , TYPE_GROUND  , PHYSICAL,   0,  30,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ONE_HIT_KO_HANDLE},
		Move{0x5B, "Dig"         , TYPE_GROUND  , PHYSICAL,  80, 100, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NEED_LOADING("dug a hole"), true},
		Move{0x5C, "Toxic"       , TYPE_POISON  , STATUS  ,   0,  85, 10, {STATUS_BADLY_POISONED, 1}},
		Move{0x5D, "Confusion"   , TYPE_PSYCHIC , SPECIAL ,  50, 100, 25, {STATUS_CONFUSED, 0.1}},
		Move{0x5E, "Psychic"     , TYPE_PSYCHIC , SPECIAL ,  90, 100, 10, NO_STATUS_CHANGE, {}, {{STATS_SPE, -1, 0.3}}},
		Move{0x5F, "Hypnosis"    , TYPE_PSYCHIC , STATUS  ,   0,  60, 20, {STATUS_ASLEEP, 1}},
		Move{0x60, "Meditate"    , TYPE_PSYCHIC , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, {{STATS_ATK, 1, 1}}},
		Move{0x61, "Agility"     , TYPE_PSYCHIC , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, {{STATS_SPD, 2, 1}}},
		Move{0x62, "Quick Attack", TYPE_NORMAL  , PHYSICAL,  40, 100, 30, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 1},
		Move{0x63, "Rage"        , TYPE_NORMAL  , PHYSICAL,  20, 100, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, {0xFFFFFFFF, 0xFFFFFFFF}},
		Move{0x64, "Teleport"    , TYPE_NORMAL  , STATUS  ,   0, 255, 20},
		Move{0x65, "Night Shade" , TYPE_GHOST   , SPECIAL ,   0, 100, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DEAL_LVL_AS_DAMAGE},
		Move{0x66, "Mimic"       , TYPE_NORMAL  , STATUS  ,   0, 255, 10},
		Move{0x67, "Screech"     , TYPE_NORMAL  , STATUS  ,   0,  85, 40, NO_STATUS_CHANGE, {}, {{STATS_DEF, -2, 1}}},
		Move{0x68, "Double Team" , TYPE_NORMAL  , STATUS  ,   0, 255, 15, NO_STATUS_CHANGE, {{STATS_ESQ, 1, 1}}},
		Move{0x69, "Recover"     , TYPE_NORMAL  , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, HEAL_HALF_HEALTH},
		Move{0x6A, "Harden"      , TYPE_NORMAL  , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, {{STATS_DEF, 1, 1}}},
		Move{0x6B, "Minimize"    , TYPE_NORMAL  , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, {{STATS_ESQ, 1, 1}}},
		Move{0x6C, "SmokeScreen" , TYPE_NORMAL  , STATUS  ,   0, 100, 20, NO_STATUS_CHANGE, {}, {{STATS_PRE, -2, 1}}},
		Move{0x6D, "Confuse Ray" , TYPE_GHOST   , STATUS  ,   0, 100, 10, {STATUS_CONFUSED, 1}},
		Move{0x6E, "Withdraw"    , TYPE_WATER   , STATUS  ,   0, 255, 40, NO_STATUS_CHANGE, {{STATS_DEF, 1, 1}}},
		Move{0x6F, "Defense Curl", TYPE_NORMAL  , STATUS  ,   0, 255, 40, NO_STATUS_CHANGE, {{STATS_DEF, 1, 1}}},
		Move{0x70, "Barrier"     , TYPE_PSYCHIC , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, {{STATS_DEF, 2, 1}}},
		Move{0x71, "Light Screen", TYPE_PSYCHIC , STATUS  ,   0, 255, 30}, //TODO: Code the move
		Move{0x72, "Haze"        , TYPE_ICE     , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, CANCEL_STATS_CHANGE},
		Move{0x73, "Reflect"     , TYPE_PSYCHIC , STATUS  ,   0, 255, 20}, //TODO: Code the move
		Move{0x74, "Focus Energy", TYPE_NORMAL  , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, SET_USER_CRIT_RATIO_TO_1_HALF},
		Move{0x75, "Bide"        , TYPE_NORMAL  , PHYSICAL,   0, 255, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, {2, 3}, "", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, STORE_DAMAGES},
		Move{0x76, "Metronome"   , TYPE_NORMAL  , STATUS  ,   0, 255, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, USE_RANDOM_MOVE},
		Move{0x77, "Mirror Move" , TYPE_NORMAL  , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, USE_LAST_FOE_MOVE},
		Move{0x78, "Selfdestruct", TYPE_NORMAL  , PHYSICAL, 200, 100,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, SUICIDE, SUICIDE_MISS},
		Move{0x79, "Egg Bomb"    , TYPE_NORMAL  , PHYSICAL, 100,  75, 10},
		Move{0x7A, "Lick"        , TYPE_GHOST   , PHYSICAL,  30, 100, 30, {STATUS_PARALYZED, 0.3}},
		Move{0x7B, "Smog"        , TYPE_POISON  , SPECIAL ,  20,  70, 20, {STATUS_POISONED, 0.4}},
		Move{0x7C, "Sludge"      , TYPE_POISON  , SPECIAL ,  65, 100, 20, {STATUS_POISONED, 0.3}},
		Move{0x7D, "Bone Club"   , TYPE_GROUND  , PHYSICAL,  65,  85, 20},
		Move{0x7E, "Fire Blast"  , TYPE_FIRE    , SPECIAL , 120,  85,  5, {STATUS_BURNED, 0.1}},
		Move{0x7F, "Waterfall"   , TYPE_WATER   , PHYSICAL,  80, 120, 15},
		Move{0x80, "Clamp"       , TYPE_WATER   , PHYSICAL,  35,  75, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, TWO_TO_FIVE_HITS, "'s attack continues!", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, WRAP_TARGET, GLITCH_HYPER_BEAM},
		Move{0x81, "Swift"       , TYPE_NORMAL  , SPECIAL ,  60, 255, 20},
		Move{0x82, "Skull Bash"  , TYPE_NORMAL  , PHYSICAL, 100, 100, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NEED_LOADING("lowered it's head")},
		Move{0x83, "Spike Cannon", TYPE_NORMAL  , PHYSICAL,  20, 100, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x84, "Constrict"   , TYPE_NORMAL  , PHYSICAL,  10, 100, 35, NO_STATUS_CHANGE, {}, {{STATS_SPD, -1, 0.1}}},
		Move{0x85, "Amnesia"     , TYPE_PSYCHIC , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, {{STATS_SPE, 2, 1}}},
		Move{0x86, "Kinesis"     , TYPE_PSYCHIC , STATUS  ,   0,  80, 15, NO_STATUS_CHANGE, {}, {{STATS_PRE, -1, 1}}},
		Move{0x87, "Softboiled"  , TYPE_NORMAL  , STATUS  ,   0, 255, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, HEAL_HALF_HEALTH},
		Move{0x88, "Hi Jump Kick", TYPE_FIGHTING, PHYSICAL,  85,  90, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, nullptr, TAKE_1DAMAGE},
		Move{0x89, "Glare"       , TYPE_NORMAL  , STATUS  ,   0,  75, 30},
		Move{0x8A, "Dream Eater" , TYPE_PSYCHIC , STATUS  ,   0, 100, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DREAM_EATER},
		Move{0x8B, "Poison Gas"  , TYPE_POISON  , STATUS  ,   0,  55, 40, {STATUS_POISONED, 1}},
		Move{0x8C, "Barrage"     , TYPE_NORMAL  , PHYSICAL,  15,  85, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x8D, "Leech Life"  , TYPE_BUG     , PHYSICAL,  20, 100, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ABSORB_HALF_DAMAGE},
		Move{0x8E, "Lovely Kiss" , TYPE_NORMAL  , STATUS  ,   0,  75, 10, {STATUS_ASLEEP, 1}},
		Move{0x8F, "Sky Attack"  , TYPE_FLYING  , PHYSICAL, 140,  90,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NEED_LOADING("is glowing")},
		Move{0x90, "Transform"   , TYPE_NORMAL  , PHYSICAL,   0, 255, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, TRANSFORM},
		Move{0x91, "Bubble"      , TYPE_WATER   , SPECIAL ,  20, 100, 30, NO_STATUS_CHANGE, {}, {{STATS_SPD, -1, 0.1}}},
		Move{0x92, "Dizzy Punch" , TYPE_NORMAL  , PHYSICAL,  70, 100, 10},
		Move{0x93, "Spore"       , TYPE_GRASS   , STATUS  ,   0, 100, 15, {STATUS_ASLEEP, 1}},
		Move{0x94, "Flash"       , TYPE_NORMAL  , STATUS  ,   0,  70, 20, NO_STATUS_CHANGE, {}, {{STATS_PRE, -1, 1}}},
		Move{0x95, "Psywave"     , TYPE_PSYCHIC , SPECIAL ,   0,  80, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DEAL_0_5_TO_1_5_LEVEL_DAMAGE},
		Move{0x96, "Splash"      , TYPE_NORMAL  , STATUS  ,   0, 255, 40},
		Move{0x97, "Acid Armor"  , TYPE_POISON  , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, {}, {{STATS_DEF, 2, 1}}},
		Move{0x98, "Crabhammer"  , TYPE_WATER   , PHYSICAL,  90,  85, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE * 8},
		Move{0x99, "Explosion"   , TYPE_NORMAL  , PHYSICAL, 340, 100,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, SUICIDE, SUICIDE_MISS},
		Move{0x9A, "Fury Swipes" , TYPE_NORMAL  , PHYSICAL,  18,  80, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x9B, "Bonemerang"  , TYPE_GROUND  , PHYSICAL,  50,  90, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, {2, 2}},
		Move{0x9C, "Rest"        , TYPE_PSYCHIC , STATUS  ,   0,   0, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, HEAL_ALL_HEALTH_AND_SLEEP},
		Move{0x9D, "Rock Slide"  , TYPE_ROCK    , PHYSICAL,  75,  90, 10},
		Move{0x9E, "Hyper Fang"  , TYPE_NORMAL  , PHYSICAL,  80,  90, 15},
		Move{0x9F, "Sharpen"     , TYPE_NORMAL  , STATUS  ,   0,   0, 30, NO_STATUS_CHANGE, {{STATS_ATK, 1, 1}}},
		Move{0xA0, "Conversion"  , TYPE_NORMAL  , STATUS  ,   0,   0, 30, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, CONVERSION},
		Move{0xA1, "Tri Attack"  , TYPE_NORMAL  , SPECIAL ,  80, 100, 10},
		Move{0xA2, "Super Fang"  , TYPE_NORMAL  , PHYSICAL,   0,  90, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DEAL_HALF_HP_DAMAGE},
		Move{0xA3, "Slash"       , TYPE_NORMAL  , PHYSICAL,  70, 100, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE * 8},
		Move{0xA4, "Substitute"  , TYPE_NORMAL  , STATUS  ,   0,   0, 10},
		Move{0xA5, "Struggle"    , TYPE_NORMAL  , PHYSICAL,  50, 100,  0, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, TAKE_HALF_MOVE_DAMAGE},
		DEFAULT_MOVE(0xA6),
		DEFAULT_MOVE(0xA7),
		DEFAULT_MOVE(0xA8),
		DEFAULT_MOVE(0xA9),
		DEFAULT_MOVE(0xAA),
		DEFAULT_MOVE(0xAB),
		DEFAULT_MOVE(0xAC),
		DEFAULT_MOVE(0xAD),
		DEFAULT_MOVE(0xAE),
		DEFAULT_MOVE(0xAF),
		DEFAULT_MOVE(0xB0),
		DEFAULT_MOVE(0xB1),
		DEFAULT_MOVE(0xB2),
		DEFAULT_MOVE(0xB3),
		DEFAULT_MOVE(0xB4),
		DEFAULT_MOVE(0xB5),
		DEFAULT_MOVE(0xB6),
		DEFAULT_MOVE(0xB7),
		DEFAULT_MOVE(0xB8),
		DEFAULT_MOVE(0xB9),
		DEFAULT_MOVE(0xBA),
		DEFAULT_MOVE(0xBB),
		DEFAULT_MOVE(0xBC),
		DEFAULT_MOVE(0xBD),
		DEFAULT_MOVE(0xBE),
		DEFAULT_MOVE(0xBF),
		DEFAULT_MOVE(0xC0),
		DEFAULT_MOVE(0xC1),
		DEFAULT_MOVE(0xC2),
		DEFAULT_MOVE(0xC3),
		DEFAULT_MOVE(0xC4),
		DEFAULT_MOVE(0xC5),
		DEFAULT_MOVE(0xC6),
		DEFAULT_MOVE(0xC7),
		DEFAULT_MOVE(0xC8),
		DEFAULT_MOVE(0xC9),
		DEFAULT_MOVE(0xCA),
		DEFAULT_MOVE(0xCB),
		DEFAULT_MOVE(0xCC),
		DEFAULT_MOVE(0xCD),
		DEFAULT_MOVE(0xCE),
		DEFAULT_MOVE(0xCF),
		DEFAULT_MOVE(0xD0),
		DEFAULT_MOVE(0xD1),
		DEFAULT_MOVE(0xD2),
		DEFAULT_MOVE(0xD3),
		DEFAULT_MOVE(0xD4),
		DEFAULT_MOVE(0xD5),
		DEFAULT_MOVE(0xD6),
		DEFAULT_MOVE(0xD7),
		DEFAULT_MOVE(0xD8),
		DEFAULT_MOVE(0xD9),
		DEFAULT_MOVE(0xDA),
		DEFAULT_MOVE(0xDB),
		DEFAULT_MOVE(0xDC),
		DEFAULT_MOVE(0xDD),
		DEFAULT_MOVE(0xDE),
		DEFAULT_MOVE(0xDF),
		DEFAULT_MOVE(0xE0),
		DEFAULT_MOVE(0xE1),
		DEFAULT_MOVE(0xE2),
		DEFAULT_MOVE(0xE3),
		DEFAULT_MOVE(0xE4),
		DEFAULT_MOVE(0xE5),
		DEFAULT_MOVE(0xE6),
		DEFAULT_MOVE(0xE7),
		DEFAULT_MOVE(0xE8),
		DEFAULT_MOVE(0xE9),
		DEFAULT_MOVE(0xEA),
		DEFAULT_MOVE(0xEB),
		DEFAULT_MOVE(0xEC),
		DEFAULT_MOVE(0xED),
		DEFAULT_MOVE(0xEE),
		DEFAULT_MOVE(0xEF),
		DEFAULT_MOVE(0xF0),
		DEFAULT_MOVE(0xF1),
		DEFAULT_MOVE(0xF2),
		DEFAULT_MOVE(0xF3),
		DEFAULT_MOVE(0xF4),
		DEFAULT_MOVE(0xF5),
		DEFAULT_MOVE(0xF6),
		DEFAULT_MOVE(0xF7),
		DEFAULT_MOVE(0xF8),
		DEFAULT_MOVE(0xF9),
		DEFAULT_MOVE(0xFA),
		DEFAULT_MOVE(0xFB),
		DEFAULT_MOVE(0xFC),
		DEFAULT_MOVE(0xFD),
		DEFAULT_MOVE(0xFE),
		DEFAULT_MOVE(0xFF)
	};
}