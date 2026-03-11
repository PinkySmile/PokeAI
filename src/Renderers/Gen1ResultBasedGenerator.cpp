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

static bool isStatAnim(unsigned animId)
{
	return animId >= PkmnCommon::SYSANIM_ATK_DECREASE_BIG &&
	       animId <= PkmnCommon::SYSANIM_EVD_INCREASE_BIG;
}

static bool isStatusAnim(unsigned animId)
{
	return std::ranges::find(statusAnims, animId) != std::end(statusAnims);
}

// Text emitted when a status condition is newly inflicted (from Pokemon.cpp messages map).
static std::string statusAppliedText(unsigned animId, const std::string &name)
{
	switch (animId) {
	case PkmnCommon::SYSANIM_NOW_ASLEEP:         return name + " fell asleep!";
	case PkmnCommon::SYSANIM_NOW_FROZEN:         return name + " was frozen solid!";
	case PkmnCommon::SYSANIM_NOW_BURNED:         return name + " was burned!";
	case PkmnCommon::SYSANIM_NOW_POISONED:       return name + " was poisoned!";
	case PkmnCommon::SYSANIM_NOW_BADLY_POISONED: return name + "'s badly poisoned!";
	case PkmnCommon::SYSANIM_NOW_PARALYZED:      return name + "'s paralyzed! It may not attack!";
	case PkmnCommon::SYSANIM_NOW_CONFUSED:       return name + " became confused!";
	default:                                     return name + " was affected!";
	}
}

// Text for standalone AnimEvents (cant, cure, end-of-turn damage, etc.)
// from Pokemon.cpp attack() / stepEnds().
static std::string standaloneAnimText(unsigned animId, const std::string &name)
{
	switch (animId) {
	case PkmnCommon::SYSANIM_ASLEEP:       return name + " is fast asleep!";
	case PkmnCommon::SYSANIM_FROZEN:       return name + " is frozen solid!";
	case PkmnCommon::SYSANIM_PARALYZED:    return name + "'s fully paralyzed!";
	case PkmnCommon::SYSANIM_RECHARGE:     return name + " must recharge!";
	case PkmnCommon::SYSANIM_CONFUSED:     return name + " is confused!";
	case PkmnCommon::SYSANIM_WAKE_UP:      return name + " woke up!";
	case PkmnCommon::SYSANIM_THAWED:       return "Fire defrosted " + name + "!";
	case PkmnCommon::SYSANIM_BACK_TO_SENSE:return name + " is confused no more!";
	case PkmnCommon::SYSANIM_CONFUSED_HIT: return "It hurt itself in its confusion!";
	case PkmnCommon::SYSANIM_SUB_BREAK:    return name + "'s SUBSTITUTE broke!";
	case PkmnCommon::SYSANIM_POISON:       return name + "'s hurt by the poison!";
	case PkmnCommon::SYSANIM_BAD_POISON:   return name + "'s hurt by the poison!";
	case PkmnCommon::SYSANIM_BURN:         return name + "'s hurt by the burn!";
	case PkmnCommon::SYSANIM_LEECHED:      return "LEECH SEED saps " + name + "!";
	default:                               return "";
	}
}

// Stat name (little string) and change text for stat-change AnimEvents,
// mirroring statToLittleString() from StatsChange.cpp and Pokemon::upStat().
//
// SYSANIM group layout (each group = 4 consecutive IDs starting at base):
//   base+0 = DECREASE_BIG, base+1 = DECREASE, base+2 = INCREASE, base+3 = INCREASE_BIG
//
// Group → Gen-1 stat → statToLittleString():
//   0 ATK  → ATK
//   1 DEF  → DEF
//   2 SPA  → SPE  (Gen-1 "Special" masqueraded as SPA anim, statToLittleString(STATS_SPE)="SPE")
//   3 SPD  → SPD  (unused in Gen 1)
//   4 SPE  → SPD  (Gen-1 Speed uses SPE anim,  statToLittleString(STATS_SPD)="SPD")
//   5 ACC  → ACC
//   6 EVD  → EVD
static std::string statAnimText(unsigned animId, const std::string &name)
{
	static const char *statNames[] = {
		"ATTACK", "DEFENSE", "SPECIAL", "", "SPEED", "ACCURACY", "EVADE"
	};
	unsigned offset  = animId - PkmnCommon::SYSANIM_ATK_DECREASE_BIG;
	unsigned group   = offset / 4;
	unsigned kind    = offset % 4;

	const char *stat   = group < 7 ? statNames[group] : "???";
	const char *change = "";
	switch (kind) {
	case 0: change = " greatly fell!";  break;
	case 1: change = " fell!";          break;
	case 2: change = " rose!";          break;
	case 3: change = " greatly rose!";  break;
	}
	return name + "'s " + stat + change;
}

// Loading-turn text for two-turn moves — mirrors Move.cpp:
//   logger(TextEvent{ owner.getName() + " " + this->_loadingMsg });
static std::string loadingText(unsigned moveId, const std::string &name)
{
	switch (moveId) {
	case PokemonGen1::Fly:        return name + " flew up high!";
	case PokemonGen1::Dig:        return name + " dug a hole!";
	case PokemonGen1::Solarbeam:  return name + " took in sunlight!";
	case PokemonGen1::Skull_Bash: return name + " lowered its head!";
	case PokemonGen1::Razor_Wind: return name + " made a whirlwind!";
	case PokemonGen1::Sky_Attack: return name + " is glowing!";
	default:                      return name + " is preparing!";
	}
}

// Whether this standalone AnimEvent type is always followed by a HealthModEvent.
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

	// Resolve attacker / target names from the state snapshot.
	const auto &myPState = event.player ? s.first  : s.second;
	const auto &opPState = event.player ? s.second : s.first;
	const auto &myPlayer = event.player ? state.p1 : state.p2;
	const auto &opPlayer = event.player ? state.p2 : state.p1;
	bool opIsP1 = !event.player;

	std::string myName = myPlayer.team[myPState.onField].name;
	std::string opName = opPlayer.team[opPState.onField].name;

	if (event.player)
		opName = "Enemy " + opName;
	else
		myName = "Enemy " + myName;

	// ── Look-ahead: consume related events ────────────────────────────────
	struct HitInfo { bool veryEffective = false; bool notVeryEffective = false; };

	std::vector<std::string> subDmg;
	bool missed = false;
	bool hasCrit = false;
	bool hasStatusCleared = false;
	bool statusClearedPlayer = false;
	std::optional<HitInfo> hitInfo;
	std::vector<unsigned> targetHPs;
	std::optional<unsigned> selfHP;
	std::vector<PkmnCommon::AnimEvent> statsEffects;
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
				selfHP = health->newHealth;
			else
				targetHPs.push_back(health->newHealth);
		} else if (auto text = std::get_if<PkmnCommon::TextEvent>(&e)) {
			if (text->message == "Critical hit!")
				hasCrit = true;
			else if (text->message.starts_with("The SUBSTITUTE took damage"))
				subDmg.push_back(text->message);
			else
				break;
		} else if (auto death = std::get_if<PkmnCommon::DeathEvent>(&e)) {
			deaths.push_back(*death);
		} else if (std::get_if<PkmnCommon::MoveMissEvent>(&e)) {
			missed = true;
		} else if (auto anim = std::get_if<PkmnCommon::AnimEvent>(&e)) {
			if (isStatusAnim(anim->animId))
				statusEffects.push_back(*anim);
			else if (isStatAnim(anim->animId))
				statsEffects.push_back(*anim);
			else
				break;
		} else if (auto sc = std::get_if<PkmnCommon::StatusClearedEvent>(&e)) {
			hasStatusCleared = true;
			statusClearedPlayer = sc->player;
		} else if (auto extra = std::get_if<PkmnCommon::ExtraAnimEvent>(&e)) {
			prepareAnim = *extra;
		} else
			break;
		usedEvents.push_back(front);
		events.pop_front();
	}

	// ── Move announcement (Move.cpp line 811) ─────────────────────────────
	if (event.isContinuation)
		output.emplace_back(PkmnCommon::TextEvent{myName + "'s attack continues!"});
	else
		output.emplace_back(PkmnCommon::TextEvent{myName + " used " + Utils::toUpper(move.getName()) + "!"});

	// ── Miss / fail / immune ───────────────────────────────────────────────
	if (missed) {
		// Move.cpp lines 779-786
		if (move.getID() == PokemonGen1::Whirlwind || move.getID() == PokemonGen1::Roar)
			output.emplace_back(PkmnCommon::TextEvent{opName + " is unaffected!"});
		else if (move.getCategory() == PokemonGen1::STATUS)
			output.emplace_back(PkmnCommon::TextEvent{"But, it failed!"});
		else
			output.emplace_back(PkmnCommon::TextEvent{myName + "'s attack missed!"});
		output.emplace_back(PkmnCommon::MoveMissEvent{PokemonGen1::moveToCommon(move.getID()), event.player});
		return;
	}

	// ── Two-turn move: loading phase (Move.cpp line 804-808) ──────────────
	if (prepareAnim.has_value()) {
		output.emplace_back(*prepareAnim);
		output.emplace_back(PkmnCommon::TextEvent{loadingText(move.getID(), myName)});
		return;
	}

	// ── Damaging move ──────────────────────────────────────────────────────
	if (!targetHPs.empty() || !subDmg.empty()) {
		HitInfo hi = hitInfo.value_or(HitInfo{});

		// First hit
		if (move.getID() == PkmnCommon::Explosion || move.getID() == PkmnCommon::Self_Destruct) {
			output.emplace_back(PkmnCommon::MoveEvent{.moveId = PokemonGen1::moveToCommon(move.getID()), .player = event.player, .hideSubstitute = true});
			output.emplace_back(PkmnCommon::HitEvent{.veryEffective = hi.veryEffective, .notVeryEffective = hi.notVeryEffective, .player = opIsP1, .hasEffect = true});
			output.emplace_back(PkmnCommon::ExtraAnimEvent{.moveId = PokemonGen1::moveToCommon(move.getID()), .index = 0, .player = event.player});
			output.emplace_back(PkmnCommon::HitEvent{hi.veryEffective, hi.notVeryEffective, opIsP1, true});
		} else {
			bool hasEffect = !move.getFoeChange().empty() ||
				!move.getOwnerChange().empty() ||
				move.getNbHits().first != 1 ||
				move.getNbHits().second != 1 ||
				move.getNbRuns().first != 1 ||
				move.getNbRuns().second != 1 ||
				move.getStatusChange().status != PokemonGen1::STATUS_NONE ||
				!move.getHitCallBackDescription().empty() ||
				!move.getMissCallBackDescription().empty();

			output.emplace_back(PkmnCommon::MoveEvent{PokemonGen1::moveToCommon(move.getID()), event.player, true});
			output.emplace_back(PkmnCommon::HitEvent{hi.veryEffective, hi.notVeryEffective, opIsP1, hasEffect});
		}

		if (!subDmg.empty()) {
			output.emplace_back(PkmnCommon::TextEvent{subDmg[0]});
			subDmg.pop_back();
		} else {
			output.emplace_back(PkmnCommon::HealthModEvent{targetHPs[0], opIsP1, true});
			targetHPs.pop_back();
		}

		if (hasCrit)
			output.emplace_back(PkmnCommon::TextEvent{"Critical hit!"});
		if (hi.notVeryEffective)
			output.emplace_back(PkmnCommon::TextEvent{"It's not very effective!"});
		else if (hi.veryEffective)
			output.emplace_back(PkmnCommon::TextEvent{"It's super effective!"});

		// Subsequent hits (multi-hit moves)
		while (!targetHPs.empty()) {
			output.emplace_back(PkmnCommon::MoveEvent{PokemonGen1::moveToCommon(move.getID()), event.player, true});
			output.emplace_back(PkmnCommon::HitEvent{hi.veryEffective, hi.notVeryEffective, opIsP1, true});
			if (!subDmg.empty()) {
				output.emplace_back(PkmnCommon::TextEvent{subDmg[0]});
				subDmg.pop_back();
			} else {
				output.emplace_back(PkmnCommon::HealthModEvent{targetHPs[0], opIsP1, true});
				targetHPs.pop_back();
			}
		}

		// Recoil / drain HP for the attacker (Move.cpp line 169/279)
		if (selfHP.has_value())
			output.emplace_back(PkmnCommon::HealthModEvent{selfHP.value(), event.player, true});

	} else if (selfHP.has_value()) {
		// Self-targeting: Recover, Soft-Boiled, Rest, Substitute HP cost, etc.
		output.emplace_back(PkmnCommon::MoveEvent{PokemonGen1::moveToCommon(move.getID()), event.player, false});

		unsigned mid = move.getID();
		if (mid == PokemonGen1::Rest) {
			// Move.cpp lines 286 + 290
			output.emplace_back(PkmnCommon::TextEvent{myName + " started sleeping!"});
			output.emplace_back(PkmnCommon::HealthModEvent{selfHP.value(), event.player, true});
			output.emplace_back(PkmnCommon::TextEvent{myName + " regained health!"});
		} else if (mid == PokemonGen1::Substitute) {
			// Move.cpp line 388
			output.emplace_back(PkmnCommon::HealthModEvent{selfHP.value(), event.player, true});
			output.emplace_back(PkmnCommon::TextEvent{"It created a SUBSTITUTE!"});
		} else {
			// Recover, Soft-Boiled, Dream Eater heal, etc. (Move.cpp line 279)
			output.emplace_back(PkmnCommon::HealthModEvent{selfHP.value(), event.player, true});
			output.emplace_back(PkmnCommon::TextEvent{myName + " regained health!"});
		}
	} else {
		// No damage, no self-HP: pure status/effect move (Swords Dance, Agility, etc.)
		output.emplace_back(PkmnCommon::MoveEvent{PokemonGen1::moveToCommon(move.getID()), event.player, false});
		if (move.getID() == PkmnCommon::Transform)
			output.emplace_back(PkmnCommon::TextEvent{myName + " transformed into " + PokemonGen1::pokemonList.at(opPlayer.team[opPState.onField].id).name + "!"});
	}

	// ── Status side effects inflicted by the move ──────────────────────────
	for (auto &anim : statusEffects) {
		std::string afflictedName = anim.player
			? state.p1.team[s.first.onField].name
			: std::string("Enemy ") + state.p2.team[s.second.onField].name;
		output.emplace_back(PkmnCommon::AnimEvent{anim.animId, move.getStatusChange().cmpVal == 0, anim.player, anim.turn});
		output.emplace_back(PkmnCommon::TextEvent{statusAppliedText(anim.animId, afflictedName)});
	}
	for (auto &anim : statsEffects) {
		if (anim.animId >= PkmnCommon::SYSANIM_SPD_DECREASE_BIG && anim.animId <= PkmnCommon::SYSANIM_SPD_INCREASE_BIG)
			continue;
		std::string afflictedName = anim.player
			? state.p1.team[s.first.onField].name
			: std::string("Enemy ") + state.p2.team[s.second.onField].name;

		output.emplace_back(PkmnCommon::AnimEvent{anim.animId, move.getCategory() == PokemonGen1::STATUS, anim.player, anim.turn});
		output.emplace_back(PkmnCommon::TextEvent{statAnimText(anim.animId, afflictedName)});
	}

	// ── Haze / full stat clear (Move.cpp line 316) ────────────────────────
	if (hasStatusCleared) {
		output.emplace_back(PkmnCommon::StatusClearedEvent{statusClearedPlayer});
		output.emplace_back(PkmnCommon::TextEvent{"All STATUS changes are eliminated!"});
	}

	// ── Faints (Pokemon.cpp line 976) ─────────────────────────────────────
	for (auto &death : deaths) {
		std::string faintedName = death.player
			? state.p1.team[s.first.onField].name
			: std::string("Enemy ") + state.p2.team[s.second.onField].name;

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

	auto val = events.front();
	auto &event = val.first;
	auto &s = val.second;

	usedEvents.push_back(val);
	events.pop_front();

	// ── Pass-through ──────────────────────────────────────────────────────
	if (std::holds_alternative<PkmnCommon::TextEvent>(event) ||
	    std::holds_alternative<PkmnCommon::TurnStartEvent>(event)) {
		output.push_back(event);

	// ── Switch ────────────────────────────────────────────────────────────
	} else if (auto switch_ = std::get_if<PkmnCommon::SwitchEvent>(&event)) {
		// BattleHandler.cpp lines 167/208
		if (switch_->player)
			output.emplace_back(PkmnCommon::TextEvent{"Go! " + std::string(state.p1.team[switch_->newPkmnId].name) + "!"});
		else
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p2.name) + " sent out " + state.p2.team[switch_->newPkmnId].name + "!"});
		output.emplace_back(*switch_);

	// ── Withdraw ──────────────────────────────────────────────────────────
	} else if (auto withdraw = std::get_if<PkmnCommon::WithdrawEvent>(&event)) {
		// BattleHandler.cpp lines 163/204
		if (withdraw->player)
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p1.team[s.first.onField].name) + " enough! Come back!"});
		else
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p2.name) + " withdrew " + state.p2.team[s.second.onField].name + "!"});
		output.emplace_back(*withdraw);

	// ── Move ──────────────────────────────────────────────────────────────
	} else if (auto move = std::get_if<PkmnCommon::MoveEvent>(&event)) {
		handleMove(state, *move, s, usedEvents, events, output);

	// ── Game start (BattleHandler.cpp lines 34-36) ────────────────────────
	} else if (std::holds_alternative<PkmnCommon::GameStartEvent>(event)) {
		output.emplace_back(PkmnCommon::GameStartEvent{});
		output.emplace_back(PkmnCommon::TextEvent{std::string(state.p2.name) + " wants to fight!"});
		output.emplace_back(PkmnCommon::TextEvent{std::string(state.p2.name) + " sent out " + state.p2.team[0].name + "!"});
		output.emplace_back(PkmnCommon::TextEvent{std::string(state.p1.team[0].name) + " go!"});

	// ── Game end (BattleHandler.cpp lines 130-131/137) ────────────────────
	} else if (auto end = std::get_if<PkmnCommon::GameEndEvent>(&event)) {
		output.emplace_back(*end);
		if (end->p1Ran)
			output.emplace_back(PkmnCommon::TextEvent{"Got away safely!"});
		else if (end->p2Ran)
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p2.name) + " ran!"});
		else if (end->p1Won)
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p1.name) + " defeated " + state.p2.name + "!"});
		else if (end->p2Won) {
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p1.name) + " is out of usable pokemon!"});
			output.emplace_back(PkmnCommon::TextEvent{std::string(state.p1.name) + " blacked out!"});
		}

	// ── Standalone death (Pokemon.cpp line 976) ───────────────────────────
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

		if (anim->animId >= PkmnCommon::SYSANIM_SPD_DECREASE_BIG && anim->animId <= PkmnCommon::SYSANIM_SPD_INCREASE_BIG)
			return true;
		if (isStatAnim(anim->animId)) {
			output.emplace_back(PkmnCommon::TextEvent{statAnimText(anim->animId, pkmnName)});
			output.emplace_back(*anim);
		} else {
			std::string text = standaloneAnimText(anim->animId, pkmnName);

			if (!text.empty())
				output.emplace_back(PkmnCommon::TextEvent{text});
			output.emplace_back(*anim);
			// Consume the following HealthModEvent for damage-dealing standalone anims.
			if (animNeedsHP(anim->animId) && !events.empty()) {
				if (auto hp = std::get_if<PkmnCommon::HealthModEvent>(&events.front().first)) {
					output.emplace_back(*hp);
					usedEvents.push_back(events.front());
					events.pop_front();
				}
			}
		}

	// ── StatusClearedEvent (Haze, Move.cpp line 316) ──────────────────────
	} else if (auto sc = std::get_if<PkmnCommon::StatusClearedEvent>(&event)) {
		output.emplace_back(*sc);
		output.emplace_back(PkmnCommon::TextEvent{"All STATUS changes are eliminated!"});

	// ── Standalone HealthModEvent (safety pass-through) ───────────────────
	} else if (auto hp = std::get_if<PkmnCommon::HealthModEvent>(&event)) {
		output.emplace_back(*hp);

	// ── Standalone MoveMissEvent (e.g. Gen 1 stat-overflow fail after a boost) ──
	} else if (auto miss = std::get_if<PkmnCommon::MoveMissEvent>(&event)) {
		output.emplace_back(PkmnCommon::TextEvent{"But, it failed!"});
		output.emplace_back(*miss);

	} else {
		throw std::runtime_error("Gen1ResultBasedGenerator: unhandled event type");
	}
	return true;
}
