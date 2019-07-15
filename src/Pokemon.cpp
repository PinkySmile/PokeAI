//
// Created by Gegel85 on 14/07/2019.
//

#include "Pokemon.hpp"
#include "PokemonTypes.hpp"
#include "Move.hpp"

#define NBR_2B(byte1, byte2) static_cast<unsigned short>((byte1 << 8U) + byte2)

namespace Pokemon
{
	Pokemon::Pokemon(PokemonRandomGenerator &random, const std::string &nickname, unsigned char level, const PokemonBase &base, const std::vector<Move> &moveSet) :
		_lastUsedMove(DEFAULT_MOVE(0x00)),
		_random{random},
		_nickname{nickname},
		_name{base.name},
		_baseStats{base.HP, base.HP, base.ATK, base.DEF, base.SPD, base.SPE},
		_upgradedStats{0, 0, 0, 0, 0, 0},
		_moveSet{moveSet},
		_types{base.typeA, base.typeB},
		_level(level),
		_catchRate{base.catchRate},
		_storingDamages(false),
		_damagesStored(0),
		_statusDuration{0, 0},
		_currentStatus(STATUS_NONE),
		_globalCritRatio(-1)
	{
	}

	Pokemon::Pokemon(PokemonRandomGenerator &random, const std::string &nickname, const std::vector<byte> &data) :
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
		_statusDuration{0, 0},
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

	void Pokemon::setGlobalCritRatio(double ratio)
	{
		this->_globalCritRatio = ratio;
	}

	void Pokemon::addStatus(StatusChange status)
	{
		if (this->_currentStatus & status)
			return;
		switch (status) {
		case STATUS_ASLEEP:
			return this->addStatus(status, this->_random(1, 3));
		case STATUS_CONFUSED:
			return this->addStatus(status, this->_random(1, 4));
		default:
			return this->addStatus(status, 0);
		}
	}

	void Pokemon::addStatus(StatusChange status, unsigned duration)
	{
		if (this->_currentStatus & status)
			return;
		this->_currentStatus |= status;
		switch (status) {
		case STATUS_ASLEEP:
			this->_statusDuration[0] = duration;
			return;
		case STATUS_CONFUSED:
			this->_statusDuration[1] = duration;
			return;
		default:
			return;
		}
	}

	void Pokemon::resetStatsChanges()
	{
		this->_upgradedStats = {0, 0, 0, 0, 0, 0};
	}

	void Pokemon::useMove(const Move &move, Pokemon &target)
	{
		this->_lastUsedMove = move;
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

	unsigned Pokemon::getSpeed() const
	{
		switch (this->_upgradedStats.SPD) {
		case -6:
			return this->_baseStats.SPD / 4;
		case -5:
			return 2 * this->_baseStats.SPD / 7;
		case -4:
			return this->_baseStats.SPD / 3;
		case -3:
			return 2 * this->_baseStats.SPD / 5;
		case -2:
			return this->_baseStats.SPD / 2;
		case -1:
			return 2 * this->_baseStats.SPD / 3;
		default:
			return (this->_upgradedStats.SPD + 2) * this->_baseStats.SPD / 2;
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

	/*
	** From Rhydon
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Tables.cs#L82
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Tables.cs#L202
	*/

	const std::vector<PokemonBase> pokemonList{
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x00) */
		{     "Rhydon", 105, 130, 120,  40,  45,   TYPE_GROUND,     TYPE_ROCK,  60, 204 }, /* RHYDON       (0x01) */
		{ "Kangaskhan", 105,  95,  80,  90,  40,   TYPE_NORMAL,   TYPE_NORMAL,  45, 175 }, /* KANGASKHAN   (0x02) */
		{   "Nidoran♂",  46,  57,  40,  50,  40,   TYPE_POISON,   TYPE_POISON, 235,  60 }, /* NIDORAN_M    (0x03) */
		{   "Clefairy",  70,  45,  48,  35,  60,   TYPE_NORMAL,   TYPE_NORMAL, 150,  68 }, /* CLEFAIRY     (0x04) */
		{    "Spearow",  40,  60,  30,  70,  31,   TYPE_NORMAL,   TYPE_FLYING, 255,  58 }, /* SPEAROW      (0x05) */
		{    "Voltorb",  40,  30,  50, 100,  55, TYPE_ELECTRIC, TYPE_ELECTRIC, 190, 103 }, /* VOLTORB      (0x06) */
		{   "Nidoking",  81,  92,  77,  85,  75,   TYPE_POISON,   TYPE_GROUND,  45, 195 }, /* NIDOKING     (0x07) */
		{    "Slowbro",  95,  75, 110,  30,  80,    TYPE_WATER,  TYPE_PSYCHIC,  75, 164 }, /* SLOWBRO      (0x08) */
		{    "Ivysaur",  60,  62,  63,  60,  80,    TYPE_GRASS,   TYPE_POISON,  45, 141 }, /* IVYSAUR      (0x09) */
		{  "Exeggutor",  95,  95,  85,  55, 125,    TYPE_GRASS,  TYPE_PSYCHIC,  45, 212 }, /* EXEGGUTOR    (0x0A) */
		{  "Lickitung",  90,  55,  75,  30,  60,   TYPE_NORMAL,   TYPE_NORMAL,  45, 127 }, /* LICKITUNG    (0x0B) */
		{  "Exeggcute",  60,  40,  80,  40,  60,    TYPE_GRASS,  TYPE_PSYCHIC,  90,  98 }, /* EXEGGCUTE    (0x0C) */
		{     "Grimer",  80,  80,  50,  25,  40,   TYPE_POISON,   TYPE_POISON, 190,  90 }, /* GRIMER       (0x0D) */
		{     "Gengar",  60,  65,  60, 110, 130,    TYPE_GHOST,   TYPE_POISON,  45, 190 }, /* GENGAR       (0x0E) */
		{   "Nidoran♀",  55,  47,  52,  41,  40,   TYPE_POISON,   TYPE_POISON, 235,  59 }, /* NIDORAN_F    (0x0F) */
		{  "Nidoqueen",  90,  82,  87,  76,  75,   TYPE_POISON,   TYPE_GROUND,  45, 194 }, /* NIDOQUEEN    (0x10) */
		{     "Cubone",  50,  50,  95,  35,  40,   TYPE_GROUND,   TYPE_GROUND, 190,  87 }, /* CUBONE       (0x11) */
		{    "Rhyhorn",  80,  85,  95,  25,  30,   TYPE_GROUND,     TYPE_ROCK, 120, 135 }, /* RHYHORN      (0x12) */
		{     "Lapras", 130,  85,  80,  60,  95,    TYPE_WATER,      TYPE_ICE,  45, 219 }, /* LAPRAS       (0x13) */
		{   "Arcanine",  90, 110,  80,  95,  80,     TYPE_FIRE,     TYPE_FIRE,  75, 213 }, /* ARCANINE     (0x14) */
		{        "Mew", 100, 100, 100, 100, 100,  TYPE_PSYCHIC,  TYPE_PSYCHIC,  45,  64 }, /* MEW          (0x15) */
		{   "Gyarados",  95, 125,  79,  81, 100,    TYPE_WATER,   TYPE_FLYING,  45, 214 }, /* GYARADOS     (0x16) */
		{   "Shellder",  30,  65, 100,  40,  45,    TYPE_WATER,    TYPE_WATER, 190,  97 }, /* SHELLDER     (0x17) */
		{  "Tentacool",  40,  40,  35,  70, 100,    TYPE_WATER,   TYPE_POISON, 190, 105 }, /* TENTACOOL    (0x18) */
		{     "Gastly",  30,  35,  30,  80, 100,    TYPE_GHOST,   TYPE_POISON, 190,  95 }, /* GASTLY       (0x19) */
		{    "Scyther",  70, 110,  80, 105,  55,      TYPE_BUG,   TYPE_FLYING,  45, 187 }, /* SCYTHER      (0x1A) */
		{     "Staryu",  30,  45,  55,  85,  70,    TYPE_WATER,    TYPE_WATER, 225, 106 }, /* STARYU       (0x1B) */
		{  "Blastoise",  79,  83, 100,  78,  85,    TYPE_WATER,    TYPE_WATER,  45, 210 }, /* BLASTOISE    (0x1C) */
		{     "Pinsir",  65, 125, 100,  85,  55,      TYPE_BUG,      TYPE_BUG,  45, 200 }, /* PINSIR       (0x1D) */
		{    "Tangela",  65,  55, 115,  60, 100,    TYPE_GRASS,    TYPE_GRASS,  45, 166 }, /* TANGELA      (0x1E) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x1F) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x20) */
		{  "Growlithe",  55,  70,  45,  60,  50,     TYPE_FIRE,     TYPE_FIRE, 190,  91 }, /* GROWLITHE    (0x21) */
		{       "Onix",  35,  45, 160,  70,  30,     TYPE_ROCK,   TYPE_GROUND,  45, 108 }, /* ONIX         (0x22) */
		{     "Fearow",  65,  90,  65, 100,  61,   TYPE_NORMAL,   TYPE_FLYING,  90, 162 }, /* FEAROW       (0x23) */
		{     "Pidgey",  40,  45,  40,  56,  35,   TYPE_NORMAL,   TYPE_FLYING, 255,  55 }, /* PIDGEY       (0x24) */
		{   "Slowpoke",  90,  65,  65,  15,  40,    TYPE_WATER,  TYPE_PSYCHIC, 190,  99 }, /* SLOWPOKE     (0x25) */
		{    "Kadabra",  40,  35,  30, 105, 120,  TYPE_PSYCHIC,  TYPE_PSYCHIC, 100, 145 }, /* KADABRA      (0x26) */
		{   "Graveler",  55,  95, 115,  35,  45,     TYPE_ROCK,   TYPE_GROUND, 120, 134 }, /* GRAVELER     (0x27) */
		{    "Chansey", 250,   5,   5,  50, 105,   TYPE_NORMAL,   TYPE_NORMAL,  30, 255 }, /* CHANSEY      (0x28) */
		{    "Machoke",  80, 100,  70,  45,  50, TYPE_FIGHTING, TYPE_FIGHTING,  90, 146 }, /* MACHOKE      (0x29) */
		{   "Mr. Mime",  40,  45,  65,  90, 100,  TYPE_PSYCHIC,  TYPE_PSYCHIC,  45, 136 }, /* MR_MIME      (0x2A) */
		{  "Hitmonlee",  50, 120,  53,  87,  35, TYPE_FIGHTING, TYPE_FIGHTING,  45, 139 }, /* HITMONLEE    (0x2B) */
		{ "Hitmonchan",  50, 105,  79,  76,  35, TYPE_FIGHTING, TYPE_FIGHTING,  45, 140 }, /* HITMONCHAN   (0x2C) */
		{      "Arbok",  60,  85,  69,  80,  65,   TYPE_POISON,   TYPE_POISON,  90, 147 }, /* ARBOK        (0x2D) */
		{   "Parasect",  60,  95,  80,  30,  80,      TYPE_BUG,    TYPE_GRASS,  75, 128 }, /* PARASECT     (0x2E) */
		{    "Psyduck",  50,  52,  48,  55,  50,    TYPE_WATER,    TYPE_WATER, 190,  80 }, /* PSYDUCK      (0x2F) */
		{    "Drowzee",  60,  48,  45,  42,  90,  TYPE_PSYCHIC,  TYPE_PSYCHIC, 190, 102 }, /* DROWZEE      (0x30) */
		{      "Golem",  80, 110, 130,  45,  55,     TYPE_ROCK,   TYPE_GROUND,  45, 177 }, /* GOLEM        (0x31) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x32) */
		{     "Magmar",  65,  95,  57,  93,  85,     TYPE_FIRE,     TYPE_FIRE,  45, 167 }, /* MAGMAR       (0x33) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x34) */
		{ "Electabuzz",  65,  83,  57, 105,  85, TYPE_ELECTRIC, TYPE_ELECTRIC,  45, 156 }, /* ELECTABUZZ   (0x35) */
		{   "Magneton",  50,  60,  95,  70, 120, TYPE_ELECTRIC, TYPE_ELECTRIC,  60, 161 }, /* MAGNETON     (0x36) */
		{    "Koffing",  40,  65,  95,  35,  60,   TYPE_POISON,   TYPE_POISON, 190, 114 }, /* KOFFING      (0x37) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x38) */
		{     "Mankey",  40,  80,  35,  70,  35, TYPE_FIGHTING, TYPE_FIGHTING, 190,  74 }, /* MANKEY       (0x39) */
		{       "Seel",  65,  45,  55,  45,  70,    TYPE_WATER,    TYPE_WATER, 190, 100 }, /* SEEL         (0x3A) */
		{    "Diglett",  10,  55,  25,  95,  45,   TYPE_GROUND,   TYPE_GROUND, 255,  81 }, /* DIGLETT      (0x3B) */
		{     "Tauros",  75, 100,  95, 110,  70,   TYPE_NORMAL,   TYPE_NORMAL,  45, 211 }, /* TAUROS       (0x3C) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x3D) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x3E) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x3F) */
		{  "Farfetchd",  52,  65,  55,  60,  58,   TYPE_NORMAL,   TYPE_FLYING,  45,  94 }, /* FARFETCH_D   (0x40) */
		{    "Venonat",  60,  55,  50,  45,  40,      TYPE_BUG,   TYPE_POISON, 190,  75 }, /* VENONAT      (0x41) */
		{  "Dragonite",  91, 134,  95,  80, 100,   TYPE_DRAGON,   TYPE_FLYING,  45, 218 }, /* DRAGONITE    (0x42) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x43) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x44) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x45) */
		{      "Doduo",  35,  85,  45,  75,  35,   TYPE_NORMAL,   TYPE_FLYING, 190,  96 }, /* DODUO        (0x46) */
		{    "Poliwag",  40,  50,  40,  90,  40,    TYPE_WATER,    TYPE_WATER, 255,  77 }, /* POLIWAG      (0x47) */
		{       "Jynx",  65,  50,  35,  95,  95,      TYPE_ICE,  TYPE_PSYCHIC,  45, 137 }, /* JYNX         (0x48) */
		{    "Moltres",  90, 100,  90,  90, 125,     TYPE_FIRE,   TYPE_FLYING,   3, 217 }, /* MOLTRES      (0x49) */
		{   "Articuno",  90,  85, 100,  85, 125,      TYPE_ICE,   TYPE_FLYING,   3, 215 }, /* ARTICUNO     (0x4A) */
		{     "Zapdos",  90,  90,  85, 100, 125, TYPE_ELECTRIC,   TYPE_FLYING,   3, 216 }, /* ZAPDOS       (0x4B) */
		{      "Ditto",  48,  48,  48,  48,  48,   TYPE_NORMAL,   TYPE_NORMAL,  35,  61 }, /* DITTO        (0x4C) */
		{     "Meowth",  40,  45,  35,  90,  40,   TYPE_NORMAL,   TYPE_NORMAL, 255,  69 }, /* MEOWTH       (0x4D) */
		{     "Krabby",  30, 105,  90,  50,  25,    TYPE_WATER,    TYPE_WATER, 225, 115 }, /* KRABBY       (0x4E) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x4F) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x50) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x51) */
		{     "Vulpix",  38,  41,  40,  65,  65,     TYPE_FIRE,     TYPE_FIRE, 190,  63 }, /* VULPIX       (0x52) */
		{  "Ninetales",  73,  76,  75, 100, 100,     TYPE_FIRE,     TYPE_FIRE,  75, 178 }, /* NINETALES    (0x53) */
		{    "Pikachu",  35,  55,  30,  90,  50, TYPE_ELECTRIC, TYPE_ELECTRIC, 190,  82 }, /* PIKACHU      (0x54) */
		{     "Raichu",  60,  90,  55, 100,  90, TYPE_ELECTRIC, TYPE_ELECTRIC,  75, 122 }, /* RAICHU       (0x55) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x56) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x57) */
		{    "Dratini",  41,  64,  45,  50,  50,   TYPE_DRAGON,   TYPE_DRAGON,  45,  67 }, /* DRATINI      (0x58) */
		{  "Dragonair",  61,  84,  65,  70,  70,   TYPE_DRAGON,   TYPE_DRAGON,  45, 144 }, /* DRAGONAIR    (0x59) */
		{     "Kabuto",  30,  80,  90,  55,  45,     TYPE_ROCK,    TYPE_WATER,  45, 119 }, /* KABUTO       (0x5A) */
		{   "Kabutops",  60, 115, 105,  80,  70,     TYPE_ROCK,    TYPE_WATER,  45, 201 }, /* KABUTOPS     (0x5B) */
		{     "Horsea",  30,  40,  70,  60,  70,    TYPE_WATER,    TYPE_WATER, 225,  83 }, /* HORSEA       (0x5C) */
		{     "Seadra",  55,  65,  95,  85,  95,    TYPE_WATER,    TYPE_WATER,  75, 155 }, /* SEADRA       (0x5D) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x5E) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x5F) */
		{  "Sandshrew",  50,  75,  85,  40,  30,   TYPE_GROUND,   TYPE_GROUND, 255,  93 }, /* SANDSHREW    (0x60) */
		{  "Sandslash",  75, 100, 110,  65,  55,   TYPE_GROUND,   TYPE_GROUND,  90, 163 }, /* SANDSLASH    (0x61) */
		{    "Omanyte",  35,  40, 100,  35,  90,     TYPE_ROCK,    TYPE_WATER,  45, 120 }, /* OMANYTE      (0x62) */
		{    "Omastar",  70,  60, 125,  55, 115,     TYPE_ROCK,    TYPE_WATER,  45, 199 }, /* OMASTAR      (0x63) */
		{ "Jigglypuff", 115,  45,  20,  20,  25,   TYPE_NORMAL,   TYPE_NORMAL, 170,  76 }, /* JIGGLYPUFF   (0x64) */
		{ "Wigglytuff", 140,  70,  45,  45,  50,   TYPE_NORMAL,   TYPE_NORMAL,  50, 109 }, /* WIGGLYTUFF   (0x65) */
		{      "Eevee",  55,  55,  50,  55,  65,   TYPE_NORMAL,   TYPE_NORMAL,  45,  92 }, /* EEVEE        (0x66) */
		{    "Flareon",  65, 130,  60,  65, 110,     TYPE_FIRE,     TYPE_FIRE,  45, 198 }, /* FLAREON      (0x67) */
		{    "Jolteon",  65,  65,  60, 130, 110, TYPE_ELECTRIC, TYPE_ELECTRIC,  45, 197 }, /* JOLTEON      (0x68) */
		{   "Vaporeon", 130,  65,  60,  65, 110,    TYPE_WATER,    TYPE_WATER,  45, 196 }, /* VAPOREON     (0x69) */
		{     "Machop",  70,  80,  50,  35,  35, TYPE_FIGHTING, TYPE_FIGHTING, 180,  88 }, /* MACHOP       (0x6A) */
		{      "Zubat",  40,  45,  35,  55,  40,   TYPE_POISON,   TYPE_FLYING, 255,  54 }, /* ZUBAT        (0x6B) */
		{      "Ekans",  35,  60,  44,  55,  40,   TYPE_POISON,   TYPE_POISON, 255,  62 }, /* EKANS        (0x6C) */
		{      "Paras",  35,  70,  55,  25,  55,      TYPE_BUG,    TYPE_GRASS, 190,  70 }, /* PARAS        (0x6D) */
		{  "Poliwhirl",  65,  65,  65,  90,  50,    TYPE_WATER,    TYPE_WATER, 120, 131 }, /* POLIWHIRL    (0x6E) */
		{  "Poliwrath",  90,  85,  95,  70,  70,    TYPE_WATER, TYPE_FIGHTING,  45, 185 }, /* POLIWRATH    (0x6F) */
		{     "Weedle",  40,  35,  30,  50,  20,      TYPE_BUG,   TYPE_POISON, 255,  52 }, /* WEEDLE       (0x70) */
		{     "Kakuna",  45,  25,  50,  35,  25,      TYPE_BUG,   TYPE_POISON, 120,  71 }, /* KAKUNA       (0x71) */
		{   "Beedrill",  65,  80,  40,  75,  45,      TYPE_BUG,   TYPE_POISON,  45, 159 }, /* BEEDRILL     (0x72) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x73) */
		{     "Dodrio",  60, 110,  70, 100,  60,   TYPE_NORMAL,   TYPE_FLYING,  45, 158 }, /* DODRIO       (0x74) */
		{   "Primeape",  65, 105,  60,  95,  60, TYPE_FIGHTING, TYPE_FIGHTING,  75, 149 }, /* PRIMEAPE     (0x75) */
		{    "Dugtrio",  35,  80,  50, 120,  70,   TYPE_GROUND,   TYPE_GROUND,  50, 153 }, /* DUGTRIO      (0x76) */
		{   "Venomoth",  70,  65,  60,  90,  90,      TYPE_BUG,   TYPE_POISON,  75, 138 }, /* VENOMOTH     (0x77) */
		{    "Dewgong",  90,  70,  80,  70,  95,    TYPE_WATER,      TYPE_ICE,  75, 176 }, /* DEWGONG      (0x78) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x79) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x7A) */
		{   "Caterpie",  45,  30,  35,  45,  20,      TYPE_BUG,      TYPE_BUG, 255,  53 }, /* CATERPIE     (0x7B) */
		{    "Metapod",  50,  20,  55,  30,  25,      TYPE_BUG,      TYPE_BUG, 120,  72 }, /* METAPOD      (0x7C) */
		{ "Butterfree",  60,  45,  50,  70,  80,      TYPE_BUG,   TYPE_FLYING,  45, 160 }, /* BUTTERFREE   (0x7D) */
		{    "Machamp",  90, 130,  80,  55,  65, TYPE_FIGHTING, TYPE_FIGHTING,  45, 193 }, /* MACHAMP      (0x7E) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x7F) */
		{    "Golduck",  80,  82,  78,  85,  80,    TYPE_WATER,    TYPE_WATER,  75, 174 }, /* GOLDUCK      (0x80) */
		{      "Hypno",  85,  73,  70,  67, 115,  TYPE_PSYCHIC,  TYPE_PSYCHIC,  75, 165 }, /* HYPNO        (0x81) */
		{     "Golbat",  75,  80,  70,  90,  75,   TYPE_POISON,   TYPE_FLYING,  90, 171 }, /* GOLBAT       (0x82) */
		{     "Mewtwo", 106, 110,  90, 130, 154,  TYPE_PSYCHIC,  TYPE_PSYCHIC,   3, 220 }, /* MEWTWO       (0x83) */
		{    "Snorlax", 160, 110,  65,  30,  65,   TYPE_NORMAL,   TYPE_NORMAL,  25, 154 }, /* SNORLAX      (0x84) */
		{   "Magikarp",  20,  10,  55,  80,  20,    TYPE_WATER,    TYPE_WATER, 255,  20 }, /* MAGIKARP     (0x85) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x86) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x87) */
		{        "Muk", 105, 105,  75,  50,  65,   TYPE_POISON,   TYPE_POISON,  75, 157 }, /* MUK          (0x88) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x89) */
		{    "Kingler",  55, 130, 115,  75,  50,    TYPE_WATER,    TYPE_WATER,  60, 206 }, /* KINGLER      (0x8A) */
		{   "Cloyster",  50,  95, 180,  70,  85,    TYPE_WATER,      TYPE_ICE,  60, 203 }, /* CLOYSTER     (0x8B) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x8C) */
		{  "Electrode",  60,  50,  70, 140,  80, TYPE_ELECTRIC, TYPE_ELECTRIC,  60, 150 }, /* ELECTRODE    (0x8D) */
		{   "Clefable",  95,  70,  73,  60,  85,   TYPE_NORMAL,   TYPE_NORMAL,  25, 129 }, /* CLEFABLE     (0x8E) */
		{    "Weezing",  65,  90, 120,  60,  85,   TYPE_POISON,   TYPE_POISON,  60, 173 }, /* WEEZING      (0x8F) */
		{    "Persian",  65,  70,  60, 115,  65,   TYPE_NORMAL,   TYPE_NORMAL,  90, 148 }, /* PERSIAN      (0x90) */
		{    "Marowak",  60,  80, 110,  45,  50,   TYPE_GROUND,   TYPE_GROUND,  75, 124 }, /* MAROWAK      (0x91) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x92) */
		{    "Haunter",  45,  50,  45,  95, 115,    TYPE_GHOST,   TYPE_POISON,  90, 126 }, /* HAUNTER      (0x93) */
		{       "Abra",  25,  20,  15,  90, 105,  TYPE_PSYCHIC,  TYPE_PSYCHIC, 200,  73 }, /* ABRA         (0x94) */
		{   "Alakazam",  55,  50,  45, 120, 135,  TYPE_PSYCHIC,  TYPE_PSYCHIC,  50, 186 }, /* ALAKAZAM     (0x95) */
		{  "Pidgeotto",  63,  60,  55,  71,  50,   TYPE_NORMAL,   TYPE_FLYING, 120, 113 }, /* PIDGEOTTO    (0x96) */
		{    "Pidgeot",  83,  80,  75,  91,  70,   TYPE_NORMAL,   TYPE_FLYING,  45, 172 }, /* PIDGEOT      (0x97) */
		{    "Starmie",  60,  75,  85, 115, 100,    TYPE_WATER,  TYPE_PSYCHIC,  60, 207 }, /* STARMIE      (0x98) */
		{  "Bulbasaur",  45,  49,  49,  45,  65,    TYPE_GRASS,   TYPE_POISON,  45,  64 }, /* BULBASAUR    (0x99) */
		{   "Venusaur",  80,  82,  83,  80, 100,    TYPE_GRASS,   TYPE_POISON,  45, 208 }, /* VENUSAUR     (0x9A) */
		{ "Tentacruel",  80,  70,  65, 100, 120,    TYPE_WATER,   TYPE_POISON,  60, 205 }, /* TENTACRUEL   (0x9B) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x9C) */
		{    "Goldeen",  45,  67,  60,  63,  50,    TYPE_WATER,    TYPE_WATER, 225, 111 }, /* GOLDEEN      (0x9D) */
		{    "Seaking",  80,  92,  65,  68,  80,    TYPE_WATER,    TYPE_WATER,  60, 170 }, /* SEAKING      (0x9E) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0x9F) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xA0) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xA1) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xA2) */
		{     "Ponyta",  50,  85,  55,  90,  65,     TYPE_FIRE,     TYPE_FIRE, 190, 152 }, /* PONYTA       (0xA3) */
		{   "Rapidash",  65, 100,  70, 105,  80,     TYPE_FIRE,     TYPE_FIRE,  60, 192 }, /* RAPIDASH     (0xA4) */
		{    "Rattata",  30,  56,  35,  72,  25,   TYPE_NORMAL,   TYPE_NORMAL, 255,  57 }, /* RATTATA      (0xA5) */
		{   "Raticate",  55,  81,  60,  97,  50,   TYPE_NORMAL,   TYPE_NORMAL,  90, 116 }, /* RATICATE     (0xA6) */
		{   "Nidorino",  61,  72,  57,  65,  55,   TYPE_POISON,   TYPE_POISON, 120, 118 }, /* NIDORINO     (0xA7) */
		{   "Nidorina",  70,  62,  67,  56,  55,   TYPE_POISON,   TYPE_POISON, 120, 117 }, /* NIDORINA     (0xA8) */
		{    "Geodude",  40,  80, 100,  20,  30,     TYPE_ROCK,   TYPE_GROUND, 255,  86 }, /* GEODUDE      (0xA9) */
		{    "Porygon",  65,  60,  70,  40,  75,   TYPE_NORMAL,   TYPE_NORMAL,  45, 130 }, /* PORYGON      (0xAA) */
		{ "Aerodactyl",  80, 105,  65, 130,  60,     TYPE_ROCK,   TYPE_FLYING,  45, 202 }, /* AERODACTYL   (0xAB) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xAC) */
		{  "Magnemite",  25,  35,  70,  45,  95, TYPE_ELECTRIC, TYPE_ELECTRIC, 190,  89 }, /* MAGNEMITE    (0xAD) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xAE) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xAF) */
		{ "Charmander",  39,  52,  43,  65,  50,     TYPE_FIRE,     TYPE_FIRE,  45,  65 }, /* CHARMANDER   (0xB0) */
		{   "Squirtle",  44,  48,  65,  43,  50,    TYPE_WATER,    TYPE_WATER,  45,  66 }, /* SQUIRTLE     (0xB1) */
		{ "Charmeleon",  58,  64,  58,  80,  65,     TYPE_FIRE,     TYPE_FIRE,  45, 142 }, /* CHARMELEON   (0xB2) */
		{  "Wartortle",  59,  63,  80,  58,  65,    TYPE_WATER,    TYPE_WATER,  45, 143 }, /* WARTORTLE    (0xB3) */
		{  "Charizard",  78,  84,  78, 100,  85,     TYPE_FIRE,   TYPE_FLYING,  45, 209 }, /* CHARIZARD    (0xB4) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xB5) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xB6) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xB7) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xB8) */
		{     "Oddish",  45,  50,  55,  30,  75,    TYPE_GRASS,   TYPE_POISON, 255,  78 }, /* ODDISH       (0xB9) */
		{      "Gloom",  60,  65,  70,  40,  85,    TYPE_GRASS,   TYPE_POISON, 120, 132 }, /* GLOOM        (0xBA) */
		{  "Vileplume",  75,  80,  85,  50, 100,    TYPE_GRASS,   TYPE_POISON,  45, 184 }, /* VILEPLUME    (0xBB) */
		{ "Bellsprout",  50,  75,  35,  40,  70,    TYPE_GRASS,   TYPE_POISON, 255,  84 }, /* BELLSPROUT   (0xBC) */
		{ "Weepinbell",  65,  90,  50,  55,  85,    TYPE_GRASS,   TYPE_POISON, 120, 151 }, /* WEEPINBELL   (0xBD) */
		{ "Victreebel",  80, 105,  65,  70, 100,    TYPE_GRASS,   TYPE_POISON,  45, 191 }, /* VICTREEBEL   (0xBE) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xBF) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC0) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC1) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC2) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC3) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC4) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC5) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC6) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC7) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC8) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xC9) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xCA) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xCB) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xCC) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xCD) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xCE) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xCF) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD0) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD1) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD2) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD3) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD4) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD5) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD6) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD7) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD8) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xD9) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xDA) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xDB) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xDC) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xDD) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xDE) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xDF) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE0) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE1) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE2) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE3) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE4) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE5) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE6) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE7) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE8) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xE9) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xEA) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xEB) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xEC) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xED) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xEE) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xEF) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF0) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF1) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF2) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF3) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF4) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF5) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF6) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF7) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF8) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xF9) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xFA) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xFB) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xFC) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xFD) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xFE) */
		{  "Missingno",   0,   0,   0,   0,   0,   TYPE_NORMAL,   TYPE_NORMAL,   0,   0 }, /* MISSINGNO    (0xFF) */
	};
}