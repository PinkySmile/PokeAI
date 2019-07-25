//
// Created by Gegel85 on 14/07/2019.
//

#include <iostream>
#include <sstream>
#include "Pokemon.hpp"
#include "PokemonTypes.hpp"
#include "Move.hpp"

#define NBR_2B(byte1, byte2) static_cast<unsigned short>((byte1 << 8U) + byte2)

namespace Pokemon
{
	Pokemon::Pokemon(PokemonRandomGenerator &random, const std::string &nickname, unsigned char level, const PokemonBase &base, const std::vector<Move> &moveSet) :
		_id(base.id),
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
		_statusDuration{0},
		_currentStatus(STATUS_NONE),
		_globalCritRatio(-1)
	{
		if (this->_nickname.size() > 10) {
			this->_log("Warning : nickname is too big");
			this->_nickname = this->_nickname.substr(0, 10);
		}
	}

	Pokemon::Pokemon(PokemonRandomGenerator &random, const std::string &nickname, const std::vector<byte> &data) :
		_id(data[0]),
		_lastUsedMove(DEFAULT_MOVE(0x00)),
		_random{random},
		_nickname{nickname},
		_name{pokemonList[data[0]].name},
		_baseStats{
			NBR_2B(data[1], data[2]),   //HP
			NBR_2B(data[34], data[35]), //maxHP
			NBR_2B(data[36], data[37]), //ATK
			NBR_2B(data[38], data[39]), //DEF
			NBR_2B(data[40], data[41]), //SPD
			NBR_2B(data[42], data[43])  //SPE
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
		_level{data[3]},
		_catchRate{data[7]},
		_storingDamages(false),
		_damagesStored(0),
		_statusDuration{0},
		_currentStatus{data[4]},
		_globalCritRatio(-1)
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

		return {
			fct(base.HP),
			fct(base.HP),
			fct(base.ATK),
			fct(base.DEF),
			fct(base.SPD),
			fct(base.SPE)
		};
	}

	void Pokemon::setGlobalCritRatio(double ratio)
	{
		this->_globalCritRatio = ratio;
	}

	void Pokemon::addStatus(StatusChange status)
	{
		if (status == STATUS_BURNED && (this->_types.first == TYPE_FIRE || this->_types.second == TYPE_FIRE))
			return;
		if (this->_currentStatus & status)
			return;
		switch (status) {
		case STATUS_ASLEEP:
			return this->addStatus(status, this->_random(1, 8));
		case STATUS_CONFUSED:
			return this->addStatus(status, this->_random(1, 5));
		default:
			return this->addStatus(status, static_cast<unsigned>(-1));
		}
	}

	void Pokemon::addStatus(StatusChange status, unsigned duration)
	{
		if (status == STATUS_BURNED && (this->_types.first == TYPE_FIRE || this->_types.second == TYPE_FIRE))
			return;
		if ((status == STATUS_POISONED || status == STATUS_BADLY_POISONED) && (this->_types.first == TYPE_POISON || this->_types.second == TYPE_POISON))
			return;
		if (this->_currentStatus & status)
			return;
		this->_log("is now " + statusToString(status));
		this->_badPoisonStage = 1;
		this->_currentStatus = status;
		this->_statusDuration = duration;
	}

	std::string Pokemon::dump() const
	{
		std::stringstream stream;

		stream << this->getName() << " (" << this->_name << ") level " << static_cast<int>(this->_level);
		stream << ", Type " << typeToString(this->_types.first);
		if (this->_types.first != this->_types.second)
			stream << "/" << typeToString(this->_types.second);
		stream << ", " << this->_baseStats.HP << "/" << this->_baseStats.maxHP << " HP";
		stream << ", " << this->_baseStats.ATK << " ATK";
		stream << ", " << this->_baseStats.DEF << " DEF";
		stream << ", " << this->_baseStats.SPD << " SPD";
		stream << ", " << this->_baseStats.SPE << " SPE";
		return stream.str();
	}

	void Pokemon::switched()
	{
		this->_log("is withdrawn");
		this->resetStatsChanges();
		if (this->_currentStatus == STATUS_BADLY_POISONED)
			this->_currentStatus = STATUS_POISONED;
	}

	void Pokemon::resetStatsChanges()
	{
		this->_upgradedStats = {0, 0, 0, 0, 0, 0};
	}

	void Pokemon::useMove(const Move &move, Pokemon &target)
	{
		if (this->_lastUsedMove.isFinished())
			this->_lastUsedMove = move;
		this->_log("uses " + this->_lastUsedMove.getName());
		this->_lastUsedMove.attack(*this, target);
	}

	void Pokemon::storeDamages(bool active)
	{
		this->_storingDamages = active;
		if (!active)
			this->_damagesStored = 0;
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

	unsigned char Pokemon::getID()
	{
		return this->_id;
	}

	unsigned Pokemon::getSpeed() const
	{
		if (this->_currentStatus & STATUS_PARALYZED)
			return this->_getUpgradedStat(this->_baseStats.SPD, this->_upgradedStats.SPD) / 4;
		return this->_getUpgradedStat(this->_baseStats.SPD, this->_upgradedStats.SPD);
	}

	unsigned Pokemon::getAttack() const
	{
		if (this->_currentStatus & STATUS_BURNED)
			return this->_getUpgradedStat(this->_baseStats.ATK, this->_upgradedStats.ATK) / 2;
		return this->_getUpgradedStat(this->_baseStats.ATK, this->_upgradedStats.ATK);
	}

	void Pokemon::endTurn()
	{
	}

	void Pokemon::_log(const std::string &msg) const
	{
		std::cout << "[" << this->getName() << "] " << msg << std::endl;
	}

	void Pokemon::attack(char moveSlot, Pokemon &target)
	{
		if (this->_currentStatus & STATUS_ASLEEP) {
			if (this->_statusDuration) {
				this->_log("is fast asleep");
				this->_statusDuration--;
			} else {
				this->_log("woke up");
				this->_currentStatus = None;
			}
			return;
		} else if ((this->_currentStatus & STATUS_PARALYZED) && !this->_random(0, 4)) {
			this->_log("is paralyzed");
			return;
		} else if ((this->_currentStatus & STATUS_CONFUSED) && !this->_random(0, 1)) {
			if (--this->_statusDuration)
				this->_log("hits itself in it's confusion");
			else {
				this->_log("woke up");
				this->_currentStatus = None;
			}
		}
		this->useMove(this->_moveSet[moveSlot], target);
		if (this->_currentStatus & STATUS_BURNED) {
			this->_log("is hurt by the burn");
			this->takeDamage(this->getMaxHealth() / 16);
		} else if ((this->_currentStatus & STATUS_POISONED) || (this->_currentStatus & STATUS_BADLY_POISONED)) {
			this->_log("is hurt by the poison");
			if (this->_currentStatus & STATUS_BADLY_POISONED)
				this->takeDamage(this->getMaxHealth() * this->_badPoisonStage++ / 16);
			else
				this->takeDamage(this->getMaxHealth() / 16);
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

	void Pokemon::takeDamage(int damage)
	{
		if (damage > static_cast<int>(this->_baseStats.HP))
			this->_baseStats.HP = 0;
		else if (damage < static_cast<int>(this->_baseStats.HP - this->_baseStats.maxHP))
			this->_baseStats.HP = this->_baseStats.maxHP;
		else
			this->_baseStats.HP -= damage;
	}

	std::pair<PokemonTypes, PokemonTypes> Pokemon::getTypes() const
	{
		return this->_types;
	}

	void Pokemon::setTypes(std::pair<PokemonTypes, PokemonTypes> types)
	{
		this->_types = types;
	}

	unsigned Pokemon::dealDamage(Pokemon &target, unsigned power, PokemonTypes damageType) const
	{
		//TODO: Code function
		return power;
	}

	std::string Pokemon::getName() const
	{
		if (this->_nickname.empty())
			return this->_name;
		return this->_nickname;
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
		for (int i = this->_moveSet.size(); i < 4; i++)
			result.push_back(0x00);

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
		result.push_back(0x00);
		result.push_back(0x00);

		//PP Ups and moves PP
		for (const Move &move : this->_moveSet)
			result.push_back(((move.getPPUp() & 0b11U) << 6U) + (move.getPP() & 0b111111U));
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

	unsigned Pokemon::_getUpgradedStat(unsigned char baseValue, char upgradeStage) const
	{
		switch (upgradeStage) {
		case -6:
			return baseValue / 4;
		case -5:
			return 2 * baseValue / 7;
		case -4:
			return baseValue / 3;
		case -3:
			return 2 * baseValue / 5;
		case -2:
			return baseValue / 2;
		case -1:
			return 2 * baseValue / 3;
		default:
			return (upgradeStage + 2) * baseValue / 2;
		}
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
		{ 0x00,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x00) */
		{ 0x01,      "Rhydon", 105, 130, 120,  40,  45,   TYPE_GROUND,     TYPE_ROCK,  60, 204 }, /* RHYDON       (0x01) */
		{ 0x02,  "Kangaskhan", 105,  95,  80,  90,  40,   TYPE_NORMAL,   TYPE_NORMAL,  45, 175 }, /* KANGASKHAN   (0x02) */
		{ 0x03,    "Nidoran♂",  46,  57,  40,  50,  40,   TYPE_POISON,   TYPE_POISON, 235,  60 }, /* NIDORAN_M    (0x03) */
		{ 0x04,    "Clefairy",  70,  45,  48,  35,  60,   TYPE_NORMAL,   TYPE_NORMAL, 150,  68 }, /* CLEFAIRY     (0x04) */
		{ 0x05,     "Spearow",  40,  60,  30,  70,  31,   TYPE_NORMAL,   TYPE_FLYING, 255,  58 }, /* SPEAROW      (0x05) */
		{ 0x06,     "Voltorb",  40,  30,  50, 100,  55, TYPE_ELECTRIC, TYPE_ELECTRIC, 190, 103 }, /* VOLTORB      (0x06) */
		{ 0x07,    "Nidoking",  81,  92,  77,  85,  75,   TYPE_POISON,   TYPE_GROUND,  45, 195 }, /* NIDOKING     (0x07) */
		{ 0x08,     "Slowbro",  95,  75, 110,  30,  80,    TYPE_WATER,  TYPE_PSYCHIC,  75, 164 }, /* SLOWBRO      (0x08) */
		{ 0x09,     "Ivysaur",  60,  62,  63,  60,  80,    TYPE_GRASS,   TYPE_POISON,  45, 141 }, /* IVYSAUR      (0x09) */
		{ 0x0a,   "Exeggutor",  95,  95,  85,  55, 125,    TYPE_GRASS,  TYPE_PSYCHIC,  45, 212 }, /* EXEGGUTOR    (0x0A) */
		{ 0x0b,   "Lickitung",  90,  55,  75,  30,  60,   TYPE_NORMAL,   TYPE_NORMAL,  45, 127 }, /* LICKITUNG    (0x0B) */
		{ 0x0c,   "Exeggcute",  60,  40,  80,  40,  60,    TYPE_GRASS,  TYPE_PSYCHIC,  90,  98 }, /* EXEGGCUTE    (0x0C) */
		{ 0x0d,      "Grimer",  80,  80,  50,  25,  40,   TYPE_POISON,   TYPE_POISON, 190,  90 }, /* GRIMER       (0x0D) */
		{ 0x0e,      "Gengar",  60,  65,  60, 110, 130,    TYPE_GHOST,   TYPE_POISON,  45, 190 }, /* GENGAR       (0x0E) */
		{ 0x0f,    "Nidoran♀",  55,  47,  52,  41,  40,   TYPE_POISON,   TYPE_POISON, 235,  59 }, /* NIDORAN_F    (0x0F) */
		{ 0x10,   "Nidoqueen",  90,  82,  87,  76,  75,   TYPE_POISON,   TYPE_GROUND,  45, 194 }, /* NIDOQUEEN    (0x10) */
		{ 0x11,      "Cubone",  50,  50,  95,  35,  40,   TYPE_GROUND,   TYPE_GROUND, 190,  87 }, /* CUBONE       (0x11) */
		{ 0x12,     "Rhyhorn",  80,  85,  95,  25,  30,   TYPE_GROUND,     TYPE_ROCK, 120, 135 }, /* RHYHORN      (0x12) */
		{ 0x13,      "Lapras", 130,  85,  80,  60,  95,    TYPE_WATER,      TYPE_ICE,  45, 219 }, /* LAPRAS       (0x13) */
		{ 0x14,    "Arcanine",  90, 110,  80,  95,  80,     TYPE_FIRE,     TYPE_FIRE,  75, 213 }, /* ARCANINE     (0x14) */
		{ 0x15,         "Mew", 100, 100, 100, 100, 100,  TYPE_PSYCHIC,  TYPE_PSYCHIC,  45,  64 }, /* MEW          (0x15) */
		{ 0x16,    "Gyarados",  95, 125,  79,  81, 100,    TYPE_WATER,   TYPE_FLYING,  45, 214 }, /* GYARADOS     (0x16) */
		{ 0x17,    "Shellder",  30,  65, 100,  40,  45,    TYPE_WATER,    TYPE_WATER, 190,  97 }, /* SHELLDER     (0x17) */
		{ 0x18,   "Tentacool",  40,  40,  35,  70, 100,    TYPE_WATER,   TYPE_POISON, 190, 105 }, /* TENTACOOL    (0x18) */
		{ 0x19,      "Gastly",  30,  35,  30,  80, 100,    TYPE_GHOST,   TYPE_POISON, 190,  95 }, /* GASTLY       (0x19) */
		{ 0x1a,     "Scyther",  70, 110,  80, 105,  55,      TYPE_BUG,   TYPE_FLYING,  45, 187 }, /* SCYTHER      (0x1A) */
		{ 0x1b,      "Staryu",  30,  45,  55,  85,  70,    TYPE_WATER,    TYPE_WATER, 225, 106 }, /* STARYU       (0x1B) */
		{ 0x1c,   "Blastoise",  79,  83, 100,  78,  85,    TYPE_WATER,    TYPE_WATER,  45, 210 }, /* BLASTOISE    (0x1C) */
		{ 0x1d,      "Pinsir",  65, 125, 100,  85,  55,      TYPE_BUG,      TYPE_BUG,  45, 200 }, /* PINSIR       (0x1D) */
		{ 0x1e,     "Tangela",  65,  55, 115,  60, 100,    TYPE_GRASS,    TYPE_GRASS,  45, 166 }, /* TANGELA      (0x1E) */
		{ 0x1f,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x1F) */
		{ 0x20,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x20) */
		{ 0x21,   "Growlithe",  55,  70,  45,  60,  50,     TYPE_FIRE,     TYPE_FIRE, 190,  91 }, /* GROWLITHE    (0x21) */
		{ 0x22,        "Onix",  35,  45, 160,  70,  30,     TYPE_ROCK,   TYPE_GROUND,  45, 108 }, /* ONIX         (0x22) */
		{ 0x23,      "Fearow",  65,  90,  65, 100,  61,   TYPE_NORMAL,   TYPE_FLYING,  90, 162 }, /* FEAROW       (0x23) */
		{ 0x24,      "Pidgey",  40,  45,  40,  56,  35,   TYPE_NORMAL,   TYPE_FLYING, 255,  55 }, /* PIDGEY       (0x24) */
		{ 0x25,    "Slowpoke",  90,  65,  65,  15,  40,    TYPE_WATER,  TYPE_PSYCHIC, 190,  99 }, /* SLOWPOKE     (0x25) */
		{ 0x26,     "Kadabra",  40,  35,  30, 105, 120,  TYPE_PSYCHIC,  TYPE_PSYCHIC, 100, 145 }, /* KADABRA      (0x26) */
		{ 0x27,    "Graveler",  55,  95, 115,  35,  45,     TYPE_ROCK,   TYPE_GROUND, 120, 134 }, /* GRAVELER     (0x27) */
		{ 0x28,     "Chansey", 250,   5,   5,  50, 105,   TYPE_NORMAL,   TYPE_NORMAL,  30, 255 }, /* CHANSEY      (0x28) */
		{ 0x29,     "Machoke",  80, 100,  70,  45,  50, TYPE_FIGHTING, TYPE_FIGHTING,  90, 146 }, /* MACHOKE      (0x29) */
		{ 0x2a,    "Mr. Mime",  40,  45,  65,  90, 100,  TYPE_PSYCHIC,  TYPE_PSYCHIC,  45, 136 }, /* MR_MIME      (0x2A) */
		{ 0x2b,   "Hitmonlee",  50, 120,  53,  87,  35, TYPE_FIGHTING, TYPE_FIGHTING,  45, 139 }, /* HITMONLEE    (0x2B) */
		{ 0x2c,  "Hitmonchan",  50, 105,  79,  76,  35, TYPE_FIGHTING, TYPE_FIGHTING,  45, 140 }, /* HITMONCHAN   (0x2C) */
		{ 0x2d,       "Arbok",  60,  85,  69,  80,  65,   TYPE_POISON,   TYPE_POISON,  90, 147 }, /* ARBOK        (0x2D) */
		{ 0x2e,    "Parasect",  60,  95,  80,  30,  80,      TYPE_BUG,    TYPE_GRASS,  75, 128 }, /* PARASECT     (0x2E) */
		{ 0x2f,     "Psyduck",  50,  52,  48,  55,  50,    TYPE_WATER,    TYPE_WATER, 190,  80 }, /* PSYDUCK      (0x2F) */
		{ 0x30,     "Drowzee",  60,  48,  45,  42,  90,  TYPE_PSYCHIC,  TYPE_PSYCHIC, 190, 102 }, /* DROWZEE      (0x30) */
		{ 0x31,       "Golem",  80, 110, 130,  45,  55,     TYPE_ROCK,   TYPE_GROUND,  45, 177 }, /* GOLEM        (0x31) */
		{ 0x32,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x32) */
		{ 0x33,      "Magmar",  65,  95,  57,  93,  85,     TYPE_FIRE,     TYPE_FIRE,  45, 167 }, /* MAGMAR       (0x33) */
		{ 0x34,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x34) */
		{ 0x35,  "Electabuzz",  65,  83,  57, 105,  85, TYPE_ELECTRIC, TYPE_ELECTRIC,  45, 156 }, /* ELECTABUZZ   (0x35) */
		{ 0x36,    "Magneton",  50,  60,  95,  70, 120, TYPE_ELECTRIC, TYPE_ELECTRIC,  60, 161 }, /* MAGNETON     (0x36) */
		{ 0x37,     "Koffing",  40,  65,  95,  35,  60,   TYPE_POISON,   TYPE_POISON, 190, 114 }, /* KOFFING      (0x37) */
		{ 0x38,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x38) */
		{ 0x39,      "Mankey",  40,  80,  35,  70,  35, TYPE_FIGHTING, TYPE_FIGHTING, 190,  74 }, /* MANKEY       (0x39) */
		{ 0x3a,        "Seel",  65,  45,  55,  45,  70,    TYPE_WATER,    TYPE_WATER, 190, 100 }, /* SEEL         (0x3A) */
		{ 0x3b,     "Diglett",  10,  55,  25,  95,  45,   TYPE_GROUND,   TYPE_GROUND, 255,  81 }, /* DIGLETT      (0x3B) */
		{ 0x3c,      "Tauros",  75, 100,  95, 110,  70,   TYPE_NORMAL,   TYPE_NORMAL,  45, 211 }, /* TAUROS       (0x3C) */
		{ 0x3d,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x3D) */
		{ 0x3e,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x3E) */
		{ 0x3f,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x3F) */
		{ 0x40,   "Farfetchd",  52,  65,  55,  60,  58,   TYPE_NORMAL,   TYPE_FLYING,  45,  94 }, /* FARFETCH_D   (0x40) */
		{ 0x41,     "Venonat",  60,  55,  50,  45,  40,      TYPE_BUG,   TYPE_POISON, 190,  75 }, /* VENONAT      (0x41) */
		{ 0x42,   "Dragonite",  91, 134,  95,  80, 100,   TYPE_DRAGON,   TYPE_FLYING,  45, 218 }, /* DRAGONITE    (0x42) */
		{ 0x43,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x43) */
		{ 0x44,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x44) */
		{ 0x45,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x45) */
		{ 0x46,       "Doduo",  35,  85,  45,  75,  35,   TYPE_NORMAL,   TYPE_FLYING, 190,  96 }, /* DODUO        (0x46) */
		{ 0x47,     "Poliwag",  40,  50,  40,  90,  40,    TYPE_WATER,    TYPE_WATER, 255,  77 }, /* POLIWAG      (0x47) */
		{ 0x48,        "Jynx",  65,  50,  35,  95,  95,      TYPE_ICE,  TYPE_PSYCHIC,  45, 137 }, /* JYNX         (0x48) */
		{ 0x49,     "Moltres",  90, 100,  90,  90, 125,     TYPE_FIRE,   TYPE_FLYING,   3, 217 }, /* MOLTRES      (0x49) */
		{ 0x4a,    "Articuno",  90,  85, 100,  85, 125,      TYPE_ICE,   TYPE_FLYING,   3, 215 }, /* ARTICUNO     (0x4A) */
		{ 0x4b,      "Zapdos",  90,  90,  85, 100, 125, TYPE_ELECTRIC,   TYPE_FLYING,   3, 216 }, /* ZAPDOS       (0x4B) */
		{ 0x4c,       "Ditto",  48,  48,  48,  48,  48,   TYPE_NORMAL,   TYPE_NORMAL,  35,  61 }, /* DITTO        (0x4C) */
		{ 0x4d,      "Meowth",  40,  45,  35,  90,  40,   TYPE_NORMAL,   TYPE_NORMAL, 255,  69 }, /* MEOWTH       (0x4D) */
		{ 0x4e,      "Krabby",  30, 105,  90,  50,  25,    TYPE_WATER,    TYPE_WATER, 225, 115 }, /* KRABBY       (0x4E) */
		{ 0x4f,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x4F) */
		{ 0x50,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x50) */
		{ 0x51,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x51) */
		{ 0x52,      "Vulpix",  38,  41,  40,  65,  65,     TYPE_FIRE,     TYPE_FIRE, 190,  63 }, /* VULPIX       (0x52) */
		{ 0x53,   "Ninetales",  73,  76,  75, 100, 100,     TYPE_FIRE,     TYPE_FIRE,  75, 178 }, /* NINETALES    (0x53) */
		{ 0x54,     "Pikachu",  35,  55,  30,  90,  50, TYPE_ELECTRIC, TYPE_ELECTRIC, 190,  82 }, /* PIKACHU      (0x54) */
		{ 0x55,      "Raichu",  60,  90,  55, 100,  90, TYPE_ELECTRIC, TYPE_ELECTRIC,  75, 122 }, /* RAICHU       (0x55) */
		{ 0x56,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x56) */
		{ 0x57,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x57) */
		{ 0x58,     "Dratini",  41,  64,  45,  50,  50,   TYPE_DRAGON,   TYPE_DRAGON,  45,  67 }, /* DRATINI      (0x58) */
		{ 0x59,   "Dragonair",  61,  84,  65,  70,  70,   TYPE_DRAGON,   TYPE_DRAGON,  45, 144 }, /* DRAGONAIR    (0x59) */
		{ 0x5a,      "Kabuto",  30,  80,  90,  55,  45,     TYPE_ROCK,    TYPE_WATER,  45, 119 }, /* KABUTO       (0x5A) */
		{ 0x5b,    "Kabutops",  60, 115, 105,  80,  70,     TYPE_ROCK,    TYPE_WATER,  45, 201 }, /* KABUTOPS     (0x5B) */
		{ 0x5c,      "Horsea",  30,  40,  70,  60,  70,    TYPE_WATER,    TYPE_WATER, 225,  83 }, /* HORSEA       (0x5C) */
		{ 0x5d,      "Seadra",  55,  65,  95,  85,  95,    TYPE_WATER,    TYPE_WATER,  75, 155 }, /* SEADRA       (0x5D) */
		{ 0x5e,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x5E) */
		{ 0x5f,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x5F) */
		{ 0x60,   "Sandshrew",  50,  75,  85,  40,  30,   TYPE_GROUND,   TYPE_GROUND, 255,  93 }, /* SANDSHREW    (0x60) */
		{ 0x61,   "Sandslash",  75, 100, 110,  65,  55,   TYPE_GROUND,   TYPE_GROUND,  90, 163 }, /* SANDSLASH    (0x61) */
		{ 0x62,     "Omanyte",  35,  40, 100,  35,  90,     TYPE_ROCK,    TYPE_WATER,  45, 120 }, /* OMANYTE      (0x62) */
		{ 0x63,     "Omastar",  70,  60, 125,  55, 115,     TYPE_ROCK,    TYPE_WATER,  45, 199 }, /* OMASTAR      (0x63) */
		{ 0x64,  "Jigglypuff", 115,  45,  20,  20,  25,   TYPE_NORMAL,   TYPE_NORMAL, 170,  76 }, /* JIGGLYPUFF   (0x64) */
		{ 0x65,  "Wigglytuff", 140,  70,  45,  45,  50,   TYPE_NORMAL,   TYPE_NORMAL,  50, 109 }, /* WIGGLYTUFF   (0x65) */
		{ 0x66,       "Eevee",  55,  55,  50,  55,  65,   TYPE_NORMAL,   TYPE_NORMAL,  45,  92 }, /* EEVEE        (0x66) */
		{ 0x67,     "Flareon",  65, 130,  60,  65, 110,     TYPE_FIRE,     TYPE_FIRE,  45, 198 }, /* FLAREON      (0x67) */
		{ 0x68,     "Jolteon",  65,  65,  60, 130, 110, TYPE_ELECTRIC, TYPE_ELECTRIC,  45, 197 }, /* JOLTEON      (0x68) */
		{ 0x69,    "Vaporeon", 130,  65,  60,  65, 110,    TYPE_WATER,    TYPE_WATER,  45, 196 }, /* VAPOREON     (0x69) */
		{ 0x6a,      "Machop",  70,  80,  50,  35,  35, TYPE_FIGHTING, TYPE_FIGHTING, 180,  88 }, /* MACHOP       (0x6A) */
		{ 0x6b,       "Zubat",  40,  45,  35,  55,  40,   TYPE_POISON,   TYPE_FLYING, 255,  54 }, /* ZUBAT        (0x6B) */
		{ 0x6c,       "Ekans",  35,  60,  44,  55,  40,   TYPE_POISON,   TYPE_POISON, 255,  62 }, /* EKANS        (0x6C) */
		{ 0x6d,       "Paras",  35,  70,  55,  25,  55,      TYPE_BUG,    TYPE_GRASS, 190,  70 }, /* PARAS        (0x6D) */
		{ 0x6e,   "Poliwhirl",  65,  65,  65,  90,  50,    TYPE_WATER,    TYPE_WATER, 120, 131 }, /* POLIWHIRL    (0x6E) */
		{ 0x6f,   "Poliwrath",  90,  85,  95,  70,  70,    TYPE_WATER, TYPE_FIGHTING,  45, 185 }, /* POLIWRATH    (0x6F) */
		{ 0x70,      "Weedle",  40,  35,  30,  50,  20,      TYPE_BUG,   TYPE_POISON, 255,  52 }, /* WEEDLE       (0x70) */
		{ 0x71,      "Kakuna",  45,  25,  50,  35,  25,      TYPE_BUG,   TYPE_POISON, 120,  71 }, /* KAKUNA       (0x71) */
		{ 0x72,    "Beedrill",  65,  80,  40,  75,  45,      TYPE_BUG,   TYPE_POISON,  45, 159 }, /* BEEDRILL     (0x72) */
		{ 0x73,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x73) */
		{ 0x74,      "Dodrio",  60, 110,  70, 100,  60,   TYPE_NORMAL,   TYPE_FLYING,  45, 158 }, /* DODRIO       (0x74) */
		{ 0x75,    "Primeape",  65, 105,  60,  95,  60, TYPE_FIGHTING, TYPE_FIGHTING,  75, 149 }, /* PRIMEAPE     (0x75) */
		{ 0x76,     "Dugtrio",  35,  80,  50, 120,  70,   TYPE_GROUND,   TYPE_GROUND,  50, 153 }, /* DUGTRIO      (0x76) */
		{ 0x77,    "Venomoth",  70,  65,  60,  90,  90,      TYPE_BUG,   TYPE_POISON,  75, 138 }, /* VENOMOTH     (0x77) */
		{ 0x78,     "Dewgong",  90,  70,  80,  70,  95,    TYPE_WATER,      TYPE_ICE,  75, 176 }, /* DEWGONG      (0x78) */
		{ 0x79,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x79) */
		{ 0x7a,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x7A) */
		{ 0x7b,    "Caterpie",  45,  30,  35,  45,  20,      TYPE_BUG,      TYPE_BUG, 255,  53 }, /* CATERPIE     (0x7B) */
		{ 0x7c,     "Metapod",  50,  20,  55,  30,  25,      TYPE_BUG,      TYPE_BUG, 120,  72 }, /* METAPOD      (0x7C) */
		{ 0x7d,  "Butterfree",  60,  45,  50,  70,  80,      TYPE_BUG,   TYPE_FLYING,  45, 160 }, /* BUTTERFREE   (0x7D) */
		{ 0x7e,     "Machamp",  90, 130,  80,  55,  65, TYPE_FIGHTING, TYPE_FIGHTING,  45, 193 }, /* MACHAMP      (0x7E) */
		{ 0x7f,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x7F) */
		{ 0x80,     "Golduck",  80,  82,  78,  85,  80,    TYPE_WATER,    TYPE_WATER,  75, 174 }, /* GOLDUCK      (0x80) */
		{ 0x81,       "Hypno",  85,  73,  70,  67, 115,  TYPE_PSYCHIC,  TYPE_PSYCHIC,  75, 165 }, /* HYPNO        (0x81) */
		{ 0x82,      "Golbat",  75,  80,  70,  90,  75,   TYPE_POISON,   TYPE_FLYING,  90, 171 }, /* GOLBAT       (0x82) */
		{ 0x83,      "Mewtwo", 106, 110,  90, 130, 154,  TYPE_PSYCHIC,  TYPE_PSYCHIC,   3, 220 }, /* MEWTWO       (0x83) */
		{ 0x84,     "Snorlax", 160, 110,  65,  30,  65,   TYPE_NORMAL,   TYPE_NORMAL,  25, 154 }, /* SNORLAX      (0x84) */
		{ 0x85,    "Magikarp",  20,  10,  55,  80,  20,    TYPE_WATER,    TYPE_WATER, 255,  20 }, /* MAGIKARP     (0x85) */
		{ 0x86,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x86) */
		{ 0x87,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x87) */
		{ 0x88,         "Muk", 105, 105,  75,  50,  65,   TYPE_POISON,   TYPE_POISON,  75, 157 }, /* MUK          (0x88) */
		{ 0x89,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x89) */
		{ 0x8a,     "Kingler",  55, 130, 115,  75,  50,    TYPE_WATER,    TYPE_WATER,  60, 206 }, /* KINGLER      (0x8A) */
		{ 0x8b,    "Cloyster",  50,  95, 180,  70,  85,    TYPE_WATER,      TYPE_ICE,  60, 203 }, /* CLOYSTER     (0x8B) */
		{ 0x8c,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x8C) */
		{ 0x8d,   "Electrode",  60,  50,  70, 140,  80, TYPE_ELECTRIC, TYPE_ELECTRIC,  60, 150 }, /* ELECTRODE    (0x8D) */
		{ 0x8e,    "Clefable",  95,  70,  73,  60,  85,   TYPE_NORMAL,   TYPE_NORMAL,  25, 129 }, /* CLEFABLE     (0x8E) */
		{ 0x8f,     "Weezing",  65,  90, 120,  60,  85,   TYPE_POISON,   TYPE_POISON,  60, 173 }, /* WEEZING      (0x8F) */
		{ 0x90,     "Persian",  65,  70,  60, 115,  65,   TYPE_NORMAL,   TYPE_NORMAL,  90, 148 }, /* PERSIAN      (0x90) */
		{ 0x91,     "Marowak",  60,  80, 110,  45,  50,   TYPE_GROUND,   TYPE_GROUND,  75, 124 }, /* MAROWAK      (0x91) */
		{ 0x92,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x92) */
		{ 0x93,     "Haunter",  45,  50,  45,  95, 115,    TYPE_GHOST,   TYPE_POISON,  90, 126 }, /* HAUNTER      (0x93) */
		{ 0x94,        "Abra",  25,  20,  15,  90, 105,  TYPE_PSYCHIC,  TYPE_PSYCHIC, 200,  73 }, /* ABRA         (0x94) */
		{ 0x95,    "Alakazam",  55,  50,  45, 120, 135,  TYPE_PSYCHIC,  TYPE_PSYCHIC,  50, 186 }, /* ALAKAZAM     (0x95) */
		{ 0x96,   "Pidgeotto",  63,  60,  55,  71,  50,   TYPE_NORMAL,   TYPE_FLYING, 120, 113 }, /* PIDGEOTTO    (0x96) */
		{ 0x97,     "Pidgeot",  83,  80,  75,  91,  70,   TYPE_NORMAL,   TYPE_FLYING,  45, 172 }, /* PIDGEOT      (0x97) */
		{ 0x98,     "Starmie",  60,  75,  85, 115, 100,    TYPE_WATER,  TYPE_PSYCHIC,  60, 207 }, /* STARMIE      (0x98) */
		{ 0x99,   "Bulbasaur",  45,  49,  49,  45,  65,    TYPE_GRASS,   TYPE_POISON,  45,  64 }, /* BULBASAUR    (0x99) */
		{ 0x9a,    "Venusaur",  80,  82,  83,  80, 100,    TYPE_GRASS,   TYPE_POISON,  45, 208 }, /* VENUSAUR     (0x9A) */
		{ 0x9b,  "Tentacruel",  80,  70,  65, 100, 120,    TYPE_WATER,   TYPE_POISON,  60, 205 }, /* TENTACRUEL   (0x9B) */
		{ 0x9c,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x9C) */
		{ 0x9d,     "Goldeen",  45,  67,  60,  63,  50,    TYPE_WATER,    TYPE_WATER, 225, 111 }, /* GOLDEEN      (0x9D) */
		{ 0x9e,     "Seaking",  80,  92,  65,  68,  80,    TYPE_WATER,    TYPE_WATER,  60, 170 }, /* SEAKING      (0x9E) */
		{ 0x9f,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x9F) */
		{ 0xa0,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xA0) */
		{ 0xa1,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xA1) */
		{ 0xa2,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xA2) */
		{ 0xa3,      "Ponyta",  50,  85,  55,  90,  65,     TYPE_FIRE,     TYPE_FIRE, 190, 152 }, /* PONYTA       (0xA3) */
		{ 0xa4,    "Rapidash",  65, 100,  70, 105,  80,     TYPE_FIRE,     TYPE_FIRE,  60, 192 }, /* RAPIDASH     (0xA4) */
		{ 0xa5,     "Rattata",  30,  56,  35,  72,  25,   TYPE_NORMAL,   TYPE_NORMAL, 255,  57 }, /* RATTATA      (0xA5) */
		{ 0xa6,    "Raticate",  55,  81,  60,  97,  50,   TYPE_NORMAL,   TYPE_NORMAL,  90, 116 }, /* RATICATE     (0xA6) */
		{ 0xa7,    "Nidorino",  61,  72,  57,  65,  55,   TYPE_POISON,   TYPE_POISON, 120, 118 }, /* NIDORINO     (0xA7) */
		{ 0xa8,    "Nidorina",  70,  62,  67,  56,  55,   TYPE_POISON,   TYPE_POISON, 120, 117 }, /* NIDORINA     (0xA8) */
		{ 0xa9,     "Geodude",  40,  80, 100,  20,  30,     TYPE_ROCK,   TYPE_GROUND, 255,  86 }, /* GEODUDE      (0xA9) */
		{ 0xaa,     "Porygon",  65,  60,  70,  40,  75,   TYPE_NORMAL,   TYPE_NORMAL,  45, 130 }, /* PORYGON      (0xAA) */
		{ 0xab,  "Aerodactyl",  80, 105,  65, 130,  60,     TYPE_ROCK,   TYPE_FLYING,  45, 202 }, /* AERODACTYL   (0xAB) */
		{ 0xac,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xAC) */
		{ 0xad,   "Magnemite",  25,  35,  70,  45,  95, TYPE_ELECTRIC, TYPE_ELECTRIC, 190,  89 }, /* MAGNEMITE    (0xAD) */
		{ 0xae,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xAE) */
		{ 0xaf,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xAF) */
		{ 0xb0,  "Charmander",  39,  52,  43,  65,  50,     TYPE_FIRE,     TYPE_FIRE,  45,  65 }, /* CHARMANDER   (0xB0) */
		{ 0xb1,    "Squirtle",  44,  48,  65,  43,  50,    TYPE_WATER,    TYPE_WATER,  45,  66 }, /* SQUIRTLE     (0xB1) */
		{ 0xb2,  "Charmeleon",  58,  64,  58,  80,  65,     TYPE_FIRE,     TYPE_FIRE,  45, 142 }, /* CHARMELEON   (0xB2) */
		{ 0xb3,   "Wartortle",  59,  63,  80,  58,  65,    TYPE_WATER,    TYPE_WATER,  45, 143 }, /* WARTORTLE    (0xB3) */
		{ 0xb4,   "Charizard",  78,  84,  78, 100,  85,     TYPE_FIRE,   TYPE_FLYING,  45, 209 }, /* CHARIZARD    (0xB4) */
		{ 0xb5,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xB5) */
		{ 0xb6,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xB6) */
		{ 0xb7,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xB7) */
		{ 0xb8,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xB8) */
		{ 0xb9,      "Oddish",  45,  50,  55,  30,  75,    TYPE_GRASS,   TYPE_POISON, 255,  78 }, /* ODDISH       (0xB9) */
		{ 0xba,       "Gloom",  60,  65,  70,  40,  85,    TYPE_GRASS,   TYPE_POISON, 120, 132 }, /* GLOOM        (0xBA) */
		{ 0xbb,   "Vileplume",  75,  80,  85,  50, 100,    TYPE_GRASS,   TYPE_POISON,  45, 184 }, /* VILEPLUME    (0xBB) */
		{ 0xbc,  "Bellsprout",  50,  75,  35,  40,  70,    TYPE_GRASS,   TYPE_POISON, 255,  84 }, /* BELLSPROUT   (0xBC) */
		{ 0xbd,  "Weepinbell",  65,  90,  50,  55,  85,    TYPE_GRASS,   TYPE_POISON, 120, 151 }, /* WEEPINBELL   (0xBD) */
		{ 0xbe,  "Victreebel",  80, 105,  65,  70, 100,    TYPE_GRASS,   TYPE_POISON,  45, 191 }, /* VICTREEBEL   (0xBE) */
		{ 0xbf,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xBF) */
		{ 0xc0,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC0) */
		{ 0xc1,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC1) */
		{ 0xc2,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC2) */
		{ 0xc3,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC3) */
		{ 0xc4,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC4) */
		{ 0xc5,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC5) */
		{ 0xc6,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC6) */
		{ 0xc7,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC7) */
		{ 0xc8,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC8) */
		{ 0xc9,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC9) */
		{ 0xca,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xCA) */
		{ 0xcb,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xCB) */
		{ 0xcc,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xCC) */
		{ 0xcd,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xCD) */
		{ 0xce,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xCE) */
		{ 0xcf,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xCF) */
		{ 0xd0,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD0) */
		{ 0xd1,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD1) */
		{ 0xd2,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD2) */
		{ 0xd3,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD3) */
		{ 0xd4,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD4) */
		{ 0xd5,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD5) */
		{ 0xd6,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD6) */
		{ 0xd7,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD7) */
		{ 0xd8,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD8) */
		{ 0xd9,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD9) */
		{ 0xda,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xDA) */
		{ 0xdb,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xDB) */
		{ 0xdc,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xDC) */
		{ 0xdd,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xDD) */
		{ 0xde,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xDE) */
		{ 0xdf,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xDF) */
		{ 0xe0,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE0) */
		{ 0xe1,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE1) */
		{ 0xe2,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE2) */
		{ 0xe3,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE3) */
		{ 0xe4,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE4) */
		{ 0xe5,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE5) */
		{ 0xe6,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE6) */
		{ 0xe7,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE7) */
		{ 0xe8,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE8) */
		{ 0xe9,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE9) */
		{ 0xea,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xEA) */
		{ 0xeb,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xEB) */
		{ 0xec,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xEC) */
		{ 0xed,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xED) */
		{ 0xee,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xEE) */
		{ 0xef,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xEF) */
		{ 0xf0,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF0) */
		{ 0xf1,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF1) */
		{ 0xf2,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF2) */
		{ 0xf3,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF3) */
		{ 0xf4,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF4) */
		{ 0xf5,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF5) */
		{ 0xf6,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF6) */
		{ 0xf7,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF7) */
		{ 0xf8,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF8) */
		{ 0xf9,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF9) */
		{ 0xfa,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xFA) */
		{ 0xfb,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xFB) */
		{ 0xfc,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xFC) */
		{ 0xfd,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xFD) */
		{ 0xfe,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xFE) */
		{ 0xff,  "Missingno.",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xFF) */
	};
}