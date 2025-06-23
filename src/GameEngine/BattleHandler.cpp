//
// Created by Gegel85 on 13/07/2019.
//

#include <thread>
#include <algorithm>
#include "BattleHandler.hpp"
#include "../Exception.hpp"

namespace PokemonGen1
{
	BattleHandler::BattleHandler(bool viewSwapped, bool logMessages) :
		_isViewSwapped(viewSwapped),
		_logMessages(logMessages)
	{
		this->_state.onBattleEnd = [this] {
			this->reset();
		};
		this->_state.onBattleStart = [this] {
			this->logBattle(this->_state.op.name + " wants to fight");
			this->logBattle(this->_state.op.name + " sent out " + this->_state.op.team[0].getNickname());
			this->logBattle(this->_state.me.team[0].getName() + " go");
		};
		this->_state.onTurnStart = [this]{
			return this->tick();
		};
	}

	void BattleHandler::logBattle(const std::string &message)
	{
		if (this->_state.battleLogger)
			this->_state.battleLogger(message + "!");
	}

	BattleState &BattleHandler::getBattleState()
	{
		return this->_state;
	}

	const BattleState &BattleHandler::getBattleState() const
	{
		return this->_state;
	}

	void BattleHandler::_makePlayersAttack(bool p1Attack, bool p2Attack)
	{
		Pokemon &p1 = this->_state.me.team[this->_state.me.pokemonOnField];
		Pokemon &p2 = this->_state.op.team[this->_state.op.pokemonOnField];
		int p1PriorityFactor = p1.getPriorityFactor(this->_state.me.nextAction - Attack1);
		int p2PriorityFactor = p2.getPriorityFactor(this->_state.op.nextAction - Attack1);
		bool p1Start = p1PriorityFactor > p2PriorityFactor;

		if (p1PriorityFactor == p2PriorityFactor)
			p1Start = (this->_state.rng() - 80 >= 0) ^ this->_isViewSwapped;

		if (!p1.getHealth())
			return;
		if (p1Attack && p1Start)
			p1.attack(this->_state.me.nextAction - Attack1, p2);
		if (!p2.getHealth())
			return;
		if (p2Attack)
			p2.attack(this->_state.op.nextAction - Attack1, p1);
		if (!p1.getHealth())
			return;
		if (p1Attack && !p1Start)
			p1.attack(this->_state.me.nextAction - Attack1, p2);
	}

	void BattleHandler::_executeBattleActions()
	{
		bool p1TeamOK = std::any_of(this->_state.me.team.begin(), this->_state.me.team.end(), [](const Pokemon &pkmn){ return pkmn.getHealth() != 0; });
		bool p2TeamOK = std::any_of(this->_state.op.team.begin(), this->_state.op.team.end(), [](const Pokemon &pkmn){ return pkmn.getHealth() != 0; });

		if (!p1TeamOK){
			this->logBattle(this->_state.me.name + " is out of usable pokemon");
			this->logBattle(this->_state.me.name + " blacked out");
			this->_finished = true;
			return;
		}
		if (!std::any_of(this->_state.op.team.begin(), this->_state.op.team.end(), [](const Pokemon &pkmn){ return pkmn.getHealth() != 0; })){
			this->logBattle(this->_state.me.name + " defeated " + this->_state.op.name);
			this->_finished = true;
			return;
		}

		bool p1Attack = false;
		bool p2Attack = false;
		bool p1Fainted = this->_state.me.team[this->_state.me.pokemonOnField].getHealth() == 0;
		bool p2Fainted = this->_state.op.team[this->_state.op.pokemonOnField].getHealth() == 0;

		if (p1Fainted || !p2Fainted)
			switch (this->_state.me.nextAction) {
			case Run:
				this->logBattle("Got away safely");
				this->_finished = true;
				return;
			case Switch1:
			case Switch2:
			case Switch3:
			case Switch4:
			case Switch5:
			case Switch6:
				if (!p1Fainted) {
					this->_state.me.team[this->_state.me.pokemonOnField].switched();
					this->logBattle(this->_state.me.team[this->_state.me.pokemonOnField].getName() + " come back");
				}
				this->_state.me.pokemonOnField = this->_state.me.nextAction - Switch1;
				this->logBattle(this->_state.me.team[this->_state.me.pokemonOnField].getName() + " go");
				break;
			case Attack1:
			case Attack2:
			case Attack3:
			case Attack4:
			case StruggleMove:
				p1Attack = true;
				break;
			default:
				this->_log("Warning: Invalid P1 move " + std::to_string(this->_state.me.nextAction));
				this->_state.me.nextAction = StruggleMove;
				p1Attack = true;
			}

		if (p2Fainted || !p1Fainted)
			switch (this->_state.op.nextAction) {
			case Run:
				this->logBattle(this->_state.op.team[this->_state.op.pokemonOnField].getName() + " ran");
				this->_finished = true;
				return;
			case Switch1:
			case Switch2:
			case Switch3:
			case Switch4:
			case Switch5:
			case Switch6:
				if (!p2Fainted) {
					this->_state.op.team[this->_state.op.pokemonOnField].switched();
					this->logBattle(this->_state.op.name + " withdrew " + this->_state.op.team[this->_state.op.pokemonOnField].getNickname());
				}
				this->_state.op.pokemonOnField = this->_state.op.nextAction - Switch1;
				this->logBattle(this->_state.op.name + " sent out " + this->_state.op.team[this->_state.op.pokemonOnField].getNickname());
				break;
			case Attack1:
			case Attack2:
			case Attack3:
			case Attack4:
			case StruggleMove:
				p2Attack = true;
				break;
			default:
				this->_log("Warning: Invalid p2 move " + std::to_string(this->_state.op.nextAction));
				this->_state.op.nextAction = StruggleMove;
				p2Attack = true;
			}

		if (p2Fainted || p1Fainted)
			return;

		this->_makePlayersAttack(p1Attack, p2Attack);
		this->_state.me.team[this->_state.me.pokemonOnField].endTurn();
		this->_state.op.team[this->_state.op.pokemonOnField].endTurn();

		p1TeamOK = std::any_of(this->_state.me.team.begin(), this->_state.me.team.end(), [](const Pokemon &pkmn){ return pkmn.getHealth() != 0; });
		p2TeamOK = std::any_of(this->_state.op.team.begin(), this->_state.op.team.end(), [](const Pokemon &pkmn){ return pkmn.getHealth() != 0; });
		if (!p1TeamOK) {
			this->logBattle(this->_state.me.name + " is out of usable pokemon");
			this->logBattle(this->_state.me.name + " blacked out");
			this->_finished = true;
			return;
		}
		if (!p2TeamOK){
			this->logBattle(this->_state.me.name + " defeated " + this->_state.op.name);
			this->_finished = true;
			return;
		}
	}

	bool BattleHandler::tick()
	{
		if (this->_finished)
			return false;
		if (this->_state.me.nextAction == NoAction || this->_state.op.nextAction == NoAction)
			throw std::runtime_error("No action selected");
		this->_executeBattleActions();
		return this->_finished;
	}

	void BattleHandler::_log(const std::string &msg)
	{
		if (this->_logMessages || msg.substr(0, 7) == "Warning")
			(msg.substr(0, 7) == "Warning" ? std::cerr : std::cout) << "[Gen1BattleHandler]: " << msg << std::endl;
	}

	bool BattleHandler::isFinished() const
	{
		return this->_finished;
	}

	void BattleHandler::reset()
	{
		this->_finished = false;
		this->_state.rng.reset();

		this->_state.me.lastAction = NoAction;
		this->_state.me.nextAction = NoAction;
		this->_state.me.pokemonOnField = 0;
		this->_state.me.discovered.fill({false, {false, false, false, false}});
		for (auto &pkmn : this->_state.me.team)
			pkmn.reset();

		this->_state.op.lastAction = NoAction;
		this->_state.op.nextAction = NoAction;
		this->_state.op.pokemonOnField = 0;
		this->_state.op.discovered.fill({false, {false, false, false, false}});
		for (auto &pkmn : this->_state.op.team)
			pkmn.reset();
	}
}
