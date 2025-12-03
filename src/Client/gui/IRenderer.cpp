//
// Created by PinkySmile on 19/11/2025.
//

#include <cstring>
#include "IRenderer.hpp"

unsigned IRenderer::getTurn()
{
	return this->_currentTurn;
}

void IRenderer::goToTurn(unsigned int turn)
{
	while (this->_currentTurn < turn && !this->_queue.empty())
		this->nextTurn();
	while (this->_currentTurn > turn)
		this->previousTurn();
}

void IRenderer::previousTurn()
{
	auto &snapshot = this->_snapshots.back();

	this->_currentTurn = snapshot.turn;
	this->_queue = snapshot.queue;
	this->state = snapshot.state;
	this->_snapshots.pop_back();
}

void IRenderer::nextTurn()
{
	auto old = this->soundDisabled;
	auto oldT = this->_currentTurn;

	this->soundDisabled = true;
	this->_skipping = true;
	while (oldT == this->_currentTurn && !this->_queue.empty())
		this->update();
	this->_skipping = false;
	this->soundDisabled = old;
}

void IRenderer::consumeEvent(const PkmnCommon::Event &event)
{
	this->_queue.push_back(event);
	for (auto &state : this->_snapshots)
		state.queue.push_back(event);
}

IRenderer::GameState fromGen1(const PokemonGen1::BattleState &state)
{
	IRenderer::GameState result;

	strcpy(result.p1.name, state.me.name.c_str());
	result.p1.substitute = state.me.team[state.me.pokemonOnField].hasSubstitute();
	result.p1.active = state.me.pokemonOnField;
	result.p1.spriteId = state.me.team[state.me.pokemonOnField].getID();
	result.p1.hidden = false;
	for (unsigned i = 0; i < result.p1.team.size() && i < state.me.team.size(); i++) {
		auto &po = result.p1.team[i];
		auto &pi = state.me.team[i];
		auto &moveSet = pi.getMoveSet();

		po.id = pi.getID();
		po.hp = pi.getHealth();
		po.maxHp = pi.getMaxHealth();
		po.atk = pi.getRawAttack();
		po.def = pi.getRawDefense();
		po.spd = pi.getRawSpeed();
		po.spe = pi.getRawSpecial();
		po.level = pi.getLevel();
		strcpy(po.name, pi.getName(false).c_str());
		po.asleep = pi.hasStatus(PokemonGen1::STATUS_ASLEEP);
		po.frozen = pi.hasStatus(PokemonGen1::STATUS_FROZEN);
		po.burned = pi.hasStatus(PokemonGen1::STATUS_BURNED);
		po.poisoned = pi.hasStatus(PokemonGen1::STATUS_POISONED);
		po.toxicPoisoned = pi.hasStatus(PokemonGen1::STATUS_BAD_POISON);
		po.paralyzed = pi.hasStatus(PokemonGen1::STATUS_PARALYZED);
		po.ko = pi.hasStatus(PokemonGen1::STATUS_KO);
		po.leeched = pi.hasStatus(PokemonGen1::STATUS_LEECHED);
		po.confused = pi.hasStatus(PokemonGen1::STATUS_CONFUSED);
		for (unsigned j = 0; j < po.moves.size() && j < moveSet.size(); j++) {
			po.moves[j].id = moveSet[j].getID();
			po.moves[j].pp = moveSet[j].getPP();
			po.moves[j].maxPp = moveSet[j].getMaxPP();
		}
	}

	strcpy(result.p2.name, state.op.name.c_str());
	result.p2.substitute = state.op.team[state.op.pokemonOnField].hasSubstitute();
	result.p2.active = state.op.pokemonOnField;
	result.p2.spriteId = state.op.team[state.me.pokemonOnField].getID();
	result.p2.hidden = false;
	for (unsigned i = 0; i < result.p2.team.size() && i < state.op.team.size(); i++) {
		auto &po = result.p2.team[i];
		auto &pi = state.op.team[i];
		auto &moveSet = pi.getMoveSet();

		po.id = pi.getID();
		po.hp = pi.getHealth();
		po.maxHp = pi.getMaxHealth();
		po.atk = pi.getRawAttack();
		po.def = pi.getRawDefense();
		po.spd = pi.getRawSpeed();
		po.spe = pi.getRawSpecial();
		po.level = pi.getLevel();
		strcpy(po.name, pi.getName(false).c_str());
		po.asleep = pi.hasStatus(PokemonGen1::STATUS_ASLEEP);
		po.frozen = pi.hasStatus(PokemonGen1::STATUS_FROZEN);
		po.burned = pi.hasStatus(PokemonGen1::STATUS_BURNED);
		po.poisoned = pi.hasStatus(PokemonGen1::STATUS_POISONED);
		po.toxicPoisoned = pi.hasStatus(PokemonGen1::STATUS_BAD_POISON);
		po.paralyzed = pi.hasStatus(PokemonGen1::STATUS_PARALYZED);
		po.ko = pi.hasStatus(PokemonGen1::STATUS_KO);
		po.leeched = pi.hasStatus(PokemonGen1::STATUS_LEECHED);
		po.confused = pi.hasStatus(PokemonGen1::STATUS_CONFUSED);
		for (unsigned j = 0; j < po.moves.size() && j < moveSet.size(); j++) {
			po.moves[j].id = moveSet[j].getID();
			po.moves[j].pp = moveSet[j].getPP();
			po.moves[j].maxPp = moveSet[j].getMaxPP();
		}
	}
	return result;
}

PokemonGen1::BattleAction toGen1(IRenderer::BattleAction ac)
{
	switch (ac) {
	case IRenderer::EmptyAction:
		return PokemonGen1::EmptyAction;
	case IRenderer::Attack1:
		return PokemonGen1::Attack1;
	case IRenderer::Attack2:
		return PokemonGen1::Attack2;
	case IRenderer::Attack3:
		return PokemonGen1::Attack3;
	case IRenderer::Attack4:
		return PokemonGen1::Attack4;
	case IRenderer::Switch1:
		return PokemonGen1::Switch1;
	case IRenderer::Switch2:
		return PokemonGen1::Switch2;
	case IRenderer::Switch3:
		return PokemonGen1::Switch3;
	case IRenderer::Switch4:
		return PokemonGen1::Switch4;
	case IRenderer::Switch5:
		return PokemonGen1::Switch5;
	case IRenderer::Switch6:
		return PokemonGen1::Switch6;
	case IRenderer::NoAction:
		return PokemonGen1::NoAction;
	case IRenderer::StruggleMove:
		return PokemonGen1::StruggleMove;
	default:
		return PokemonGen1::Run;
	}
}
