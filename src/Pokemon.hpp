//
// Created by Gegel85 on 14/07/2019.
//

#ifndef POKEAI_POKEMON_HPP
#define POKEAI_POKEMON_HPP


#include <string>
#include <vector>
#include <optional>
#include "Move.hpp"
#include "PokemonTypes.hpp"
#include "StatusChange.hpp"
#include "PokemonRandomGenerator.hpp"

typedef unsigned char byte;

namespace PokemonGen1
{
	class GameHandle;

	struct PokemonBase {
		unsigned char  id;
		std::string    name;
		unsigned       HP;
		unsigned short ATK;
		unsigned short DEF;
		unsigned short SPD;
		unsigned short SPE;
		PokemonTypes   typeA;
		PokemonTypes   typeB;
		unsigned char  catchRate;
		unsigned int   baseXpYield;
	};

	struct BaseStats
	{
		unsigned       HP;
		unsigned       maxHP;
		unsigned short ATK;
		unsigned short DEF;
		unsigned short SPD;
		unsigned short SPE;
	};

	struct UpgradableStats {
		char	ATK;
		char	DEF;
		char	SPD;
		char	SPE;
		char	ESQ;
		char	PRE;
	};

	extern const std::vector<PokemonBase> pokemonList;
	extern const std::vector<std::vector<unsigned>> expTable;

	class Pokemon {
	private:
		struct PokemonState {
			BaseStats                             stats;
			unsigned char                         id;
			std::vector<Move>                     moves;
			std::pair<PokemonTypes, PokemonTypes> types;
		};

		PokemonState                          _oldState{};
		unsigned char                         _id;
		bool                                  _flinched = false;
		bool                                  _needsRecharge = false;
		bool                                  _invincible = false;
		bool                                  _chargingUp = false;
		bool                                  _thrashing = false;
		bool                                  _enraged = false;
		bool                                  _enemy;
		Move                                  _lastUsedMove;
		PokemonRandomGenerator	              &_random;
		std::string                           _nickname;
		std::string                           _name;
		BaseStats                             _baseStats;
		UpgradableStats                       _upgradedStats;
		std::vector<Move>                     _moveSet;
		std::pair<PokemonTypes, PokemonTypes> _types;
		unsigned char                         _level;
		unsigned char                         _catchRate;
		bool                                  _transformed = false;
		bool                                  _wrapped = false;
		bool                                  _storingDamages;
		unsigned int                          _damagesStored;
		unsigned char                         _badPoisonStage = 0;
		unsigned short                        _currentStatus;
		double                                _globalCritRatio;
		GameHandle                            &_game;

		void _log(const std::string &msg) const;
		double _getUpgradedStat(unsigned short baseValue, char upgradeStage) const;

	public:
		struct DamageResult {
			bool critical;
			unsigned damages;
			bool affect;
			bool isVeryEffective;
			bool isNotVeryEffective;
		};

		Pokemon(PokemonRandomGenerator &random, GameHandle &game, const std::string &nickname, unsigned char level, const PokemonBase &base, const std::vector<Move> &moveSet, bool enemy = false);
		Pokemon(PokemonRandomGenerator &random, GameHandle &game, const std::string &nickname, const std::vector<byte> &data, bool enemy = false);

		void setGlobalCritRatio(double ratio);
		bool addStatus(StatusChange status);
		void setStatus(StatusChange status);
		void setStatus(StatusChange status, unsigned duration);
		bool addStatus(StatusChange status, unsigned duration);
		void resetStatsChanges();
		void changeStat(StatsChange stat, char nb);
		void useMove(const Move &move, Pokemon &target);
		void storeDamages(bool active);
		bool hasStatus(StatusChange status) const;
		void takeDamage(int damage);
		void attack(unsigned char moveSlot, Pokemon &target);
		bool canGetHitBy(unsigned char moveId);
		DamageResult calcDamage(Pokemon &target, unsigned power, PokemonTypes damageType, MoveCategory category, double critRate) const;
		void endTurn();
		void switched();
		int getPriorityFactor(unsigned char moveSlot);
		void setWrapped(bool isWrapped);
		void glitchHyperBeam();
		void transform(const Pokemon &target);
		std::vector<unsigned char> encode() const;
		std::string dump() const;

		PokemonRandomGenerator &getRandomGenerator();
		unsigned char getID() const;
		unsigned getDamagesStored() const;
		unsigned getSpeed() const;
		unsigned getLevel() const;
		unsigned getAttack() const;
		unsigned getSpecial() const;
		unsigned getDefense() const;
		unsigned getRawAttack() const;
		unsigned getRawSpecial() const;
		unsigned getRawDefense() const;
		unsigned getHealth() const;
		std::string getName() const;
		std::string getNickname() const;
		unsigned getMaxHealth() const;
		double getAccuracy() const;
		double getEvasion() const;
		const Move &getLastUsedMove() const;
		std::pair<PokemonTypes, PokemonTypes> getTypes() const;
		const std::vector<Move> getMoveSet() const;
		BaseStats getBaseStats() const;
		UpgradableStats getStatsUpgradeStages() const;
		std::string getSpeciesName() const;

		void setTypes(std::pair<PokemonTypes, PokemonTypes> types);

		static BaseStats makeStats(unsigned char level, const PokemonBase &base);
	};

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
		Exeggutor = 0x0a,
		Lickitung = 0x0b,
		Exeggcute = 0x0c,
		Grimer = 0x0d,
		Gengar = 0x0e,
		Nidoran_F = 0x0f,
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
		Scyther = 0x1a,
		Staryu = 0x1b,
		Blastoise = 0x1c,
		Pinsir = 0x1d,
		Tangela = 0x1e,
		Growlithe = 0x21,
		Onix = 0x22,
		Fearow = 0x23,
		Pidgey = 0x24,
		Slowpoke = 0x25,
		Kadabra = 0x26,
		Graveler = 0x27,
		Chansey = 0x28,
		Machoke = 0x29,
		Mr_Mime = 0x2a,
		Hitmonlee = 0x2b,
		Hitmonchan = 0x2c,
		Arbok = 0x2d,
		Parasect = 0x2e,
		Psyduck = 0x2f,
		Drowzee = 0x30,
		Golem = 0x31,
		Magmar = 0x33,
		Electabuzz = 0x35,
		Magneton = 0x36,
		Koffing = 0x37,
		Mankey = 0x39,
		Seel = 0x3a,
		Diglett = 0x3b,
		Tauros = 0x3c,
		Farfetchd = 0x40,
		Venonat = 0x41,
		Dragonite = 0x42,
		Doduo = 0x46,
		Poliwag = 0x47,
		Jynx = 0x48,
		Moltres = 0x49,
		Articuno = 0x4a,
		Zapdos = 0x4b,
		Ditto = 0x4c,
		Meowth = 0x4d,
		Krabby = 0x4e,
		Vulpix = 0x52,
		Ninetales = 0x53,
		Pikachu = 0x54,
		Raichu = 0x55,
		Dratini = 0x58,
		Dragonair = 0x59,
		Kabuto = 0x5a,
		Kabutops = 0x5b,
		Horsea = 0x5c,
		Seadra = 0x5d,
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
		Machop = 0x6a,
		Zubat = 0x6b,
		Ekans = 0x6c,
		Paras = 0x6d,
		Poliwhirl = 0x6e,
		Poliwrath = 0x6f,
		Weedle = 0x70,
		Kakuna = 0x71,
		Beedrill = 0x72,
		Dodrio = 0x74,
		Primeape = 0x75,
		Dugtrio = 0x76,
		Venomoth = 0x77,
		Dewgong = 0x78,
		Caterpie = 0x7b,
		Metapod = 0x7c,
		Butterfree = 0x7d,
		Machamp = 0x7e,
		Golduck = 0x80,
		Hypno = 0x81,
		Golbat = 0x82,
		Mewtwo = 0x83,
		Snorlax = 0x84,
		Magikarp = 0x85,
		Muk = 0x88,
		Kingler = 0x8a,
		Cloyster = 0x8b,
		Electrode = 0x8d,
		Clefable = 0x8e,
		Weezing = 0x8f,
		Persian = 0x90,
		Marowak = 0x91,
		Haunter = 0x93,
		Abra = 0x94,
		Alakazam = 0x95,
		Pidgeotto = 0x96,
		Pidgeot = 0x97,
		Starmie = 0x98,
		Bulbasaur = 0x99,
		Venusaur = 0x9a,
		Tentacruel = 0x9b,
		Goldeen = 0x9d,
		Seaking = 0x9e,
		Ponyta = 0xa3,
		Rapidash = 0xa4,
		Rattata = 0xa5,
		Raticate = 0xa6,
		Nidorino = 0xa7,
		Nidorina = 0xa8,
		Geodude = 0xa9,
		Porygon = 0xaa,
		Aerodactyl = 0xab,
		Magnemite = 0xad,
		Charmander = 0xb0,
		Squirtle = 0xb1,
		Charmeleon = 0xb2,
		Wartortle = 0xb3,
		Charizard = 0xb4,
		Oddish = 0xb9,
		Gloom = 0xba,
		Vileplume = 0xbb,
		Bellsprout = 0xbc,
		Weepinbell = 0xbd,
		Victreebel = 0xbe,
		Missingno = 0xff
	};
}


#endif //POKEAI_POKEMON_HPP
