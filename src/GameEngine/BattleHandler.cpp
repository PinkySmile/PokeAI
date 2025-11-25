//
// Created by PinkySmile on 13/07/2019.
//

#include <thread>
#include <iostream>
#include <algorithm>
#include <fstream>
#include "BattleHandler.hpp"
#include "Exception.hpp"
#include "Team.hpp"
#include "nlohmann/json.hpp"

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
			this->start();
		};
		this->_state.onTurnStart = [this]{
			return this->tick();
		};
	}

	void BattleHandler::start()
	{
		this->logBattle(this->_state.op.name + " wants to fight");
		this->logBattle(this->_state.op.name + " sent out " + this->_state.op.team[this->_state.op.pokemonOnField].getName(false));
		this->logBattle(this->_state.me.team[this->_state.me.pokemonOnField].getName() + " go");
		this->_state.me.discovered[this->_state.me.pokemonOnField].first = true;
		this->_state.op.discovered[this->_state.me.pokemonOnField].first = true;
		this->_log("Game start!");
		this->_log(this->_state.me.name + "'s team (P1)");
		for (const Pokemon &pkmn : this->_state.me.team)
			this->_log(pkmn.dump());
		this->_log(this->_state.op.name + "'s team (P2)");
		for (const Pokemon &pkmn : this->_state.op.team)
			this->_log(pkmn.dump());
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

		if (p1Attack) {
			this->_state.me.lastAttack =
				this->_state.me.nextAction == StruggleMove ? Struggle :
				static_cast<AvailableMove>(p1.getMoveSet()[this->_state.me.nextAction - Attack1].getID());
			if (p1.getForcedAttack())
				this->_state.me.nextAction = static_cast<BattleAction>(Attack1 + p1.getForcedAttack() - 1);
		}
		if (p2Attack) {
			this->_state.op.lastAttack =
				this->_state.op.nextAction == StruggleMove ? Struggle :
				static_cast<AvailableMove>(p2.getMoveSet()[this->_state.op.nextAction - Attack1].getID());
			if (p2.getForcedAttack())
				this->_state.op.nextAction = static_cast<BattleAction>(Attack1 + p2.getForcedAttack() - 1);
		}

		if (p1PriorityFactor == p2PriorityFactor)
			p1Start = (this->_state.rng() < 0x80) ^ this->_isViewSwapped;

		if (!p1.getHealth() || !p2.getHealth())
			return;
		if (p1Start) {
			if (p1Attack) {
				if (this->_state.me.nextAction <= Attack4)
					this->_state.op.discovered[this->_state.me.pokemonOnField].second[this->_state.me.nextAction - Attack1] = true;
				p1.attack(this->_state.me.nextAction - Attack1, p2);
			}
			p1.stepEnds(p2);
		}

		if (!p1.getHealth() || !p2.getHealth())
			return;
		if (p2Attack) {
			if (this->_state.op.nextAction <= Attack4)
				this->_state.me.discovered[this->_state.op.pokemonOnField].second[this->_state.op.nextAction - Attack1] = true;
			p2.attack(this->_state.op.nextAction - Attack1, p1);
		}
		p2.stepEnds(p1);

		if (!p1.getHealth() || !p2.getHealth())
			return;
		if (!p1Start) {
			if (p1Attack) {
				if (this->_state.me.nextAction <= Attack4)
					this->_state.op.discovered[this->_state.me.pokemonOnField].second[this->_state.me.nextAction - Attack1] = true;
				p1.attack(this->_state.me.nextAction - Attack1, p2);
			}
			p1.stepEnds(p2);
		}
	}

	void BattleHandler::_executeBattleActions()
	{
		bool p1TeamOK = std::any_of(this->_state.me.team.begin(), this->_state.me.team.end(), [](const Pokemon &pkmn){ return pkmn.getHealth() != 0; });
		bool p2TeamOK = std::any_of(this->_state.op.team.begin(), this->_state.op.team.end(), [](const Pokemon &pkmn){ return pkmn.getHealth() != 0; });

		if (!p1TeamOK) {
			this->logBattle(this->_state.me.name + " is out of usable pokemon");
			this->logBattle(this->_state.me.name + " blacked out");
			this->_finished = true;
			return;
		}
		if (!p2TeamOK) {
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
					this->_state.op.team[this->_state.op.pokemonOnField].opponentSwitched();
					this->logBattle(this->_state.me.team[this->_state.me.pokemonOnField].getName(false) + " come back");
				}
				this->_state.me.pokemonOnField = this->_state.me.nextAction - Switch1;
				this->logBattle(this->_state.me.team[this->_state.me.pokemonOnField].getName(false) + " go");
				this->_state.op.discovered[this->_state.me.pokemonOnField].first = true;
				this->_state.me.team[this->_state.me.pokemonOnField].applyStatusDebuff();
				break;
			case Attack1:
			case Attack2:
			case Attack3:
			case Attack4:
			case StruggleMove:
			case NoAction:
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
				this->logBattle(this->_state.op.name + " ran");
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
					this->_state.me.team[this->_state.me.pokemonOnField].opponentSwitched();
					this->logBattle(this->_state.op.name + " withdrew " + this->_state.op.team[this->_state.op.pokemonOnField].getName(false));
				}
				this->_state.op.pokemonOnField = this->_state.op.nextAction - Switch1;
				this->logBattle(this->_state.op.name + " sent out " + this->_state.op.team[this->_state.op.pokemonOnField].getName(false));
				this->_state.me.discovered[this->_state.op.pokemonOnField].first = true;
				this->_state.op.team[this->_state.op.pokemonOnField].applyStatusDebuff();
				break;
			case Attack1:
			case Attack2:
			case Attack3:
			case Attack4:
			case StruggleMove:
			case NoAction:
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

	#define INVALID(name, msg) do { bad = true; message = "Warning: Invalid " + player + " action: Can only do " + name + " when " + msg; goto end; } while (0)
	#define FIX(ok, action) do { if (ok) { myState.nextAction = static_cast<BattleAction>(action); return; } }  while (0)
	void BattleHandler::_checkAction(PlayerState &myState, const PlayerState &opState)
	{
		bool bad = false;
		std::string message;
		std::string player = (&myState == &this->_state.me) ? "P1" : "P2";
		auto &pkmn = myState.team[myState.pokemonOnField];
		auto &opPkmn = opState.team[opState.pokemonOnField];
		auto &moveSet = pkmn.getMoveSet();

		if (myState.nextAction == NoAction) {
			if (pkmn.getHealth() == 0)
				INVALID("NoAction", "alive");
			if (!pkmn.isWrapped() && opPkmn.getHealth() != 0)
				INVALID("NoAction", "trapped or the opponent fainted, but not us");
		} else if (myState.nextAction == StruggleMove) {
			if (pkmn.getHealth() == 0)
				INVALID("StruggleMove", "alive");
			if (pkmn.isWrapped())
				INVALID("StruggleMove", "not trapped");
			for (size_t i = 0; i < moveSet.size(); i++)
				bad |= moveSet[i].getID() != 0 && moveSet[i].getPP() != 0 && i + 1 != pkmn.getMoveDisabled();
			if (bad)
				INVALID("StruggleMove", "no moves are left (all either have no PP, are an empty slot, or disabled)");
		} else if (Attack1 <= myState.nextAction && myState.nextAction <= Attack4) {
			if (pkmn.getHealth() == 0)
				INVALID(BattleActionToString(myState.nextAction), "alive");
			if (pkmn.isWrapped())
				INVALID(BattleActionToString(myState.nextAction), "not trapped");

			size_t id = myState.nextAction - Attack1;

			if (id >= moveSet.size() || moveSet[id].getID() == None)
				INVALID(BattleActionToString(myState.nextAction), "it's not an empty slot");
			if (moveSet[id].getPP() == 0)
				INVALID(BattleActionToString(myState.nextAction), "it has PP left");
			if (id + 1 == pkmn.getMoveDisabled())
				INVALID(BattleActionToString(myState.nextAction), "it's not disabled");
		} else if (Switch1 <= myState.nextAction && myState.nextAction <= Switch6) {
			if (pkmn.getHealth() != 0 && opPkmn.getHealth() == 0)
				INVALID(BattleActionToString(myState.nextAction), "pokemon on field fainted, or the opponent didn't faint");

			size_t id = myState.nextAction - Switch1;

			if (id == myState.pokemonOnField)
				INVALID(BattleActionToString(myState.nextAction), "the target isn't already on the field");
			if (id >= myState.team.size())
				INVALID(BattleActionToString(myState.nextAction), "the target exists in the team");
			if (myState.team[id].getHealth() == 0)
				INVALID(BattleActionToString(myState.nextAction), "the target hasn't fainted");
		}

	end:
		if (!bad)
			return;
		if (this->_state.badAction != BADACTION_THROW)
			std::cout << message << std::endl;
		switch (this->_state.badAction) {
		case BADACTION_STRUGGLE:
			myState.nextAction = StruggleMove;
			return;
		case BADACTION_NOACTION:
			myState.nextAction = NoAction;
			return;
		case BADACTION_THROW:
			throw std::runtime_error(message);
		case BADACTION_IGNORE:
			return;
		case BADACTION_FIX:
			break;
		}

		size_t firstUsableAttack = 0;
		size_t firstSwitchablePokemon = 0;

		while (firstUsableAttack < moveSet.size() && (
			moveSet[firstUsableAttack].getPP() == 0 ||
			moveSet[firstUsableAttack].getID() == None ||
			firstUsableAttack + 1 == pkmn.getMoveDisabled()
		))
			firstUsableAttack++;
		while (firstSwitchablePokemon < myState.team.size() && myState.team[firstSwitchablePokemon].getHealth() == 0)
			firstSwitchablePokemon++;
		if (myState.nextAction == NoAction) {
			if (pkmn.getHealth() == 0) {
				FIX(firstSwitchablePokemon < myState.team.size(), Switch1 + firstSwitchablePokemon);
				throw std::runtime_error("No possible actions remaining");
			} else {
				FIX(firstUsableAttack < moveSet.size(), Attack1 + firstUsableAttack);
				myState.nextAction = StruggleMove;
			}
		} else if (myState.nextAction == StruggleMove) {
			if (pkmn.getHealth() == 0) {
				FIX(firstSwitchablePokemon < myState.team.size(), Switch1 + firstSwitchablePokemon);
				throw std::runtime_error("No possible actions remaining");
			}
			if (pkmn.isWrapped())
				myState.nextAction = NoAction;
			else
				myState.nextAction = static_cast<BattleAction>(Attack1 + firstUsableAttack);
		} else if (Attack1 <= myState.nextAction && myState.nextAction <= Attack4) {
			if (pkmn.getHealth() == 0) {
				FIX(firstSwitchablePokemon < myState.team.size(), Switch1 + firstSwitchablePokemon);
				throw std::runtime_error("No possible actions remaining");
			}
			if (!pkmn.isWrapped()) {
				FIX(firstUsableAttack < moveSet.size(), Attack1 + firstUsableAttack);
				myState.nextAction = StruggleMove;
			} else
				myState.nextAction = NoAction;
		} else if (Switch1 <= myState.nextAction && myState.nextAction <= Switch6) {
			FIX(firstSwitchablePokemon < myState.team.size(), Switch1 + firstSwitchablePokemon);
			if (pkmn.getHealth() == 0)
				throw std::runtime_error("No possible actions remaining");
			if (!pkmn.isWrapped()) {
				FIX(firstUsableAttack < moveSet.size(), Attack1 + firstUsableAttack);
				myState.nextAction = StruggleMove;
			} else
				myState.nextAction = NoAction;
		}
	}

	void BattleHandler::_checkActions()
	{
		auto &me = this->_state.me.team[this->_state.me.pokemonOnField];
		auto &op = this->_state.op.team[this->_state.op.pokemonOnField];

		if (!me.hasStatus(STATUS_ASLEEP) && !me.hasStatus(STATUS_FROZEN))
			this->_checkAction(this->_state.me, this->_state.op);
		if (!op.hasStatus(STATUS_ASLEEP) && !op.hasStatus(STATUS_FROZEN))
			this->_checkAction(this->_state.op, this->_state.me);
	}

	bool BattleHandler::tick()
	{
		if (!this->_started) {
			this->_populateStartParams();
			this->_started = true;
		}
		if (this->_finished)
			return false;
		if (this->_playingReplay) {
			if (!this->_isViewSwapped) {
				this->_state.me.nextAction = this->_replayInputs.front().first;
				this->_state.op.nextAction = this->_replayInputs.front().second;
			} else {
				this->_state.op.nextAction = this->_replayInputs.front().first;
				this->_state.me.nextAction = this->_replayInputs.front().second;
			}
			this->_replayInputs.pop_front();
		}
		this->_checkActions();

		auto &me = this->_state.me.team[this->_state.me.pokemonOnField];
		auto &op = this->_state.op.team[this->_state.op.pokemonOnField];

		if (me.hasStatus(STATUS_ASLEEP) || me.hasStatus(STATUS_FROZEN)) {
			this->_state.me.nextAction = this->_state.me.lastAction;
			if (Switch1 <= this->_state.me.nextAction && this->_state.me.nextAction <= Switch6)
				this->_state.me.nextAction = NoAction;
			if (this->_state.me.nextAction == StruggleMove)
				this->_state.me.nextAction = NoAction;
		}
		if (op.hasStatus(STATUS_ASLEEP) || op.hasStatus(STATUS_FROZEN)) {
			this->_state.op.nextAction = this->_state.op.lastAction;
			if (Switch1 <= this->_state.op.nextAction && this->_state.op.nextAction <= Switch6)
				this->_state.op.nextAction = NoAction;
			if (this->_state.op.nextAction == StruggleMove)
				this->_state.op.nextAction = NoAction;
		}
		if (this->_state.me.nextAction == EmptyAction || this->_state.op.nextAction == EmptyAction)
			throw std::runtime_error("No action selected");
		this->_state.me.discovered[this->_state.op.pokemonOnField].first = true;
		this->_state.op.discovered[this->_state.me.pokemonOnField].first = true;
		this->_replayData.input.emplace_back(this->_state.me.nextAction, this->_state.op.nextAction);
		this->_log("P1 will do " + BattleActionToString(this->_state.me.nextAction));
		this->_log("P2 will do " + BattleActionToString(this->_state.op.nextAction));
		this->_executeBattleActions();
		this->_log(this->_state.me.name + "'s team (P1)");
		for (const Pokemon &pkmn : this->_state.me.team)
			this->_log(pkmn.dump());
		this->_log(this->_state.op.name + "'s team (P2)");
		for (const Pokemon &pkmn : this->_state.op.team)
			this->_log(pkmn.dump());
		this->_log("Game is " + std::string(this->_finished ? "" : "NOT ") + "finished");
		this->_state.me.lastAction = this->_state.me.nextAction;
		this->_state.op.lastAction = this->_state.op.nextAction;
		this->_state.me.nextAction = EmptyAction;
		this->_state.op.nextAction = EmptyAction;
		if (this->_playingReplay && this->_replayInputs.empty())
			this->_finished = true;
		return this->_finished;
	}

	void BattleHandler::_log(const std::string &msg)
	{
		if (this->_logMessages || msg.substr(0, 7) == "Warning")
			(msg.substr(0, 7) == "Warning" ? std::cerr : std::cout) << "[Gen1Battle]: " << msg << std::endl;
	}

	bool BattleHandler::isFinished() const
	{
		return this->_finished;
	}

	void BattleHandler::reset()
	{
		this->_finished = false;
		this->_started = false;
		this->_state.rng.reset();
		this->_replayData.input.clear();

		this->_state.me.lastAction = EmptyAction;
		this->_state.me.nextAction = EmptyAction;
		this->_state.me.pokemonOnField = 0;
		this->_state.me.discovered.fill({false, {false, false, false, false}});
		this->_state.me.discovered[0].first = true;
		for (auto &pkmn : this->_state.me.team)
			pkmn.reset();

		this->_state.op.lastAction = EmptyAction;
		this->_state.op.nextAction = EmptyAction;
		this->_state.op.pokemonOnField = 0;
		this->_state.op.discovered.fill({false, {false, false, false, false}});
		this->_state.op.discovered[0].first = true;
		for (auto &pkmn : this->_state.op.team)
			pkmn.reset();
	}

	void BattleHandler::_populateStartParams()
	{
		this->_replayData.nameP1 = this->_state.me.name;
		this->_replayData.nameP2 = this->_state.op.name;
		this->_replayData.teamP1 = this->_state.me.team;
		this->_replayData.teamP2 = this->_state.op.team;
		this->_replayData.rngList = this->_state.rng.getList();
	}

	bool BattleHandler::saveReplay(const std::string &path)
	{
		std::ofstream stream{path};

		if (stream.fail())
			return false;

		auto p1 = saveTrainer({this->_replayData.nameP1, this->_replayData.teamP1});
		auto p2 = saveTrainer({this->_replayData.nameP2, this->_replayData.teamP2});
		auto &list = this->_replayData.rngList;
		auto &in = this->_replayData.input;
		unsigned char size;

		stream.write(reinterpret_cast<const char *>(p1.data()), p1.size());
		stream.write(reinterpret_cast<const char *>(p2.data()), p2.size());

		size = list.size();
		stream.write(reinterpret_cast<const char *>(&size), 1);
		stream.write(reinterpret_cast<const char *>(list.data()), list.size());

		size = in.size();
		stream.write(reinterpret_cast<const char *>(&size), 1);
		stream.write(reinterpret_cast<const char *>(in.data()), sizeof(*in.data()) * in.size());
		return true;
	}

	void BattleHandler::loadReplay(const std::string &path)
	{
		std::ifstream stream{path};
		BattleHandler::ReplayData data;
		std::vector<unsigned char> buffer;
		Trainer tmp;
		unsigned char size = 0;

		if (stream.fail())
			throw std::runtime_error(std::string("Can't open ") + path + " for reading");
		buffer.resize(TRAINER_DATA_SIZE);

		stream.read(reinterpret_cast<char *>(buffer.data()), TRAINER_DATA_SIZE);
		if (stream.fail())
			throw std::runtime_error("Reached EOF early");
		tmp = loadTrainer(buffer, this->_state);
		data.nameP1 = tmp.first;
		data.teamP1 = tmp.second;

		stream.read(reinterpret_cast<char *>(buffer.data()), TRAINER_DATA_SIZE);
		if (stream.fail())
			throw std::runtime_error("Reached EOF early");
		tmp = loadTrainer(buffer, this->_state);
		data.nameP2 = tmp.first;
		data.teamP2 = tmp.second;

		stream.read(reinterpret_cast<char *>(&size), 1);
		data.rngList.resize(size);
		stream.read(reinterpret_cast<char *>(data.rngList.data()), size);
		if (stream.fail())
			throw std::runtime_error("Reached EOF early");

		stream.read(reinterpret_cast<char *>(&size), 1);
		data.input.resize(size);
		stream.read(reinterpret_cast<char *>(data.input.data()), size * sizeof(*data.input.data()));
		if (stream.fail())
			throw std::runtime_error("Reached EOF early");

		this->_replayData = data;
		this->_replayData.input.clear();
		this->_replayInputs = { data.input.begin(), data.input.end() };
		this->_playingReplay = true;
		this->_finished = false;
		this->_started = false;
		this->_state.rng.setList(this->_replayData.rngList);

		this->_state.me.name = this->_replayData.nameP1;
		this->_state.me.team = this->_replayData.teamP1;
		this->_state.me.lastAction = EmptyAction;
		this->_state.me.nextAction = EmptyAction;
		this->_state.me.pokemonOnField = 0;
		this->_state.me.discovered.fill({false, {false, false, false, false}});

		this->_state.op.name = this->_replayData.nameP2;
		this->_state.op.team = this->_replayData.teamP2;
		this->_state.op.lastAction = EmptyAction;
		this->_state.op.nextAction = EmptyAction;
		this->_state.op.pokemonOnField = 0;
		this->_state.op.discovered.fill({false, {false, false, false, false}});
	}

	void BattleHandler::stopReplay()
	{
		this->_playingReplay = false;
		this->_replayInputs.clear();
	}

	bool BattleHandler::playingReplay() const
	{
		return this->_playingReplay;
	}

	bool BattleHandler::saveState(const std::string &path)
	{
		std::ofstream stream{path};

		if (stream.fail())
			return false;

		nlohmann::json state;

		state["state"] = this->_state.serialize();
		state["replay"] = this->_playingReplay;
		state["replayInfo"] = {
			{ "p1", {
				{ "name", this->_replayData.nameP1 },
				{ "team", nlohmann::json::array() }
			}},
			{ "p2", {
				{ "name", this->_replayData.nameP1 },
				{ "team", nlohmann::json::array() }
			}},
			{ "rng", this->_replayData.rngList },
			{ "inputs", nlohmann::json::array() }
		};
		for (auto &pkmn : this->_replayData.teamP1)
			state["replayInfo"]["p1"]["team"].push_back(pkmn.serialize());
		for (auto &pkmn : this->_replayData.teamP2)
			state["replayInfo"]["p2"]["team"].push_back(pkmn.serialize());
		for (auto &input : this->_replayData.input)
			state["replayInfo"]["inputs"].push_back(nlohmann::json::array({input.first, input.second}));
		if (this->_playingReplay) {
			for (auto &input : this->_replayInputs)
				state["replayInputs"].push_back(nlohmann::json::array({input.first, input.second}));
		}
		stream << state.dump(4);
		return true;
	}

	bool BattleHandler::loadState(const std::string &path)
	{
		std::ifstream stream{path};

		if (stream.fail())
			return false;

		nlohmann::json state;

		stream >> state;
		this->_state.deserialize(state["state"]);
		this->_playingReplay = state["replay"];
		this->_replayData.nameP1 = state["replayInfo"]["p1"]["name"];
		this->_replayData.nameP2 = state["replayInfo"]["p2"]["name"];
		this->_replayData.teamP1.clear();
		for (auto &j : state["replayInfo"]["p1"]["team"])
			this->_replayData.teamP1.emplace_back(this->_state, j);
		this->_replayData.teamP2.clear();
		for (auto &j : state["replayInfo"]["p2"]["team"])
			this->_replayData.teamP2.emplace_back(this->_state, j);
		this->_replayData.rngList = state["replayInfo"]["rng"].get<std::vector<unsigned char>>();
		this->_replayData.input.clear();
		for (auto &j : state["replayInfo"]["inputs"])
			this->_replayData.input.emplace_back(j[0], j[1]);
		if (this->_playingReplay) {
			this->_replayInputs.clear();
			for (auto &j: state["replayInputs"])
				this->_replayInputs.emplace_back(j[0], j[1]);
		}
		return true;
	}
}
