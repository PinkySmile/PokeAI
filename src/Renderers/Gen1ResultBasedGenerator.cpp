//
// Created by PinkySmile on 22/02/2026.
//

#include <stdexcept>
#include "Gen1ResultBasedGenerator.hpp"

#include "GameEngine/Gen1/Utils.hpp"

// AnimIds that represent "now inflicted with status" — consumed by handleMove as side effects.
static const unsigned statusAnims[] = {
	PkmnCommon::SYSANIM_NOW_FROZEN,
	PkmnCommon::SYSANIM_NOW_ASLEEP,
	PkmnCommon::SYSANIM_NOW_BURNED,
	PkmnCommon::SYSANIM_NOW_POISONED,
	PkmnCommon::SYSANIM_NOW_PARALYZED,
	PkmnCommon::SYSANIM_NOW_BADLY_POISONED,
	PkmnCommon::SYSANIM_NOW_CONFUSED,
};

// Stat animation range — SYSANIM_ATK_DECREASE_BIG through SYSANIM_EVD_INCREASE_BIG.
static bool isStatAnim(unsigned animId)
{
	return animId >= PkmnCommon::SYSANIM_ATK_DECREASE_BIG &&
	       animId <= PkmnCommon::SYSANIM_EVD_INCREASE_BIG;
}

static bool isStatusAnim(unsigned animId)
{
	return std::ranges::find(statusAnims, animId) != std::end(statusAnims);
}

// Text for status newly applied.
static std::string statusAppliedText(unsigned animId, const std::string &name)
{
	switch (animId) {
	case PkmnCommon::SYSANIM_NOW_ASLEEP:        return name + " fell asleep!";
	case PkmnCommon::SYSANIM_NOW_FROZEN:        return name + " was frozen solid!";
	case PkmnCommon::SYSANIM_NOW_BURNED:        return name + " was burned!";
	case PkmnCommon::SYSANIM_NOW_POISONED:      return name + " was poisoned!";
	case PkmnCommon::SYSANIM_NOW_BADLY_POISONED:return name + " was badly poisoned!";
	case PkmnCommon::SYSANIM_NOW_PARALYZED:     return name + "'s paralyzed! It may not attack!";
	case PkmnCommon::SYSANIM_NOW_CONFUSED:      return name + " became confused!";
	default:                                    return name + " was affected!";
	}
}

// Text for standalone AnimEvents (cant, cure, etc.).
static std::string standaloneAnimText(unsigned animId, const std::string &name)
{
	switch (animId) {
	case PkmnCommon::SYSANIM_ASLEEP:      return name + " is fast asleep!";
	case PkmnCommon::SYSANIM_FROZEN:      return name + " is frozen solid!";
	case PkmnCommon::SYSANIM_PARALYZED:   return name + " is fully paralyzed!";
	case PkmnCommon::SYSANIM_RECHARGE:    return name + " must recharge!";
	case PkmnCommon::SYSANIM_CONFUSED:    return name + " is confused!";
	case PkmnCommon::SYSANIM_WAKE_UP:     return name + " woke up!";
	case PkmnCommon::SYSANIM_THAWED:      return name + " thawed out!";
	case PkmnCommon::SYSANIM_BACK_TO_SENSE: return name + " snapped out of confusion!";
	case PkmnCommon::SYSANIM_CONFUSED_HIT: return name + " hurt itself in its confusion!";
	case PkmnCommon::SYSANIM_SUB_BREAK:   return name + "'s SUBSTITUTE broke!";
	case PkmnCommon::SYSANIM_POISON:      return name + " is hurt by poison!";
	case PkmnCommon::SYSANIM_BAD_POISON:  return name + " is hurt by poison!";
	case PkmnCommon::SYSANIM_BURN:        return name + " is hurt by the burn!";
	case PkmnCommon::SYSANIM_LEECHED:     return name + " had its energy drained!";
	default:                              return "";
	}
}

// Text for stat change AnimEvents.
static std::string statAnimText(unsigned animId, const std::string &name)
{
	static const char *statNames[] = {
		"ATK", "DEF", "SPE", "",
		"SPD", "ACC", "EVD"
	};
	unsigned offset = animId - PkmnCommon::SYSANIM_ATK_DECREASE_BIG;
	unsigned statIdx = offset / 4;
	unsigned kind    = offset % 4;
	// kind: 0=DEC_BIG, 1=DEC, 2=INC, 3=INC_BIG
	const char *change = "";
	switch (kind) {
	case 0: change = " fell harshly!"; break;
	case 1: change = " fell!";         break;
	case 2: change = " rose!";         break;
	case 3: change = " rose sharply!"; break;
	}
	const char *stat = statIdx < 7 ? statNames[statIdx] : "???";
	return name + "'s " + stat + change;
}

// Loading-turn text for two-turn moves.
static std::string loadingText(unsigned moveId, const std::string &name)
{
	switch (moveId) {
	case PokemonGen1::Fly:        return name + " flew up high!";
	case PokemonGen1::Dig:        return name + " dug a hole!";
	case PokemonGen1::Solarbeam:  return name + " absorbed sunlight!";
	case PokemonGen1::Skull_Bash: return name + " lowered its head!";
	case PokemonGen1::Razor_Wind: return name + " whipped up a whirlwind!";
	case PokemonGen1::Sky_Attack: return name + " is glowing!";
	default:                      return name + " is preparing!";
	}
}

// Whether this AnimEvent type should be followed by consuming an HP event.
static bool animNeedsHP(unsigned animId)
{
	return animId == PkmnCommon::SYSANIM_POISON ||
	       animId == PkmnCommon::SYSANIM_BAD_POISON ||
	       animId == PkmnCommon::SYSANIM_BURN ||
	       animId == PkmnCommon::SYSANIM_LEECHED ||
	       animId == PkmnCommon::SYSANIM_CONFUSED_HIT;
}

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

	// Identify attacker and target names using the state snapshot.
	const auto &myPState = event.player ? s.first : s.second;
	const auto &opPState = event.player ? s.second : s.first;
	const auto &myPlayer = event.player ? state.p1 : state.p2;
	const auto &opPlayer = event.player ? state.p2 : state.p1;
	bool opIsP1 = !event.player;

	std::string myName = myPlayer.team[myPState.onField].name;
	std::string opName = opPlayer.team[opPState.onField].name;

	// ── Look-ahead: consume related events from the queue ─────────────────
	struct HitInfo { bool veryEffective = false; bool notVeryEffective = false; };

	bool missed = false;
	bool hasCrit = false;
	bool hasStatusCleared = false;
	bool statusClearedPlayer = false;
	std::optional<HitInfo> hitInfo;
	std::vector<unsigned> targetHPs;
	std::optional<unsigned> selfHP;
	std::vector<PkmnCommon::AnimEvent> statusEffects;
	std::vector<PkmnCommon::DeathEvent> deaths;
	std::optional<PkmnCommon::ExtraAnimEvent> prepareAnim;

	while (!events.empty()) {
		auto &front = events.front();
		auto &e = front.first;

		if (auto hit = std::get_if<PkmnCommon::HitEvent>(&e)) {
			if (!hitInfo)
				hitInfo = HitInfo{hit->veryEffective, hit->notVeryEffective};
		} else if (auto health = std::get_if<PkmnCommon::HealthModEvent>(&e)) {
			if (health->player == event.player)
				selfHP = health->newHealth;        // recoil / drain / self-heal
			else
				targetHPs.push_back(health->newHealth);
		} else if (auto text = std::get_if<PkmnCommon::TextEvent>(&e)) {
			if (text->message == "Critical hit!")
				hasCrit = true;
			else
				break;
		} else if (auto death = std::get_if<PkmnCommon::DeathEvent>(&e)) {
			deaths.push_back(*death);
		} else if (auto miss = std::get_if<PkmnCommon::MoveMissEvent>(&e)) {
			(void)miss;
			missed = true;
		} else if (auto anim = std::get_if<PkmnCommon::AnimEvent>(&e)) {
			if (isStatusAnim(anim->animId))
				statusEffects.push_back(*anim);
			else
				break;
		} else if (auto sc = std::get_if<PkmnCommon::StatusClearedEvent>(&e)) {
			hasStatusCleared = true;
			statusClearedPlayer = sc->player;
		} else if (auto extra = std::get_if<PkmnCommon::ExtraAnimEvent>(&e)) {
			prepareAnim = *extra;
		} else {
			break;
		}
		usedEvents.push_back(front);
		events.pop_front();
	}

	// ── Emit move announcement ─────────────────────────────────────────────
	output.emplace_back(PkmnCommon::TextEvent{myName + " used " + Utils::toUpper(move.getName()) + "!"});

	// ── Miss / fail / immune ───────────────────────────────────────────────
	if (missed) {
		if (move.getID() == PokemonGen1::Whirlwind || move.getID() == PokemonGen1::Roar)
			output.emplace_back(PkmnCommon::TextEvent{opName + " is unaffected!"});
		else if (move.getCategory() == PokemonGen1::STATUS)
			output.emplace_back(PkmnCommon::TextEvent{"But, it failed!"});
		else
			output.emplace_back(PkmnCommon::TextEvent{myName + "'s attack missed!"});
		output.emplace_back(PkmnCommon::MoveMissEvent{move.getID(), event.player});
		return;
	}

	// ── Two-turn move: loading phase ───────────────────────────────────────
	if (prepareAnim.has_value()) {
		output.emplace_back(PkmnCommon::TextEvent{loadingText(move.getID(), myName)});
		output.emplace_back(*prepareAnim);
		return;
	}

	// ── Damaging move ──────────────────────────────────────────────────────
	if (!targetHPs.empty()) {
		HitInfo hi = hitInfo.value_or(HitInfo{});
		bool hideSub = false;

		// First hit
		output.emplace_back(PkmnCommon::MoveEvent{move.getID(), event.player, hideSub});
		output.emplace_back(PkmnCommon::HitEvent{hi.veryEffective, hi.notVeryEffective, opIsP1, true});
		output.emplace_back(PkmnCommon::HealthModEvent{targetHPs[0], opIsP1, true});
		if (hasCrit)
			output.emplace_back(PkmnCommon::TextEvent{"Critical hit!"});
		if (hi.notVeryEffective)
			output.emplace_back(PkmnCommon::TextEvent{"It's not very effective..."});
		else if (hi.veryEffective)
			output.emplace_back(PkmnCommon::TextEvent{"It's super effective!"});

		// Subsequent hits (multi-hit moves)
		for (size_t i = 1; i < targetHPs.size(); i++) {
			output.emplace_back(PkmnCommon::MoveEvent{move.getID(), event.player, true});
			output.emplace_back(PkmnCommon::HitEvent{hi.veryEffective, hi.notVeryEffective, opIsP1, true});
			output.emplace_back(PkmnCommon::HealthModEvent{targetHPs[i], opIsP1, true});
		}

		// Recoil / drain HP for attacker
		if (selfHP.has_value())
			output.emplace_back(PkmnCommon::HealthModEvent{selfHP.value(), event.player, true});

	} else if (selfHP.has_value()) {
		// Self-targeting move: Recover, Soft-Boiled, Rest, Substitute, etc.
		output.emplace_back(PkmnCommon::MoveEvent{move.getID(), event.player, false});

		unsigned mid = move.getID();
		if (mid == PokemonGen1::Rest) {
			output.emplace_back(PkmnCommon::TextEvent{myName + " slept and became healthy!"});
		} else if (mid == PokemonGen1::Substitute) {
			output.emplace_back(PkmnCommon::TextEvent{myName + " made a SUBSTITUTE!"});
		}
		output.emplace_back(PkmnCommon::HealthModEvent{selfHP.value(), event.player, true});
	} else {
		// No damage, no self-HP: status/effect move (Swords Dance, Agility, Growl, etc.)
		output.emplace_back(PkmnCommon::MoveEvent{move.getID(), event.player, false});
	}

	// ── Status side effects inflicted by the move ──────────────────────────
	for (auto &anim : statusEffects) {
		std::string afflictedName = anim.player
			? state.p1.team[s.first.onField].name
			: state.p2.team[s.second.onField].name;

		output.emplace_back(PkmnCommon::TextEvent{statusAppliedText(anim.animId, afflictedName)});
		output.emplace_back(PkmnCommon::AnimEvent{anim.animId, move.getStatusChange().cmpVal == 0, anim.player, anim.turn});
	}

	// ── Haze / stat clear ─────────────────────────────────────────────────
	if (hasStatusCleared) {
		output.emplace_back(PkmnCommon::StatusClearedEvent{statusClearedPlayer});
		output.emplace_back(PkmnCommon::TextEvent{"All stat changes were eliminated!"});
	}

	// ── Faints ────────────────────────────────────────────────────────────
	for (auto &death : deaths) {
		std::string faintedName = death.player
			? state.p1.team[s.first.onField].name
			: state.p2.team[s.second.onField].name;
		output.emplace_back(PkmnCommon::TextEvent{faintedName + " fainted!"});
		output.emplace_back(death);
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

	auto val   = events.front();
	auto &event = val.first;
	auto &s     = val.second;

	usedEvents.push_back(val);
	events.pop_front();

	// ── Pass-through ──────────────────────────────────────────────────────
	if (std::holds_alternative<PkmnCommon::TextEvent>(event) ||
	    std::holds_alternative<PkmnCommon::TurnStartEvent>(event)) {
		output.push_back(event);

	// ── Switch ────────────────────────────────────────────────────────────
	} else if (auto switch_ = std::get_if<PkmnCommon::SwitchEvent>(&event)) {
		if (switch_->player)
			output.emplace_back(PkmnCommon::TextEvent{"Go! " + std::string(state.p1.team[switch_->newPkmnId].name) + "!"});
		else
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p2.name) + " sent out " + state.p2.team[switch_->newPkmnId].name + "!"});
		output.emplace_back(*switch_);

	// ── Withdraw ──────────────────────────────────────────────────────────
	} else if (auto withdraw = std::get_if<PkmnCommon::WithdrawEvent>(&event)) {
		if (withdraw->player)
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p1.team[s.first.onField].name) + ", enough! Come back!"});
		else
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p2.name) + " withdrew " + state.p2.team[s.second.onField].name + "!"});
		output.emplace_back(*withdraw);

	// ── Move ──────────────────────────────────────────────────────────────
	} else if (auto move = std::get_if<PkmnCommon::MoveEvent>(&event)) {
		handleMove(state, *move, s, usedEvents, events, output);

	// ── Game start ────────────────────────────────────────────────────────
	} else if (std::holds_alternative<PkmnCommon::GameStartEvent>(event)) {
		output.emplace_back(PkmnCommon::GameStartEvent{});
		output.emplace_back(PkmnCommon::TextEvent{std::string(state.p1.name) + " wants to fight!"});
		output.emplace_back(PkmnCommon::TextEvent{std::string(state.p2.name) + " sent out " + state.p2.team[0].name + "!"});
		output.emplace_back(PkmnCommon::TextEvent{std::string(state.p1.team[0].name) + " go!"});

	// ── Game end ──────────────────────────────────────────────────────────
	} else if (auto end = std::get_if<PkmnCommon::GameEndEvent>(&event)) {
		output.emplace_back(*end);
		if (end->p1Won)
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p1.name) + " won the battle!"});
		else if (end->p2Won)
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p2.name) + " won the battle!"});

	// ── Standalone death (Pokemon fainted outside of a move context) ──────
	} else if (auto death = std::get_if<PkmnCommon::DeathEvent>(&event)) {
		std::string faintedName = death->player
			? state.p1.team[s.first.onField].name
			: state.p2.team[s.second.onField].name;
		output.emplace_back(PkmnCommon::TextEvent{faintedName + " fainted!"});
		output.emplace_back(*death);

	// ── Standalone AnimEvent ──────────────────────────────────────────────
	} else if (auto anim = std::get_if<PkmnCommon::AnimEvent>(&event)) {
		std::string pkmnName = anim->player
			? state.p1.team[s.first.onField].name
			: state.p2.team[s.second.onField].name;

		if (isStatAnim(anim->animId)) {
			output.emplace_back(PkmnCommon::TextEvent{statAnimText(anim->animId, pkmnName)});
			output.emplace_back(*anim);
		} else {
			std::string text = standaloneAnimText(anim->animId, pkmnName);
			if (!text.empty())
				output.emplace_back(PkmnCommon::TextEvent{text});
			output.emplace_back(*anim);
			// Consume a following HealthModEvent for damage-dealing standalone anims
			if (animNeedsHP(anim->animId) && !events.empty()) {
				if (auto hp = std::get_if<PkmnCommon::HealthModEvent>(&events.front().first)) {
					output.emplace_back(*hp);
					usedEvents.push_back(events.front());
					events.pop_front();
				}
			}
		}

	// ── StatusClearedEvent (e.g. from Haze, standalone) ───────────────────
	} else if (auto sc = std::get_if<PkmnCommon::StatusClearedEvent>(&event)) {
		output.emplace_back(*sc);
		output.emplace_back(PkmnCommon::TextEvent{"All stat changes were eliminated!"});

	// ── Standalone HealthModEvent (rare: burn/leech damage without anim) ────
	} else if (auto hp = std::get_if<PkmnCommon::HealthModEvent>(&event)) {
		output.emplace_back(*hp);

	} else {
		throw std::runtime_error("Gen1ResultBasedGenerator: unhandled event type");
	}
	return true;
}
