//
// Created by PinkySmile on 21/06/25.
//

#include <nlohmann/json.hpp>
#include "State.hpp"

namespace PokemonGen1
{
	#define CHECK_ADVANCE(i) do {if (it > data.end() - i) throw std::invalid_argument("Data too small"); } while (false)

	nlohmann::json BattleState::serialize()
	{
		return {
			{ "p1", this->me.serialize() },
			{ "p2", this->op.serialize() },
			{ "rng", this->rng.getList() },
			{ "rngIndex", this->rng.getIndex() }
		};
	}

	void BattleState::deserialize(const nlohmann::json &json)
	{
		this->rng.setList(json["rng"]);
		this->rng.setIndex(json["rngIndex"]);
		this->me.deserialize(json["p1"], *this);
		this->op.deserialize(json["p2"], *this);
	}

	nlohmann::json PlayerState::serialize()
	{
		nlohmann::json json{
			{ "name", this->name },
			{ "lastAction", this->lastAction },
			{ "nextAction", this->nextAction },
			{ "pokemonOnField", this->pokemonOnField },
			{ "team", nlohmann::json::array() },
			{ "discovered", this->discovered }
		};

		for (const Pokemon &pkmn : this->team)
			json["team"].push_back(pkmn.serialize());
		return json;
	}

	void PlayerState::deserialize(const nlohmann::json &json, BattleState &state)
	{
		this->name = json["name"];
		this->lastAction = json["lastAction"];
		this->nextAction = json["nextAction"];
		this->pokemonOnField = json["pokemonOnField"];
		this->discovered = json["discovered"];
		this->team.clear();
		for (const auto &pkmn : json["team"])
			this->team.emplace_back(state, pkmn);
	}

	std::string BattleActionToString(BattleAction action)
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
}