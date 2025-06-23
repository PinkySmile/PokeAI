//
// Created by PinkySmile on 21/06/25.
//

#include <nlohmann/json.hpp>
#include "State.hpp"
#include "EmulatorGameHandle.hpp"

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
		this->me.deserialize(json["p1"], this->rng, this->battleLogger);
		this->op.deserialize(json["p2"], this->rng, this->battleLogger);
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

	void PlayerState::deserialize(const nlohmann::json &json, PokemonRandomGenerator &rng, const BattleLogger &logger)
	{
		this->name = json["name"];
		this->lastAction = json["lastAction"];
		this->nextAction = json["nextAction"];
		this->pokemonOnField = json["pokemonOnField"];
		this->discovered = json["discovered"];
		this->team.clear();
		for (const auto &pkmn : json["team"])
			this->team.emplace_back(rng, logger, pkmn);
	}
}