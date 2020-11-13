//
// Created by andgel on 08/09/2020
//

#include "AIHeuristic.hpp"

#define KILL_PROBABILITY_SCORE 1000000
#define DAMAGE_SCORE 1
#define BUFFS_SCORE 1
#define DEBUFFS_SCORE 1

#define ATTACK_SCORE 200
#define DEFENCE_SCORE 200
#define SPEED_SCORE 1000
#define SPECIAL_SCORE 200
#define EVADE_SCORE 200
#define ACCURACY_SCORE 200

#define STATUS_FROZEN_SCORE 6000
#define STATUS_ASLEEP_SCORE 5000
#define STATUS_PARALYZED_SCORE 4500
#define STATUS_BADLY_POISONED_SCORE 4000
#define STATUS_BURNED_SCORE 3000
#define STATUS_CONFUSED_SCORE 2500
#define STATUS_POISONED_SCORE 1200
#define STATUS_LEECHED_SCORE 1000

namespace PokemonGen1
{
	static std::string BattleActionToString(BattleAction action)
	{
		switch (action) {
		case Attack1:
			return "Attack1";
		case Attack2:
			return "Attack2";
		case Attack3:
			return "Attack3";
		case Attack4:
			return "Attack4";
		case Switch1:
			return "Switch1";
		case Switch2:
			return "Switch2";
		case Switch3:
			return "Switch3";
		case Switch4:
			return "Switch4";
		case Switch5:
			return "Switch5";
		case Switch6:
			return "Switch6";
		case StruggleMove:
			return "StruggleMove";
		case Run:
			return "Run";
		case NoAction:
			break;
		}
		return "Unknown";
	}

	AIHeuristic::AIHeuristic(const PokemonGen1::GameHandle &gameHandle) :
		_gameHandle(gameHandle)
	{
	}

	BattleAction AIHeuristic::getNextMove()
	{
		std::map<BattleAction, float> scores{
			{ Attack1, -std::numeric_limits<double>::infinity() },
			{ Attack2, -std::numeric_limits<double>::infinity() },
			{ Attack3, -std::numeric_limits<double>::infinity() },
			{ Attack4, -std::numeric_limits<double>::infinity() },
			{ Switch1, 1 },
			{ Switch2, 1 },
			{ Switch3, 1 },
			{ Switch4, 1 },
			{ Switch5, 1 },
			{ Switch6, 1 },
			{ StruggleMove, -std::numeric_limits<double>::infinity() },
			{ Run, 0 },
		};
		const auto &state = this->_gameHandle.getBattleState();
		const auto &me = state.team[state.pokemonOnField];
		const auto &opponent = state.opponentTeam[state.opponentPokemonOnField];
		const auto &moves = me.getMoveSet();
		//const auto &opponentMoves = opponent.getMoveSet();
		unsigned char unusableMoves = 0;

		for (unsigned i = state.team.size(); i < 6; i++)
			scores[static_cast<BattleAction>(Switch1 + i)] = -std::numeric_limits<double>::infinity();

		for (const auto & move : moves)
			unusableMoves += !move.getPP();

		auto bestMove = this->_getBestMoveScore(me, opponent);

		scores[static_cast<BattleAction>(Attack1 + bestMove.first)] = bestMove.second;

		if (unusableMoves == 4 && getAttackDamageMultiplier(TYPE_NORMAL, opponent.getTypes()) != 0)
			scores[StruggleMove] = 2;

		for (unsigned i = 0; i < state.team.size(); i++) {
			const auto &pkmn = state.team[i];
			auto &score = scores[static_cast<BattleAction>(Switch1 + i)];

			if (!pkmn.getHealth() || i == state.pokemonOnField) {
				score = -std::numeric_limits<double>::infinity();
				continue;
			}
		}

		std::pair<BattleAction, int> bestScore = *scores.begin();

		for (auto &score : scores) {
			std::cout << "[HAI]: Action " << BattleActionToString(score.first) << " has " << score.second << " points" << std::endl;
			if (score.second > bestScore.second)
				bestScore = score;
		}
		std::cout << "[HAI]: Best action is " << BattleActionToString(bestScore.first) << " with " << bestScore.second << " points" << std::endl;
		return bestScore.first;
	}

	double AIHeuristic::_getProbabilityToKill(const Pokemon &owner, const Pokemon &target, const Move &move, bool calcOp)
	{
		double hitFirstProb = ((owner.getSpeed() >= target.getSpeed()) + (owner.getSpeed() > target.getSpeed())) / 2.;
		double critChance = pokemonList[owner.getID()].SPD / 2 * move.getCritChance() / 255.;
		auto withCrit = this->_getDamageRange(owner, target, move, true);
		auto withoutCrit = this->_getDamageRange(owner, target, move, false);
		auto probWithCrit = withCrit.first != withCrit.second ?
			std::min(1.,
				std::max(0.,
					(0. * withCrit.first - target.getHealth()) / (withCrit.first - withCrit.second)
				)
			) : 0;
		auto probWithoutCrit = withoutCrit.first != withoutCrit.second ?
			    std::min(1.,
				     std::max(0.,
					      (0. * withoutCrit.first - target.getHealth()) / (withoutCrit.first - withoutCrit.second)
				     )
			    ) : 0;
		double betterProba = 0;
		double sameProba = 0;
		double worseProba = 0;

		if (calcOp)
			for (auto &opMove : target.getMoveSet()) {
				switch ((move.getPriority() > opMove.getPriority()) - (move.getPriority() < opMove.getPriority())) {
				case -1:
					betterProba = std::max(betterProba, this->_getProbabilityToKill(target, owner, opMove, false));
					break;
				case 0:
					sameProba = std::max(sameProba, this->_getProbabilityToKill(target, owner, opMove, false));
					break;
				default:
					worseProba = std::max(worseProba, this->_getProbabilityToKill(target, owner, opMove, false));
				}
			}

		return  (
			probWithCrit * critChance +
			probWithoutCrit * (1 - critChance)
		) * (1 - std::max(
			betterProba, std::max(
			sameProba / 2,
			worseProba * (1 - hitFirstProb)
		)));
	}

	std::pair<unsigned, unsigned> AIHeuristic::_getDamageRange(const Pokemon &owner, const Pokemon &target, const Move &move, bool critical)
	{
		auto damageType = move.getType();
		double effectiveness = getAttackDamageMultiplier(damageType, target.getTypes());

		if (effectiveness == 0)
			return {0, 0};

		unsigned defense;
		unsigned attack;
		unsigned level = owner.getLevel() * (1 + critical);

		switch (move.getCategory()) {
		case SPECIAL:
			attack  = critical ? owner.getRawSpecial()  : owner.getSpecial();
			defense = critical ? target.getRawSpecial() : target.getSpecial();
			break;
		case PHYSICAL:
			attack =  critical ? owner.getRawAttack()   : owner.getAttack();
			defense = critical ? target.getRawDefense() : target.getDefense();
			break;
		default:
			return {0, 0};
		}

		if (attack > 255 || defense > 255) {
			attack = attack / 4 % 256;
			defense = defense / 4 % 256;
		}

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
							) * attack * move.getPower() / defense
						) / 50
					)
				) + 2
			),
			1
		);

		damages *= effectiveness * (1 + (owner.getTypes().first == damageType || owner.getTypes().second == damageType) / 2.);

		return {damages * 217 / 255, damages};
	}

	double AIHeuristic::_getBuffsValue(const Pokemon &target, const Pokemon &owner, const Move &move)
	{
		double score = 0;
		const auto &o = move.getOwnerChange();
		const auto &stats = owner.getStatsUpgradeStages();

		for (auto &change : o)
			score += this->_getStatValue(change.stat, owner, target) *
				change.nb *
				change.cmpVal / 256. *
				(std::abs(getStat(stats, change.stat) + change.nb) <= 6);
		return score;
	}

	double AIHeuristic::_getDebuffsValue(const Pokemon &target, const Pokemon &owner, const Move &move)
	{
		double score = 0;
		const auto &o = move.getFoeChange();
		const auto &status = move.getStatusChange();
		const auto &stats = target.getStatsUpgradeStages();

		for (auto &change : o)
			score += this->_getStatValue(change.stat, target, owner) *
				-change.nb *
				(change.cmpVal ?: 256) / 256. *
				(std::abs(getStat(stats, change.stat) + change.nb) <= 6) * 0.75;
		score += this->_getStatusChangeValue(target, status.status) * (status.cmpVal ?: 256) / 256.;
		return score;
	}

	double AIHeuristic::_getStatValue(StatsChange stat, const Pokemon &owner, const Pokemon &target)
	{
		unsigned atk = 0;
		unsigned spe = 0;
		unsigned def = 0;
		unsigned spd = owner.getSpeed() <= target.getSpeed();
		unsigned eva = 1;
		unsigned acc = 1;

		for (auto &move : owner.getMoveSet()) {
			atk += move.getCategory() == PHYSICAL;
			spe += move.getCategory() == SPECIAL;
		}
		for (auto &move : target.getMoveSet()) {
			def += move.getCategory() == PHYSICAL;
			spe += move.getCategory() == SPECIAL;
		}

		switch (stat) {
		case STATS_ATK:
			return atk * ATTACK_SCORE;
		case STATS_DEF:
			return def * DEFENCE_SCORE;
		case STATS_SPD:
			return spd * SPEED_SCORE;
		case STATS_SPE:
			return spe * SPECIAL_SCORE;
		case STATS_ESQ:
			return eva * EVADE_SCORE;
		case STATS_ACC:
			return acc * ACCURACY_SCORE;
		default:
			return 0;
		}
	}

	double AIHeuristic::_getStatusChangeValue(const Pokemon &owner, StatusChange status)
	{
		switch (status) {
		case STATUS_FROZEN:
		case STATUS_ASLEEP:
		case STATUS_PARALYZED:
		case STATUS_BADLY_POISONED:
		case STATUS_BURNED:
		case STATUS_POISONED:
			if (owner.hasStatus(STATUS_ANY_NON_VOLATILE_STATUS))
				return 0;
			break;
		case STATUS_CONFUSED:
		case STATUS_LEECHED:
			if (owner.hasStatus(status))
				return 0;
			break;
		default:
			return 0;
		}

		switch (status) {
		case STATUS_FROZEN:
			return STATUS_FROZEN_SCORE;
		case STATUS_ASLEEP:
			return STATUS_ASLEEP_SCORE;
		case STATUS_PARALYZED:
			return STATUS_PARALYZED_SCORE;
		case STATUS_BADLY_POISONED:
			return STATUS_BADLY_POISONED_SCORE;
		case STATUS_BURNED:
			return STATUS_BURNED_SCORE;
		case STATUS_CONFUSED:
			return STATUS_CONFUSED_SCORE;
		case STATUS_POISONED:
			return STATUS_POISONED_SCORE;
		case STATUS_LEECHED:
			return STATUS_LEECHED_SCORE;
		default:
			return 0;
		}
	}

	std::pair<unsigned char, double> AIHeuristic::_getBestMoveScore(const Pokemon &pkmn, const Pokemon &opponent)
	{
		std::pair<unsigned char, double> best{0, -std::numeric_limits<double>::infinity()};
		const auto &moves = pkmn.getMoveSet();

		for (unsigned i = 0; i < moves.size(); i++) {
			const auto &move = moves[i];
			double score = 0;

			if (
				!pkmn.getHealth() ||
				!move.getID() ||
				!move.getPP() ||
				(move.getPower() && getAttackDamageMultiplier(move.getType(), opponent.getTypes()) == 0)
			)
				continue;

			//TODO: Handle when target is invicible

			auto accuracy = move.getAccuracy();
			double critChance = pokemonList[pkmn.getID()].SPD / 2 * move.getCritChance() / 255.;
			auto withCrit = this->_getDamageRange(pkmn, opponent, move, true);
			auto withoutCrit = this->_getDamageRange(pkmn, opponent, move, false);
			auto dmgWithCrit = withCrit.first + withCrit.second;
			auto dmgWithoutCrit = withoutCrit.first + withoutCrit.second;
			double hitAccuracy = accuracy > 100 ? 1. : (
				std::max(255., accuracy * 2.55 * pkmn.getAccuracy() * opponent.getEvasion()) *
				(
					(opponent.canGetHit() || opponent.getSpeed() < pkmn.getSpeed()) /
					(1. + (opponent.getSpeed() == pkmn.getSpeed() && !opponent.canGetHit()))
				) / 256
			);

			score += this->_getBuffsValue(opponent, pkmn, move) * BUFFS_SCORE;
			score += this->_getDebuffsValue(opponent, pkmn, move) * DEBUFFS_SCORE;
			score += (dmgWithCrit * critChance + dmgWithoutCrit * (1 - critChance)) * DAMAGE_SCORE;
			score += this->_getProbabilityToKill(pkmn, opponent, move) * KILL_PROBABILITY_SCORE;
			if (move.needsLoading())
				score /= move.makesInvulnerable() ? 1.25 : 2;
			if (move.getPriority() > 0)
				score *= 1.5 * move.getPriority();
			score *= hitAccuracy;
			if (best.second < score)
				best = {i, score};
		}
		return best;
	}
}
