//
// Created by PinkySmile on 22/02/2026.
//

#include <stdexcept>
#include "Gen1ResultBasedGenerator.hpp"

#include "GameEngine/Gen1/Utils.hpp"

static unsigned statusAnims[] = {
	PkmnCommon::SYSANIM_NOW_FROZEN,
	PkmnCommon::SYSANIM_NOW_ASLEEP,
	PkmnCommon::SYSANIM_NOW_BURNED,
	PkmnCommon::SYSANIM_NOW_POISONED,
	PkmnCommon::SYSANIM_NOW_PARALYZED,
	PkmnCommon::SYSANIM_NOW_PARALYZED,
	PkmnCommon::SYSANIM_NOW_CONFUSED
};

static void handleMove(
	const PkmnRenderer::GameState &state,
	const PkmnCommon::MoveEvent &event,
	IResultBasedGenerator::State &s,
	std::deque<std::pair<PkmnCommon::Event, IResultBasedGenerator::State>> &usedEvents,
	std::deque<std::pair<PkmnCommon::Event, IResultBasedGenerator::State>> &events,
	std::vector<PkmnCommon::Event> &output
)
{
	const auto &move = PokemonGen1::availableMoves.at(event.moveId);
	std::vector<PkmnCommon::Event> others;
	bool killed = false;
	bool missed = false;
	std::optional<unsigned> meTargetHP;
	std::optional<unsigned> opTargetHP;
	std::vector<PkmnCommon::Event> status;

	others.reserve(events.size());
	while (!events.empty()) {
		auto &e = events.front().first;

		if (auto death = std::get_if<PkmnCommon::DeathEvent>(&e)) {
			if (death->player != event.player)
				killed = true;
		} else if (auto miss = std::get_if<PkmnCommon::MoveMissEvent>(&e)) {
			if (miss->player != event.player)
				break;
			missed = true;
		} else if (auto health = std::get_if<PkmnCommon::HealthModEvent>(&e)) {
			(health->player == event.player ? meTargetHP : opTargetHP) = health->newHealth;
		} else if (auto anim = std::get_if<PkmnCommon::AnimEvent>(&e)) {
			if (std::ranges::find(statusAnims, anim->animId) != std::end(statusAnims))
				status.push_back(e);
			else
				break;
		} else
			break;
		others.push_back(e);
		events.pop_front();
	}

	std::string myName = (event.player ? state.p1 : state.p2).name;
	std::string opName = (event.player ? state.p2 : state.p1).name;

	if (missed) {
		output.emplace_back(PkmnCommon::TextEvent{myName + " used " + Utils::toUpper(move.getName()) + "!"});
		if (move.getID() == PokemonGen1::Whirlwind || move.getID() == PokemonGen1::Roar)
			output.emplace_back(PkmnCommon::TextEvent{opName + " is unaffected!"});
		else if (move.getCategory() == PokemonGen1::STATUS)
			output.emplace_back(PkmnCommon::TextEvent{"But, it failed!"});
		else
			output.emplace_back(PkmnCommon::TextEvent{myName + "'s attack missed!"});
		output.emplace_back(PkmnCommon::MoveMissEvent{move.getID(), event.player});
		return;
	}
	if (move.needsLoading()) {
		throw std::invalid_argument("Unsupported");
	}
}

bool Gen1ResultBasedGenerator::convertEvent(
	const PkmnRenderer::GameState &state,
	std::deque<std::pair<PkmnCommon::Event, State>> &usedEvents,
	std::deque<std::pair<PkmnCommon::Event, State>> &events,
	std::vector<PkmnCommon::Event> &output
)
{
	if (events.empty())
		return false;

	auto val = events.front();
	auto &event = val.first;
	auto &s = val.second;

	usedEvents.push_back(val);
	events.pop_front();
	if (
		std::holds_alternative<PkmnCommon::TextEvent>(event) ||
		std::holds_alternative<PkmnCommon::TurnStartEvent>(event)
	) {
		output.push_back(event);
	} else if (auto switch_ = std::get_if<PkmnCommon::SwitchEvent>(&event)) {
		if (switch_->player)
			output.emplace_back(PkmnCommon::TextEvent{"Go! " + std::string(state.p1.team[switch_->newPkmnId].name) + "!"});
		else
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p2.name) + " sent out " + state.p2.team[switch_->newPkmnId].name + "!"});
		output.emplace_back(*switch_);
	} else if (auto withdraw = std::get_if<PkmnCommon::WithdrawEvent>(&event)) {
		if (withdraw->player)
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p1.team[s.first.onField].name) + " enough! Come back!"});
		else
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p2.name) + " withdrew " + state.p2.team[s.second.onField].name + "!"});
		output.emplace_back(*withdraw);
	} else if (auto move = std::get_if<PkmnCommon::MoveEvent>(&event)) {
		handleMove(state, *move, s, usedEvents, events, output);
	} else if (std::holds_alternative<PkmnCommon::GameStartEvent>(event)) {
		output.emplace_back(PkmnCommon::GameStartEvent{});
		output.emplace_back(PkmnCommon::TextEvent{std::string(state.p1.name) + " wants to fight!"});
		output.emplace_back(PkmnCommon::TextEvent{std::string(state.p2.name) + " sent out " + state.p2.team[0].name + "!"});
		output.emplace_back(PkmnCommon::TextEvent{std::string(state.p1.team[0].name) + " go!"});
	} else {
		throw std::runtime_error("Not implemented");
	}
	return true;
	//PkmnCommon::MoveEvent;
	//PkmnCommon::AnimEvent;
	//PkmnCommon::ExtraAnimEvent;
	//PkmnCommon::HealthModEvent;
	//PkmnCommon::SwitchEvent;
	//PkmnCommon::WithdrawEvent;
	//PkmnCommon::DeathEvent;
	//PkmnCommon::HitEvent;
	//PkmnCommon::StatusClearedEvent;
	//PkmnCommon::TurnStartEvent;
	//PkmnCommon::MoveMissEvent;
	//PkmnCommon::GameEndEvent;
}
