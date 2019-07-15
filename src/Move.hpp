//
// Created by Gegel85 on 14/07/2019.
//

#ifndef POKEAI_MOVE_HPP
#define POKEAI_MOVE_HPP


#include <string>
#include <functional>
#include "PokemonTypes.hpp"
#include "StatusChange.hpp"
#include "StatsChange.hpp"


#define DEFAULT_MOVE(id) {id, "Move "#id, TYPE_NORMAL, PHYSICAL, 0, 0, 0}
#define NO_STATS_CHANGE {}, {}
#define DEFAULT_HITS {{1, 1}}
#define TWO_TO_FIVE_HITS {{2, 0.375}, {3, 0.375}, {4, 0.125}, {5, 0.125}}
#define DEFAULT_CRIT_CHANCE (1 / 24.)
#define NO_STATUS_CHANGE {STATUS_NONE, 0}

#define ONE_HIT_KO_HANDLE [](Pokemon &owner, Pokemon &target, unsigned, bool){\
	if (owner.getSpeed() >= target.getSpeed()) {\
                target.takeDamage(target.getHealth());\
                return true;\
        }\
        return false;\
}

#define TAKE_1DAMAGE [](Pokemon &owner){\
	owner.takeDamage(1);\
	return true;\
}

#define TAKE_QUATER_MOVE_DAMAGE [](Pokemon &owner, Pokemon &, unsigned damages, bool){\
	owner.takeDamage(damages / 4);\
	return true;\
}

#define TAKE_HALF_MOVE_DAMAGE [](Pokemon &owner, Pokemon &, unsigned damages, bool){\
	owner.takeDamage(damages / 2);\
	return true;\
}

#define CONFUSE_ON_LAST [](Pokemon &owner, Pokemon &, unsigned, bool last){\
	if (last)\
		owner.addStatus(STATUS_CONFUSED);\
	return true;\
}

#define DEAL_20_DAMAGE [](Pokemon &, Pokemon &target, unsigned, bool){\
	target.takeDamage(20);\
	return true;\
}

#define DEAL_40_DAMAGE [](Pokemon &, Pokemon &target, unsigned, bool){\
	target.takeDamage(40);\
	return true;\
}

#define DEAL_LVL_AS_DAMAGE [](Pokemon &owner, Pokemon &target, unsigned, bool){\
	target.takeDamage(owner.getLevel());\
	return true;\
}

#define DEAL_0_5_TO_1_5_LEVEL_DAMAGE [](Pokemon &owner, Pokemon &target, unsigned, bool){\
	target.takeDamage(owner.getLevel() * (owner.getRandomGenerator()(50, 150) / 100.));\
	return true;\
}

#define ABSORB_HALF_DAMAGE [](Pokemon &owner, Pokemon &, unsigned damages, bool){\
	owner.takeDamage(-damages / 2);\
	return true;\
}

#define HEAL_HALF_HEALTH [](Pokemon &owner, Pokemon &, unsigned, bool){\
	owner.takeDamage(-owner.getMaxHealth() / 2);\
	return true;\
}

#define HEAL_ALL_HEALTH [](Pokemon &owner, Pokemon &, unsigned, bool){\
	owner.takeDamage(-owner.getMaxHealth());\
	owner.addStatus(STATUS_ASLEEP, 2);\
	return true;\
}

#define CANCEL_STATS_CHANGE [](Pokemon &owner, Pokemon &target, unsigned, bool){\
	owner.resetStatsChanges();\
	target.resetStatsChanges();\
	return true;\
}

#define SET_USER_CRIT_RATIO_TO_1_HALF [](Pokemon &owner, Pokemon &, unsigned, bool){\
	owner.setGlobalCritRatio(0.5);\
	return true;\
}

#define STORE_DAMAGES [](Pokemon &owner, Pokemon &target, unsigned, bool last){\
	if (last)\
		target.takeDamage(owner.getDamagesStored() * 2);\
	owner.storeDamages(!last);\
	return true;\
}

#define USE_RANDOM_MOVE [](Pokemon &owner, Pokemon &target, unsigned, bool){\
	owner.useMove(availableMoves[owner.getRandomGenerator()(1, 0xA5)], target);\
	return true;\
}

#define USE_LAST_FOE_MOVE [](Pokemon &owner, Pokemon &target, unsigned, bool){\
	owner.useMove(target.getLastUsedMove(), target);\
	return true;\
}

#define SUICIDE [](Pokemon &owner, Pokemon &, unsigned, bool){\
	owner.takeDamage(owner.getHealth());\
	return true;\
}

#define DREAM_EATER [](Pokemon &owner, Pokemon &target, unsigned, bool){\
	if (!target.hasStatus(STATUS_ASLEEP))\
		return false;\
	owner.takeDamage(-owner.dealDamage(target, 100, TYPE_PSYCHIC) / 2);\
	return true;\
}

#define CONVERSION [](Pokemon &owner, Pokemon &target, unsigned, bool){\
	owner.setTypes(target.getTypes());\
	return true;\
}

#define DEAL_HALF_HP_DAMAGE [](Pokemon &, Pokemon &target, unsigned, bool){\
	target.takeDamage(target.getHealth() / 2);\
	return true;\
}

namespace Pokemon
{
	class Pokemon;

	enum MoveCategory {
		PHYSICAL,
		SPECIAL,
		STATUS,
	};

	class Move {
	private:
		struct HitsProb {
			int	count;
			double	prob;
		};

		struct StatusChangeProb {
			StatusChange	status;
			double		prob;
		};

		struct StatsChangeProb {
			StatsChange	stat;
			char		nb;
			double		prob;
		};

		std::function<bool (Pokemon &owner, Pokemon &target, unsigned damage, bool lastRun)> _hitCallback;
		std::function<bool (Pokemon &owner)> _missCallback;
		double _critChance;
		std::string _name;
		PokemonTypes _type;
		MoveCategory _category;
		unsigned int _power;
		unsigned char _id;
		unsigned char _pp;
		unsigned char _ppup;
		unsigned char _maxpp;
		unsigned char _nbHit;
		unsigned char _accuracy;
		std::vector<unsigned> _nbRuns;
		std::vector<HitsProb> _nbHits;
		StatusChangeProb _statusChange;
		std::vector<StatsChangeProb> _ownerChange;
		std::vector<StatsChangeProb> _foeChange;
		char _priority;
		bool _needLoading;
		bool _invulnerableDuringLoading;
		bool _needRecharge;

	public:
		Move(
			unsigned char id,
			const std::string &name,
			PokemonTypes type,
			MoveCategory category,
			unsigned int power,
			unsigned char accuracy,
			unsigned char maxpp,
			StatusChangeProb statusChange = NO_STATUS_CHANGE,
			std::vector<StatsChangeProb> ownerChange = {},
			std::vector<StatsChangeProb> foeChange = {},
			std::vector<HitsProb> nbHits = DEFAULT_HITS,
			std::vector<unsigned> nbRuns = {1},
			char priority = 0,
			double critChance = DEFAULT_CRIT_CHANCE,
			bool needLoading = false,
			bool invulnerableDuringLoading = false,
			bool needRecharge = false,
			std::function<bool(Pokemon &owner, Pokemon &target, unsigned damage, bool lastRun)> hitCallback = nullptr,
			std::function<bool(Pokemon &owner)> missCallback = nullptr
		);

		unsigned char getMaxPP();
		unsigned char getPP();

		void setPP(unsigned char pp);
		void setPPUp(unsigned char nb);

		bool attack(Pokemon &owner, Pokemon &target);
	};

	extern const std::vector<Move> availableMoves;
}


#endif //POKEAI_MOVE_HPP
