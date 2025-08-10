# cython: language_level=3

from libcpp cimport bool
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.string cimport string
from libcpp.functional cimport function
from libcpp.set cimport set

from Type cimport Type
from Move cimport Move, AvailableMove, MoveCategory
from StatsChange cimport StatsChange
from StatusChange cimport StatusChange
from RandomGenerator cimport RandomGenerator


cdef extern from "<array>" namespace "std" nogil:
	cdef cppclass arrayPkmn "array<PokemongGen1::Pokemon::Base, 256>":
		arrayPkmn() except+
		Move &operator[](size_t)


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
			BaseStats          statsAtLevel[256]
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

		struct DamageResult:
			bool critical
			unsigned damage
			bool affect
			bool isVeryEffective
			bool isNotVeryEffective
			DamageResult()

		Pokemon(const Pokemon &)
		Pokemon(RandomGenerator &random, const Logger &battleLogger, const string &nickname, unsigned char level, const Base &base, const vector[Move] &moveSet, bool enemy) except +
		#Pokemon(RandomGenerator &random, const Logger &battleLogger, const string &nickname, const array[byte, ENCODED_SIZE] &data, bool enemy)
		#Pokemon(RandomGenerator &random, const Logger &battleLogger, const nlohmann::json &json)

		void setGlobalCritRatio(double ratio)
		void setStatus(StatusChange status)
		void setNonVolatileStatus(StatusChange status)
		void setNonVolatileStatus(StatusChange status, unsigned duration)
		bool addStatus(StatusChange status)
		bool addStatus(StatusChange status, unsigned duration)
		void resetStatsChanges()
		bool changeStat(StatsChange stat, char nb)
		void useMove(const Move &move, Pokemon &target)
		void storeDamages(bool active)
		bool hasStatus(StatusChange status) const
		void takeDamage(int damage)
		void attack(unsigned char moveSlot, Pokemon &target)
		DamageResult calcDamage(Pokemon &target, unsigned power, Type damageType, MoveCategory category, bool critical, bool randomized)
		void endTurn()
		void switched()
		int getPriorityFactor(unsigned char moveSlot)
		void setWrapped(bool isWrapped)
		void setRecharging(bool recharging)
		void transform(const Pokemon &target)
		const set[AvailableMove] &getLearnableMoveSet() const
		#array[unsigned char, ENCODED_SIZE] encode() const
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
		string getName(bool hasEnemy)
		string getNickname()
		unsigned getMaxHealth()
		double getAccuracy()
		double getEvasion()
		const Move &getLastUsedMove()
		pair[Type, Type] getTypes()
		const vector[Move] &getMoveSet()
		BaseStats getBaseStats()
		UpgradableStats getStatsUpgradeStages()
		string getSpeciesName()
		const BaseStats &getDvs()
		const BaseStats &getStatExps()
		bool isEnemy()
		void setStatExps(const BaseStats &statExps)

		void setTypes(pair[Type, Type] types)

		@staticmethod
		BaseStats makeStats(unsigned char level, const Base &base, const BaseStats &dvs, const BaseStats &evs)

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
	f = msg
	(<object> func_p)(f.decode('ASCII'))