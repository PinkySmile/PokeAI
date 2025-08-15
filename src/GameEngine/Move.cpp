//
// Created by PinkySmile on 14/07/2019.
//

#include <sstream>
#include "Pokemon.hpp"
#include "Move.hpp"
#include "Exception.hpp"
#include "Utils.hpp"

namespace PokemonGen1
{
	Move::Move(
		unsigned char id,
		const std::string &name,
		Type type,
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
		const std::string &hitCallBackDescription,
		const std::function<bool(Pokemon &owner, Pokemon &target, bool last, const std::function<void(const std::string &msg)> &logger)> &&missCallback,
		const std::string &missCallBackDescription
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
		_needRecharge(needRecharge),
		_hitCallBackDescription(hitCallBackDescription),
		_missCallBackDescription(missCallBackDescription)
	{
		if (category != STATUS)
			switch (type) {
			case TYPE_FIRE:
			case TYPE_WATER:
			case TYPE_GRASS:
			case TYPE_ELECTRIC:
			case TYPE_ICE:
			case TYPE_PSYCHIC:
			case TYPE_DRAGON:
			case TYPE_NEUTRAL_SPECIAL:
				if (category != SPECIAL)
					throw InvalidStateException(this->_name + " is invalid: Type " + typeToString(type) + " expects category SPECIAL.");
				break;
			case TYPE_NORMAL:
			case TYPE_FIGHTING:
			case TYPE_FLYING:
			case TYPE_POISON:
			case TYPE_GROUND:
			case TYPE_ROCK:
			case TYPE_BUG:
			case TYPE_GHOST:
			case TYPE_NEUTRAL_PHYSICAL:
				if (category != PHYSICAL)
					throw InvalidStateException(this->_name + " is invalid: Type " + typeToString(type) + " expects category PHYSICAL.");
				break;
			default:
				throw InvalidStateException(this->_name + " is invalid: Type is invalid.");
			}

		std::stringstream stream;

		if (this->_critChance != DEFAULT_CRIT_CHANCE)
			stream << "Crit multiplier: " << this->_critChance << std::endl;
		if (this->_needRecharge)
			stream << "Needs to recharge" << std::endl;
		if (this->_needLoading)
			stream << "Needs to load" << std::endl;
		if (this->_invulnerableDuringLoading)
			stream << "Has invulnerability during loading turn" << std::endl;
		if (this->_nbHits.first != 1 || this->_nbHits.second != 1) {
			stream << "Can hit " << this->_nbHits.first;
			if (this->_nbHits.first != this->_nbHits.second)
				stream << " to " << this->_nbHits.second;
			stream << " times" << std::endl;
		}
		if (this->_nbRuns.first != 1 || this->_nbRuns.second != 1) {
			stream << "Keep going for " << this->_nbRuns.first;
			if (this->_nbRuns.first != this->_nbRuns.second)
				stream << " to " << this->_nbRuns.second;
			stream <<" turns" << std::endl;
		}
		if (this->_priority)
			stream << "Priority: " << static_cast<int>(this->_priority) << std::endl;
		if (this->_statusChange.status != STATUS_NONE) {
			if (!this->_statusChange.cmpVal)
				stream << "Makes foe ";
			else
				stream << (this->_statusChange.cmpVal * 100 / 256.) << "% chance to make foe ";
			stream << statusToString(this->_statusChange.status) << std::endl;
		}
		for (auto &change : this->_ownerChange) {
			if (!change.cmpVal)
				stream << (change.nb > 0 ? "Increase" : "Decrease");
			else
				stream << "Has " << (change.cmpVal * 100. / 256) << "% chance to " << (change.nb > 0 ? "increase" : "decrease");
			stream << " user's " << statToString(change.stat);
			stream << " by " << std::abs(change.nb) << " stage" << (std::abs(change.nb) != 1 ? "s" : "") << std::endl;
		}
		for (auto &change : this->_foeChange) {
			if (!change.cmpVal)
				stream << (change.nb > 0 ? "Increase" : "Decrease");
			else
				stream << "Has " << (change.cmpVal * 100. / 256) << "% chance to " << (change.nb > 0 ? "increase" : "decrease");
			stream << " foe's " << statToString(change.stat);
			stream << " by " << std::abs(change.nb) << " stage" << (std::abs(change.nb) != 1 ? "s" : "") << std::endl;
		}
		if (!this->_hitCallBackDescription.empty())
			stream << "On hit: " << this->_hitCallBackDescription << std::endl;
		if (!this->_missCallBackDescription.empty())
			stream << "On miss: " << this->_missCallBackDescription << std::endl;
		this->_fullDescription = stream.str();
		for (auto it = this->_fullDescription.end(); it != this->_fullDescription.begin() && std::isspace(*(it - 1)); it--)
			this->_fullDescription.erase(it - 1);
	}

	Move::Move(const Move &other) :
		_hitCallback(other._hitCallback),
		_missCallback(other._missCallback),
		_critChance(other._critChance),
		_loadingMsg(other._loadingMsg),
		_keepGoingMsg(other._keepGoingMsg),
		_name(other._name),
		_type(other._type),
		_category(other._category),
		_power(other._power),
		_id(other._id),
		_pp(other._maxpp),
		_ppup(other._ppup),
		_maxpp(other._maxpp),
		_nbHit(0),
		_accuracy(other._accuracy),
		_nbRuns(other._nbRuns),
		_nbHits(other._nbHits),
		_statusChange(other._statusChange),
		_ownerChange(other._ownerChange),
		_foeChange(other._foeChange),
		_priority(other._priority),
		_needLoading(other._needLoading),
		_invulnerableDuringLoading(other._invulnerableDuringLoading),
		_needRecharge(other._needRecharge),
		_hitCallBackDescription(other._hitCallBackDescription),
		_missCallBackDescription(other._missCallBackDescription),
		_fullDescription(other._fullDescription)
	{
	}

	Move &Move::operator=(const Move &other)
	{
		this->_hitCallback = other._hitCallback;
		this->_missCallback = other._missCallback;
		this->_critChance = other._critChance;
		this->_loadingMsg = other._loadingMsg;
		this->_keepGoingMsg = other._keepGoingMsg;
		this->_name = other._name;
		this->_type = other._type;
		this->_category = other._category;
		this->_power = other._power;
		this->_id = other._id;
		this->_pp = other._maxpp;
		this->_ppup = other._ppup;
		this->_maxpp = other._maxpp;
		this->_nbHit = 0;
		this->_accuracy = other._accuracy;
		this->_nbRuns = other._nbRuns;
		this->_nbHits = other._nbHits;
		this->_statusChange = other._statusChange;
		this->_ownerChange = other._ownerChange;
		this->_foeChange = other._foeChange;
		this->_priority = other._priority;
		this->_needLoading = other._needLoading;
		this->_invulnerableDuringLoading = other._invulnerableDuringLoading;
		this->_needRecharge = other._needRecharge;
		this->_hitCallBackDescription = other._hitCallBackDescription;
		this->_missCallBackDescription = other._missCallBackDescription;
		this->_fullDescription = other._fullDescription;
		return *this;
	}

	bool Move::isFinished() const
	{
		return this->_nbHit == 0;
	}

	unsigned char Move::getHitsLeft() const
	{
		return this->_nbHit;
	}

	void Move::setHitsLeft(unsigned char nb)
	{
		this->_nbHit = nb;
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
		this->_nbHit = 1;
	}

	bool Move::attack(Pokemon &owner, Pokemon &target, const std::function<void(const std::string &msg)> &logger)
	{
		double multiplier = target.getEvasion() * owner.getAccuracy();
		std::string msg = this->_keepGoingMsg;

		if (!target.canGetHit() && this->_accuracy <= 100) {
			this->_nbHit--;
			if (this->_missCallback)
				this->_missCallback(owner, target, this->isFinished(), logger);
			return false;
		}

		owner.setInvincible(false);
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
				logger(owner.getName() + " " + this->_loadingMsg + "!");
				owner.setInvincible(this->_invulnerableDuringLoading);
				return true;
			}
			msg = "";
		}

		this->_nbHit--;

		if (!msg.empty())
			logger(owner.getName() + msg);
		else
			logger(owner.getName() + " used " + Utils::toUpper(this->_name) + "!");

		Pokemon::DamageResult damages{
			.critical = false,
			.damage = 0,
			.affect = true,
			.isVeryEffective = false,
			.isNotVeryEffective = false,
		};

		if (this->_category != STATUS && getAttackDamageMultiplier(this->_type, target.getTypes()) == 0) {
			logger("It doesn't affect " + target.getName() + "!");
			if (this->_missCallback)
				this->_missCallback(owner, target, this->isFinished(), logger);
			return true;
		}

		if (this->_power) {
			unsigned char r = target.getRandomGenerator()();
			unsigned char spd = std::min<unsigned int>(pokemonList.at(owner.getID()).SPD / 2 * this->_critChance, 255);

			r = (r << 3U) | ((r & 0b11100000U) >> 5U);
			damages = owner.calcDamage(target, this->_power, this->_type, this->_category, (r < spd));
		}

		if (this->_accuracy <= 100) {
			unsigned random = owner.getRandomGenerator()();

			if (random / 2.55 >= this->_accuracy * multiplier) {
				this->_nbHit = 0;
				if (this->_missCallback)
					this->_missCallback(owner, target, this->isFinished(), logger);
				return false;
			} else if (!this->_nbHit)
				owner.setRecharging(this->_needRecharge);
		}

		if (damages.critical)
			logger("Critical hit!");

		if (damages.isNotVeryEffective)
			logger("It's not very effective!");

		if (damages.isVeryEffective)
			logger("It's super effective!");

		unsigned hits = 0;

		if (this->_nbHits.second == this->_nbHits.first)
			hits = this->_nbHits.first;
		else if (this->_nbHits.second - 1 == this->_nbHits.first)
			hits = this->_nbHits.first + (owner.getRandomGenerator()() & 1U);
		else if (this->_nbHits.second - 3 == this->_nbHits.first) {
			hits = owner.getRandomGenerator()() & 3U;
			if (hits >= 2)
				hits = owner.getRandomGenerator()() & 3U;
			hits += this->_nbHits.first;
		}

		if (hits > 1)
			logger("Hits " + std::to_string(hits) + " times!");

		if (this->_power)
			target.takeDamage(damages.damage * hits);

		if (!target.getHealth()) {
			owner.setRecharging(false);
			if (this->_hitCallback)
				return this->_hitCallback(owner, target, damages.damage, this->isFinished(), logger);
			return true;
		}

		if (
			!this->_statusChange.cmpVal ||
			(
				target.getTypes().first != this->_type &&
				target.getTypes().second != this->_type &&
				target.canHaveStatus(this->_statusChange.status) &&
				owner.getRandomGenerator()() < this->_statusChange.cmpVal
			)
		)
			target.addStatus(this->_statusChange.status);

		bool addedStatus = false;

		for (const auto &val : this->_foeChange)
			if (!val.cmpVal || owner.getRandomGenerator()() < val.cmpVal)
				addedStatus |= target.changeStat(val.stat, val.nb);
		for (const auto &val : this->_ownerChange)
			if (!val.cmpVal || owner.getRandomGenerator()() < val.cmpVal)
				addedStatus |= owner.changeStat(val.stat, val.nb);
		if (addedStatus)
			target.applyStatusDebuff();

		if (this->_hitCallback)
			return this->_hitCallback(owner, target, damages.damage, this->isFinished(), logger);

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

	Type Move::getType() const
	{
		return this->_type;
	}

	unsigned int Move::getPower() const
	{
		return this->_power;
	}

	char Move::getPriority() const
	{
		return this->_priority;
	}

	const std::string &Move::getDescription() const
	{
		return this->_fullDescription;
	}

	double Move::getCritChance() const
	{
		return this->_critChance;
	}

	const std::pair<unsigned int, unsigned int> &Move::getNbRuns() const
	{
		return this->_nbRuns;
	}

	const std::pair<unsigned int, unsigned int> &Move::getNbHits() const
	{
		return this->_nbHits;
	}

	const Move::StatusChangeProb &Move::getStatusChange() const
	{
		return this->_statusChange;
	}

	const std::vector<Move::StatsChangeProb> &Move::getOwnerChange() const
	{
		return this->_ownerChange;
	}

	const std::vector<Move::StatsChangeProb> &Move::getFoeChange() const
	{
		return this->_foeChange;
	}

	bool Move::needsLoading() const
	{
		return this->_needLoading;
	}

	bool Move::isInvulnerableDuringLoading() const
	{
		return this->_invulnerableDuringLoading;
	}

	bool Move::needsRecharge() const
	{
		return this->_needRecharge;
	}

	const std::string &Move::getHitCallBackDescription() const
	{
		return this->_hitCallBackDescription;
	}

	const std::string &Move::getMissCallBackDescription() const
	{
		return this->_missCallBackDescription;
	}

	void Move::reset()
	{
		this->setPP(this->getMaxPP());
		this->setHitsLeft(0);
	}

	/*
	** From pokemondb, bulbapedia and Rhydon
	** https://pokemondb.net/move/generation/1
	** https://bulbapedia.bulbagarden.net
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Tables.cs#L145
	*/

	//This is only relevant for Gen 1 (Some moves changed)
	const std::array<Move, 256> availableMoves{
		Move{0x00, "-"           , TYPE_INVALID , STATUS,    0,    0,  0, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, nullptr, "No move. This move is invalid and will cause desync when used."},
		Move{0x01, "Pound"       , TYPE_NORMAL  , PHYSICAL,  40, 100, 35},
		Move{0x02, "Karate Chop" , TYPE_NORMAL  , PHYSICAL,  50, 100, 25, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE * 8},
		Move{0x03, "DoubleSlap"  , TYPE_NORMAL  , PHYSICAL,  15,  85, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x04, "Comet Punch" , TYPE_NORMAL  , PHYSICAL,  18,  85, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x05, "Mega Punch"  , TYPE_NORMAL  , PHYSICAL,  80,  85, 20},
		Move{0x06, "Pay Day"     , TYPE_NORMAL  , PHYSICAL,  40, 100, 20},
		Move{0x07, "Fire Punch"  , TYPE_FIRE    , SPECIAL,   75, 100, 15, {STATUS_BURNED, 0x1A}},
		Move{0x08, "Ice Punch"   , TYPE_ICE     , SPECIAL,   75, 100, 15, {STATUS_FROZEN, 0x1A}},
		Move{0x09, "ThunderPunch", TYPE_ELECTRIC, SPECIAL,   75, 100, 15, {STATUS_PARALYZED, 0x1A}},
		Move{0x0A, "Scratch"     , TYPE_NORMAL  , PHYSICAL,  40, 100, 35},
		Move{0x0B, "ViceGrip"    , TYPE_NORMAL  , PHYSICAL,  55, 100, 30},
		Move{0x0C, "Guillotine"  , TYPE_NORMAL  , PHYSICAL,   0,  30,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ONE_HIT_KO_HANDLE},
		Move{0x0D, "Razor Wind"  , TYPE_NORMAL  , PHYSICAL,  80,  75, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NEED_LOADING("whipped up a whirlwind!")},
		Move{0x0E, "Swords Dance", TYPE_NORMAL  , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, {{STATS_ATK, 2, 0}}},
		Move{0x0F, "Cut"         , TYPE_NORMAL  , PHYSICAL,  50,  95, 30},
		Move{0x10, "Gust"        , TYPE_NORMAL  , PHYSICAL,  40, 100, 35},
		Move{0x11, "Wing Attack" , TYPE_FLYING  , PHYSICAL,  35, 100, 35},
		Move{0x12, "Whirlwind"   , TYPE_NORMAL  , STATUS  ,   0, 100, 20},
		Move{0x13, "Fly"         , TYPE_FLYING  , PHYSICAL,  90,  95, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NEED_LOADING("flew up high!"), true},
		Move{0x14, "Bind"        , TYPE_NORMAL  , PHYSICAL,  15,  75, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, TWO_TO_FIVE_HITS, "'s attack continues!", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, WRAP_TARGET, GLITCH_HYPER_BEAM},
		Move{0x15, "Slam"        , TYPE_NORMAL  , PHYSICAL,  80,  75, 20},
		Move{0x16, "Vine Whip"   , TYPE_GRASS   , SPECIAL ,  35, 100, 25},
		Move{0x17, "Stomp"       , TYPE_NORMAL  , PHYSICAL,  65, 100, 20},
		Move{0x18, "Double Kick" , TYPE_FIGHTING, PHYSICAL,  65, 100, 20},
		Move{0x19, "Mega Kick"   , TYPE_NORMAL  , PHYSICAL, 120,  75,  5},
		Move{0x1A, "Jump Kick"   , TYPE_FIGHTING, PHYSICAL,  70,  95, 25, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, NO_CALLBACK, TAKE_1DAMAGE},
		Move{0x1B, "Rolling Kick", TYPE_FIGHTING, PHYSICAL,  60,  85, 15},
		Move{0x1C, "Sand-Attack" , TYPE_NORMAL  , STATUS  ,   0, 100, 15, NO_STATUS_CHANGE, {}, {{STATS_ACC, -1, 0}}},
		Move{0x1D, "Headbutt"    , TYPE_NORMAL  , PHYSICAL,  70, 100, 15, {STATUS_FLINCHED, 0x4D}},
		Move{0x1E, "Horn Attack" , TYPE_NORMAL  , PHYSICAL,  65, 100, 25},
		Move{0x1F, "Fury Attack" , TYPE_NORMAL  , PHYSICAL,  15,  85, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x20, "Horn Drill"  , TYPE_NORMAL  , PHYSICAL,   0,  30,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ONE_HIT_KO_HANDLE},
		Move{0x21, "Tackle"      , TYPE_NORMAL  , PHYSICAL,  35,  95, 35},
		Move{0x22, "Body Slam"   , TYPE_NORMAL  , PHYSICAL,  85, 100, 15, {STATUS_PARALYZED, 0x4D}},
		Move{0x23, "Wrap"        , TYPE_NORMAL  , PHYSICAL,  15,  90, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, TWO_TO_FIVE_HITS, "'s attack continues!", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, WRAP_TARGET, GLITCH_HYPER_BEAM},
		Move{0x24, "Take down"   , TYPE_NORMAL  , PHYSICAL,  90,  85, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, TAKE_QUARTER_MOVE_DAMAGE},
		Move{0x25, "Thrash"      , TYPE_NORMAL  , PHYSICAL,  90, 100, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, {3, 4}, "'s thrashing about!", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, CONFUSE_ON_LAST, CONFUSE_ON_LAST_MISS},
		Move{0x26, "Double Edge" , TYPE_NORMAL  , PHYSICAL, 100, 100, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, TAKE_QUARTER_MOVE_DAMAGE},
		Move{0x27, "Tail Whip"   , TYPE_NORMAL  , STATUS  ,   0, 100, 30, NO_STATUS_CHANGE, {}, {{STATS_DEF, -1, 0}}},
		Move{0x28, "Poison Sting", TYPE_POISON  , PHYSICAL,  15, 100, 35, {STATUS_POISONED, 0x34}},
		Move{0x29, "Twineedle"   , TYPE_BUG     , PHYSICAL,  25, 100, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, {2, 2}},
		Move{0x2A, "Pin Missile" , TYPE_BUG     , PHYSICAL,  14,  85, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x2B, "Leer"        , TYPE_NORMAL  , STATUS  ,   0, 100, 30, NO_STATUS_CHANGE, {}, {{STATS_DEF, -1, 0}}},
		Move{0x2C, "Bite"        , TYPE_NORMAL  , PHYSICAL,  60, 100, 25, {STATUS_FLINCHED, 0x1A}},
		Move{0x2D, "Growl"       , TYPE_NORMAL  , STATUS  ,   0, 100, 40, NO_STATUS_CHANGE, {}, {{STATS_ATK, -1, 0}}},
		Move{0x2E, "Roar"        , TYPE_NORMAL  , STATUS  ,   0, 100, 20},
		Move{0x2F, "Sing"        , TYPE_NORMAL  , STATUS  ,   0,  55, 15, {STATUS_ASLEEP, 0}},
		Move{0x30, "Supersonic"  , TYPE_NORMAL  , STATUS  ,   0,  55, 20, {STATUS_CONFUSED, 0}},
		Move{0x31, "SonicBoom"   , TYPE_NORMAL  , STATUS  ,   0,  90, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DEAL_20_DAMAGE},
		Move{0x32, "Disable"     , TYPE_NORMAL  , STATUS  ,   0,  55, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, NOT_IMPLEMENTED}, //TODO: Code this move
		Move{0x33, "Acid"        , TYPE_POISON  , PHYSICAL,  40, 100, 30},
		Move{0x34, "Ember"       , TYPE_FIRE    , SPECIAL ,  40, 100, 25, {STATUS_BURNED, 0x1A}},
		Move{0x35, "Flamethrower", TYPE_FIRE    , SPECIAL ,  95, 100, 25, {STATUS_BURNED, 0x1A}},
		Move{0x36, "Mist"        , TYPE_NORMAL  , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, NOT_IMPLEMENTED}, //TODO: Code this move
		Move{0x37, "Water Gun"   , TYPE_WATER   , SPECIAL ,  40, 100, 25},
		Move{0x38, "Hydro Pump"  , TYPE_WATER   , SPECIAL , 120,  80,  5},
		Move{0x39, "Surf"        , TYPE_WATER   , SPECIAL ,  90, 100, 15},
		Move{0x3A, "Ice Beam"    , TYPE_ICE     , SPECIAL ,  95, 100, 10, {STATUS_FROZEN, 0x1A}},
		Move{0x3B, "Blizzard"    , TYPE_ICE     , SPECIAL , 120,  90,  5, {STATUS_FROZEN, 0x1A}},
		Move{0x3C, "Psybeam"     , TYPE_PSYCHIC , SPECIAL ,  65, 100, 20, {STATUS_CONFUSED, 0x19}},
		Move{0x3D, "Bubblebeam"  , TYPE_WATER   , SPECIAL ,  65, 100, 20, NO_STATUS_CHANGE, {}, {{STATS_SPD, -1, 0x55}}},
		Move{0x3E, "Aurora Beam" , TYPE_ICE     , SPECIAL ,  65, 100, 20, NO_STATUS_CHANGE, {}, {{STATS_ATK, -1, 0x55}}},
		Move{0x3F, "Hyper Beam"  , TYPE_NORMAL  , PHYSICAL, 150,  90,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, true},
		Move{0x40, "Peck"        , TYPE_FLYING  , PHYSICAL,  35, 100, 35},
		Move{0x41, "Drill Peck"  , TYPE_FLYING  , PHYSICAL,  80, 100, 20},
		Move{0x42, "Submission"  , TYPE_FIGHTING, PHYSICAL,  80, 80, 25, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, TAKE_QUARTER_MOVE_DAMAGE},
		Move{0x43, "Low Kick"    , TYPE_FIGHTING, PHYSICAL,  50, 100, 20, {STATUS_FLINCHED, 0x4D}},
		Move{0x44, "Counter"     , TYPE_FIGHTING, PHYSICAL,  80,  80, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, -5, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, NOT_IMPLEMENTED}, //TODO: Code this move
		Move{0x45, "Seismic Toss", TYPE_FIGHTING, PHYSICAL,   0, 100, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DEAL_LVL_AS_DAMAGE},
		Move{0x46, "Strength"    , TYPE_NORMAL  , PHYSICAL,  80, 100, 15},
		Move{0x47, "Absorb"      , TYPE_GRASS   , SPECIAL ,  20, 100, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ABSORB_HALF_DAMAGE},
		Move{0x48, "Mega Drain"  , TYPE_GRASS   , SPECIAL ,  40, 100, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ABSORB_HALF_DAMAGE},
		Move{0x49, "Leech Seed"  , TYPE_GRASS   , STATUS  ,   0,  90, 10, {STATUS_LEECHED, 0}},
		Move{0x4A, "Growth"      , TYPE_NORMAL  , STATUS  ,   0, 255, 40, NO_STATUS_CHANGE, {{STATS_ATK, 1, 0}, {STATS_SPE, 1, 0}}},
		Move{0x4B, "Razor Leaf"  , TYPE_GRASS   , SPECIAL ,  55,  95, 25, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE * 8},
		Move{0x4C, "SolarBeam"   , TYPE_GRASS   , SPECIAL , 120, 100, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NEED_LOADING("took in sunlight!")},
		Move{0x4D, "PoisonPowder", TYPE_POISON  , STATUS  ,   0,  75, 35, {STATUS_POISONED, 0}},
		Move{0x4E, "Stun Spore"  , TYPE_GRASS   , STATUS  ,   0,  75, 30, {STATUS_PARALYZED, 0}},
		Move{0x4F, "Sleep Powder", TYPE_GRASS   , STATUS  ,   0,  75, 15, {STATUS_ASLEEP, 0}},
		Move{0x50, "Petal Dance" , TYPE_GRASS   , SPECIAL ,  70, 100, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, {3, 4}, "'s thrashing about!", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, CONFUSE_ON_LAST, CONFUSE_ON_LAST_MISS},
		Move{0x51, "String Shot" , TYPE_BUG     , STATUS  ,   0,  90, 40, NO_STATUS_CHANGE, {}, {{STATS_SPD, 1, 0}}},
		Move{0x52, "Dragon Rage" , TYPE_DRAGON  , SPECIAL ,   0, 100, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DEAL_40_DAMAGE},
		Move{0x53, "Fire Spin"   , TYPE_FIRE    , SPECIAL ,  35,  70, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, TWO_TO_FIVE_HITS, "'s attack continues!", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, WRAP_TARGET, GLITCH_HYPER_BEAM},
		Move{0x54, "ThunderShock", TYPE_ELECTRIC, SPECIAL ,  40, 100, 30, {STATUS_PARALYZED, 0x1A}},
		Move{0x55, "Thunderbolt" , TYPE_ELECTRIC, SPECIAL ,  90, 100, 15, {STATUS_PARALYZED, 0x1A}},
		Move{0x56, "Thunder Wave", TYPE_ELECTRIC, STATUS  ,   0, 100, 30, {STATUS_PARALYZED, 0}},
		Move{0x57, "Thunder"     , TYPE_ELECTRIC, SPECIAL , 120,  70, 10, {STATUS_PARALYZED, 0x1A}},
		Move{0x58, "Rock Throw"  , TYPE_ROCK    , PHYSICAL,  50,  65, 15},
		Move{0x59, "Earthquake"  , TYPE_GROUND  , PHYSICAL, 100, 100, 10},
		Move{0x5A, "Fissure"     , TYPE_GROUND  , PHYSICAL,   0,  30,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ONE_HIT_KO_HANDLE},
		Move{0x5B, "Dig"         , TYPE_GROUND  , PHYSICAL, 100, 100, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NEED_LOADING("dug a hole!"), true},
		Move{0x5C, "Toxic"       , TYPE_POISON  , STATUS  ,   0,  85, 10, {STATUS_BADLY_POISONED, 0}},
		Move{0x5D, "Confusion"   , TYPE_PSYCHIC , SPECIAL ,  50, 100, 25, {STATUS_CONFUSED, 0x19}},
		Move{0x5E, "Psychic"     , TYPE_PSYCHIC , SPECIAL ,  90, 100, 10, NO_STATUS_CHANGE, {}, {{STATS_SPE, -1, 0x55}}},
		Move{0x5F, "Hypnosis"    , TYPE_PSYCHIC , STATUS  ,   0,  60, 20, {STATUS_ASLEEP, 0}},
		Move{0x60, "Meditate"    , TYPE_PSYCHIC , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, {{STATS_ATK, 1, 0}}},
		Move{0x61, "Agility"     , TYPE_PSYCHIC , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, {{STATS_SPD, 2, 0}}},
		Move{0x62, "Quick Attack", TYPE_NORMAL  , PHYSICAL,  40, 100, 30, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 1},
		Move{0x63, "Rage"        , TYPE_NORMAL  , PHYSICAL,  20, 100, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, {0xFFFFFFFF, 0xFFFFFFFF}},
		Move{0x64, "Teleport"    , TYPE_NORMAL  , STATUS  ,   0, 255, 20},
		Move{0x65, "Night Shade" , TYPE_GHOST   , PHYSICAL,   0, 100, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DEAL_LVL_AS_DAMAGE},
		Move{0x66, "Mimic"       , TYPE_NORMAL  , STATUS  ,   0, 255, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, NOT_IMPLEMENTED},
		Move{0x67, "Screech"     , TYPE_NORMAL  , STATUS  ,   0,  85, 40, NO_STATUS_CHANGE, {}, {{STATS_DEF, -2, 0}}},
		Move{0x68, "Double Team" , TYPE_NORMAL  , STATUS  ,   0, 255, 15, NO_STATUS_CHANGE, {{STATS_EVD, 1, 0}}},
		Move{0x69, "Recover"     , TYPE_NORMAL  , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, HEAL_HALF_HEALTH},
		Move{0x6A, "Harden"      , TYPE_NORMAL  , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, {{STATS_DEF, 1, 0}}},
		Move{0x6B, "Minimize"    , TYPE_NORMAL  , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, {{STATS_EVD, 1, 0}}},
		Move{0x6C, "SmokeScreen" , TYPE_NORMAL  , STATUS  ,   0, 100, 20, NO_STATUS_CHANGE, {}, {{STATS_ACC, -1, 0}}},
		Move{0x6D, "Confuse Ray" , TYPE_GHOST   , STATUS  ,   0, 100, 10, {STATUS_CONFUSED, 0}},
		Move{0x6E, "Withdraw"    , TYPE_WATER   , STATUS  ,   0, 255, 40, NO_STATUS_CHANGE, {{STATS_DEF, 1, 0}}},
		Move{0x6F, "Defense Curl", TYPE_NORMAL  , STATUS  ,   0, 255, 40, NO_STATUS_CHANGE, {{STATS_DEF, 1, 0}}},
		Move{0x70, "Barrier"     , TYPE_PSYCHIC , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, {{STATS_DEF, 2, 0}}},
		Move{0x71, "Light Screen", TYPE_PSYCHIC , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, NOT_IMPLEMENTED}, //TODO: Code the move
		Move{0x72, "Haze"        , TYPE_ICE     , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, CANCEL_STATS_CHANGE},
		Move{0x73, "Reflect"     , TYPE_PSYCHIC , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, NOT_IMPLEMENTED}, //TODO: Code the move
		Move{0x74, "Focus Energy", TYPE_NORMAL  , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, SET_USER_CRIT_RATIO_TO_1_QUARTER},
		Move{0x75, "Bide"        , TYPE_NORMAL  , PHYSICAL,   0, 255, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, {2, 3}, "", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, STORE_DAMAGES},
		Move{0x76, "Metronome"   , TYPE_NORMAL  , STATUS  ,   0, 255, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, USE_RANDOM_MOVE},
		Move{0x77, "Mirror Move" , TYPE_NORMAL  , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, USE_LAST_FOE_MOVE},
		Move{0x78, "Selfdestruct", TYPE_NORMAL  , PHYSICAL, 260, 100,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, SUICIDE, SUICIDE_MISS},
		Move{0x79, "Egg Bomb"    , TYPE_NORMAL  , PHYSICAL, 100,  75, 10},
		Move{0x7A, "Lick"        , TYPE_GHOST   , PHYSICAL,  30, 100, 30, {STATUS_PARALYZED, 0x4D}},
		Move{0x7B, "Smog"        , TYPE_POISON  , PHYSICAL,  20,  70, 20, {STATUS_POISONED, 0x67}},
		Move{0x7C, "Sludge"      , TYPE_POISON  , PHYSICAL,  65, 100, 20, {STATUS_POISONED, 0x67}},
		Move{0x7D, "Bone Club"   , TYPE_GROUND  , PHYSICAL,  65,  85, 20},
		Move{0x7E, "Fire Blast"  , TYPE_FIRE    , SPECIAL , 120,  85,  5, {STATUS_BURNED, 0x1A}},
		Move{0x7F, "Waterfall"   , TYPE_WATER   , SPECIAL ,  80, 100, 15},
		Move{0x80, "Clamp"       , TYPE_WATER   , SPECIAL,   35,  75, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, TWO_TO_FIVE_HITS, "'s attack continues!", 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, WRAP_TARGET, GLITCH_HYPER_BEAM},
		Move{0x81, "Swift"       , TYPE_NORMAL  , PHYSICAL,  60, 255, 20},
		Move{0x82, "Skull Bash"  , TYPE_NORMAL  , PHYSICAL, 100, 100, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NEED_LOADING("lowered it's head!")},
		Move{0x83, "Spike Cannon", TYPE_NORMAL  , PHYSICAL,  20, 100, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x84, "Constrict"   , TYPE_NORMAL  , PHYSICAL,  10, 100, 35, NO_STATUS_CHANGE, {}, {{STATS_SPD, -1, 0x55}}},
		Move{0x85, "Amnesia"     , TYPE_PSYCHIC , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, {{STATS_SPE, 2, 0}}},
		Move{0x86, "Kinesis"     , TYPE_PSYCHIC , STATUS  ,   0,  80, 15, NO_STATUS_CHANGE, {}, {{STATS_ACC, -1, 0}}},
		Move{0x87, "Softboiled"  , TYPE_NORMAL  , STATUS  ,   0, 255, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, HEAL_HALF_HEALTH},
		Move{0x88, "Hi Jump Kick", TYPE_FIGHTING, PHYSICAL,  85,  90, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, NO_CALLBACK, TAKE_1DAMAGE},
		Move{0x89, "Glare"       , TYPE_NORMAL  , STATUS  ,   0,  75, 30, {STATUS_PARALYZED, 0}},
		Move{0x8A, "Dream Eater" , TYPE_PSYCHIC , STATUS  ,   0, 100, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ABSORB_HALF_DAMAGE},
		Move{0x8B, "Poison Gas"  , TYPE_POISON  , STATUS  ,   0,  55, 40, {STATUS_POISONED, 0}},
		Move{0x8C, "Barrage"     , TYPE_NORMAL  , PHYSICAL,  15,  85, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x8D, "Leech Life"  , TYPE_BUG     , PHYSICAL,  20, 100, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, ABSORB_HALF_DAMAGE},
		Move{0x8E, "Lovely Kiss" , TYPE_NORMAL  , STATUS  ,   0,  75, 10, {STATUS_ASLEEP, 0}},
		Move{0x8F, "Sky Attack"  , TYPE_FLYING  , PHYSICAL, 140,  90,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NEED_LOADING("is glowing!")},
		Move{0x90, "Transform"   , TYPE_NORMAL  , PHYSICAL,   0, 255, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, TRANSFORM},
		Move{0x91, "Bubble"      , TYPE_WATER   , SPECIAL ,  20, 100, 30, NO_STATUS_CHANGE, {}, {{STATS_SPD, -1, 0x55}}},
		Move{0x92, "Dizzy Punch" , TYPE_NORMAL  , PHYSICAL,  70, 100, 10},
		Move{0x93, "Spore"       , TYPE_GRASS   , STATUS  ,   0, 100, 15, {STATUS_ASLEEP, 0}},
		Move{0x94, "Flash"       , TYPE_NORMAL  , STATUS  ,   0,  70, 20, NO_STATUS_CHANGE, {}, {{STATS_ACC, -1, 0}}},
		Move{0x95, "Psywave"     , TYPE_PSYCHIC , SPECIAL ,   0,  80, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DEAL_1_DAMAGE_TO_1_5_LEVEL_DAMAGE},
		Move{0x96, "Splash"      , TYPE_NORMAL  , STATUS  ,   0, 255, 40},
		Move{0x97, "Acid Armor"  , TYPE_POISON  , STATUS  ,   0, 255, 20, NO_STATUS_CHANGE, {{STATS_DEF, 2, 0}}},
		Move{0x98, "Crabhammer"  , TYPE_WATER   , SPECIAL ,  90,  85, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE * 8},
		Move{0x99, "Explosion"   , TYPE_NORMAL  , PHYSICAL, 340, 100,  5, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, SUICIDE, SUICIDE_MISS},
		Move{0x9A, "Fury Swipes" , TYPE_NORMAL  , PHYSICAL,  18,  80, 15, NO_STATUS_CHANGE, NO_STATS_CHANGE, TWO_TO_FIVE_HITS},
		Move{0x9B, "Bonemerang"  , TYPE_GROUND  , PHYSICAL,  50,  90, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, {2, 2}},
		Move{0x9C, "Rest"        , TYPE_PSYCHIC , STATUS  ,   0, 255, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, HEAL_ALL_HEALTH_AND_SLEEP},
		Move{0x9D, "Rock Slide"  , TYPE_ROCK    , PHYSICAL,  75,  90, 10},
		Move{0x9E, "Hyper Fang"  , TYPE_NORMAL  , PHYSICAL,  80,  90, 15},
		Move{0x9F, "Sharpen"     , TYPE_NORMAL  , STATUS  ,   0,   0, 30, NO_STATUS_CHANGE, {{STATS_ATK, 1, 0}}},
		Move{0xA0, "Conversion"  , TYPE_NORMAL  , STATUS  ,   0, 255, 30, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, CONVERSION},
		Move{0xA1, "Tri Attack"  , TYPE_NORMAL  , PHYSICAL,  80, 100, 10},
		Move{0xA2, "Super Fang"  , TYPE_NORMAL  , PHYSICAL,   0,  90, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, DEAL_HALF_HP_DAMAGE},
		Move{0xA3, "Slash"       , TYPE_NORMAL  , PHYSICAL,  70, 100, 20, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE * 8},
		Move{0xA4, "Substitute"  , TYPE_NORMAL  , STATUS  ,   0,   0, 10, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, NOT_IMPLEMENTED}, //TODO: code this move
		Move{0xA5, "Struggle"    , TYPE_NORMAL  , PHYSICAL,  50, 100, 63, NO_STATUS_CHANGE, NO_STATS_CHANGE, DEFAULT_HITS, ONE_RUN, 0, DEFAULT_CRIT_CHANCE, NO_LOADING, false, false, TAKE_HALF_MOVE_DAMAGE},
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