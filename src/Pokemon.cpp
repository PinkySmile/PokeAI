//
// Created by Gegel85 on 14/07/2019.
//

#include <iostream>
#include <sstream>
#include "Pokemon.hpp"
#include "PokemonTypes.hpp"
#include "Move.hpp"
#include "GameHandle.hpp"

#define NBR_2B(byte1, byte2) static_cast<unsigned short>((byte1 << 8U) + byte2)

namespace PokemonGen1
{
	Pokemon::Pokemon(PokemonRandomGenerator &random, GameHandle &game, const std::string &nickname, unsigned char level, const PokemonBase &base, const std::vector<Move> &moveSet, bool enemy) :
		_id(base.id),
		_enemy(enemy),
		_lastUsedMove(DEFAULT_MOVE(0x00)),
		_random{random},
		_nickname{nickname},
		_name{base.name},
		_baseStats{makeStats(level, base)},
		_upgradedStats{0, 0, 0, 0, 0, 0},
		_moveSet{moveSet},
		_types{base.typeA, base.typeB},
		_level(level),
		_catchRate{base.catchRate},
		_storingDamages(false),
		_damagesStored(0),
		_currentStatus(STATUS_NONE),
		_globalCritRatio(-1),
		_game(game)
	{
		if (this->_nickname.size() > 10) {
			this->_log(" Warning : nickname is too big");
			this->_nickname = this->_nickname.substr(0, 10);
		}
		for (int i = moveSet.size(); i < 4; i++)
			this->_moveSet.push_back(availableMoves[0]);
	}

	Pokemon::Pokemon(PokemonRandomGenerator &random, GameHandle &game, const std::string &nickname, const std::vector<byte> &data, bool enemy) :
		_id(data[0]),
		_enemy(enemy),
		_lastUsedMove(DEFAULT_MOVE(0x00)),
		_random{random},
		_nickname{nickname},
		_name{pokemonList[data[0]].name},
		_baseStats{
			static_cast<unsigned>(fmin(999, fmax(1, NBR_2B(data[1],  data[2])))),   //HP
			static_cast<unsigned>(fmin(999, fmax(1, NBR_2B(data[34], data[35])))), //maxHP
			static_cast<unsigned short>(fmin(999, fmax(1, NBR_2B(data[36], data[37])))), //ATK
			static_cast<unsigned short>(fmin(999, fmax(1, NBR_2B(data[38], data[39])))), //DEF
			static_cast<unsigned short>(fmin(999, fmax(1, NBR_2B(data[40], data[41])))), //SPD
			static_cast<unsigned short>(fmin(999, fmax(1, NBR_2B(data[42], data[43]))))  //SPE
		},
		_upgradedStats{0, 0, 0, 0, 0, 0},
		_moveSet{
			availableMoves[data[8]],
			availableMoves[data[9]],
			availableMoves[data[10]],
			availableMoves[data[11]]
		},
		_types{
			static_cast<PokemonTypes>(data[5]),
			static_cast<PokemonTypes>(data[6])
		},
		_level{data[33]},
		_catchRate{data[7]},
		_storingDamages(false),
		_damagesStored(0),
		_currentStatus{data[4]},
		_globalCritRatio(-1),
		_game(game)
	{
		this->_moveSet[0].setPPUp(data[29] >> 6U);
		this->_moveSet[1].setPPUp(data[30] >> 6U);
		this->_moveSet[2].setPPUp(data[31] >> 6U);
		this->_moveSet[3].setPPUp(data[32] >> 6U);
		this->_moveSet[0].setPP(data[29] & 0b111111U);
		this->_moveSet[1].setPP(data[30] & 0b111111U);
		this->_moveSet[2].setPP(data[31] & 0b111111U);
		this->_moveSet[3].setPP(data[32] & 0b111111U);
	}

	BaseStats Pokemon::makeStats(unsigned char level, const PokemonBase &base)
	{
		std::function<unsigned short(unsigned short)> fct = [level](unsigned short val){
			return static_cast<unsigned short>(
				(2 * (val + 15.) + 63.75) * level / 100.0 + 5
			);
		};

		auto hp = fmin(999, fmax(1, fct(base.HP))) + 5 + level;

		return {
			static_cast<unsigned>(hp),
			static_cast<unsigned>(hp),
			static_cast<unsigned short>(fmin(999, fmax(1, fct(base.ATK)))),
			static_cast<unsigned short>(fmin(999, fmax(1, fct(base.DEF)))),
			static_cast<unsigned short>(fmin(999, fmax(1, fct(base.SPD)))),
			static_cast<unsigned short>(fmin(999, fmax(1, fct(base.SPE))))
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
		if (status == STATUS_NONE)
			return true;
		switch (status) {
		case STATUS_ASLEEP:
			return this->addStatus(STATUS_ASLEEP_FOR_1_TURN, this->_random() / 37 + 1);
		case STATUS_CONFUSED:
			return this->addStatus(STATUS_CONFUSED_FOR_1_TURN, (this->_random() & 3) + 2);
		default:
			return this->addStatus(status, 1);
		}
	}

	bool Pokemon::addStatus(StatusChange status, unsigned duration)
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
		if (
			(
				status & STATUS_ASLEEP ||
				status == STATUS_BURNED ||
				status == STATUS_POISONED ||
				status == STATUS_BADLY_POISONED ||
				status == STATUS_FROZEN ||
				status == STATUS_PARALYZED
			) && (
				this->hasStatus(STATUS_ASLEEP) ||
				this->hasStatus(STATUS_BURNED) ||
				this->hasStatus(STATUS_POISONED) ||
				this->hasStatus(STATUS_BADLY_POISONED) ||
				this->hasStatus(STATUS_FROZEN) ||
				this->hasStatus(STATUS_PARALYZED)
			)
		)
			return false;

		this->_log(" is now " + statusToString(status));
		this->_badPoisonStage = 1;
		this->_currentStatus |= (status * duration);
		return true;
	}

	std::string Pokemon::dump() const
	{
		std::stringstream stream;

		stream << this->getName() << " (" << this->_name << ") level " << static_cast<int>(this->_level);
		stream << ", Type " << typeToString(this->_types.first);
		if (this->_types.first != this->_types.second)
			stream << "/" << typeToString(this->_types.second);
		stream << ", " << this->getHealth()  << "/" << this->getMaxHealth() << " HP";
		stream << ", " << this->getAttack()  << " ATK";
		stream << ", " << this->getDefense() << " DEF";
		stream << ", " << this->getSpeed()   << " SPD";
		stream << ", " << this->getSpecial() << " SPE";
		stream << ", Status: " << std::hex << static_cast<int>(this->_currentStatus) << " ";
		if (this->_currentStatus) {
			for (unsigned i = 0; i < sizeof(this->_currentStatus) * 8; i++)
				if (this->_currentStatus & (1U << i))
					stream << statusToString(static_cast<StatusChange>(1U << i)) << ", ";
		} else
			stream << "OK, ";
		stream << "Moves: ";
		if (this->_moveSet.empty())
			stream << "No moves, ";
		for (const Move &move : this->_moveSet)
			stream << move.getName() << ", ";
		return stream.str().substr(0, stream.str().size() - 2);
	}

	void Pokemon::switched()
	{
		this->resetStatsChanges();
		this->_lastUsedMove = DEFAULT_MOVE(0x00);
		if (this->_transformed) {
			this->_id = this->_oldState.id;
			this->_moveSet = this->_oldState.moves;
			this->_baseStats= this->_oldState.stats;
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
		return (move ? move->getPriority() : 0) * 262140 + this->getSpeed();
	}

	void Pokemon::useMove(const Move &move, Pokemon &target)
	{
		//TODO: Implement the Hyper beam glitch
		if (this->_lastUsedMove.isFinished()) {
			if (move.getID() == Struggle) {
				Move struggle = move;

				if (!struggle.attack(*this, target,[this](const std::string &msg) { this->_game.logBattle(msg); }))
					this->_log("'s attack missed!");
				return;
			}
			this->_lastUsedMove = move;
		}
		if (!this->_lastUsedMove.attack(*this, target, [this](const std::string &msg) { this->_game.logBattle(msg); }))
			this->_log("'s attack missed!");
	}

	void Pokemon::storeDamages(bool active)
	{
		this->_storingDamages = active;
		if (!active)
			this->_damagesStored = 0;
		else
			this->_log(" is storing damages");
	}

	bool Pokemon::hasStatus(StatusChange status) const
	{
		return this->_currentStatus & status;
	}

	PokemonRandomGenerator &Pokemon::getRandomGenerator()
	{
		return this->_random;
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
		if (this->_currentStatus & STATUS_PARALYZED)
			return fmin(999, fmax(1, this->_getUpgradedStat(this->_baseStats.SPD, this->_upgradedStats.SPD) / 4));
		return fmin(999, fmax(1, this->_getUpgradedStat(this->_baseStats.SPD, this->_upgradedStats.SPD)));
	}

	unsigned Pokemon::getAttack() const
	{
		if (this->_currentStatus & STATUS_BURNED)
			return fmin(999, fmax(1, this->_getUpgradedStat(this->_baseStats.ATK, this->_upgradedStats.ATK) / 2));
		return fmin(999, fmax(1, this->_getUpgradedStat(this->_baseStats.ATK, this->_upgradedStats.ATK)));
	}

	unsigned Pokemon::getSpecial() const
	{
		return fmin(999, fmax(1, this->_getUpgradedStat(this->_baseStats.SPE, this->_upgradedStats.SPE)));
	}

	unsigned Pokemon::getDefense() const
	{
		return fmin(999, fmax(1, this->_getUpgradedStat(this->_baseStats.DEF, this->_upgradedStats.DEF)));
	}

	void Pokemon::endTurn()
	{
		if (this->_currentStatus & STATUS_BURNED) {
			this->_log(" is hurt by the burn!");
			this->takeDamage(this->getHealth() / 16);
		} else if ((this->_currentStatus & STATUS_POISONED) || (this->_currentStatus & STATUS_BADLY_POISONED)) {
			this->_log(" is hurt by the poison!");
			if (this->_currentStatus & STATUS_BADLY_POISONED)
				this->takeDamage(this->getHealth() * this->_badPoisonStage++ / 16);
			else
				this->takeDamage(this->getHealth() / 16);
		}
	}

	void Pokemon::_log(const std::string &msg) const
	{
		this->_game.logBattle(this->getName() + msg);
	}

	void Pokemon::attack(unsigned char moveSlot, Pokemon &target)
	{
		if (this->_currentStatus & STATUS_ASLEEP) {
			this->_currentStatus--;
			if (this->_currentStatus)
				this->_log(" is fast asleep!");
			else {
				this->_log(" woke up!");
				this->_currentStatus &= 0xFF00U;
			}
			return;
		}
		if (this->_currentStatus & STATUS_FROZEN) {
			this->_log(" is frozen solid!");
			return;
		}
		if (this->_wrapped) {
			this->_log(" can't move!");
			return;
		}
		if ((this->_currentStatus & STATUS_CONFUSED)) {
			this->_log(" is confused!");
			this->_currentStatus -= STATUS_CONFUSED_FOR_1_TURN;
			if (this->_random() >= 0x80) {
				this->_log(" hurts itself in it's confusion!");
				this->takeDamage(this->calcDamage(*this, 40, TYPE_0x0A, PHYSICAL, 0).damages);
				this->_lastUsedMove = DEFAULT_MOVE(0x00);
				return;
			}
		}
		if ((this->_currentStatus & STATUS_PARALYZED) && this->_random() < 0x3F) {
			this->_log(" is fully paralyzed!");
			this->_lastUsedMove = DEFAULT_MOVE(0x00);
			return;
		}
		if (moveSlot >= 4)
			this->useMove(availableMoves[Struggle], target);
		else if (moveSlot < this->_moveSet.size() && this->_moveSet[moveSlot].getID()) {
			this->useMove(this->_moveSet[moveSlot], target);
			this->_moveSet[moveSlot].setPP(this->_moveSet[moveSlot].getPP() ? this->_moveSet[moveSlot].getPP() - 1 : 63);
		}
	}

	unsigned Pokemon::getLevel() const
	{
		return this->_level;
	}

	unsigned Pokemon::getHealth() const
	{
		return this->_baseStats.HP;
	}

	unsigned Pokemon::getMaxHealth() const
	{
		return this->_baseStats.maxHP;
	}

	const Move &Pokemon::getLastUsedMove() const
	{
		return this->_lastUsedMove;
	}

	void Pokemon::changeStat(StatsChange stat, char nb)
	{
		std::string statName;
		char *stats = reinterpret_cast<char *>(&this->_upgradedStats);

		//TODO: Implement the stats glitch
		if (!nb)
			return;

		switch (stat) {
		case STATS_ATK:
			statName = "ATTACK";
			break;
		case STATS_DEF:
			statName = "DEFENSE";
			break;
		case STATS_SPD:
			statName = "SPEED";
			break;
		case STATS_SPE:
			statName = "SPECIAL";
			break;
		case STATS_ESQ:
			statName = "EVADE";
			break;
		case STATS_PRE:
			statName = "ACCURACY";
			break;
		default:
			return;
		}

		if ((stats[stat] >= 6 && nb > 0) || (stats[stat] <= -6 && nb < 0)) {
			this->_game.logBattle("Nothing happened.");
			return;
		}

		if (nb < -1)
			this->_log("'s " + statName + " greatly fell!");
		else if (nb == -1)
			this->_log("'s " + statName + " fell!");
		else if (nb == 1)
			this->_log("'s " + statName + " rose!");
		else if (nb > 1)
			this->_log("'s " + statName + " greatly rose!");

		stats[stat] += nb;
		if (stats[stat] > 6)
			stats[stat] = 6;
		else if (stats[stat] < -6)
			stats[stat] = -6;
	}

	void Pokemon::takeDamage(int damage)
	{
		if (!damage)
			damage = 1;

		if (damage > static_cast<int>(this->_baseStats.HP))
			this->_baseStats.HP = 0;
		else if (damage < static_cast<int>(this->_baseStats.HP - this->_baseStats.maxHP))
			this->_baseStats.HP = this->_baseStats.maxHP;
		else
			this->_baseStats.HP -= damage;

		if (!this->_baseStats.HP)
			this->_log(" fainted");
	}

	bool Pokemon::canGetHitBy(unsigned char moveId)
	{
		if (moveId == Swift || moveId == Transform || moveId == Dig)
			return true;
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

	void Pokemon::glitchHyperBeam()
	{
		if (this->_lastUsedMove.getID() == Hyper_Beam) {
			this->_needsRecharge = false;
			this->_lastUsedMove.glitch();
		}
	}

	Pokemon::DamageResult Pokemon::calcDamage(Pokemon &target, unsigned power, PokemonTypes damageType, MoveCategory category, double critRate) const
	{
		double effectiveness = getAttackDamageMultiplier(damageType, target.getTypes());

		if (effectiveness == 0)
			return {
				.critical = false,
				.damages = 0,
				.affect = false,
				.isVeryEffective = false,
				.isNotVeryEffective = false,
			};

		bool critical = (this->_random() < (pokemonList[this->_id].SPD / 2 * critRate));
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
				.damages = 0,
				.affect = true,
				.isVeryEffective = false,
				.isNotVeryEffective = false,
			};
		}

		if (attack > 255 || defense > 255) {
			attack = attack / 4 % 256;
			defense = defense / 4 % 256;
		}

		if (this->_types.first == damageType || this->_types.second == damageType)
			effectiveness *= 1.5;

		unsigned char r;

		do {
			r = this->_random();
			r = (r >> 1U) | ((r & 0x01U) << 7U);
		} while (r < 217);

		//From Zarel/honko-damagecalc ->
		//https://github.com/Zarel/honko-damagecalc/blob/dfff275e362ede0857b7564b3e5e2e6fc0e6782d/calc/src/mechanics/gen1.ts#L95
		double damages = fmax(
			(
				fmin(
					997,
					floor(
						floor(
							floor(
								2. * level / 5 + 2
							) * attack * power / defense
						) / 50
					)
				) + 2
			) * effectiveness * r / 255,
			1
		);

		return {
			.critical = critical,
			.damages = static_cast<unsigned int>(floor(damages)),
			.affect = true,
			.isVeryEffective = effectiveness > 1,
			.isNotVeryEffective = effectiveness < 1,
		};
	}

	std::string Pokemon::getName() const
	{
		std::string str = this->getNickname();

		if (str.empty())
			str = this->getSpeciesName();
		if (this->_enemy)
			return "Enemy " + str;
		return str;
	}

	double Pokemon::getAccuracy() const
	{
		return this->_getUpgradedStat(1, this->_upgradedStats.PRE);
	}

	double Pokemon::getEvasion() const
	{
		return this->_getUpgradedStat(1, -this->_upgradedStats.ESQ);
	}

	std::vector<unsigned char> Pokemon::encode() const
	{
		std::vector<unsigned char> result;

		//Sprite (and battle cry) ID
		result.push_back(this->_id);

		//Current HP
		result.push_back(this->_baseStats.HP >> 8U);
		result.push_back(this->_baseStats.HP >> 0U);

		//Level
		result.push_back(this->_level);

		//Status
		result.push_back(this->_currentStatus);

		//Type
		result.push_back(this->_types.first);
		result.push_back(this->_types.second);

		//Catch rate
		result.push_back(this->_catchRate);

		//Moves ID
		for (const Move &move : this->_moveSet)
			result.push_back(move.getID());

		//Trainer ID
		result.push_back(0x00);
		result.push_back(0x00);

		//EXP
		result.push_back(0x00);
		result.push_back(0x00);
		result.push_back(0x00);

		//StatEXP HP
		result.push_back(0x00);
		result.push_back(0x00);

		//StatEXP ATK
		result.push_back(0x00);
		result.push_back(0x00);

		//StatEXP DEF
		result.push_back(0x00);
		result.push_back(0x00);

		//StatEXP SPD
		result.push_back(0x00);
		result.push_back(0x00);

		//StatEXP SPE
		result.push_back(0x00);
		result.push_back(0x00);

		//DVs
		result.push_back(0xFF);
		result.push_back(0xFF);

		//PP Ups and moves PP
		for (const Move &move : this->_moveSet)
			result.push_back(((move.getPPUp() & 0b11U) << 6U) | (move.getPP() & 0b111111U));
		for (int i = this->_moveSet.size(); i < 4; i++)
			result.push_back(0x00);

		//Current Level
		result.push_back(this->_level);

		//Max HP
		result.push_back(this->_baseStats.maxHP >> 8U);
		result.push_back(this->_baseStats.maxHP >> 0U);

		//Attack
		result.push_back(this->_baseStats.ATK >> 8U);
		result.push_back(this->_baseStats.ATK >> 0U);

		//Defense
		result.push_back(this->_baseStats.DEF >> 8U);
		result.push_back(this->_baseStats.DEF >> 0U);

		//Speed/Agility
		result.push_back(this->_baseStats.SPD >> 8U);
		result.push_back(this->_baseStats.SPD >> 0U);

		//Special
		result.push_back(this->_baseStats.SPE >> 8U);
		result.push_back(this->_baseStats.SPE >> 0U);
		return result;
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

	UpgradableStats Pokemon::getStatsUpgradeStages() const
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

	BaseStats Pokemon::getBaseStats() const
	{
		return this->_baseStats;
	}

	/*
	** From Rhydon
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Tables.cs#L425
	*/
	const std::vector<std::vector<unsigned>> expTable{
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		{8, 15, 4, 9, 6, 10},
		{27, 52, 13, 57, 21, 33},
		{64, 122, 32, 96, 51, 80},
		{125, 237, 65, 135, 100, 156},
		{216, 406, 112, 179, 172, 270},
		{343, 637, 178, 236, 274, 428},
		{512, 942, 276, 314, 409, 640},
		{729, 1326, 393, 419, 583, 911},
		{1000, 1800, 540, 560, 800, 1250},
		{1331, 2369, 745, 742, 1064, 1663},
		{1728, 3041, 967, 973, 1382, 2160},
		{2197, 3822, 1230, 1261, 1757, 2746},
		{2744, 4719, 1591, 1612, 2195, 3430},
		{3375, 5737, 1957, 2035, 2700, 4218},
		{4096, 6881, 2457, 2535, 3276, 5120},
		{4913, 8155, 3046, 3120, 3930, 6141},
		{5832, 9564, 3732, 3798, 4665, 7290},
		{6859, 11111, 4526, 4575, 5487, 8573},
		{8000, 12800, 5440, 5460, 6400, 10000},
		{9261, 14632, 6482, 6458, 7408, 11576},
		{10648, 16610, 7666, 7577, 8518, 13310},
		{12167, 18737, 9003, 8825, 9733, 15208},
		{13824, 21012, 10506, 10208, 11059, 17280},
		{15625, 23437, 12187, 11735, 12500, 19531},
		{17576, 26012, 14060, 13411, 14060, 21970},
		{19683, 28737, 16140, 15244, 15746, 24603},
		{21952, 31610, 18439, 17242, 17561, 27440},
		{24389, 34632, 20974, 19411, 19511, 30486},
		{27000, 37800, 23760, 21760, 21600, 33750},
		{29791, 41111, 26811, 24294, 23832, 37238},
		{32768, 44564, 30146, 27021, 26214, 40960},
		{35937, 48155, 33780, 29949, 28749, 44921},
		{39304, 51881, 37731, 33084, 31443, 49130},
		{42875, 55737, 42017, 36435, 34300, 53593},
		{46656, 59719, 46656, 40007, 37324, 58320},
		{50653, 63822, 50653, 43808, 40522, 63316},
		{54872, 68041, 55969, 47846, 43897, 68590},
		{59319, 72369, 60505, 52127, 47455, 74148},
		{64000, 76800, 66560, 56660, 51200, 80000},
		{68921, 81326, 71677, 61450, 55136, 86151},
		{74088, 85942, 78533, 66505, 59270, 92610},
		{79507, 90637, 84277, 71833, 63605, 99383},
		{85184, 95406, 91998, 77440, 68147, 106480},
		{91125, 100237, 98415, 83335, 72900, 113906},
		{97336, 105122, 107069, 89523, 77868, 121670},
		{103823, 110052, 114205, 96012, 83058, 129778},
		{110592, 115015, 123863, 102810, 88473, 138240},
		{117649, 120001, 131766, 109923, 94119, 147061},
		{125000, 125000, 142500, 117360, 100000, 156250},
		{132651, 131324, 151222, 125126, 106120, 165813},
		{140608, 137795, 163105, 133229, 112486, 175760},
		{148877, 144410, 172697, 141677, 119101, 186096},
		{157464, 151165, 185807, 150476, 125971, 196830},
		{166375, 158056, 196322, 159635, 133100, 207968},
		{175616, 165079, 210739, 169159, 140492, 219520},
		{185193, 172229, 222231, 179056, 148154, 231491},
		{195112, 179503, 238036, 189334, 156089, 243890},
		{205379, 186894, 250562, 199999, 164303, 256723},
		{216000, 194400, 267840, 211060, 172800, 270000},
		{226981, 202013, 281456, 222522, 181584, 283726},
		{238328, 209728, 300293, 234393, 190662, 297910},
		{250047, 217540, 315059, 246681, 200037, 312558},
		{262144, 225443, 335544, 259392, 209715, 327680},
		{274625, 233431, 351520, 272535, 219700, 343281},
		{287496, 241496, 373744, 286115, 229996, 359370},
		{300763, 249633, 390991, 300140, 240610, 375953},
		{314432, 257834, 415050, 314618, 251545, 393040},
		{328509, 267406, 433631, 329555, 262807, 410636},
		{343000, 276458, 459620, 344960, 274400, 428750},
		{357911, 286328, 479600, 360838, 286328, 447388},
		{373248, 296358, 507617, 377197, 298598, 466560},
		{389017, 305767, 529063, 394045, 311213, 486271},
		{405224, 316074, 559209, 411388, 324179, 506530},
		{421875, 326531, 582187, 429235, 337500, 527343},
		{438976, 336255, 614566, 447591, 351180, 548720},
		{456533, 346965, 639146, 466464, 365226, 570666},
		{474552, 357812, 673863, 485862, 379641, 593190},
		{493039, 367807, 700115, 505791, 394431, 616298},
		{512000, 378880, 737280, 526260, 409600, 640000},
		{531441, 390077, 765275, 547274, 425152, 664301},
		{551368, 400293, 804997, 568841, 441094, 689210},
		{571787, 411686, 834809, 590969, 457429, 714733},
		{592704, 423190, 877201, 613664, 474163, 740880},
		{614125, 433572, 908905, 636935, 491300, 767656},
		{636056, 445239, 954084, 660787, 508844, 795070},
		{658503, 457001, 987754, 685228, 526802, 823128},
		{681472, 467489, 1035837, 710266, 545177, 851840},
		{704969, 479378, 1071552, 735907, 563975, 881211},
		{729000, 491346, 1122660, 762160, 583200, 911250},
		{753571, 501878, 1160499, 789030, 602856, 941963},
		{778688, 513934, 1214753, 816525, 622950, 973360},
		{804357, 526049, 1254796, 844653, 643485, 1005446},
		{830584, 536557, 1312322, 873420, 664467, 1038230},
		{857375, 548720, 1354652, 902835, 685900, 1071718},
		{884736, 560922, 1415577, 932903, 707788, 1105920},
		{912673, 571333, 1460276, 963632, 730138, 1140841},
		{941192, 583539, 1524731, 995030, 752953, 1176490},
		{970299, 591882, 1571884, 1027103, 776239, 1212873},
		{1000000, 600000, 1640000, 1059860, 800000, 1250000},
	};

	/*
	** From Rhydon
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Tables.cs#L82
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Tables.cs#L202
	*/
	const std::vector<PokemonBase> pokemonList{
		{ 0X00,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X00) */
		{ 0X01,      "RHYDON", 105, 130, 120,  40,  45,   TYPE_GROUND,     TYPE_ROCK,  60, 204 }, /* RHYDON       (0X01) */
		{ 0X02,  "KANGASKHAN", 105,  95,  80,  90,  40,   TYPE_NORMAL,   TYPE_NORMAL,  45, 175 }, /* KANGASKHAN   (0X02) */
		{ 0X03,    "NIDORAN~",  46,  57,  40,  50,  40,   TYPE_POISON,   TYPE_POISON, 235,  60 }, /* NIDORAN_M    (0X03) */
		{ 0X04,    "CLEFAIRY",  70,  45,  48,  35,  60,   TYPE_NORMAL,   TYPE_NORMAL, 150,  68 }, /* CLEFAIRY     (0X04) */
		{ 0X05,     "SPEAROW",  40,  60,  30,  70,  31,   TYPE_NORMAL,   TYPE_FLYING, 255,  58 }, /* SPEAROW      (0X05) */
		{ 0X06,     "VOLTORB",  40,  30,  50, 100,  55, TYPE_ELECTRIC, TYPE_ELECTRIC, 190, 103 }, /* VOLTORB      (0X06) */
		{ 0X07,    "NIDOKING",  81,  92,  77,  85,  75,   TYPE_POISON,   TYPE_GROUND,  45, 195 }, /* NIDOKING     (0X07) */
		{ 0X08,     "SLOWBRO",  95,  75, 110,  30,  80,    TYPE_WATER,  TYPE_PSYCHIC,  75, 164 }, /* SLOWBRO      (0X08) */
		{ 0X09,     "IVYSAUR",  60,  62,  63,  60,  80,    TYPE_GRASS,   TYPE_POISON,  45, 141 }, /* IVYSAUR      (0X09) */
		{ 0X0A,   "EXEGGUTOR",  95,  95,  85,  55, 125,    TYPE_GRASS,  TYPE_PSYCHIC,  45, 212 }, /* EXEGGUTOR    (0X0A) */
		{ 0X0B,   "LICKITUNG",  90,  55,  75,  30,  60,   TYPE_NORMAL,   TYPE_NORMAL,  45, 127 }, /* LICKITUNG    (0X0B) */
		{ 0X0C,   "EXEGGCUTE",  60,  40,  80,  40,  60,    TYPE_GRASS,  TYPE_PSYCHIC,  90,  98 }, /* EXEGGCUTE    (0X0C) */
		{ 0X0D,      "GRIMER",  80,  80,  50,  25,  40,   TYPE_POISON,   TYPE_POISON, 190,  90 }, /* GRIMER       (0X0D) */
		{ 0X0E,      "GENGAR",  60,  65,  60, 110, 130,    TYPE_GHOST,   TYPE_POISON,  45, 190 }, /* GENGAR       (0X0E) */
		{ 0X0F,    "NIDORAN`",  55,  47,  52,  41,  40,   TYPE_POISON,   TYPE_POISON, 235,  59 }, /* NIDORAN_F    (0X0F) */
		{ 0X10,   "NIDOQUEEN",  90,  82,  87,  76,  75,   TYPE_POISON,   TYPE_GROUND,  45, 194 }, /* NIDOQUEEN    (0X10) */
		{ 0X11,      "CUBONE",  50,  50,  95,  35,  40,   TYPE_GROUND,   TYPE_GROUND, 190,  87 }, /* CUBONE       (0X11) */
		{ 0X12,     "RHYHORN",  80,  85,  95,  25,  30,   TYPE_GROUND,     TYPE_ROCK, 120, 135 }, /* RHYHORN      (0X12) */
		{ 0X13,      "LAPRAS", 130,  85,  80,  60,  95,    TYPE_WATER,      TYPE_ICE,  45, 219 }, /* LAPRAS       (0X13) */
		{ 0X14,    "ARCANINE",  90, 110,  80,  95,  80,     TYPE_FIRE,     TYPE_FIRE,  75, 213 }, /* ARCANINE     (0X14) */
		{ 0X15,         "MEW", 100, 100, 100, 100, 100,  TYPE_PSYCHIC,  TYPE_PSYCHIC,  45,  64 }, /* MEW          (0X15) */
		{ 0X16,    "GYARADOS",  95, 125,  79,  81, 100,    TYPE_WATER,   TYPE_FLYING,  45, 214 }, /* GYARADOS     (0X16) */
		{ 0X17,    "SHELLDER",  30,  65, 100,  40,  45,    TYPE_WATER,    TYPE_WATER, 190,  97 }, /* SHELLDER     (0X17) */
		{ 0X18,   "TENTACOOL",  40,  40,  35,  70, 100,    TYPE_WATER,   TYPE_POISON, 190, 105 }, /* TENTACOOL    (0X18) */
		{ 0X19,      "GASTLY",  30,  35,  30,  80, 100,    TYPE_GHOST,   TYPE_POISON, 190,  95 }, /* GASTLY       (0X19) */
		{ 0X1A,     "SCYTHER",  70, 110,  80, 105,  55,      TYPE_BUG,   TYPE_FLYING,  45, 187 }, /* SCYTHER      (0X1A) */
		{ 0X1B,      "STARYU",  30,  45,  55,  85,  70,    TYPE_WATER,    TYPE_WATER, 225, 106 }, /* STARYU       (0X1B) */
		{ 0X1C,   "BLASTOISE",  79,  83, 100,  78,  85,    TYPE_WATER,    TYPE_WATER,  45, 210 }, /* BLASTOISE    (0X1C) */
		{ 0X1D,      "PINSIR",  65, 125, 100,  85,  55,      TYPE_BUG,      TYPE_BUG,  45, 200 }, /* PINSIR       (0X1D) */
		{ 0X1E,     "TANGELA",  65,  55, 115,  60, 100,    TYPE_GRASS,    TYPE_GRASS,  45, 166 }, /* TANGELA      (0X1E) */
		{ 0X1F,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X1F) */
		{ 0X20,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X20) */
		{ 0X21,   "GROWLITHE",  55,  70,  45,  60,  50,     TYPE_FIRE,     TYPE_FIRE, 190,  91 }, /* GROWLITHE    (0X21) */
		{ 0X22,        "ONIX",  35,  45, 160,  70,  30,     TYPE_ROCK,   TYPE_GROUND,  45, 108 }, /* ONIX         (0X22) */
		{ 0X23,      "FEAROW",  65,  90,  65, 100,  61,   TYPE_NORMAL,   TYPE_FLYING,  90, 162 }, /* FEAROW       (0X23) */
		{ 0X24,      "PIDGEY",  40,  45,  40,  56,  35,   TYPE_NORMAL,   TYPE_FLYING, 255,  55 }, /* PIDGEY       (0X24) */
		{ 0X25,    "SLOWPOKE",  90,  65,  65,  15,  40,    TYPE_WATER,  TYPE_PSYCHIC, 190,  99 }, /* SLOWPOKE     (0X25) */
		{ 0X26,     "KADABRA",  40,  35,  30, 105, 120,  TYPE_PSYCHIC,  TYPE_PSYCHIC, 100, 145 }, /* KADABRA      (0X26) */
		{ 0X27,    "GRAVELER",  55,  95, 115,  35,  45,     TYPE_ROCK,   TYPE_GROUND, 120, 134 }, /* GRAVELER     (0X27) */
		{ 0X28,     "CHANSEY", 250,   5,   5,  50, 105,   TYPE_NORMAL,   TYPE_NORMAL,  30, 255 }, /* CHANSEY      (0X28) */
		{ 0X29,     "MACHOKE",  80, 100,  70,  45,  50, TYPE_FIGHTING, TYPE_FIGHTING,  90, 146 }, /* MACHOKE      (0X29) */
		{ 0X2A,    "MR. MIME",  40,  45,  65,  90, 100,  TYPE_PSYCHIC,  TYPE_PSYCHIC,  45, 136 }, /* MR_MIME      (0X2A) */
		{ 0X2B,   "HITMONLEE",  50, 120,  53,  87,  35, TYPE_FIGHTING, TYPE_FIGHTING,  45, 139 }, /* HITMONLEE    (0X2B) */
		{ 0X2C,  "HITMONCHAN",  50, 105,  79,  76,  35, TYPE_FIGHTING, TYPE_FIGHTING,  45, 140 }, /* HITMONCHAN   (0X2C) */
		{ 0X2D,       "ARBOK",  60,  85,  69,  80,  65,   TYPE_POISON,   TYPE_POISON,  90, 147 }, /* ARBOK        (0X2D) */
		{ 0X2E,    "PARASECT",  60,  95,  80,  30,  80,      TYPE_BUG,    TYPE_GRASS,  75, 128 }, /* PARASECT     (0X2E) */
		{ 0X2F,     "PSYDUCK",  50,  52,  48,  55,  50,    TYPE_WATER,    TYPE_WATER, 190,  80 }, /* PSYDUCK      (0X2F) */
		{ 0X30,     "DROWZEE",  60,  48,  45,  42,  90,  TYPE_PSYCHIC,  TYPE_PSYCHIC, 190, 102 }, /* DROWZEE      (0X30) */
		{ 0X31,       "GOLEM",  80, 110, 130,  45,  55,     TYPE_ROCK,   TYPE_GROUND,  45, 177 }, /* GOLEM        (0X31) */
		{ 0X32,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X32) */
		{ 0X33,      "MAGMAR",  65,  95,  57,  93,  85,     TYPE_FIRE,     TYPE_FIRE,  45, 167 }, /* MAGMAR       (0X33) */
		{ 0X34,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X34) */
		{ 0X35,  "ELECTABUZZ",  65,  83,  57, 105,  85, TYPE_ELECTRIC, TYPE_ELECTRIC,  45, 156 }, /* ELECTABUZZ   (0X35) */
		{ 0X36,    "MAGNETON",  50,  60,  95,  70, 120, TYPE_ELECTRIC, TYPE_ELECTRIC,  60, 161 }, /* MAGNETON     (0X36) */
		{ 0X37,     "KOFFING",  40,  65,  95,  35,  60,   TYPE_POISON,   TYPE_POISON, 190, 114 }, /* KOFFING      (0X37) */
		{ 0X38,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X38) */
		{ 0X39,      "MANKEY",  40,  80,  35,  70,  35, TYPE_FIGHTING, TYPE_FIGHTING, 190,  74 }, /* MANKEY       (0X39) */
		{ 0X3A,        "SEEL",  65,  45,  55,  45,  70,    TYPE_WATER,    TYPE_WATER, 190, 100 }, /* SEEL         (0X3A) */
		{ 0X3B,     "DIGLETT",  10,  55,  25,  95,  45,   TYPE_GROUND,   TYPE_GROUND, 255,  81 }, /* DIGLETT      (0X3B) */
		{ 0X3C,      "TAUROS",  75, 100,  95, 110,  70,   TYPE_NORMAL,   TYPE_NORMAL,  45, 211 }, /* TAUROS       (0X3C) */
		{ 0X3D,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X3D) */
		{ 0X3E,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X3E) */
		{ 0X3F,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X3F) */
		{ 0X40,   "FARFETCHD",  52,  65,  55,  60,  58,   TYPE_NORMAL,   TYPE_FLYING,  45,  94 }, /* FARFETCH_D   (0X40) */
		{ 0X41,     "VENONAT",  60,  55,  50,  45,  40,      TYPE_BUG,   TYPE_POISON, 190,  75 }, /* VENONAT      (0X41) */
		{ 0X42,   "DRAGONITE",  91, 134,  95,  80, 100,   TYPE_DRAGON,   TYPE_FLYING,  45, 218 }, /* DRAGONITE    (0X42) */
		{ 0X43,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X43) */
		{ 0X44,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X44) */
		{ 0X45,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X45) */
		{ 0X46,       "DODUO",  35,  85,  45,  75,  35,   TYPE_NORMAL,   TYPE_FLYING, 190,  96 }, /* DODUO        (0X46) */
		{ 0X47,     "POLIWAG",  40,  50,  40,  90,  40,    TYPE_WATER,    TYPE_WATER, 255,  77 }, /* POLIWAG      (0X47) */
		{ 0X48,        "JYNX",  65,  50,  35,  95,  95,      TYPE_ICE,  TYPE_PSYCHIC,  45, 137 }, /* JYNX         (0X48) */
		{ 0X49,     "MOLTRES",  90, 100,  90,  90, 125,     TYPE_FIRE,   TYPE_FLYING,   3, 217 }, /* MOLTRES      (0X49) */
		{ 0X4A,    "ARTICUNO",  90,  85, 100,  85, 125,      TYPE_ICE,   TYPE_FLYING,   3, 215 }, /* ARTICUNO     (0X4A) */
		{ 0X4B,      "ZAPDOS",  90,  90,  85, 100, 125, TYPE_ELECTRIC,   TYPE_FLYING,   3, 216 }, /* ZAPDOS       (0X4B) */
		{ 0X4C,       "DITTO",  48,  48,  48,  48,  48,   TYPE_NORMAL,   TYPE_NORMAL,  35,  61 }, /* DITTO        (0X4C) */
		{ 0X4D,      "MEOWTH",  40,  45,  35,  90,  40,   TYPE_NORMAL,   TYPE_NORMAL, 255,  69 }, /* MEOWTH       (0X4D) */
		{ 0X4E,      "KRABBY",  30, 105,  90,  50,  25,    TYPE_WATER,    TYPE_WATER, 225, 115 }, /* KRABBY       (0X4E) */
		{ 0X4F,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X4F) */
		{ 0X50,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X50) */
		{ 0X51,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X51) */
		{ 0X52,      "VULPIX",  38,  41,  40,  65,  65,     TYPE_FIRE,     TYPE_FIRE, 190,  63 }, /* VULPIX       (0X52) */
		{ 0X53,   "NINETALES",  73,  76,  75, 100, 100,     TYPE_FIRE,     TYPE_FIRE,  75, 178 }, /* NINETALES    (0X53) */
		{ 0X54,     "PIKACHU",  35,  55,  30,  90,  50, TYPE_ELECTRIC, TYPE_ELECTRIC, 190,  82 }, /* PIKACHU      (0X54) */
		{ 0X55,      "RAICHU",  60,  90,  55, 100,  90, TYPE_ELECTRIC, TYPE_ELECTRIC,  75, 122 }, /* RAICHU       (0X55) */
		{ 0X56,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X56) */
		{ 0X57,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X57) */
		{ 0X58,     "DRATINI",  41,  64,  45,  50,  50,   TYPE_DRAGON,   TYPE_DRAGON,  45,  67 }, /* DRATINI      (0X58) */
		{ 0X59,   "DRAGONAIR",  61,  84,  65,  70,  70,   TYPE_DRAGON,   TYPE_DRAGON,  45, 144 }, /* DRAGONAIR    (0X59) */
		{ 0X5A,      "KABUTO",  30,  80,  90,  55,  45,     TYPE_ROCK,    TYPE_WATER,  45, 119 }, /* KABUTO       (0X5A) */
		{ 0X5B,    "KABUTOPS",  60, 115, 105,  80,  70,     TYPE_ROCK,    TYPE_WATER,  45, 201 }, /* KABUTOPS     (0X5B) */
		{ 0X5C,      "HORSEA",  30,  40,  70,  60,  70,    TYPE_WATER,    TYPE_WATER, 225,  83 }, /* HORSEA       (0X5C) */
		{ 0X5D,      "SEADRA",  55,  65,  95,  85,  95,    TYPE_WATER,    TYPE_WATER,  75, 155 }, /* SEADRA       (0X5D) */
		{ 0X5E,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X5E) */
		{ 0X5F,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X5F) */
		{ 0X60,   "SANDSHREW",  50,  75,  85,  40,  30,   TYPE_GROUND,   TYPE_GROUND, 255,  93 }, /* SANDSHREW    (0X60) */
		{ 0X61,   "SANDSLASH",  75, 100, 110,  65,  55,   TYPE_GROUND,   TYPE_GROUND,  90, 163 }, /* SANDSLASH    (0X61) */
		{ 0X62,     "OMANYTE",  35,  40, 100,  35,  90,     TYPE_ROCK,    TYPE_WATER,  45, 120 }, /* OMANYTE      (0X62) */
		{ 0X63,     "OMASTAR",  70,  60, 125,  55, 115,     TYPE_ROCK,    TYPE_WATER,  45, 199 }, /* OMASTAR      (0X63) */
		{ 0X64,  "JIGGLYPUFF", 115,  45,  20,  20,  25,   TYPE_NORMAL,   TYPE_NORMAL, 170,  76 }, /* JIGGLYPUFF   (0X64) */
		{ 0X65,  "WIGGLYTUFF", 140,  70,  45,  45,  50,   TYPE_NORMAL,   TYPE_NORMAL,  50, 109 }, /* WIGGLYTUFF   (0X65) */
		{ 0X66,       "EEVEE",  55,  55,  50,  55,  65,   TYPE_NORMAL,   TYPE_NORMAL,  45,  92 }, /* EEVEE        (0X66) */
		{ 0X67,     "FLAREON",  65, 130,  60,  65, 110,     TYPE_FIRE,     TYPE_FIRE,  45, 198 }, /* FLAREON      (0X67) */
		{ 0X68,     "JOLTEON",  65,  65,  60, 130, 110, TYPE_ELECTRIC, TYPE_ELECTRIC,  45, 197 }, /* JOLTEON      (0X68) */
		{ 0X69,    "VAPOREON", 130,  65,  60,  65, 110,    TYPE_WATER,    TYPE_WATER,  45, 196 }, /* VAPOREON     (0X69) */
		{ 0X6A,      "MACHOP",  70,  80,  50,  35,  35, TYPE_FIGHTING, TYPE_FIGHTING, 180,  88 }, /* MACHOP       (0X6A) */
		{ 0X6B,       "ZUBAT",  40,  45,  35,  55,  40,   TYPE_POISON,   TYPE_FLYING, 255,  54 }, /* ZUBAT        (0X6B) */
		{ 0X6C,       "EKANS",  35,  60,  44,  55,  40,   TYPE_POISON,   TYPE_POISON, 255,  62 }, /* EKANS        (0X6C) */
		{ 0X6D,       "PARAS",  35,  70,  55,  25,  55,      TYPE_BUG,    TYPE_GRASS, 190,  70 }, /* PARAS        (0X6D) */
		{ 0X6E,   "POLIWHIRL",  65,  65,  65,  90,  50,    TYPE_WATER,    TYPE_WATER, 120, 131 }, /* POLIWHIRL    (0X6E) */
		{ 0X6F,   "POLIWRATH",  90,  85,  95,  70,  70,    TYPE_WATER, TYPE_FIGHTING,  45, 185 }, /* POLIWRATH    (0X6F) */
		{ 0X70,      "WEEDLE",  40,  35,  30,  50,  20,      TYPE_BUG,   TYPE_POISON, 255,  52 }, /* WEEDLE       (0X70) */
		{ 0X71,      "KAKUNA",  45,  25,  50,  35,  25,      TYPE_BUG,   TYPE_POISON, 120,  71 }, /* KAKUNA       (0X71) */
		{ 0X72,    "BEEDRILL",  65,  80,  40,  75,  45,      TYPE_BUG,   TYPE_POISON,  45, 159 }, /* BEEDRILL     (0X72) */
		{ 0X73,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X73) */
		{ 0X74,      "DODRIO",  60, 110,  70, 100,  60,   TYPE_NORMAL,   TYPE_FLYING,  45, 158 }, /* DODRIO       (0X74) */
		{ 0X75,    "PRIMEAPE",  65, 105,  60,  95,  60, TYPE_FIGHTING, TYPE_FIGHTING,  75, 149 }, /* PRIMEAPE     (0X75) */
		{ 0X76,     "DUGTRIO",  35,  80,  50, 120,  70,   TYPE_GROUND,   TYPE_GROUND,  50, 153 }, /* DUGTRIO      (0X76) */
		{ 0X77,    "VENOMOTH",  70,  65,  60,  90,  90,      TYPE_BUG,   TYPE_POISON,  75, 138 }, /* VENOMOTH     (0X77) */
		{ 0X78,     "DEWGONG",  90,  70,  80,  70,  95,    TYPE_WATER,      TYPE_ICE,  75, 176 }, /* DEWGONG      (0X78) */
		{ 0X79,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X79) */
		{ 0X7A,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X7A) */
		{ 0X7B,    "CATERPIE",  45,  30,  35,  45,  20,      TYPE_BUG,      TYPE_BUG, 255,  53 }, /* CATERPIE     (0X7B) */
		{ 0X7C,     "METAPOD",  50,  20,  55,  30,  25,      TYPE_BUG,      TYPE_BUG, 120,  72 }, /* METAPOD      (0X7C) */
		{ 0X7D,  "BUTTERFREE",  60,  45,  50,  70,  80,      TYPE_BUG,   TYPE_FLYING,  45, 160 }, /* BUTTERFREE   (0X7D) */
		{ 0X7E,     "MACHAMP",  90, 130,  80,  55,  65, TYPE_FIGHTING, TYPE_FIGHTING,  45, 193 }, /* MACHAMP      (0X7E) */
		{ 0X7F,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X7F) */
		{ 0X80,     "GOLDUCK",  80,  82,  78,  85,  80,    TYPE_WATER,    TYPE_WATER,  75, 174 }, /* GOLDUCK      (0X80) */
		{ 0X81,       "HYPNO",  85,  73,  70,  67, 115,  TYPE_PSYCHIC,  TYPE_PSYCHIC,  75, 165 }, /* HYPNO        (0X81) */
		{ 0X82,      "GOLBAT",  75,  80,  70,  90,  75,   TYPE_POISON,   TYPE_FLYING,  90, 171 }, /* GOLBAT       (0X82) */
		{ 0X83,      "MEWTWO", 106, 110,  90, 130, 154,  TYPE_PSYCHIC,  TYPE_PSYCHIC,   3, 220 }, /* MEWTWO       (0X83) */
		{ 0X84,     "SNORLAX", 160, 110,  65,  30,  65,   TYPE_NORMAL,   TYPE_NORMAL,  25, 154 }, /* SNORLAX      (0X84) */
		{ 0X85,    "MAGIKARP",  20,  10,  55,  80,  20,    TYPE_WATER,    TYPE_WATER, 255,  20 }, /* MAGIKARP     (0X85) */
		{ 0X86,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X86) */
		{ 0X87,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X87) */
		{ 0X88,         "MUK", 105, 105,  75,  50,  65,   TYPE_POISON,   TYPE_POISON,  75, 157 }, /* MUK          (0X88) */
		{ 0X89,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X89) */
		{ 0X8A,     "KINGLER",  55, 130, 115,  75,  50,    TYPE_WATER,    TYPE_WATER,  60, 206 }, /* KINGLER      (0X8A) */
		{ 0X8B,    "CLOYSTER",  50,  95, 180,  70,  85,    TYPE_WATER,      TYPE_ICE,  60, 203 }, /* CLOYSTER     (0X8B) */
		{ 0X8C,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X8C) */
		{ 0X8D,   "ELECTRODE",  60,  50,  70, 140,  80, TYPE_ELECTRIC, TYPE_ELECTRIC,  60, 150 }, /* ELECTRODE    (0X8D) */
		{ 0X8E,    "CLEFABLE",  95,  70,  73,  60,  85,   TYPE_NORMAL,   TYPE_NORMAL,  25, 129 }, /* CLEFABLE     (0X8E) */
		{ 0X8F,     "WEEZING",  65,  90, 120,  60,  85,   TYPE_POISON,   TYPE_POISON,  60, 173 }, /* WEEZING      (0X8F) */
		{ 0X90,     "PERSIAN",  65,  70,  60, 115,  65,   TYPE_NORMAL,   TYPE_NORMAL,  90, 148 }, /* PERSIAN      (0X90) */
		{ 0X91,     "MAROWAK",  60,  80, 110,  45,  50,   TYPE_GROUND,   TYPE_GROUND,  75, 124 }, /* MAROWAK      (0X91) */
		{ 0X92,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X92) */
		{ 0X93,     "HAUNTER",  45,  50,  45,  95, 115,    TYPE_GHOST,   TYPE_POISON,  90, 126 }, /* HAUNTER      (0X93) */
		{ 0X94,        "ABRA",  25,  20,  15,  90, 105,  TYPE_PSYCHIC,  TYPE_PSYCHIC, 200,  73 }, /* ABRA         (0X94) */
		{ 0X95,    "ALAKAZAM",  55,  50,  45, 120, 135,  TYPE_PSYCHIC,  TYPE_PSYCHIC,  50, 186 }, /* ALAKAZAM     (0X95) */
		{ 0X96,   "PIDGEOTTO",  63,  60,  55,  71,  50,   TYPE_NORMAL,   TYPE_FLYING, 120, 113 }, /* PIDGEOTTO    (0X96) */
		{ 0X97,     "PIDGEOT",  83,  80,  75,  91,  70,   TYPE_NORMAL,   TYPE_FLYING,  45, 172 }, /* PIDGEOT      (0X97) */
		{ 0X98,     "STARMIE",  60,  75,  85, 115, 100,    TYPE_WATER,  TYPE_PSYCHIC,  60, 207 }, /* STARMIE      (0X98) */
		{ 0X99,   "BULBASAUR",  45,  49,  49,  45,  65,    TYPE_GRASS,   TYPE_POISON,  45,  64 }, /* BULBASAUR    (0X99) */
		{ 0X9A,    "VENUSAUR",  80,  82,  83,  80, 100,    TYPE_GRASS,   TYPE_POISON,  45, 208 }, /* VENUSAUR     (0X9A) */
		{ 0X9B,  "TENTACRUEL",  80,  70,  65, 100, 120,    TYPE_WATER,   TYPE_POISON,  60, 205 }, /* TENTACRUEL   (0X9B) */
		{ 0X9C,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X9C) */
		{ 0X9D,     "GOLDEEN",  45,  67,  60,  63,  50,    TYPE_WATER,    TYPE_WATER, 225, 111 }, /* GOLDEEN      (0X9D) */
		{ 0X9E,     "SEAKING",  80,  92,  65,  68,  80,    TYPE_WATER,    TYPE_WATER,  60, 170 }, /* SEAKING      (0X9E) */
		{ 0X9F,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0X9F) */
		{ 0XA0,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XA0) */
		{ 0XA1,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XA1) */
		{ 0XA2,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XA2) */
		{ 0XA3,      "PONYTA",  50,  85,  55,  90,  65,     TYPE_FIRE,     TYPE_FIRE, 190, 152 }, /* PONYTA       (0XA3) */
		{ 0XA4,    "RAPIDASH",  65, 100,  70, 105,  80,     TYPE_FIRE,     TYPE_FIRE,  60, 192 }, /* RAPIDASH     (0XA4) */
		{ 0XA5,     "RATTATA",  30,  56,  35,  72,  25,   TYPE_NORMAL,   TYPE_NORMAL, 255,  57 }, /* RATTATA      (0XA5) */
		{ 0XA6,    "RATICATE",  55,  81,  60,  97,  50,   TYPE_NORMAL,   TYPE_NORMAL,  90, 116 }, /* RATICATE     (0XA6) */
		{ 0XA7,    "NIDORINO",  61,  72,  57,  65,  55,   TYPE_POISON,   TYPE_POISON, 120, 118 }, /* NIDORINO     (0XA7) */
		{ 0XA8,    "NIDORINA",  70,  62,  67,  56,  55,   TYPE_POISON,   TYPE_POISON, 120, 117 }, /* NIDORINA     (0XA8) */
		{ 0XA9,     "GEODUDE",  40,  80, 100,  20,  30,     TYPE_ROCK,   TYPE_GROUND, 255,  86 }, /* GEODUDE      (0XA9) */
		{ 0XAA,     "PORYGON",  65,  60,  70,  40,  75,   TYPE_NORMAL,   TYPE_NORMAL,  45, 130 }, /* PORYGON      (0XAA) */
		{ 0XAB,  "AERODACTYL",  80, 105,  65, 130,  60,     TYPE_ROCK,   TYPE_FLYING,  45, 202 }, /* AERODACTYL   (0XAB) */
		{ 0XAC,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XAC) */
		{ 0XAD,   "MAGNEMITE",  25,  35,  70,  45,  95, TYPE_ELECTRIC, TYPE_ELECTRIC, 190,  89 }, /* MAGNEMITE    (0XAD) */
		{ 0XAE,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XAE) */
		{ 0XAF,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XAF) */
		{ 0XB0,  "CHARMANDER",  39,  52,  43,  65,  50,     TYPE_FIRE,     TYPE_FIRE,  45,  65 }, /* CHARMANDER   (0XB0) */
		{ 0XB1,    "SQUIRTLE",  44,  48,  65,  43,  50,    TYPE_WATER,    TYPE_WATER,  45,  66 }, /* SQUIRTLE     (0XB1) */
		{ 0XB2,  "CHARMELEON",  58,  64,  58,  80,  65,     TYPE_FIRE,     TYPE_FIRE,  45, 142 }, /* CHARMELEON   (0XB2) */
		{ 0XB3,   "WARTORTLE",  59,  63,  80,  58,  65,    TYPE_WATER,    TYPE_WATER,  45, 143 }, /* WARTORTLE    (0XB3) */
		{ 0XB4,   "CHARIZARD",  78,  84,  78, 100,  85,     TYPE_FIRE,   TYPE_FLYING,  45, 209 }, /* CHARIZARD    (0XB4) */
		{ 0XB5,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XB5) */
		{ 0XB6,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XB6) */
		{ 0XB7,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XB7) */
		{ 0XB8,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XB8) */
		{ 0XB9,      "ODDISH",  45,  50,  55,  30,  75,    TYPE_GRASS,   TYPE_POISON, 255,  78 }, /* ODDISH       (0XB9) */
		{ 0XBA,       "GLOOM",  60,  65,  70,  40,  85,    TYPE_GRASS,   TYPE_POISON, 120, 132 }, /* GLOOM        (0XBA) */
		{ 0XBB,   "VILEPLUME",  75,  80,  85,  50, 100,    TYPE_GRASS,   TYPE_POISON,  45, 184 }, /* VILEPLUME    (0XBB) */
		{ 0XBC,  "BELLSPROUT",  50,  75,  35,  40,  70,    TYPE_GRASS,   TYPE_POISON, 255,  84 }, /* BELLSPROUT   (0XBC) */
		{ 0XBD,  "WEEPINBELL",  65,  90,  50,  55,  85,    TYPE_GRASS,   TYPE_POISON, 120, 151 }, /* WEEPINBELL   (0XBD) */
		{ 0XBE,  "VICTREEBEL",  80, 105,  65,  70, 100,    TYPE_GRASS,   TYPE_POISON,  45, 191 }, /* VICTREEBEL   (0XBE) */
		{ 0XBF,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XBF) */
		{ 0XC0,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XC0) */
		{ 0XC1,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XC1) */
		{ 0XC2,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XC2) */
		{ 0XC3,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XC3) */
		{ 0XC4,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XC4) */
		{ 0XC5,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XC5) */
		{ 0XC6,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XC6) */
		{ 0XC7,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XC7) */
		{ 0XC8,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XC8) */
		{ 0XC9,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XC9) */
		{ 0XCA,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XCA) */
		{ 0XCB,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XCB) */
		{ 0XCC,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XCC) */
		{ 0XCD,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XCD) */
		{ 0XCE,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XCE) */
		{ 0XCF,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XCF) */
		{ 0XD0,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XD0) */
		{ 0XD1,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XD1) */
		{ 0XD2,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XD2) */
		{ 0XD3,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XD3) */
		{ 0XD4,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XD4) */
		{ 0XD5,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XD5) */
		{ 0XD6,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XD6) */
		{ 0XD7,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XD7) */
		{ 0XD8,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XD8) */
		{ 0XD9,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XD9) */
		{ 0XDA,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XDA) */
		{ 0XDB,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XDB) */
		{ 0XDC,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XDC) */
		{ 0XDD,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XDD) */
		{ 0XDE,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XDE) */
		{ 0XDF,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XDF) */
		{ 0XE0,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XE0) */
		{ 0XE1,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XE1) */
		{ 0XE2,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XE2) */
		{ 0XE3,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XE3) */
		{ 0XE4,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XE4) */
		{ 0XE5,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XE5) */
		{ 0XE6,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XE6) */
		{ 0XE7,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XE7) */
		{ 0XE8,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XE8) */
		{ 0XE9,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XE9) */
		{ 0XEA,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XEA) */
		{ 0XEB,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XEB) */
		{ 0XEC,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XEC) */
		{ 0XED,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XED) */
		{ 0XEE,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XEE) */
		{ 0XEF,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XEF) */
		{ 0XF0,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XF0) */
		{ 0XF1,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XF1) */
		{ 0XF2,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XF2) */
		{ 0XF3,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XF3) */
		{ 0XF4,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XF4) */
		{ 0XF5,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XF5) */
		{ 0XF6,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XF6) */
		{ 0XF7,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XF7) */
		{ 0XF8,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XF8) */
		{ 0XF9,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XF9) */
		{ 0XFA,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XFA) */
		{ 0XFB,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XFB) */
		{ 0XFC,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XFC) */
		{ 0XFD,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XFD) */
		{ 0XFE,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XFE) */
		{ 0XFF,  "MISSINGNO.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO.   (0XFF) */
	};
}