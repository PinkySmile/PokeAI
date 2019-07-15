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

namespace Pokemon
{
	struct PokemonBase {
		std::string	name;
		unsigned	HP;
		unsigned short	ATK;
		unsigned short	DEF;
		unsigned short	SPD;
		unsigned short	SPE;
		PokemonTypes	typeA;
		PokemonTypes	typeB;
		unsigned char	catchRate;
		unsigned int	baseXpYield;
	};

	struct BaseStats
	{
		unsigned	HP;
		unsigned	maxHP;
		unsigned short	ATK;
		unsigned short	DEF;
		unsigned short	SPD;
		unsigned short	SPE;
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

	class Pokemon {
	private:
		Move					_lastUsedMove;
		PokemonRandomGenerator			&_random;
		std::string				_nickname;
		std::string				_name;
		BaseStats				_baseStats;
		UpgradableStats				_upgradedStats;
		std::vector<Move>			_moveSet;
		std::pair<PokemonTypes, PokemonTypes>	_types;
		unsigned char				_level;
		unsigned char				_catchRate;
		bool					_storingDamages;
		unsigned int				_damagesStored;
		unsigned char				_statusDuration[2];
		unsigned char				_currentStatus;
		double					_globalCritRatio;

	public:
		Pokemon(PokemonRandomGenerator &random, const std::string &nickname, unsigned char level, const PokemonBase &base, const std::vector<Move> &moveSet);
		Pokemon(PokemonRandomGenerator &random, const std::string &nickname, const std::vector<byte> &data);

		void setGlobalCritRatio(double ratio);
		void addStatus(StatusChange status);
		void addStatus(StatusChange status, unsigned duration);
		void resetStatsChanges();
		void useMove(const Move &move, Pokemon &target);
		void storeDamages(bool active);
		bool hasStatus(StatusChange status) const;
		void takeDamage(int damage);
		unsigned dealDamage(Pokemon &target, unsigned power, PokemonTypes damageType) const;

		PokemonRandomGenerator &getRandomGenerator();
		unsigned getDamagesStored() const;
		unsigned getSpeed() const;
		unsigned getLevel() const;
		unsigned getHealth() const;
		unsigned getMaxHealth() const;
		const Move &getLastUsedMove() const;
		std::pair<PokemonTypes, PokemonTypes> getTypes() const;

		void setTypes(std::pair<PokemonTypes, PokemonTypes> types);
	};
}


#endif //POKEAI_POKEMON_HPP
