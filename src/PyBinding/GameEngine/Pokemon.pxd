# cython: language_level=3

from libcpp cimport bool
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.string cimport string
from libcpp.functional cimport function
from libcpp.set cimport set

from Damage cimport DamageResult
from Type cimport Type
from Move cimport Move, AvailableMove, MoveCategory
from StatsChange cimport StatsChange
from StatusChange cimport StatusChange
from RandomGenerator cimport RandomGenerator


cdef extern from "<array>" namespace "std" nogil:
	cdef cppclass DataArray "std::array<unsigned char, PokemonGen1::Pokemon::ENCODED_SIZE>":
		DataArray() except+
		unsigned char &operator[](size_t)


cdef extern from "../../Emulator/EmulatorGameHandle.hpp" namespace "PokemonGen1":
	cdef cppclass EmulatorGameHandle:
		void setReady(bool ready)
		#Gen1ConnectionStage getStage()
		#void setStage(Gen1ConnectionStage stage)
		bool isConnected()
		bool isReady()
		#std::pair<unsigned, unsigned> getBattleSendingProgress()

		@staticmethod
		vector[unsigned char] convertString(const string &str)
		@staticmethod
		string convertString(const vector[unsigned char] &str)


cdef extern from "../../GameEngine/Pokemon.hpp" namespace "PokemonGen1":
	cdef cppclass Pokemon:
		ctypedef function[void (const string &)] Logger

		struct BaseStats:
			unsigned       HP
			unsigned       maxHP
			unsigned short ATK
			unsigned short DeF "DEF"
			unsigned short SPD
			unsigned short SPE
			BaseStats()

		struct Base:
			unsigned char      id
			unsigned char      dexId
			string             name
			unsigned           HP
			unsigned short     ATK
			unsigned short     DeF "DEF"
			unsigned short     SPD
			unsigned short     SPE
			Type               typeA
			Type               typeB
			unsigned char      catchRate
			unsigned int       baseXpYield
			set[AvailableMove] movePool
			Pokemon.BaseStats  statsAtLevel[256]
			Base(const Base &)

		struct UpgradableStats:
			char ATK
			char DeF "DEF"
			char SPD
			char SPE
			char EVD
			char ACC
			UpgradableStats()
			unsigned char get(StatsChange stat)

		Pokemon(const Pokemon &)
		Pokemon(RandomGenerator &random, const Pokemon.Logger &battleLogger, const string &nickname, unsigned char level, const Pokemon.Base &base, const vector[Move] &moveSet, bool enemy) except +
		Pokemon(RandomGenerator &random, const Logger &battleLogger, const string &nickname, const DataArray &data, bool enemy)
		#Pokemon(RandomGenerator &random, const Logger &battleLogger, const nlohmann::json &json)

		void setGlobalCritRatio(double ratio)
		void setStatus(StatusChange status)
		void setNonVolatileStatus(StatusChange status)
		bool addStatus(StatusChange status)
		bool addStatus(StatusChange status, unsigned duration)
		void resetStatsChanges()
		bool changeStat(StatsChange stat, char nb)
		void useMove(const Move &move, Pokemon &target)
		void storeDamages(bool active)
		bool hasStatus(StatusChange status) const
		void heal(int health)
		void takeDamage(int damage, bool ignoreSubstitute)
		void attack(unsigned char moveSlot, Pokemon &target)
		Pokemon.DamageResult calcDamage(Pokemon &target, unsigned power, Type damageType, MoveCategory category, bool critical, bool randomized, bool halfDefense)
		void endTurn()
		void switched()
		int getPriorityFactor(unsigned char moveSlot)
		void setWrapped(bool isWrapped)
		void setRecharging(bool recharging)
		void transform(const Pokemon &target)
		const set[AvailableMove] &getLearnableMoveSet() const
		DataArray encode() const
		string dump() const
		#nlohmann::json serialize()

		void setInvincible(bool invincible)
		void setId(unsigned char id, bool recomputeStats)
		void setNickname(const string &nickname)
		void setLevel(unsigned char level)
		void setMove(unsigned char index, const Move &move)
		void reset()
		void applyStatusDebuff()

		RandomGenerator &getRandomGenerator()
		bool canHaveStatus(StatusChange status)
		unsigned short getStatus()
		unsigned short getNonVolatileStatus()
		bool canGetHit()
		unsigned char getID()
		unsigned getDamagesStored()
		unsigned getSpeed()
		unsigned char getLevel()
		unsigned getAttack()
		unsigned getSpecial()
		unsigned getDefense()
		unsigned getRawAttack()
		unsigned getRawSpecial()
		unsigned getRawDefense()
		unsigned getRawSpeed()
		unsigned getHealth()
		unsigned short getSubstituteHealth()
		bool hasSubstitute()
		string getName(bool hasEnemy)
		string getNickname()
		unsigned getMaxHealth()
		unsigned int getAccuracy(unsigned int accuracy)
		unsigned int getEvasion(unsigned int accuracy)
		double getAccuracyMul()
		double getEvasionMul()
		const Move &getLastUsedMove()
		pair[Type, Type] getTypes()
		const vector[Move] &getMoveSet()
		Pokemon.BaseStats getBaseStats()
		Pokemon.UpgradableStats getStatsUpgradeStages()
		string getSpeciesName()
		const Pokemon.BaseStats &getDvs()
		const Pokemon.BaseStats &getStatExps()
		bool isEnemy()
		void setStatExps(const Pokemon.BaseStats &statExps)

		void setTypes(pair[Type, Type] types)

		@staticmethod
		Pokemon.BaseStats makeStats(unsigned char level, const Pokemon.Base &base, const Pokemon.BaseStats &dvs, const Pokemon.BaseStats &evs)

	ctypedef enum PokemonSpecies:
		Rhydon,
		Kangaskhan,
		Nidoran_M,
		Clefairy,
		Spearow,
		Voltorb,
		Nidoking,
		Slowbro,
		Ivysaur,
		Exeggutor,
		Lickitung,
		Exeggcute,
		Grimer,
		Gengar,
		Nidoran_F,
		Nidoqueen,
		Cubone,
		Rhyhorn,
		Lapras,
		Arcanine,
		Mew,
		Gyarados,
		Shellder,
		Tentacool,
		Gastly,
		Scyther,
		Staryu,
		Blastoise,
		Pinsir,
		Tangela,
		Growlithe,
		Onix,
		Fearow,
		Pidgey,
		Slowpoke,
		Kadabra,
		Graveler,
		Chansey,
		Machoke,
		Mr_Mime,
		Hitmonlee,
		Hitmonchan,
		Arbok,
		Parasect,
		Psyduck,
		Drowzee,
		Golem,
		Magmar,
		Electabuzz,
		Magneton,
		Koffing,
		Mankey,
		Seel,
		Diglett,
		Tauros,
		Farfetchd,
		Venonat,
		Dragonite,
		Doduo,
		Poliwag,
		Jynx,
		Moltres,
		Articuno,
		Zapdos,
		Ditto,
		Meowth,
		Krabby,
		Vulpix,
		Ninetales,
		Pikachu,
		Raichu,
		Dratini,
		Dragonair,
		Kabuto,
		Kabutops,
		Horsea,
		Seadra,
		Sandshrew,
		Sandslash,
		Omanyte,
		Omastar,
		Jigglypuff,
		Wigglytuff,
		Eevee,
		Flareon,
		Jolteon,
		Vaporeon,
		Machop,
		Zubat,
		Ekans,
		Paras,
		Poliwhirl,
		Poliwrath,
		Weedle,
		Kakuna,
		Beedrill,
		Dodrio,
		Primeape,
		Dugtrio,
		Venomoth,
		Dewgong,
		Caterpie,
		Metapod,
		Butterfree,
		Machamp,
		Golduck,
		Hypno,
		Golbat,
		Mewtwo,
		Snorlax,
		Magikarp,
		Muk,
		Kingler,
		Cloyster,
		Electrode,
		Clefable,
		Weezing,
		Persian,
		Marowak,
		Haunter,
		Abra,
		Alakazam,
		Pidgeotto,
		Pidgeot,
		Starmie,
		Bulbasaur,
		Venusaur,
		Tentacruel,
		Goldeen,
		Seaking,
		Ponyta,
		Rapidash,
		Rattata,
		Raticate,
		Nidorino,
		Nidorina,
		Geodude,
		Porygon,
		Aerodactyl,
		Magnemite,
		Charmander,
		Squirtle,
		Charmeleon,
		Wartortle,
		Charizard,
		Oddish,
		Gloom,
		Vileplume,
		Bellsprout,
		Weepinbell,
		Victreebel,
		Missingno

	cdef Pokemon.Logger pythonLoggerLambda(void *python_function, void (*eval)(void *, const string &))

	const map[unsigned char, Pokemon.Base] pokemonList

cdef inline void evalLogger(void *func_p, const string &msg) noexcept:
	f: bytes = msg
	(<object> func_p)(f.decode('ASCII'))