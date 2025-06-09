//
// Created by andgel on 08/09/2020
//

#ifndef POKEAI_AIHEURISTIC_HPP
#define POKEAI_AIHEURISTIC_HPP


#include "../GameEngine/GameHandle.hpp"
#include "AI.hpp"

namespace PokemonGen1
{
	class AIHeuristic : public AI {
	private:
		const GameHandle &_gameHandle;

		std::pair<unsigned, unsigned> _getDamageRange(const Pokemon &owner, const Pokemon &target, const Move &move, bool critical);
		double _getProbabilityToKill(const Pokemon &owner, const Pokemon &target, const Move &move, bool calcOp = true);
		double _getBuffsValue(const Pokemon &target, const Pokemon &owner, const Move &move);
		double _getDebuffsValue(const Pokemon &target, const Pokemon &owner, const Move &move);
		double _getMoveScore(const Pokemon &pkmn, const Pokemon &opponent, const Move &move);
		double _getStatValue(StatsChange stat, const Pokemon &owner, const Pokemon &target);
		double _getStatusChangeValue(const Pokemon &owner, StatusChange status);

	public:
		AIHeuristic(const GameHandle &gameHandle);
		BattleAction getNextMove() override;
	};
}


#endif //POKEAI_AIHEURISTIC_HPP
