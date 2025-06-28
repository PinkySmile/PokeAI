//
// Created by Gegel85 on 14/07/2019.
//

#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>
#include "Pokemon.hpp"
#include "PokemonTypes.hpp"
#include "Move.hpp"
#include "BattleHandler.hpp"

#define NBR_2B(byte1, byte2) static_cast<unsigned short>((byte1 << 8U) + byte2)

namespace PokemonGen1
{
	Pokemon::Pokemon(PokemonRandomGenerator &random, const Logger &battleLogger, const std::string &nickname, unsigned char level, const Base &base, const std::vector<Move> &moveSet, bool enemy) :
		_id(base.id),
		_enemy(enemy),
		_lastUsedMove(availableMoves[0x00]),
		_random{&random},
		_nickname{nickname},
		_name{base.name},
		_dvs{0xF, 0xF, 0xF, 0xF, 0xF, 0xF},
		_statExps{0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},
		_baseStats{makeStats(level, base, this->_dvs, this->_statExps)},
		_upgradedStats{0, 0, 0, 0, 0, 0},
		_moveSet{moveSet},
		_types{base.typeA, base.typeB},
		_level(level),
		_catchRate{base.catchRate},
		_storingDamages(false),
		_damagesStored(0),
		_currentStatus(STATUS_NONE),
		_globalCritRatio(-1),
		_battleLogger(&battleLogger)
	{
		if (this->_nickname.size() > 10) {
			this->_log("Warning : nickname is too big");
			this->_nickname = this->_nickname.substr(0, NICK_SIZE);
		}
		for (int i = moveSet.size(); i < 4; i++)
			this->_moveSet.push_back(availableMoves[0]);
		this->_computedStats = this->_baseStats;
	}

	Pokemon::Pokemon(PokemonRandomGenerator &random, const Logger &battleLogger, const std::string &nickname, const std::array<byte, ENCODED_SIZE> &data, bool enemy) :
		_id(data[PACK_SPECIES]),
		_enemy(enemy),
		_lastUsedMove(availableMoves[0x00]),
		_random{&random},
		_nickname{nickname},
		_name{pokemonList.at(data[0]).name},
		_dvs{
			0,
			0,
			static_cast<unsigned short>((data[PACK_DVS_ATK_DEF] >> 4U) & 0xFU),
			static_cast<unsigned short>((data[PACK_DVS_ATK_DEF] >> 0U) & 0xFU),
			static_cast<unsigned short>((data[PACK_DVS_SPD_SPE] >> 4U) & 0xFU),
			static_cast<unsigned short>((data[PACK_DVS_SPD_SPE] >> 0U) & 0xFU)
		},
		_statExps{
			NBR_2B(data[PACK_STAT_EXP_HP_HB], data[PACK_STAT_EXP_HP_LB]),
			NBR_2B(data[PACK_STAT_EXP_HP_HB], data[PACK_STAT_EXP_HP_LB]),
			NBR_2B(data[PACK_STAT_EXP_ATK_HB], data[PACK_STAT_EXP_ATK_LB]),
			NBR_2B(data[PACK_STAT_EXP_DEF_HB], data[PACK_STAT_EXP_DEF_LB]),
			NBR_2B(data[PACK_STAT_EXP_SPD_HB], data[PACK_STAT_EXP_SPD_LB]),
			NBR_2B(data[PACK_STAT_EXP_SPE_HB], data[PACK_STAT_EXP_SPE_LB]),
		},
		_baseStats{
			static_cast<unsigned>(NBR_2B(data[PACK_HP_HB],  data[PACK_HP_LB])),
			static_cast<unsigned>(NBR_2B(data[PACK_MAX_HP_HB], data[PACK_MAX_HP_LB])),
			static_cast<unsigned short>(NBR_2B(data[PACK_ATK_HB], data[PACK_ATK_LB])), //ATK
			static_cast<unsigned short>(NBR_2B(data[PACK_DEF_HB], data[PACK_DEF_LB])), //DEF
			static_cast<unsigned short>(NBR_2B(data[PACK_SPD_HB], data[PACK_SPD_LB])), //SPD
			static_cast<unsigned short>(NBR_2B(data[PACK_SPE_HB], data[PACK_SPE_LB]))  //SPE
		},
		_upgradedStats{0, 0, 0, 0, 0, 0},
		_types{
			static_cast<PokemonTypes>(data[PACK_TYPEA]),
			static_cast<PokemonTypes>(data[PACK_TYPEB])
		},
		_level{data[PACK_CURR_LEVEL]},
		_catchRate{data[PACK_CATCH_RATE]},
		_storingDamages(false),
		_damagesStored(0),
		_currentStatus{data[PACK_STATUS]},
		_globalCritRatio(-1),
		_battleLogger(&battleLogger)
	{
		this->_dvs.maxHP = this->_dvs.HP =
			((this->_dvs.ATK & 0x1U) << 3U) |
			((this->_dvs.DEF & 0b1U) << 2U) |
			((this->_dvs.SPD & 0b1U) << 1U) |
			((this->_dvs.SPE & 0b1U) << 0U);

		this->_moveSet.reserve(4);
		for (int i = 0; i < 4; i++) {
			this->_moveSet.push_back(availableMoves[data[PACK_MOVE1 + i]]);
			this->_moveSet[i].setPPUp(data[PACK_PPS_MOVE1 + i] >> 6U);
			this->_moveSet[i].setPP(data[PACK_PPS_MOVE1 + i] & 0b111111U);
		}
		this->_computedStats = this->_baseStats;
	}

	Pokemon::Pokemon(PokemonRandomGenerator &random, const Pokemon::Logger &battleLogger, const nlohmann::json &json) :
		_oldState{
			.stats = {
				.HP   = json["oldState"]["stats"]["HP"],
				.maxHP= json["oldState"]["stats"]["maxHP"],
				.ATK  = json["oldState"]["stats"]["ATK"],
				.DEF  = json["oldState"]["stats"]["DEF"],
				.SPD  = json["oldState"]["stats"]["SPD"],
				.SPE  = json["oldState"]["stats"]["SPE"],
			},
			.id    = json["oldState"]["id"],
			.moves = {},
			.types = { json["types"]["oldState"][0], json["types"]["oldState"][1] },
		},
		_id(json["id"]),
		_enemy(json["enemy"]),
		_lastUsedMove(availableMoves[json["lastUsedMove"]["id"]]),
		_random{&random},
		_nickname{json["nickname"]},
		_name{json["name"]},
		_dvs{
			.HP   = json["dvs"]["HP"],
			.maxHP= json["dvs"]["maxHP"],
			.ATK  = json["dvs"]["ATK"],
			.DEF  = json["dvs"]["DEF"],
			.SPD  = json["dvs"]["SPD"],
			.SPE  = json["dvs"]["SPE"],
		},
		_statExps{
			.HP   = json["statExps"]["HP"],
			.maxHP= json["statExps"]["maxHP"],
			.ATK  = json["statExps"]["ATK"],
			.DEF  = json["statExps"]["DEF"],
			.SPD  = json["statExps"]["SPD"],
			.SPE  = json["statExps"]["SPE"],
		},
		_baseStats{
			.HP   = json["baseStats"]["HP"],
			.maxHP= json["baseStats"]["maxHP"],
			.ATK  = json["baseStats"]["ATK"],
			.DEF  = json["baseStats"]["DEF"],
			.SPD  = json["baseStats"]["SPD"],
			.SPE  = json["baseStats"]["SPE"],
		},
		_upgradedStats{
			.ATK = json["upgradedStats"]["ATK"].get<char>(),
			.DEF = json["upgradedStats"]["DEF"].get<char>(),
			.SPD = json["upgradedStats"]["SPD"].get<char>(),
			.SPE = json["upgradedStats"]["SPE"].get<char>(),
			.EVD = json["upgradedStats"]["EVA"].get<char>(),
			.ACC = json["upgradedStats"]["ACC"].get<char>(),
		},
		_types{ json["types"][0], json["types"][1] },
		_level{json["level"]},
		_catchRate{json["catchRate"]},
		_storingDamages(json["storingDamages"]),
		_damagesStored(json["damagesStored"]),
		_currentStatus{json["currentStatus"]},
		_globalCritRatio(json["globalCritRatio"]),
		_battleLogger(&battleLogger)
	{
		this->_moveSet.reserve(4);
		for (auto &move : json["moveSet"]) {
			this->_moveSet.push_back(availableMoves[move["id"]]);
			this->_moveSet.back().setPP(move["pp"]);
			this->_moveSet.back().setPPUp(move["ppup"]);
		}
		this->_oldState.moves.reserve(4);
		for (auto &move : json["moveSet"]) {
			this->_oldState.moves.push_back(availableMoves[move["id"]]);
			this->_oldState.moves.back().setPP(move["pp"]);
			this->_oldState.moves.back().setPPUp(move["ppup"]);
		}
		this->_lastUsedMove.setHitsLeft(json["lastUsedMove"]["hitsLeft"]);
		this->_computedStats = this->_baseStats;
	}

	Pokemon::BaseStats Pokemon::makeStats(unsigned char level, const Base &base, const BaseStats &dvs, const BaseStats &evs)
	{
		auto formula = [level](unsigned short baseStat, unsigned short IV, unsigned short EV){
			unsigned short E = // Fixed formula via http://www.smogon.com/ingame/guides/rby_gsc_stats
				std::floor(
					std::min<unsigned>(
						255,
						std::floor(
							std::sqrt(
								std::max(0, EV - 1)
							) + 1
						)
					) / 4.0
				);

			return std::floor<unsigned short>((2 * (baseStat + IV) + E) * level / 100.0 + 5);
		};

		auto hp = fmin(999, fmax(1, formula(base.HP, dvs.HP, evs.HP) + 5 + level));

		return {
			static_cast<unsigned>(hp),
			static_cast<unsigned>(hp),
			static_cast<unsigned short>(fmin(999, fmax(1, formula(base.ATK, dvs.ATK, evs.ATK)))),
			static_cast<unsigned short>(fmin(999, fmax(1, formula(base.DEF, dvs.DEF, evs.DEF)))),
			static_cast<unsigned short>(fmin(999, fmax(1, formula(base.SPD, dvs.SPD, evs.SPD)))),
			static_cast<unsigned short>(fmin(999, fmax(1, formula(base.SPE, dvs.SPE, evs.SPE))))
		};
	}

	void Pokemon::setGlobalCritRatio(double ratio)
	{
		this->_globalCritRatio = ratio;
	}

	void Pokemon::setStatus(PokemonGen1::StatusChange status)
	{
		this->_currentStatus = STATUS_NONE;
		this->addStatus(status);
	}

	void Pokemon::setStatus(PokemonGen1::StatusChange status, unsigned duration)
	{
		this->_currentStatus = STATUS_NONE;
		this->addStatus(status, duration);
	}

	bool Pokemon::addStatus(StatusChange status)
	{
		unsigned char randomVal = 0;

		//TODO: Add the Sleep + Hyper beam glitch
		if (status == STATUS_NONE)
			return true;

		if (!this->canHaveStatus(status))
			return false;

		switch (status) {
		case STATUS_ASLEEP:
			while (!randomVal)
				randomVal = (*this->_random)() & 7;
			return this->addStatus(STATUS_ASLEEP_FOR_1_TURN, randomVal);
		case STATUS_CONFUSED:
			return this->addStatus(STATUS_CONFUSED_FOR_1_TURN, ((*this->_random)() & 3) + 2);
		default:
			return this->addStatus(status, 1);
		}
	}

	bool Pokemon::addStatus(StatusChange status, unsigned duration)
	{
		if (!this->canHaveStatus(status))
			return false;

		if (status == STATUS_FLINCHED) {
			this->_flinched = true;
			return true;
		}
		this->_log(" is now " + statusToString(status));
		if (status == STATUS_BADLY_POISONED)
			this->_badPoisonStage = 1;
		this->_currentStatus |= (status * duration);
		if (status == STATUS_PARALYZED || status == STATUS_BURNED)
			this->applyStatusDebuff();
		return true;
	}

	std::string Pokemon::dump() const
	{
		std::stringstream stream;

		stream << std::setw(10) << this->getName(false) << " (" << std::setw(10) << this->_name << ") l" << std::setw(3) << static_cast<int>(this->_level);
		stream << ", " << typeToStringShort(this->_types.first);
		if (this->_types.first != this->_types.second)
			stream << "/" << typeToStringShort(this->_types.second);
		else
			stream << "    ";
		stream << ", " << std::setw(3) << this->getHealth()   << "/" << std::setw(3) << this->getMaxHealth() << "HP";
		stream << ", " << std::setw(3) << this->getAttack()   << "ATK (" << std::setw(3) << this->getRawAttack()  << "@" << std::showpos << static_cast<int>(this->_upgradedStats.ATK) << std::noshowpos << ")";
		stream << ", " << std::setw(3) << this->getDefense()  << "DEF (" << std::setw(3) << this->getRawDefense() << "@" << std::showpos << static_cast<int>(this->_upgradedStats.DEF) << std::noshowpos << ")";
		stream << ", " << std::setw(3) << this->getSpecial()  << "SPE (" << std::setw(3) << this->getRawSpecial() << "@" << std::showpos << static_cast<int>(this->_upgradedStats.SPE) << std::noshowpos << ")";
		stream << ", " << std::setw(3) << this->getSpeed()    << "SPD (" << std::setw(3) << this->getRawSpeed()   << "@" << std::showpos << static_cast<int>(this->_upgradedStats.SPD) << std::noshowpos << ")";
		stream << ", " << std::setprecision(4) << this->getAccuracy() * 100 << "%ACC (" << std::showpos << static_cast<int>(this->_upgradedStats.ACC) << std::noshowpos << ")";
		stream << ", " << std::setprecision(4) << this->getEvasion()  * 100 << "%EVD (" << std::showpos << static_cast<int>(this->_upgradedStats.EVD) << std::noshowpos << ")";
		stream << ", Status: " << std::hex << static_cast<int>(this->_currentStatus) << " ";
		if (this->_currentStatus) {
			for (unsigned i = 0; i < sizeof(this->_currentStatus) * 8; i++)
				if (this->_currentStatus & (1U << i))
					stream << statusToString(static_cast<StatusChange>(1U << i)) << ", ";
		} else
			stream << "OK, ";
		stream << "Moves: ";
		if (this->_moveSet.empty())
			stream << "None";

		bool first = true;

		for (const Move &move : this->_moveSet) {
			if (move.getID() == 0)
				continue;
			if (!first)
				stream << ", ";
			stream << move.getName() << " " << std::dec << static_cast<int>(move.getPP()) << "/" << static_cast<int>(move.getMaxPP()) << "PP";
			first = false;
		}
		return stream.str();
	}

	nlohmann::json Pokemon::serialize() const
	{
		nlohmann::json json{
			{ "id",              this->_id },
			{ "flinched",        this->_flinched },
			{ "needsRecharge",   this->_needsRecharge },
			{ "invincible",      this->_invincible },
			{ "enemy",           this->_enemy },
			{ "moveSet",         nlohmann::json::array() },
			{ "types",           { this->_types.first, this->_types.second } },
			{ "level",           this->_level },
			{ "catchRate",       this->_catchRate },
			{ "transformed",     this->_transformed },
			{ "wrapped",         this->_wrapped },
			{ "storingDamages",  this->_storingDamages },
			{ "damagesStored",   this->_damagesStored },
			{ "badPoisonStage",  this->_badPoisonStage },
			{ "currentStatus",   this->_currentStatus },
			{ "globalCritRatio", this->_globalCritRatio },
			{ "nickname",        this->_nickname },
			{ "name",            this->_name },
			{ "oldState", {
				{ "stats", {
					{ "HP",    this->_oldState.stats.HP },
					{ "maxHP", this->_oldState.stats.maxHP },
					{ "ATK",   this->_oldState.stats.ATK },
					{ "DEF",   this->_oldState.stats.DEF },
					{ "SPD",   this->_oldState.stats.SPD },
					{ "SPE",   this->_oldState.stats.SPE }
				} },
				{ "id", this->_oldState.id },
				{ "moves", nlohmann::json::array() }
			} },
			{ "lastUsedMove", {
				{ "id",       this->_lastUsedMove.getID() },
				{ "hitsLeft", this->_lastUsedMove.getHitsLeft() }
			} },
			{ "dvs", {
				{ "HP",    this->_dvs.HP },
				{ "maxHP", this->_dvs.maxHP },
				{ "ATK",   this->_dvs.ATK },
				{ "DEF",   this->_dvs.DEF },
				{ "SPD",   this->_dvs.SPD },
				{ "SPE",   this->_dvs.SPE }
			} },
			{ "statExps", {
				{ "HP",    this->_statExps.HP },
				{ "maxHP", this->_statExps.maxHP },
				{ "ATK",   this->_statExps.ATK },
				{ "DEF",   this->_statExps.DEF },
				{ "SPD",   this->_statExps.SPD },
				{ "SPE",   this->_statExps.SPE }
			} },
			{ "baseStats", {
				{ "HP",    this->_baseStats.HP },
				{ "maxHP", this->_baseStats.maxHP },
				{ "ATK",   this->_baseStats.ATK },
				{ "DEF",   this->_baseStats.DEF },
				{ "SPD",   this->_baseStats.SPD },
				{ "SPE",   this->_baseStats.SPE }
			} },
			{ "computedStats", {
				{ "HP",    this->_computedStats.HP },
				{ "maxHP", this->_computedStats.maxHP },
				{ "ATK",   this->_computedStats.ATK },
				{ "DEF",   this->_computedStats.DEF },
				{ "SPD",   this->_computedStats.SPD },
				{ "SPE",   this->_computedStats.SPE }
			} },
			{ "upgradedStats", {
				{ "ATK", this->_upgradedStats.ATK },
				{ "DEF", this->_upgradedStats.DEF },
				{ "SPD", this->_upgradedStats.SPD },
				{ "SPE", this->_upgradedStats.SPE },
				{ "EVA", this->_upgradedStats.EVD },
				{ "ACC", this->_upgradedStats.ACC },
			} }
		};

		for (auto &move : this->_oldState.moves) {
			nlohmann::json j{
				{ "id", move.getID() },
				{ "pp", move.getPP() },
				{ "ppUp", move.getPPUp() }
			};

			json["oldState"]["moves"].push_back(j);
		}
		for (auto &move : this->_moveSet) {
			nlohmann::json j{
				{ "id", move.getID() },
				{ "pp", move.getPP() },
				{ "ppUp", move.getPPUp() }
			};

			json["moveSet"].push_back(j);
		}
		return json;
	}

	void Pokemon::switched()
	{
		this->resetStatsChanges();
		this->_lastUsedMove = availableMoves[0x00];
		if (this->_transformed) {
			this->_id = this->_oldState.id;
			this->_moveSet = this->_oldState.moves;
			this->_baseStats.ATK = this->_oldState.stats.ATK;
			this->_baseStats.DEF = this->_oldState.stats.DEF;
			this->_baseStats.SPD = this->_oldState.stats.SPD;
			this->_baseStats.SPE = this->_oldState.stats.SPE;
			this->_types = this->_oldState.types;
			this->_transformed = false;
		}
		if (this->_currentStatus == STATUS_BADLY_POISONED)
			this->_currentStatus = STATUS_POISONED;
	}

	void Pokemon::resetStatsChanges()
	{
		this->_upgradedStats = {0, 0, 0, 0, 0, 0};
	}

	int Pokemon::getPriorityFactor(unsigned char moveSlot)
	{
		const Move *move = &this->_lastUsedMove;

		if (moveSlot == 0xE)
			move = &availableMoves[Struggle];
		else if (this->_lastUsedMove.isFinished())
			move = moveSlot < this->_moveSet.size() ? &this->_moveSet[moveSlot] : nullptr;
		return (move ? move->getPriority() : 0) * 262140 + static_cast<int>(this->getSpeed());
	}

	void Pokemon::useMove(const Move &move, Pokemon &target)
	{
		if (this->_lastUsedMove.isFinished())
			this->_lastUsedMove = move;
		if (!this->_lastUsedMove.attack(*this, target, *this->_battleLogger))
			this->_log("'s attack missed");
	}

	void Pokemon::storeDamages(bool active)
	{
		if (!active)
			this->_damagesStored = 0;
		else if (!this->_storingDamages)
			this->_log(" is storing damages");
		this->_storingDamages = active;
	}

	bool Pokemon::hasStatus(StatusChange status) const
	{
		return this->_currentStatus & status;
	}

	PokemonRandomGenerator &Pokemon::getRandomGenerator()
	{
		return *this->_random;
	}

	unsigned Pokemon::getDamagesStored() const
	{
		return this->_damagesStored;
	}

	unsigned char Pokemon::getID() const
	{
		return this->_id;
	}

	void Pokemon::setWrapped(bool isWrapped)
	{
		this->_wrapped = isWrapped;
	}

	unsigned Pokemon::getSpeed() const
	{
		return this->_computedStats.SPD;
	}

	unsigned Pokemon::getAttack() const
	{
		return this->_computedStats.ATK;
	}

	unsigned Pokemon::getSpecial() const
	{
		return this->_computedStats.SPE;
	}

	unsigned Pokemon::getDefense() const
	{
		return this->_computedStats.DEF;
	}

	void Pokemon::endTurn()
	{
		this->_flinched = false;
		this->_wrapped = false;
		if (this->_currentStatus & STATUS_BURNED) {
			this->_log("'s hurt by the burn");
			this->takeDamage(this->getHealth() / 16);
		} else if ((this->_currentStatus & STATUS_POISONED) || (this->_currentStatus & STATUS_BADLY_POISONED)) {
			this->_log("'s hurt by the poison");
			if (this->_currentStatus & STATUS_BADLY_POISONED)
				this->takeDamage(this->getHealth() * this->_badPoisonStage++ / 16);
			else
				this->takeDamage(this->getHealth() / 16);
		}
	}

	void Pokemon::_log(const std::string &msg) const
	{
		(*this->_battleLogger)(this->getName() + msg);
	}

	void Pokemon::attack(unsigned char moveSlot, Pokemon &target)
	{
		if (this->_wrapped) {
			this->_log(" can't move");
			return;
		}

		if (this->_flinched) {
			this->_log(" flinched");
			return;
		}

		if (this->_currentStatus & STATUS_ASLEEP) {
			this->_currentStatus--;
			if (this->_currentStatus & STATUS_ASLEEP)
				this->_log(" is fast asleep");
			else {
				this->_log(" woke up");
				this->_currentStatus &= ~STATUS_ANY_NON_VOLATILE_STATUS;
			}
			return;
		}

		if (this->_currentStatus & STATUS_FROZEN) {
			this->_log(" is frozen solid");
			return;
		}

		if (this->_currentStatus & STATUS_CONFUSED) {
			this->_currentStatus -= STATUS_CONFUSED_FOR_1_TURN;
			if ((this->_currentStatus & STATUS_CONFUSED)) {
				this->_log(" is confused");
				if ((*this->_random)() >= 0x80) {
					this->setRecharging(false);
					this->_log(" hurts itself in it's confusion");
					this->takeDamage(this->calcDamage(*this, 40, TYPE_NEUTRAL_PHYSICAL, PHYSICAL, false, false).damage);
					this->_lastUsedMove = availableMoves[0x00];
					return;
				}
			} else if (!(this->_currentStatus & STATUS_CONFUSED))
				this->_log(" is confused no more");
		}

		if ((this->_currentStatus & STATUS_PARALYZED) && (*this->_random)() < 0x3F) {
			this->_log("'s fully paralyzed");
			this->_lastUsedMove = availableMoves[0x00];
			return;
		}

		if (this->_needsRecharge) {
			this->_log("must recharge");
			return;
		}

		if (moveSlot >= 4)
			this->useMove(availableMoves[Struggle], target);
		else if (moveSlot < this->_moveSet.size() && this->_moveSet[moveSlot].getID()) {
			this->useMove(this->_moveSet[moveSlot], target);
			this->_moveSet[moveSlot].setPP(this->_moveSet[moveSlot].getPP() ? this->_moveSet[moveSlot].getPP() - 1 : 63);
		}
	}

	unsigned char Pokemon::getLevel() const
	{
		return this->_level;
	}

	unsigned Pokemon::getHealth() const
	{
		return this->_computedStats.HP;
	}

	unsigned Pokemon::getMaxHealth() const
	{
		return this->_computedStats.maxHP;
	}

	const Move &Pokemon::getLastUsedMove() const
	{
		return this->_lastUsedMove;
	}

	bool Pokemon::changeStat(StatsChange stat, char nb)
	{
		std::string statName;
		auto stats = reinterpret_cast<char *>(&this->_upgradedStats);
		auto bStats = reinterpret_cast<unsigned short *>(&this->_baseStats.ATK);
		auto cStats = reinterpret_cast<unsigned short *>(&this->_computedStats.ATK);

		if (!nb)
			return false;

		statName = statToString(stat);
		if ((stats[stat] >= 6 && nb > 0) || (stats[stat] <= -6 && nb < 0)) {
			(*this->_battleLogger)("Nothing happened");
			return false;
		}
		if ((stat == STATS_EVD || stat == STATS_ACC) && cStats[stat] == 999) {
			(*this->_battleLogger)("Nothing happened");
			return false;
		}

		stats[stat] += nb;
		if (stats[stat] > 6)
			stats[stat] = 6;
		else if (stats[stat] < -6)
			stats[stat] = -6;

		if (stat != STATS_EVD && stat != STATS_ACC)
			cStats[stat] = fmin(999, this->_getUpgradedStat(bStats[stat], stats[stat]));
		if (nb < -1)
			this->_log("'s " + statName + " greatly fell");
		else if (nb == -1)
			this->_log("'s " + statName + " fell");
		else if (nb == 1)
			this->_log("'s " + statName + " rose");
		else if (nb > 1)
			this->_log("'s " + statName + " greatly rose");
		return true;
	}

	void Pokemon::takeDamage(int damage)
	{
		if (!this->_computedStats.HP)
			return;

		if (!damage)
			return;

		if (damage > static_cast<int>(this->_computedStats.HP))
			this->_computedStats.HP = 0;
		else if (damage < static_cast<int>(this->_computedStats.HP - this->_computedStats.maxHP))
			this->_computedStats.HP = this->_computedStats.maxHP;
		else
			this->_computedStats.HP -= damage;

		if (!this->_computedStats.HP)
			this->_log(" fainted");
	}

	bool Pokemon::canGetHit() const
	{
		return !this->_invincible;
	}

	std::pair<PokemonTypes, PokemonTypes> Pokemon::getTypes() const
	{
		return this->_types;
	}

	void Pokemon::setTypes(std::pair<PokemonTypes, PokemonTypes> types)
	{
		this->_types = types;
	}

	unsigned Pokemon::getRawAttack() const
	{
		return this->_baseStats.ATK;
	}

	unsigned Pokemon::getRawSpecial() const
	{
		return this->_baseStats.SPE;
	}

	unsigned Pokemon::getRawDefense() const
	{
		return this->_baseStats.DEF;
	}

	unsigned Pokemon::getRawSpeed() const
	{
		return this->_baseStats.SPD;
	}

	Pokemon::DamageResult Pokemon::calcDamage(Pokemon &target, unsigned power, PokemonTypes damageType, MoveCategory category, bool critical, bool randomized) const
	{
		double effectiveness = getAttackDamageMultiplier(damageType, target.getTypes());

		std::cout << this->getName() << " calc damage for " << power << " " << damageType << " " << category << " " << critical << " " << randomized << std::endl;
		if (effectiveness == 0)
			return {
				.critical = false,
				.damage = 0,
				.affect = false,
				.isVeryEffective = false,
				.isNotVeryEffective = false,
			};

		unsigned defense;
		unsigned attack;
		unsigned level = this->_level * (1 + critical);

		switch (category) {
		case SPECIAL:
			attack  = critical ? this->getRawSpecial()  : this->getSpecial();
			defense = critical ? target.getRawSpecial() : target.getSpecial();
			break;
		case PHYSICAL:
			attack =  critical ? this->getRawAttack()   : this->getAttack();
			defense = critical ? target.getRawDefense() : target.getDefense();
			break;
		default:
			return {
				.critical = false,
				.damage = 0,
				.affect = true,
				.isVeryEffective = false,
				.isNotVeryEffective = false,
			};
		}

		std::cout << "def " << defense << ", atk " << attack << std::endl;
		if (attack > 255 || defense > 255) {
			attack = attack / 4 % 256;
			defense = defense / 4 % 256;
		}
		std::cout << "def " << defense << ", atk " << attack << std::endl;

		unsigned char r = 255;

		if (randomized)
			do {
				r = (*this->_random)();
				r = (r >> 1U) | ((r & 0x01U) << 7U);
			} while (r < 217);

		std::cout << "Roll " << (int)r << std::endl;
		//From Zarel/honko-damagecalc ->
		//https://github.com/Zarel/honko-damagecalc/blob/dfff275e362ede0857b7564b3e5e2e6fc0e6782d/calc/src/mechanics/gen1.ts#L95
		double damage = floor(floor(floor(2. * level / 5 + 2) * attack * power / defense) / 50);

		if (damage > 997)
			damage = 997;
		damage += 2;
		if (damage < 1)
			damage = 1;
		std::cout << "Base damage " << damage << std::endl;
		damage *= effectiveness;
		damage *= r / 255.;
		if (this->_types.first == damageType || this->_types.second == damageType)
			damage *= 1.5;
		std::cout << "Resulting " << damage << std::endl;

		return {
			.critical = critical,
			.damage = static_cast<unsigned int>(floor(damage)),
			.affect = true,
			.isVeryEffective = effectiveness > 1,
			.isNotVeryEffective = effectiveness < 1,
		};
	}

	std::string Pokemon::getName(bool hasEnemy) const
	{
		std::string str = this->getNickname();

		if (str.empty())
			str = this->getSpeciesName();
		if (this->_enemy && hasEnemy)
			return "Enemy " + str;
		return str;
	}

	double Pokemon::getAccuracy() const
	{
		return this->_getUpgradedStat(1, this->_upgradedStats.ACC);
	}

	double Pokemon::getEvasion() const
	{
		return this->_getUpgradedStat(1, -this->_upgradedStats.EVD);
	}

	#define LOW(b) static_cast<unsigned char>((b) & 0xFFU)
	#define HIGH(b) static_cast<unsigned char>(((b) >> 8U) & 0xFFU)
	#define SHORT(b) HIGH(b), LOW(b)
	#define PAIR(b) LOW(b.first), LOW(b.second)
	#define MOVE(b, i) (b.size() > i ? LOW(b[i].getID()) : static_cast<unsigned char>(0))
	#define PP(b, i) (b.size() > i ? LOW(((b[i].getPPUp() & 0b11U) << 6U) | (b[i].getPP() & 0b111111U)) : static_cast<unsigned char>(0))

	std::array<unsigned char, Pokemon::ENCODED_SIZE> Pokemon::encode() const
	{
		return {
			/*  0 */ this->_id,
			/*  1 */ SHORT(this->_baseStats.HP),
			/*  3 */ this->_level,
			/*  4 */ LOW(this->_currentStatus),
			/*  5 */ PAIR(this->_types),
			/*  7 */ this->_catchRate,
			/*  8 */ MOVE(this->_moveSet, 0),
			/*  9 */ MOVE(this->_moveSet, 1),
			/* 10 */ MOVE(this->_moveSet, 2),
			/* 11 */ MOVE(this->_moveSet, 3),
			/* 12 */ SHORT(0), // Trainer ID,
			/* 14 */ 0, 0, 0,  // EXP
			/* 17 */ SHORT(this->_statExps.HP),
			/* 19 */ SHORT(this->_statExps.ATK),
			/* 21 */ SHORT(this->_statExps.DEF),
			/* 23 */ SHORT(this->_statExps.SPD),
			/* 25 */ SHORT(this->_statExps.SPE),
			/* 27 */ LOW(this->_dvs.SPD << 4U | this->_dvs.SPE),
			/* 28 */ LOW(this->_dvs.ATK << 4U | this->_dvs.DEF),
			/* 29 */ PP(this->_moveSet, 0),
			/* 30 */ PP(this->_moveSet, 1),
			/* 31 */ PP(this->_moveSet, 2),
			/* 32 */ PP(this->_moveSet, 3),
			/* 33 */ this->_level,
			/* 34 */ SHORT(this->_baseStats.maxHP),
			/* 36 */ SHORT(this->_baseStats.ATK),
			/* 38 */ SHORT(this->_baseStats.DEF),
			/* 40 */ SHORT(this->_baseStats.SPD),
			/* 42 */ SHORT(this->_baseStats.SPE),
			// 44
		};
	}

	const std::vector<Move> Pokemon::getMoveSet() const
	{
		return this->_moveSet;
	}

	double Pokemon::_getUpgradedStat(unsigned short baseValue, char upgradeStage) const
	{
		if (upgradeStage < 0)
			return 2. * baseValue / (2 - upgradeStage);
		return (upgradeStage + 2.) * baseValue / 2;
	}

	Pokemon::UpgradableStats Pokemon::getStatsUpgradeStages() const
	{
		return this->_upgradedStats;
	}

	std::string Pokemon::getSpeciesName() const
	{
		return this->_name;
	}

	std::string Pokemon::getNickname() const
	{
		return this->_nickname;
	}

	void Pokemon::transform(const PokemonGen1::Pokemon &target)
	{
		this->_oldState.id = this->_id;
		this->_oldState.moves = this->_moveSet;
		this->_oldState.types = this->_types;
		this->_oldState.stats = this->_baseStats;

		auto stats = target.getBaseStats();

		this->_id = target.getID();
		this->_baseStats.ATK = stats.ATK;
		this->_baseStats.DEF = stats.DEF;
		this->_baseStats.SPD = stats.SPD;
		this->_baseStats.SPE = stats.SPE;
		this->_upgradedStats = target.getStatsUpgradeStages();
		this->_moveSet = target.getMoveSet();
		this->_types = target.getTypes();
		for (auto &move : this->_moveSet)
			move.setPP(5);
		this->_transformed = true;
	}

	Pokemon::BaseStats Pokemon::getBaseStats() const
	{
		return this->_baseStats;
	}

	void Pokemon::setRecharging(bool recharging)
	{
		this->_needsRecharge = recharging * 2;
	}

	const Pokemon::BaseStats &Pokemon::getDvs() const
	{
		return this->_dvs;
	}

	const Pokemon::BaseStats &Pokemon::getStatExps() const
	{
		return this->_statExps;
	}

	void Pokemon::setStatExps(const BaseStats &statExps)
	{
		this->_statExps = statExps;
	}

	void Pokemon::setId(unsigned char id, bool recomputeStats)
	{
		auto &base = pokemonList.at(id);

		this->_id = base.id;
		this->_name = base.name;
		if (recomputeStats) {
			this->_baseStats = makeStats(this->_level, base, this->_dvs, this->_statExps);
			this->_computedStats = this->_baseStats;
			this->_catchRate = base.catchRate;
		}
	}

	void Pokemon::setNickname(const std::string &nickname)
	{
		this->_nickname = nickname;
		if (this->_nickname.size() > 10) {
			this->_log(" Warning : nickname is too big");
			this->_nickname = this->_nickname.substr(0, 10);
		}
	}

	void Pokemon::setLevel(unsigned char level)
	{
		this->_level = level;
		this->_baseStats = makeStats(level, pokemonList.at(this->getID()), this->_dvs, this->_statExps);
		this->_computedStats = this->_baseStats;
	}

	void Pokemon::setMove(unsigned char index, const Move &move)
	{
		if (index > 4)
			throw std::out_of_range("");
		this->_moveSet[index] = move;
	}

	void Pokemon::setInvincible(bool invincible)
	{
		this->_invincible = invincible;
	}

	bool Pokemon::canHaveStatus(StatusChange status) const
	{
		if (status == STATUS_NONE)
			return false;

		if (status == STATUS_BURNED && (this->_types.first == TYPE_FIRE || this->_types.second == TYPE_FIRE))
			return false;

		if (
			(status == STATUS_POISONED || status == STATUS_BADLY_POISONED) &&
			(this->_types.first == TYPE_POISON || this->_types.second == TYPE_POISON)
		)
			return false;

		if ((status & STATUS_ANY_NON_VOLATILE_STATUS) && (this->_currentStatus & STATUS_ANY_NON_VOLATILE_STATUS))
			return false;

		return !this->hasStatus(status);
	}

	bool Pokemon::isEnemy() const
	{
		return this->_enemy;
	}

	void Pokemon::reset()
	{
		this->_lastUsedMove = availableMoves[0x00];
		if (this->_transformed) {
			this->_id = this->_oldState.id;
			this->_moveSet = this->_oldState.moves;
			this->_baseStats = this->_oldState.stats;
			this->_types = this->_oldState.types;
			this->_transformed = false;
		}
		this->_baseStats.HP = this->_baseStats.maxHP;
		this->_computedStats = this->_baseStats;
		this->_flinched = false;
		this->_needsRecharge = 0;
		this->_invincible = false;
		this->_lastUsedMove = availableMoves.at(0);
		this->_upgradedStats = {0, 0, 0, 0, 0, 0};
		for (auto &move : this->_moveSet)
			move.reset();
		this->_wrapped = false;
		this->_storingDamages = false;
		this->_damagesStored = 0;
		this->_badPoisonStage = 0;
		this->_currentStatus = STATUS_NONE;
		this->_globalCritRatio = -1;
	}

	void Pokemon::applyStatusDebuff()
	{
		if (this->_currentStatus & STATUS_BURNED)
			this->_computedStats.ATK /= 2;
		if (this->_currentStatus & STATUS_PARALYZED)
			this->_computedStats.SPD /= 4;
	}

	Pokemon::Base::Base(
		unsigned char id,
		std::string name,
		unsigned int HP,
		unsigned short ATK,
		unsigned short DEF,
		unsigned short SPD,
		unsigned short SPE,
		PokemonTypes typeA,
		PokemonTypes typeB,
		unsigned char catchRate,
		unsigned int baseXpYield
	) :
		id(id),
		name(std::move(name)),
		HP(HP),
		ATK(ATK),
		DEF(DEF),
		SPD(SPD),
		SPE(SPE),
		typeA(typeA),
		typeB(typeB),
		catchRate(catchRate),
		baseXpYield(baseXpYield)
	{
		for (int i = 0; i < 256; i++)
			this->statsAtLevel[i] = Pokemon::makeStats(i, *this, {0xF, 0xF, 0xF, 0xF, 0xF, 0xF}, {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF});
	}

	unsigned char Pokemon::UpgradableStats::get(StatsChange stat) const
	{
		switch (stat) {
		case STATS_ATK:
			return this->ATK;
		case STATS_DEF:
			return this->DEF;
		case STATS_SPD:
			return this->SPD;
		case STATS_SPE:
			return this->SPE;
		case STATS_EVD:
			return this->EVD;
		case STATS_ACC:
			return this->ACC;
		default:
			return 0;
		}
	}

	/*
	** From Rhydon
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Tables.cs#L82
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Tables.cs#L202
	*/
	const std::map<unsigned char, Pokemon::Base> pokemonList{
		{ 0x01, { 0x01,      "RHYDON", 105, 130, 120,  40,  45, TYPE_GROUND,   TYPE_ROCK,     60,  204 } },
		{ 0x02, { 0x02,  "KANGASKHAN", 105,  95,  80,  90,  40, TYPE_NORMAL,   TYPE_NORMAL,   45,  175 } },
		{ 0x03, { 0x03,    "NIDORAN~",  46,  57,  40,  50,  40, TYPE_POISON,   TYPE_POISON,   235, 60  } }, // Nidoran♂
		{ 0x04, { 0x04,    "CLEFAIRY",  70,  45,  48,  35,  60, TYPE_NORMAL,   TYPE_NORMAL,   150, 68  } },
		{ 0x05, { 0x05,     "SPEAROW",  40,  60,  30,  70,  31, TYPE_NORMAL,   TYPE_FLYING,   255, 58  } },
		{ 0x06, { 0x06,     "VOLTORB",  40,  30,  50, 100,  55, TYPE_ELECTRIC, TYPE_ELECTRIC, 190, 103 } },
		{ 0x07, { 0x07,    "NIDOKING",  81,  92,  77,  85,  75, TYPE_POISON,   TYPE_GROUND,   45,  195 } },
		{ 0x08, { 0x08,     "SLOWBRO",  95,  75, 110,  30,  80, TYPE_WATER,    TYPE_PSYCHIC,  75,  164 } },
		{ 0x09, { 0x09,     "IVYSAUR",  60,  62,  63,  60,  80, TYPE_GRASS,    TYPE_POISON,   45,  141 } },
		{ 0x0A, { 0x0A,   "EXEGGUTOR",  95,  95,  85,  55, 125, TYPE_GRASS,    TYPE_PSYCHIC,  45,  212 } },
		{ 0x0B, { 0x0B,   "LICKITUNG",  90,  55,  75,  30,  60, TYPE_NORMAL,   TYPE_NORMAL,   45,  127 } },
		{ 0x0C, { 0x0C,   "EXEGGCUTE",  60,  40,  80,  40,  60, TYPE_GRASS,    TYPE_PSYCHIC,  90,  98  } },
		{ 0x0D, { 0x0D,      "GRIMER",  80,  80,  50,  25,  40, TYPE_POISON,   TYPE_POISON,   190, 90  } },
		{ 0x0E, { 0x0E,      "GENGAR",  60,  65,  60, 110, 130, TYPE_GHOST,    TYPE_POISON,   45,  190 } },
		{ 0x0F, { 0x0F,    "NIDORAN`",  55,  47,  52,  41,  40, TYPE_POISON,   TYPE_POISON,   235, 59  } }, // Nidoran♀
		{ 0x10, { 0x10,   "NIDOQUEEN",  90,  82,  87,  76,  75, TYPE_POISON,   TYPE_GROUND,   45,  194 } },
		{ 0x11, { 0x11,      "CUBONE",  50,  50,  95,  35,  40, TYPE_GROUND,   TYPE_GROUND,   190, 87  } },
		{ 0x12, { 0x12,     "RHYHORN",  80,  85,  95,  25,  30, TYPE_GROUND,   TYPE_ROCK,     120, 135 } },
		{ 0x13, { 0x13,      "LAPRAS", 130,  85,  80,  60,  95, TYPE_WATER,    TYPE_ICE,      45,  219 } },
		{ 0x14, { 0x14,    "ARCANINE",  90, 110,  80,  95,  80, TYPE_FIRE,     TYPE_FIRE,     75,  213 } },
		{ 0x15, { 0x15,         "MEW", 100, 100, 100, 100, 100, TYPE_PSYCHIC,  TYPE_PSYCHIC,  45,  64  } },
		{ 0x16, { 0x16,    "GYARADOS",  95, 125,  79,  81, 100, TYPE_WATER,    TYPE_FLYING,   45,  214 } },
		{ 0x17, { 0x17,    "SHELLDER",  30,  65, 100,  40,  45, TYPE_WATER,    TYPE_WATER,    190, 97  } },
		{ 0x18, { 0x18,   "TENTACOOL",  40,  40,  35,  70, 100, TYPE_WATER,    TYPE_POISON,   190, 105 } },
		{ 0x19, { 0x19,      "GASTLY",  30,  35,  30,  80, 100, TYPE_GHOST,    TYPE_POISON,   190, 95  } },
		{ 0x1A, { 0x1A,     "SCYTHER",  70, 110,  80, 105,  55, TYPE_BUG,      TYPE_FLYING,   45,  187 } },
		{ 0x1B, { 0x1B,      "STARYU",  30,  45,  55,  85,  70, TYPE_WATER,    TYPE_WATER,    225, 106 } },
		{ 0x1C, { 0x1C,   "BLASTOISE",  79,  83, 100,  78,  85, TYPE_WATER,    TYPE_WATER,    45,  210 } },
		{ 0x1D, { 0x1D,      "PINSIR",  65, 125, 100,  85,  55, TYPE_BUG,      TYPE_BUG,      45,  200 } },
		{ 0x1E, { 0x1E,     "TANGELA",  65,  55, 115,  60, 100, TYPE_GRASS,    TYPE_GRASS,    45,  166 } },
		{ 0x21, { 0x21,   "GROWLITHE",  55,  70,  45,  60,  50, TYPE_FIRE,     TYPE_FIRE,     190, 91  } },
		{ 0x22, { 0x22,        "ONIX",  35,  45, 160,  70,  30, TYPE_ROCK,     TYPE_GROUND,   45,  108 } },
		{ 0x23, { 0x23,      "FEAROW",  65,  90,  65, 100,  61, TYPE_NORMAL,   TYPE_FLYING,   90,  162 } },
		{ 0x24, { 0x24,      "PIDGEY",  40,  45,  40,  56,  35, TYPE_NORMAL,   TYPE_FLYING,   255, 55  } },
		{ 0x25, { 0x25,    "SLOWPOKE",  90,  65,  65,  15,  40, TYPE_WATER,    TYPE_PSYCHIC,  190, 99  } },
		{ 0x26, { 0x26,     "KADABRA",  40,  35,  30, 105, 120, TYPE_PSYCHIC,  TYPE_PSYCHIC,  100, 145 } },
		{ 0x27, { 0x27,    "GRAVELER",  55,  95, 115,  35,  45, TYPE_ROCK,     TYPE_GROUND,   120, 134 } },
		{ 0x28, { 0x28,     "CHANSEY", 250,   5,   5,  50, 105, TYPE_NORMAL,   TYPE_NORMAL,   30,  255 } },
		{ 0x29, { 0x29,     "MACHOKE",  80, 100,  70,  45,  50, TYPE_FIGHTING, TYPE_FIGHTING, 90,  146 } },
		{ 0x2A, { 0x2A,    "MR. MIME",  40,  45,  65,  90, 100, TYPE_PSYCHIC,  TYPE_PSYCHIC,  45,  136 } },
		{ 0x2B, { 0x2B,   "HITMONLEE",  50, 120,  53,  87,  35, TYPE_FIGHTING, TYPE_FIGHTING, 45,  139 } },
		{ 0x2C, { 0x2C,  "HITMONCHAN",  50, 105,  79,  76,  35, TYPE_FIGHTING, TYPE_FIGHTING, 45,  140 } },
		{ 0x2D, { 0x2D,       "ARBOK",  60,  85,  69,  80,  65, TYPE_POISON,   TYPE_POISON,   90,  147 } },
		{ 0x2E, { 0x2E,    "PARASECT",  60,  95,  80,  30,  80, TYPE_BUG,      TYPE_GRASS,    75,  128 } },
		{ 0x2F, { 0x2F,     "PSYDUCK",  50,  52,  48,  55,  50, TYPE_WATER,    TYPE_WATER,    190, 80  } },
		{ 0x30, { 0x30,     "DROWZEE",  60,  48,  45,  42,  90, TYPE_PSYCHIC,  TYPE_PSYCHIC,  190, 102 } },
		{ 0x31, { 0x31,       "GOLEM",  80, 110, 130,  45,  55, TYPE_ROCK,     TYPE_GROUND,   45,  177 } },
		{ 0x33, { 0x33,      "MAGMAR",  65,  95,  57,  93,  85, TYPE_FIRE,     TYPE_FIRE,     45,  167 } },
		{ 0x35, { 0x35,  "ELECTABUZZ",  65,  83,  57, 105,  85, TYPE_ELECTRIC, TYPE_ELECTRIC, 45,  156 } },
		{ 0x36, { 0x36,    "MAGNETON",  50,  60,  95,  70, 120, TYPE_ELECTRIC, TYPE_ELECTRIC, 60,  161 } },
		{ 0x37, { 0x37,     "KOFFING",  40,  65,  95,  35,  60, TYPE_POISON,   TYPE_POISON,   190, 114 } },
		{ 0x39, { 0x39,      "MANKEY",  40,  80,  35,  70,  35, TYPE_FIGHTING, TYPE_FIGHTING, 190, 74  } },
		{ 0x3A, { 0x3A,        "SEEL",  65,  45,  55,  45,  70, TYPE_WATER,    TYPE_WATER,    190, 100 } },
		{ 0x3B, { 0x3B,     "DIGLETT",  10,  55,  25,  95,  45, TYPE_GROUND,   TYPE_GROUND,   255, 81  } },
		{ 0x3C, { 0x3C,      "TAUROS",  75, 100,  95, 110,  70, TYPE_NORMAL,   TYPE_NORMAL,   45,  211 } },
		{ 0x40, { 0x40,   "FARFETCHD",  52,  65,  55,  60,  58, TYPE_NORMAL,   TYPE_FLYING,   45,  94  } },
		{ 0x41, { 0x41,     "VENONAT",  60,  55,  50,  45,  40, TYPE_BUG,      TYPE_POISON,   190, 75  } },
		{ 0x42, { 0x42,   "DRAGONITE",  91, 134,  95,  80, 100, TYPE_DRAGON,   TYPE_FLYING,   45,  218 } },
		{ 0x46, { 0x46,       "DODUO",  35,  85,  45,  75,  35, TYPE_NORMAL,   TYPE_FLYING,   190, 96  } },
		{ 0x47, { 0x47,     "POLIWAG",  40,  50,  40,  90,  40, TYPE_WATER,    TYPE_WATER,    255, 77  } },
		{ 0x48, { 0x48,        "JYNX",  65,  50,  35,  95,  95, TYPE_ICE,      TYPE_PSYCHIC,  45,  137 } },
		{ 0x49, { 0x49,     "MOLTRES",  90, 100,  90,  90, 125, TYPE_FIRE,     TYPE_FLYING,   3,   217 } },
		{ 0x4A, { 0x4A,    "ARTICUNO",  90,  85, 100,  85, 125, TYPE_ICE,      TYPE_FLYING,   3,   215 } },
		{ 0x4B, { 0x4B,      "ZAPDOS",  90,  90,  85, 100, 125, TYPE_ELECTRIC, TYPE_FLYING,   3,   216 } },
		{ 0x4C, { 0x4C,       "DITTO",  48,  48,  48,  48,  48, TYPE_NORMAL,   TYPE_NORMAL,   35,  61  } },
		{ 0x4D, { 0x4D,      "MEOWTH",  40,  45,  35,  90,  40, TYPE_NORMAL,   TYPE_NORMAL,   255, 69  } },
		{ 0x4E, { 0x4E,      "KRABBY",  30, 105,  90,  50,  25, TYPE_WATER,    TYPE_WATER,    225, 115 } },
		{ 0x52, { 0x52,      "VULPIX",  38,  41,  40,  65,  65, TYPE_FIRE,     TYPE_FIRE,     190, 63  } },
		{ 0x53, { 0x53,   "NINETALES",  73,  76,  75, 100, 100, TYPE_FIRE,     TYPE_FIRE,     75,  178 } },
		{ 0x54, { 0x54,     "PIKACHU",  35,  55,  30,  90,  50, TYPE_ELECTRIC, TYPE_ELECTRIC, 190, 82  } },
		{ 0x55, { 0x55,      "RAICHU",  60,  90,  55, 100,  90, TYPE_ELECTRIC, TYPE_ELECTRIC, 75,  122 } },
		{ 0x58, { 0x58,     "DRATINI",  41,  64,  45,  50,  50, TYPE_DRAGON,   TYPE_DRAGON,   45,  67  } },
		{ 0x59, { 0x59,   "DRAGONAIR",  61,  84,  65,  70,  70, TYPE_DRAGON,   TYPE_DRAGON,   45,  144 } },
		{ 0x5A, { 0x5A,      "KABUTO",  30,  80,  90,  55,  45, TYPE_ROCK,     TYPE_WATER,    45,  119 } },
		{ 0x5B, { 0x5B,    "KABUTOPS",  60, 115, 105,  80,  70, TYPE_ROCK,     TYPE_WATER,    45,  201 } },
		{ 0x5C, { 0x5C,      "HORSEA",  30,  40,  70,  60,  70, TYPE_WATER,    TYPE_WATER,    225, 83  } },
		{ 0x5D, { 0x5D,      "SEADRA",  55,  65,  95,  85,  95, TYPE_WATER,    TYPE_WATER,    75,  155 } },
		{ 0x60, { 0x60,   "SANDSHREW",  50,  75,  85,  40,  30, TYPE_GROUND,   TYPE_GROUND,   255, 93  } },
		{ 0x61, { 0x61,   "SANDSLASH",  75, 100, 110,  65,  55, TYPE_GROUND,   TYPE_GROUND,   90,  163 } },
		{ 0x62, { 0x62,     "OMANYTE",  35,  40, 100,  35,  90, TYPE_ROCK,     TYPE_WATER,    45,  120 } },
		{ 0x63, { 0x63,     "OMASTAR",  70,  60, 125,  55, 115, TYPE_ROCK,     TYPE_WATER,    45,  199 } },
		{ 0x64, { 0x64,  "JIGGLYPUFF", 115,  45,  20,  20,  25, TYPE_NORMAL,   TYPE_NORMAL,   170, 76  } },
		{ 0x65, { 0x65,  "WIGGLYTUFF", 140,  70,  45,  45,  50, TYPE_NORMAL,   TYPE_NORMAL,   50,  109 } },
		{ 0x66, { 0x66,       "EEVEE",  55,  55,  50,  55,  65, TYPE_NORMAL,   TYPE_NORMAL,   45,  92  } },
		{ 0x67, { 0x67,     "FLAREON",  65, 130,  60,  65, 110, TYPE_FIRE,     TYPE_FIRE,     45,  198 } },
		{ 0x68, { 0x68,     "JOLTEON",  65,  65,  60, 130, 110, TYPE_ELECTRIC, TYPE_ELECTRIC, 45,  197 } },
		{ 0x69, { 0x69,    "VAPOREON", 130,  65,  60,  65, 110, TYPE_WATER,    TYPE_WATER,    45,  196 } },
		{ 0x6A, { 0x6A,      "MACHOP",  70,  80,  50,  35,  35, TYPE_FIGHTING, TYPE_FIGHTING, 180, 88  } },
		{ 0x6B, { 0x6B,       "ZUBAT",  40,  45,  35,  55,  40, TYPE_POISON,   TYPE_FLYING,   255, 54  } },
		{ 0x6C, { 0x6C,       "EKANS",  35,  60,  44,  55,  40, TYPE_POISON,   TYPE_POISON,   255, 62  } },
		{ 0x6D, { 0x6D,       "PARAS",  35,  70,  55,  25,  55, TYPE_BUG,      TYPE_GRASS,    190, 70  } },
		{ 0x6E, { 0x6E,   "POLIWHIRL",  65,  65,  65,  90,  50, TYPE_WATER,    TYPE_WATER,    120, 131 } },
		{ 0x6F, { 0x6F,   "POLIWRATH",  90,  85,  95,  70,  70, TYPE_WATER,    TYPE_FIGHTING, 45,  185 } },
		{ 0x70, { 0x70,      "WEEDLE",  40,  35,  30,  50,  20, TYPE_BUG,      TYPE_POISON,   255, 52  } },
		{ 0x71, { 0x71,      "KAKUNA",  45,  25,  50,  35,  25, TYPE_BUG,      TYPE_POISON,   120, 71  } },
		{ 0x72, { 0x72,    "BEEDRILL",  65,  80,  40,  75,  45, TYPE_BUG,      TYPE_POISON,   45,  159 } },
		{ 0x74, { 0x74,      "DODRIO",  60, 110,  70, 100,  60, TYPE_NORMAL,   TYPE_FLYING,   45,  158 } },
		{ 0x75, { 0x75,    "PRIMEAPE",  65, 105,  60,  95,  60, TYPE_FIGHTING, TYPE_FIGHTING, 75,  149 } },
		{ 0x76, { 0x76,     "DUGTRIO",  35,  80,  50, 120,  70, TYPE_GROUND,   TYPE_GROUND,   50,  153 } },
		{ 0x77, { 0x77,    "VENOMOTH",  70,  65,  60,  90,  90, TYPE_BUG,      TYPE_POISON,   75,  138 } },
		{ 0x78, { 0x78,     "DEWGONG",  90,  70,  80,  70,  95, TYPE_WATER,    TYPE_ICE,      75,  176 } },
		{ 0x7B, { 0x7B,    "CATERPIE",  45,  30,  35,  45,  20, TYPE_BUG,      TYPE_BUG,      255, 53  } },
		{ 0x7C, { 0x7C,     "METAPOD",  50,  20,  55,  30,  25, TYPE_BUG,      TYPE_BUG,      120, 72  } },
		{ 0x7D, { 0x7D,  "BUTTERFREE",  60,  45,  50,  70,  80, TYPE_BUG,      TYPE_FLYING,   45,  160 } },
		{ 0x7E, { 0x7E,     "MACHAMP",  90, 130,  80,  55,  65, TYPE_FIGHTING, TYPE_FIGHTING, 45,  193 } },
		{ 0x80, { 0x80,     "GOLDUCK",  80,  82,  78,  85,  80, TYPE_WATER,    TYPE_WATER,    75,  174 } },
		{ 0x81, { 0x81,       "HYPNO",  85,  73,  70,  67, 115, TYPE_PSYCHIC,  TYPE_PSYCHIC,  75,  165 } },
		{ 0x82, { 0x82,      "GOLBAT",  75,  80,  70,  90,  75, TYPE_POISON,   TYPE_FLYING,   90,  171 } },
		{ 0x83, { 0x83,      "MEWTWO", 106, 110,  90, 130, 154, TYPE_PSYCHIC,  TYPE_PSYCHIC,  3,   220 } },
		{ 0x84, { 0x84,     "SNORLAX", 160, 110,  65,  30,  65, TYPE_NORMAL,   TYPE_NORMAL,   25,  154 } },
		{ 0x85, { 0x85,    "MAGIKARP",  20,  10,  55,  80,  20, TYPE_WATER,    TYPE_WATER,    255, 20  } },
		{ 0x88, { 0x88,         "MUK", 105, 105,  75,  50,  65, TYPE_POISON,   TYPE_POISON,   75,  157 } },
		{ 0x8A, { 0x8A,     "KINGLER",  55, 130, 115,  75,  50, TYPE_WATER,    TYPE_WATER,    60,  206 } },
		{ 0x8B, { 0x8B,    "CLOYSTER",  50,  95, 180,  70,  85, TYPE_WATER,    TYPE_ICE,      60,  203 } },
		{ 0x8D, { 0x8D,   "ELECTRODE",  60,  50,  70, 140,  80, TYPE_ELECTRIC, TYPE_ELECTRIC, 60,  150 } },
		{ 0x8E, { 0x8E,    "CLEFABLE",  95,  70,  73,  60,  85, TYPE_NORMAL,   TYPE_NORMAL,   25,  129 } },
		{ 0x8F, { 0x8F,     "WEEZING",  65,  90, 120,  60,  85, TYPE_POISON,   TYPE_POISON,   60,  173 } },
		{ 0x90, { 0x90,     "PERSIAN",  65,  70,  60, 115,  65, TYPE_NORMAL,   TYPE_NORMAL,   90,  148 } },
		{ 0x91, { 0x91,     "MAROWAK",  60,  80, 110,  45,  50, TYPE_GROUND,   TYPE_GROUND,   75,  124 } },
		{ 0x93, { 0x93,     "HAUNTER",  45,  50,  45,  95, 115, TYPE_GHOST,    TYPE_POISON,   90,  126 } },
		{ 0x94, { 0x94,        "ABRA",  25,  20,  15,  90, 105, TYPE_PSYCHIC,  TYPE_PSYCHIC,  200, 73  } },
		{ 0x95, { 0x95,    "ALAKAZAM",  55,  50,  45, 120, 135, TYPE_PSYCHIC,  TYPE_PSYCHIC,  50,  186 } },
		{ 0x96, { 0x96,   "PIDGEOTTO",  63,  60,  55,  71,  50, TYPE_NORMAL,   TYPE_FLYING,   120, 113 } },
		{ 0x97, { 0x97,     "PIDGEOT",  83,  80,  75,  91,  70, TYPE_NORMAL,   TYPE_FLYING,   45,  172 } },
		{ 0x98, { 0x98,     "STARMIE",  60,  75,  85, 115, 100, TYPE_WATER,    TYPE_PSYCHIC,  60,  207 } },
		{ 0x99, { 0x99,   "BULBASAUR",  45,  49,  49,  45,  65, TYPE_GRASS,    TYPE_POISON,   45,  64  } },
		{ 0x9A, { 0x9A,    "VENUSAUR",  80,  82,  83,  80, 100, TYPE_GRASS,    TYPE_POISON,   45,  208 } },
		{ 0x9B, { 0x9B,  "TENTACRUEL",  80,  70,  65, 100, 120, TYPE_WATER,    TYPE_POISON,   60,  205 } },
		{ 0x9D, { 0x9D,     "GOLDEEN",  45,  67,  60,  63,  50, TYPE_WATER,    TYPE_WATER,    225, 111 } },
		{ 0x9E, { 0x9E,     "SEAKING",  80,  92,  65,  68,  80, TYPE_WATER,    TYPE_WATER,    60,  170 } },
		{ 0xA3, { 0xA3,      "PONYTA",  50,  85,  55,  90,  65, TYPE_FIRE,     TYPE_FIRE,     190, 152 } },
		{ 0xA4, { 0xA4,    "RAPIDASH",  65, 100,  70, 105,  80, TYPE_FIRE,     TYPE_FIRE,     60,  192 } },
		{ 0xA5, { 0xA5,     "RATTATA",  30,  56,  35,  72,  25, TYPE_NORMAL,   TYPE_NORMAL,   255, 57  } },
		{ 0xA6, { 0xA6,    "RATICATE",  55,  81,  60,  97,  50, TYPE_NORMAL,   TYPE_NORMAL,   90,  116 } },
		{ 0xA7, { 0xA7,    "NIDORINO",  61,  72,  57,  65,  55, TYPE_POISON,   TYPE_POISON,   120, 118 } },
		{ 0xA8, { 0xA8,    "NIDORINA",  70,  62,  67,  56,  55, TYPE_POISON,   TYPE_POISON,   120, 117 } },
		{ 0xA9, { 0xA9,     "GEODUDE",  40,  80, 100,  20,  30, TYPE_ROCK,     TYPE_GROUND,   255, 86  } },
		{ 0xAA, { 0xAA,     "PORYGON",  65,  60,  70,  40,  75, TYPE_NORMAL,   TYPE_NORMAL,   45,  130 } },
		{ 0xAB, { 0xAB,  "AERODACTYL",  80, 105,  65, 130,  60, TYPE_ROCK,     TYPE_FLYING,   45,  202 } },
		{ 0xAD, { 0xAD,   "MAGNEMITE",  25,  35,  70,  45,  95, TYPE_ELECTRIC, TYPE_ELECTRIC, 190, 89  } },
		{ 0xB0, { 0xB0,  "CHARMANDER",  39,  52,  43,  65,  50, TYPE_FIRE,     TYPE_FIRE,     45,  65  } },
		{ 0xB1, { 0xB1,    "SQUIRTLE",  44,  48,  65,  43,  50, TYPE_WATER,    TYPE_WATER,    45,  66  } },
		{ 0xB2, { 0xB2,  "CHARMELEON",  58,  64,  58,  80,  65, TYPE_FIRE,     TYPE_FIRE,     45,  142 } },
		{ 0xB3, { 0xB3,   "WARTORTLE",  59,  63,  80,  58,  65, TYPE_WATER,    TYPE_WATER,    45,  143 } },
		{ 0xB4, { 0xB4,   "CHARIZARD",  78,  84,  78, 100,  85, TYPE_FIRE,     TYPE_FLYING,   45,  209 } },
		{ 0xB9, { 0xB9,      "ODDISH",  45,  50,  55,  30,  75, TYPE_GRASS,    TYPE_POISON,   255, 78  } },
		{ 0xBA, { 0xBA,       "GLOOM",  60,  65,  70,  40,  85, TYPE_GRASS,    TYPE_POISON,   120, 132 } },
		{ 0xBB, { 0xBB,   "VILEPLUME",  75,  80,  85,  50, 100, TYPE_GRASS,    TYPE_POISON,   45,  184 } },
		{ 0xBC, { 0xBC,  "BELLSPROUT",  50,  75,  35,  40,  70, TYPE_GRASS,    TYPE_POISON,   255, 84  } },
		{ 0xBD, { 0xBD,  "WEEPINBELL",  65,  90,  50,  55,  85, TYPE_GRASS,    TYPE_POISON,   120, 151 } },
		{ 0xBE, { 0xBE,  "VICTREEBEL",  80, 105,  65,  70, 100, TYPE_GRASS,    TYPE_POISON,   45,  191 } },

		{ 0x00, { 0x00,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x1F, { 0x1F,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x20, { 0x20,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x32, { 0x32,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x34, { 0x34,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x38, { 0x38,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x3D, { 0x3D,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x3E, { 0x3E,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x3F, { 0x3F,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x43, { 0x43,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x44, { 0x44,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x45, { 0x45,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x4F, { 0x4F,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x50, { 0x50,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x51, { 0x51,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x5E, { 0x5E,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x5F, { 0x5F,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x56, { 0x56,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x57, { 0x57,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x73, { 0x73,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x79, { 0x79,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x7A, { 0x7A,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x7F, { 0x7F,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x86, { 0x86,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x87, { 0x87,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x89, { 0x89,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x8C, { 0x8C,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x92, { 0x92,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x9C, { 0x9C,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0x9F, { 0x9F,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xA0, { 0xA0,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xA1, { 0xA1,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xA2, { 0xA2,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xAC, { 0xAC,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xAE, { 0xAE,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xAF, { 0xAF,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xB5, { 0xB5,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xB6, { 0xB6,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xB7, { 0xB7,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xB8, { 0xB8,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xBF, { 0xBF,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xC0, { 0xC0,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xC1, { 0xC1,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xC2, { 0xC2,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xC3, { 0xC3,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xC4, { 0xC4,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xC5, { 0xC5,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xC6, { 0xC6,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xC7, { 0xC7,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xC8, { 0xC8,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xC9, { 0xC9,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xCA, { 0xCA,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xCB, { 0xCB,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xCC, { 0xCC,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xCD, { 0xCD,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xCE, { 0xCE,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xCF, { 0xCF,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xD0, { 0xD0,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xD1, { 0xD1,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xD2, { 0xD2,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xD3, { 0xD3,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xD4, { 0xD4,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xD5, { 0xD5,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xD6, { 0xD6,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xD7, { 0xD7,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xD8, { 0xD8,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xD9, { 0xD9,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xDA, { 0xDA,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xDB, { 0xDB,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xDC, { 0xDC,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xDD, { 0xDD,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xDE, { 0xDE,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xDF, { 0xDF,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xE0, { 0xE0,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xE1, { 0xE1,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xE2, { 0xE2,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xE3, { 0xE3,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xE4, { 0xE4,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xE5, { 0xE5,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xE6, { 0xE6,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xE7, { 0xE7,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xE8, { 0xE8,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xE9, { 0xE9,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xEA, { 0xEA,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xEB, { 0xEB,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xEC, { 0xEC,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xED, { 0xED,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xEE, { 0xEE,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xEF, { 0xEF,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xF0, { 0xF0,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xF1, { 0xF1,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xF2, { 0xF2,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xF3, { 0xF3,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xF4, { 0xF4,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xF5, { 0xF5,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xF6, { 0xF6,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xF7, { 0xF7,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xF8, { 0xF8,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xF9, { 0xF9,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xFA, { 0xFA,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xFB, { 0xFB,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xFC, { 0xFC,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xFD, { 0xFD,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xFE, { 0xFE,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
		{ 0xFF, { 0xFF,  "MISSINGNO.",   0,   0,   0,   0,   0, TYPE_INVALID,  TYPE_INVALID,  0,   0   } },
	};
}