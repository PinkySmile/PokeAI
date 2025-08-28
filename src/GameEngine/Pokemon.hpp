//
// Created by PinkySmile on 14/07/2019.
//

#ifndef POKEAI_POKEMON_HPP
#define POKEAI_POKEMON_HPP


#include <string>
#include <vector>
#include <set>
#include <optional>
#include <map>
#include <nlohmann/json_fwd.hpp>
#include "Move.hpp"
#include "Type.hpp"
#include "StatusChange.hpp"
#include "RandomGenerator.hpp"
#include "Damage.hpp"

typedef unsigned char byte;

namespace PokemonGen1
{
	class Pokemon {
	public:
		struct BaseStats {
			unsigned       HP;
			unsigned       maxHP;
			unsigned short ATK;
			unsigned short DEF;
			unsigned short SPD;
			unsigned short SPE;
		};

		struct Base {
			unsigned char           id;
			unsigned char           dexId;
			std::string             name;
			unsigned                HP;
			unsigned short          ATK;
			unsigned short          DEF;
			unsigned short          SPD;
			unsigned short          SPE;
			Type                    typeA;
			Type                    typeB;
			unsigned char           catchRate;
			unsigned int            baseXpYield;
			std::set<AvailableMove> movePool;
			BaseStats               statsAtLevel[256];

			Base(
				unsigned char  id,
				unsigned char  dexId,
				std::string    name,
				unsigned       HP,
				unsigned short ATK,
				unsigned short DEF,
				unsigned short SPD,
				unsigned short SPE,
				Type           typeA,
				Type           typeB,
				unsigned char  catchRate,
				unsigned int   baseXpYield,
				const std::set<AvailableMove> &movePool
			);
		};

		struct UpgradableStats {
			char ATK;
			char DEF;
			char SPD;
			char SPE;
			char EVD;
			char ACC;

			char get(StatsChange stat) const;
		};

		typedef std::function<void (const std::string &)> Logger;

		enum PacketElems {
			PACK_SPECIES,
			PACK_HP_HB,
			PACK_HP_LB,
			PACK_BOX_LEVEL,
			PACK_STATUS,
			PACK_TYPEA,
			PACK_TYPEB,
			PACK_CATCH_RATE,
			PACK_MOVE1,
			PACK_MOVE2,
			PACK_MOVE3,
			PACK_MOVE4,
			PACK_TID_HB,
			PACK_TID_LB,
			PACK_EXP_HB,
			PACK_EXP_MB,
			PACK_EXP_LB,
			PACK_STAT_EXP_HP_HB,
			PACK_STAT_EXP_HP_LB,
			PACK_STAT_EXP_ATK_HB,
			PACK_STAT_EXP_ATK_LB,
			PACK_STAT_EXP_DEF_HB,
			PACK_STAT_EXP_DEF_LB,
			PACK_STAT_EXP_SPD_HB,
			PACK_STAT_EXP_SPD_LB,
			PACK_STAT_EXP_SPE_HB,
			PACK_STAT_EXP_SPE_LB,
			PACK_DVS_SPD_SPE,
			PACK_DVS_ATK_DEF,
			PACK_PPS_MOVE1,
			PACK_PPS_MOVE2,
			PACK_PPS_MOVE3,
			PACK_PPS_MOVE4,
			PACK_CURR_LEVEL,
			PACK_MAX_HP_HB,
			PACK_MAX_HP_LB,
			PACK_ATK_HB,
			PACK_ATK_LB,
			PACK_DEF_HB,
			PACK_DEF_LB,
			PACK_SPD_HB,
			PACK_SPD_LB,
			PACK_SPE_HB,
			PACK_SPE_LB,
			PACK_SIZE
		};

	private:
		struct PokemonState {
			BaseStats                             stats;
			unsigned char                         id;
			std::vector<Move>                     moves;
			std::pair<Type, Type> types;
		};

		PokemonState                          _oldState{};
		unsigned char                         _id;
		bool                                  _flinched = false;
		bool                                  _needsRecharge = 0;
		bool                                  _invincible = false;
		bool                                  _enemy;
		Move                                  _lastUsedMove;
		RandomGenerator	              *_random;
		std::string                           _nickname;
		std::string                           _name;
		BaseStats                             _dvs;
		BaseStats                             _statExps;
		BaseStats                             _baseStats;
		BaseStats                             _computedStats;
		UpgradableStats                       _upgradedStats;
		std::vector<Move>                     _moveSet;
		std::pair<Type, Type> _types;
		unsigned char                         _level;
		unsigned char                         _catchRate;
		bool                                  _transformed = false;
		bool                                  _wrapped = false;
		bool                                  _stopWrapped = false;
		bool                                  _storingDamages;
		unsigned int                          _damageStored;
		unsigned char                         _badPoisonStage = 0;
		unsigned short                        _currentStatus;
		double                                _globalCritRatio;
		unsigned short _subHealth = 0;
		unsigned char _forcedAttack = 0;
		bool _hasSub = false;
		const Logger *_battleLogger;

		static const std::pair<unsigned char, unsigned char> _ratios[13];

		void _log(const std::string &msg) const;
		unsigned int _getUpgradedStat(unsigned short baseValue, char upgradeStage) const;

	public:

		static constexpr unsigned NICK_SIZE = 10;
		static constexpr unsigned ENCODED_SIZE = PACK_SIZE;

		Pokemon(RandomGenerator &random, const Logger &battleLogger, const std::string &nickname, unsigned char level, const Base &base, const std::vector<Move> &moveSet, bool enemy = false);
		Pokemon(RandomGenerator &random, const Logger &battleLogger, const std::string &nickname, const std::array<byte, ENCODED_SIZE> &data, bool enemy = false);
		Pokemon(RandomGenerator &random, const Logger &battleLogger, const nlohmann::json &json);

		unsigned short getSubstituteHealth() const;
		bool hasSubstitute() const;
		void setSubstitute();
		void setSubstituteHealth(unsigned short health);
		void setGlobalCritRatio(double ratio);
		void setStatus(StatusChange status);
		void setNonVolatileStatus(StatusChange status);
		bool addStatus(StatusChange status);
		bool addStatus(StatusChange status, unsigned duration);
		void resetStatsChanges();
		bool changeStat(enum StatsChange stat, char nb);
		bool useMove(const Move &move, Pokemon &target);
		void storeDamages(bool active);
		bool hasStatus(StatusChange status) const;
		void heal(unsigned short health);
		void takeDamage(unsigned short damage, bool skipSubstitute);
		void attack(unsigned char moveSlot, Pokemon &target);
		DamageResult calcDamage(Pokemon &target, unsigned power, Type damageType, MoveCategory category, bool critical, bool randomized, bool halfDefense) const;
		void endTurn();
		void switched();
		void opponentSwitched();
		int getPriorityFactor(unsigned char moveSlot);
		void setWrapped(bool isWrapped);
		void setRecharging(bool recharging = true);
		void transform(const Pokemon &target);
		const std::set<AvailableMove> &getLearnableMoveSet() const;
		[[nodiscard]] std::array<unsigned char, ENCODED_SIZE> encode() const;
		[[nodiscard]] std::string dump() const;
		[[nodiscard]] nlohmann::json serialize() const;

		void setInvincible(bool invincible);
		void setId(unsigned char id, bool recomputeStats = true);
		void setNickname(const std::string &nickname);
		void setLevel(unsigned char level);
		void setMove(unsigned char index, const Move &move);
		void reset();
		void applyStatusDebuff();

		[[nodiscard]] RandomGenerator &getRandomGenerator();
		[[nodiscard]] bool canHaveStatus(StatusChange status) const;
		[[nodiscard]] unsigned short getStatus() const;
		[[nodiscard]] unsigned short getNonVolatileStatus() const;
		[[nodiscard]] bool canGetHit() const;
		[[nodiscard]] bool isRecharging() const;
		[[nodiscard]] unsigned char getID() const;
		[[nodiscard]] unsigned getDamagesStored() const;
		[[nodiscard]] unsigned getSpeed() const;
		[[nodiscard]] unsigned char getLevel() const;
		[[nodiscard]] unsigned getAttack() const;
		[[nodiscard]] unsigned getSpecial() const;
		[[nodiscard]] unsigned getDefense() const;
		[[nodiscard]] unsigned getRawAttack() const;
		[[nodiscard]] unsigned getRawSpecial() const;
		[[nodiscard]] unsigned getRawDefense() const;
		[[nodiscard]] unsigned getRawSpeed() const;
		[[nodiscard]] unsigned getHealth() const;
		[[nodiscard]] std::string getName(bool hasEnemy = true) const;
		[[nodiscard]] std::string getNickname() const;
		[[nodiscard]] unsigned getMaxHealth() const;
		[[nodiscard]] unsigned int getAccuracy(unsigned int accuracy) const;
		[[nodiscard]] unsigned int getEvasion(unsigned int accuracy) const;
		[[nodiscard]] double getAccuracyMul() const;
		[[nodiscard]] double getEvasionMul() const;
		[[nodiscard]] const Move &getLastUsedMove() const;
		[[nodiscard]] std::pair<Type, Type> getTypes() const;
		[[nodiscard]] const std::vector<Move> &getMoveSet() const;
		[[nodiscard]] BaseStats getBaseStats() const;
		[[nodiscard]] UpgradableStats getStatsUpgradeStages() const;
		[[nodiscard]] std::string getSpeciesName() const;
		[[nodiscard]] const BaseStats &getDvs() const;
		[[nodiscard]] const BaseStats &getStatExps() const;
		[[nodiscard]] bool isEnemy() const;
		void setStatExps(const BaseStats &statExps);

		void setTypes(std::pair<Type, Type> types);

		static BaseStats makeStats(unsigned char level, const Base &base, const BaseStats &dvs, const BaseStats &evs);
	};

#ifdef __PYX_EXTERN_C
	inline Pokemon::Logger pythonLoggerLambda(void *python_function, std::function<void (void *, const std::string &msg)> eval)
	{
		return [=](const std::string &x) { return eval(python_function, x); };
	}
#endif

	/*
	** From Rhydon
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/PK1.cs#L376
	*/
	enum PokemonSpecies
	{
		Rhydon = 0x01,
		Kangaskhan = 0x02,
		Nidoran_M = 0x03,
		Clefairy = 0x04,
		Spearow = 0x05,
		Voltorb = 0x06,
		Nidoking = 0x07,
		Slowbro = 0x08,
		Ivysaur = 0x09,
		Exeggutor = 0x0A,
		Lickitung = 0x0B,
		Exeggcute = 0x0C,
		Grimer = 0x0D,
		Gengar = 0x0E,
		Nidoran_F = 0x0F,
		Nidoqueen = 0x10,
		Cubone = 0x11,
		Rhyhorn = 0x12,
		Lapras = 0x13,
		Arcanine = 0x14,
		Mew = 0x15,
		Gyarados = 0x16,
		Shellder = 0x17,
		Tentacool = 0x18,
		Gastly = 0x19,
		Scyther = 0x1A,
		Staryu = 0x1B,
		Blastoise = 0x1C,
		Pinsir = 0x1D,
		Tangela = 0x1E,
		Growlithe = 0x21,
		Onix = 0x22,
		Fearow = 0x23,
		Pidgey = 0x24,
		Slowpoke = 0x25,
		Kadabra = 0x26,
		Graveler = 0x27,
		Chansey = 0x28,
		Machoke = 0x29,
		Mr_Mime = 0x2A,
		Hitmonlee = 0x2B,
		Hitmonchan = 0x2C,
		Arbok = 0x2D,
		Parasect = 0x2E,
		Psyduck = 0x2F,
		Drowzee = 0x30,
		Golem = 0x31,
		Magmar = 0x33,
		Electabuzz = 0x35,
		Magneton = 0x36,
		Koffing = 0x37,
		Mankey = 0x39,
		Seel = 0x3A,
		Diglett = 0x3B,
		Tauros = 0x3C,
		Farfetchd = 0x40,
		Venonat = 0x41,
		Dragonite = 0x42,
		Doduo = 0x46,
		Poliwag = 0x47,
		Jynx = 0x48,
		Moltres = 0x49,
		Articuno = 0x4A,
		Zapdos = 0x4B,
		Ditto = 0x4C,
		Meowth = 0x4D,
		Krabby = 0x4E,
		Vulpix = 0x52,
		Ninetales = 0x53,
		Pikachu = 0x54,
		Raichu = 0x55,
		Dratini = 0x58,
		Dragonair = 0x59,
		Kabuto = 0x5A,
		Kabutops = 0x5B,
		Horsea = 0x5C,
		Seadra = 0x5D,
		Sandshrew = 0x60,
		Sandslash = 0x61,
		Omanyte = 0x62,
		Omastar = 0x63,
		Jigglypuff = 0x64,
		Wigglytuff = 0x65,
		Eevee = 0x66,
		Flareon = 0x67,
		Jolteon = 0x68,
		Vaporeon = 0x69,
		Machop = 0x6A,
		Zubat = 0x6B,
		Ekans = 0x6C,
		Paras = 0x6D,
		Poliwhirl = 0x6E,
		Poliwrath = 0x6F,
		Weedle = 0x70,
		Kakuna = 0x71,
		Beedrill = 0x72,
		Dodrio = 0x74,
		Primeape = 0x75,
		Dugtrio = 0x76,
		Venomoth = 0x77,
		Dewgong = 0x78,
		Caterpie = 0x7B,
		Metapod = 0x7C,
		Butterfree = 0x7D,
		Machamp = 0x7E,
		Golduck = 0x80,
		Hypno = 0x81,
		Golbat = 0x82,
		Mewtwo = 0x83,
		Snorlax = 0x84,
		Magikarp = 0x85,
		Muk = 0x88,
		Kingler = 0x8A,
		Cloyster = 0x8B,
		Electrode = 0x8D,
		Clefable = 0x8E,
		Weezing = 0x8F,
		Persian = 0x90,
		Marowak = 0x91,
		Haunter = 0x93,
		Abra = 0x94,
		Alakazam = 0x95,
		Pidgeotto = 0x96,
		Pidgeot = 0x97,
		Starmie = 0x98,
		Bulbasaur = 0x99,
		Venusaur = 0x9A,
		Tentacruel = 0x9B,
		Goldeen = 0x9D,
		Seaking = 0x9E,
		Ponyta = 0xA3,
		Rapidash = 0xA4,
		Rattata = 0xA5,
		Raticate = 0xA6,
		Nidorino = 0xA7,
		Nidorina = 0xA8,
		Geodude = 0xA9,
		Porygon = 0xAA,
		Aerodactyl = 0xAB,
		Magnemite = 0xAD,
		Charmander = 0xB0,
		Squirtle = 0xB1,
		Charmeleon = 0xB2,
		Wartortle = 0xB3,
		Charizard = 0xB4,
		Oddish = 0xB9,
		Gloom = 0xBA,
		Vileplume = 0xBB,
		Bellsprout = 0xBC,
		Weepinbell = 0xBD,
		Victreebel = 0xBE,
		Missingno = 0xFF
	};

	extern const std::map<unsigned char, Pokemon::Base> pokemonList;
}


#endif //POKEAI_POKEMON_HPP
