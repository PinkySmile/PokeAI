//
// Created by PinkySmile on 01/07/25.
//

#include "RandomAI.hpp"

PokemonGen1::BattleAction PokemonGen1::RandomAI::getNextMove(const PokemonGen1::BattleState &state, bool side)
{
	std::vector<BattleAction> validActions;
	const auto &mySide = side ? state.op : state.me;
	const auto &me = mySide.team[mySide.pokemonOnField];
	const auto &moves = me.getMoveSet();
	bool noAttack = true;

	if (me.getHealth())
		for (size_t i = 0; i < moves.size(); i++) {
			auto &move = moves[i];

			if (move.getID() && move.getPP()) {
				validActions.push_back(static_cast<BattleAction>(i + Attack1));
				validActions.push_back(static_cast<BattleAction>(i + Attack1));
				validActions.push_back(static_cast<BattleAction>(i + Attack1));
				noAttack = false;
			}
		}
	if (noAttack)
		validActions.push_back(StruggleMove);
	for (size_t i = 0; i < mySide.team.size(); i++) {
		auto &pkmn = mySide.team[i];

		if (pkmn.getHealth() && i != mySide.pokemonOnField)
			validActions.push_back(static_cast<BattleAction>(i + Switch1));
	}
	return validActions[this->_rand() % validActions.size()];
}
