import math
from typing import Callable
from numpy.random import Generator
from PokeBattle.Gen1.State import PlayerState, BattleAction
from PokeBattle.Gen1.Move import MoveCategory, AvailableMove
from PokeBattle.Gen1.StatusChange import StatusChange
from PokeBattle.Gen1.Type import get_attack_damage_multiplier, Type


ATTACK1 = 0
ATTACK2 = 1
ATTACK3 = 2
ATTACK4 = 3
SWITCH1 = 4
SWITCH2 = 5
SWITCH3 = 6
SWITCH4 = 7
SWITCH5 = 8
SWITCH6 = 9
NOACTION = 10
STRUGGLE = 11

result_index_to_action_array = [
	BattleAction.Attack1,
	BattleAction.Attack2,
	BattleAction.Attack3,
	BattleAction.Attack4,
	BattleAction.Switch1,
	BattleAction.Switch2,
	BattleAction.Switch3,
	BattleAction.Switch4,
	BattleAction.Switch5,
	BattleAction.Switch6,
	BattleAction.NoAction,
	BattleAction.StruggleMove
]

Layer = Callable[[PlayerState, PlayerState, Generator], list[float]]
BiasedLayer = tuple[float, Layer]


# ─── StatsChange key mapping ─────────────────────────────────────────────────
# Lazily initialised on first use; maps StatsChange enum int → stats_upgrade_stages key.
_STAT_TO_KEY: dict[int, str] = {}


def _init_stat_keys() -> None:
	global _STAT_TO_KEY
	try:
		from PokeBattle.Gen1.StatsChange import StatsChange
		for attr, key in [
			('Atk', 'ATK'), ('Def', 'DEF'), ('Spd', 'SPD'),
			('Spe', 'SPE'), ('Evd', 'EVD'), ('Acc', 'ACC'),
		]:
			if hasattr(StatsChange, attr):
				_STAT_TO_KEY[int(getattr(StatsChange, attr))] = key
	except ImportError:
		# Fallback: assume enum values follow declaration order
		_STAT_TO_KEY = {0: 'ATK', 1: 'DEF', 2: 'SPD', 3: 'SPE', 4: 'EVD', 5: 'ACC'}


def _get_stat_key(stat_val) -> str | None:
	if not _STAT_TO_KEY:
		_init_stat_keys()
	return _STAT_TO_KEY.get(int(stat_val))


# ─── Trapping move IDs ────────────────────────────────────────────────────────
_TRAPPING_MOVE_IDS = frozenset([
	int(AvailableMove.Wrap),
	int(AvailableMove.Bind),
	int(AvailableMove.Clamp),
	int(AvailableMove.Fire_Spin),
])

# ─── Status score tables ──────────────────────────────────────────────────────
_NV_STATUS_SCORES: dict[int, float] = {
	int(StatusChange.Frozen):          0.65,
	int(StatusChange.Asleep):          0.60,
	int(StatusChange.Paralyzed):       0.45,
	int(StatusChange.Badly_poisoned):  0.40,
	int(StatusChange.Burned):          0.30,
	int(StatusChange.Poisoned):        0.15,
}
_VOLATILE_STATUS_SCORES: dict[int, float] = {
	int(StatusChange.Confused): 0.20,
	int(StatusChange.Leeched):  0.12,
}


# ═══════════════════════════════════════════════════════════════════════════════
# Helper functions
# ═══════════════════════════════════════════════════════════════════════════════

def _is_stab(move, pokemon) -> bool:
	types = pokemon.types
	return move.type == types[0] or move.type == types[1]


def _estimate_damage(attacker, defender, move, critical: bool = False) -> tuple[int, int]:
	"""
	Returns (min_dmg, max_dmg) using the authentic Gen 1 damage formula.
	Returns (0, 0) for status/zero-power moves or immune type matchups.
	"""
	if move.category == MoveCategory.Status or move.power == 0:
		return (0, 0)

	effectiveness = get_attack_damage_multiplier(move.type, defender.types)
	if effectiveness == 0.0:
		return (0, 0)

	level = attacker.level * (2 if critical else 1)

	if move.category == MoveCategory.Physical:
		atk  = attacker.raw_attack   if critical else attacker.attack
		defd = defender.raw_defense  if critical else defender.defense
	else:  # SPECIAL
		atk  = attacker.raw_special  if critical else attacker.special
		defd = defender.raw_special  if critical else defender.special

	# Gen 1 overflow: when either stat exceeds 255, wrap them
	if atk > 255 or defd > 255:
		atk  = (atk  // 4) % 256
		defd = (defd // 4) % 256

	# Explosion / Self-Destruct halve the defender's defense before calculation
	if move.id in (AvailableMove.Explosion, AvailableMove.Self_Destruct):
		defd //= 2

	defd = max(1, defd)

	base = max(1, min(997, math.floor(
		math.floor(math.floor(2 * level / 5 + 2) * atk * move.power / defd) / 50
	)) + 2)

	stab   = 1.5 if _is_stab(move, attacker) else 1.0
	damage = base * stab * effectiveness

	return (int(damage * 217 / 255), int(damage))


def _crit_chance(attacker, move) -> float:
	"""Gen 1 critical-hit probability. move.crit_chance is a stage multiplier (1 = normal, 8 = high-crit)."""
	base_spd = attacker.base_stats.get('SPD', attacker.raw_speed)
	return min(1.0, base_spd / 2.0 * move.crit_chance / 255.0)


def _hit_accuracy(move, attacker, defender) -> float:
	"""
	Effective hit probability (0–1) accounting for accuracy/evasion stages and
	semi-invulnerability from Dig/Fly, mirroring the Gen 1 AIHeuristic logic.
	"""
	if move.accuracy > 100:
		return 1.0

	raw = min(255.0, move.accuracy * 2.55 * attacker.accuracy_mul * defender.evasion_mul)

	if not defender.can_get_hit:
		if defender.speed > attacker.speed:
			# Defender goes first and is underground/airborne – we miss
			return 0.0
		elif defender.speed == attacker.speed:
			# Speed tie during invulnerable state: halved
			return raw / 256.0 / 2.0
		# We outspeed: treat as a normal hit on an exposed target
	return raw / 256.0


def _expected_damage(attacker, defender, move) -> float:
	"""Expected raw HP damage, weighting over the crit/non-crit split and accuracy."""
	if move.category == MoveCategory.Status or move.power == 0:
		return 0.0

	crit = _crit_chance(attacker, move)
	acc  = _hit_accuracy(move, attacker, defender)
	min_nc, max_nc = _estimate_damage(attacker, defender, move, critical=False)
	min_c,  max_c  = _estimate_damage(attacker, defender, move, critical=True)
	avg_nc = (min_nc + max_nc) / 2.0
	avg_c  = (min_c  + max_c)  / 2.0
	return acc * (avg_c * crit + avg_nc * (1.0 - crit))


def _ko_probability(attacker, defender, move) -> float:
	"""Probability (0–1) that this single move KOs the defender this turn."""
	if move.category == MoveCategory.Status or move.power == 0:
		return 0.0

	hp = defender.health
	if hp <= 0:
		return 0.0

	# Moves directed at a substitute target the substitute's HP instead
	sub = defender.substitute
	effective_hp = sub if (sub is not None and sub > 0) else hp

	crit = _crit_chance(attacker, move)
	acc  = _hit_accuracy(move, attacker, defender)
	min_nc, max_nc = _estimate_damage(attacker, defender, move, critical=False)
	min_c,  max_c  = _estimate_damage(attacker, defender, move, critical=True)

	def _prob(lo: int, hi: int) -> float:
		if hi <= 0 or effective_hp <= 0:
			return 0.0
		if lo >= effective_hp:
			return 1.0
		if hi <= effective_hp:
			return 0.0
		# Damage rolls are roughly uniform between lo and hi
		return (hi - effective_hp) / max(1, hi - lo)

	return acc * (_prob(min_c, max_c) * crit + _prob(min_nc, max_nc) * (1.0 - crit))


def _best_ko_prob(attacker, defender) -> float:
	"""Best single-move KO probability the attacker has against the defender."""
	best = 0.0
	for move in attacker.move_set:
		if move.id == 0 or move.pp == 0:
			continue
		best = max(best, _ko_probability(attacker, defender, move))
	return best


def _best_damage_fraction(attacker, defender) -> float:
	"""Best expected damage as a fraction of the defender's max HP."""
	max_hp = max(1, defender.max_health)
	best   = 0.0
	for move in attacker.move_set:
		if move.id == 0 or move.pp == 0:
			continue
		best = max(best, _expected_damage(attacker, defender, move) / max_hp)
	return best


def _best_type_effectiveness(attacker, defender) -> float:
	"""
	Best (effectiveness × STAB) the attacker can achieve against the defender,
	computed from known moves plus a type-based STAB estimate as a fallback.
	"""
	best = 0.0
	has_attacking_move = False
	for m in attacker.move_set:
		if m.id == 0 or m.power == 0:
			continue
		has_attacking_move = True
		eff  = get_attack_damage_multiplier(m.type, defender.types)
		stab = 1.5 if _is_stab(m, attacker) else 1.0
		best = max(best, eff * stab)
	if not has_attacking_move:
		# Estimate from the attacker's types (assumes it can get STAB coverage)
		for t in attacker.types:
			eff  = get_attack_damage_multiplier(t, defender.types)
			best = max(best, eff * 1.5)
	return best


# ═══════════════════════════════════════════════════════════════════════════════
# Default layer – marks invalid actions as −∞
# ═══════════════════════════════════════════════════════════════════════════════

def layer_invalid(me: PlayerState, op: PlayerState, rng: Generator) -> list[float]:
	p = me.pokemon_on_field
	if op.pokemon_on_field.health == 0 and p.health != 0:
		result = [-math.inf] * 12
		result[NOACTION] = 0
		return result

	result = [0.0] * 12
	if p.health == 0:
		result[ATTACK1:ATTACK4 + 1] = [-math.inf, -math.inf, -math.inf, -math.inf]
		result[NOACTION] = -math.inf
		result[STRUGGLE] = -math.inf
	elif p.wrapped:
		result[ATTACK1:ATTACK4 + 1] = [-math.inf, -math.inf, -math.inf, -math.inf]
		result[STRUGGLE] = -math.inf
	else:
		result[NOACTION] = -math.inf
		result[ATTACK1:ATTACK4 + 1] = (-math.inf if i >= len(p.move_set) or p.move_disabled == i or p.move_set[i].pp == 0 else 0 for i in range(4))
		if not all(map(lambda x: x == -math.inf, result[ATTACK1:ATTACK4 + 1])):
			result[STRUGGLE] = -math.inf
	result[SWITCH1:SWITCH6 + 1] = (-math.inf if i >= len(me.team) or i == me.pokemon_on_field_index or me.team[i].health == 0 else 0 for i in range(6))
	return result


# ═══════════════════════════════════════════════════════════════════════════════
# Layer 1 – Expected damage
# ═══════════════════════════════════════════════════════════════════════════════

def layer_damage(me: PlayerState, op: PlayerState, rng: Generator) -> list[float]:
	"""
	Score each attack slot by expected damage as a fraction of the opponent's
	maximum HP. Accounts for crit probability, accuracy, STAB, and type
	effectiveness using the genuine Gen 1 damage formula.
	"""
	result = [0.0] * 12
	p      = me.pokemon_on_field
	opp    = op.pokemon_on_field
	max_hp = max(1, opp.max_health)

	for i, move in enumerate(p.move_set):
		if i >= 4:
			break
		if move.id == 0 or move.power == 0:
			continue
		result[ATTACK1 + i] = _expected_damage(p, opp, move) / max_hp

	return result


# ═══════════════════════════════════════════════════════════════════════════════
# Layer 2 – KO bonus
# ═══════════════════════════════════════════════════════════════════════════════

def layer_ko_bonus(me: PlayerState, op: PlayerState, rng: Generator) -> list[float]:
	"""
	Large bonus for moves that have a real chance to KO the opponent this turn.
	The bonus is discounted when we're slower and the opponent can KO us first,
	because our attack would never land.
	"""
	result = [0.0] * 12
	p   = me.pokemon_on_field
	opp = op.pokemon_on_field

	we_outspeed  = p.speed > opp.speed
	we_underspec = p.speed < opp.speed
	op_best_ko   = _best_ko_prob(opp, p) if we_underspec else 0.0

	for i, move in enumerate(p.move_set):
		if i >= 4:
			break
		if move.id == 0 or move.power == 0:
			continue

		ko = _ko_probability(p, opp, move)
		if ko <= 0.0:
			continue

		if move.priority > 0:
			# Priority moves always go first (barring tie) – full credit
			result[ATTACK1 + i] = ko
		elif we_underspec:
			# Opponent moves first; if they can KO us the move never lands
			result[ATTACK1 + i] = ko * (1.0 - op_best_ko)
		else:
			result[ATTACK1 + i] = ko

	# Struggle KO check (power 50, Normal type, uses Attack/Defense)
	if get_attack_damage_multiplier(int(Type.Normal), opp.types) != 0:
		raw = max(1, min(997, math.floor(
			math.floor(math.floor(2 * p.level / 5 + 2) * p.attack * 50 / max(1, opp.defense)) / 50
		)) + 2)
		if raw >= opp.health > 0:
			result[STRUGGLE] = 0.25

	return result


# ═══════════════════════════════════════════════════════════════════════════════
# Layer 3 – Status infliction
# ═══════════════════════════════════════════════════════════════════════════════

def layer_status_infliction(me: PlayerState, op: PlayerState, rng: Generator) -> list[float]:
	"""
	Reward moves that inflict status conditions on the opponent, scaled by the
	condition's strategic value, its application probability, and move accuracy.
	Non-volatile statuses are skipped when the opponent already has one.
	"""
	result = [0.0] * 12
	p   = me.pokemon_on_field
	opp = op.pokemon_on_field

	op_has_nv = opp.has_status(StatusChange.Any_non_volatile_status)

	for i, move in enumerate(p.move_set):
		if i >= 4:
			break
		if move.id == 0:
			continue

		sc      = move.status_change
		status  = int(sc['status'])
		cmp_val = sc['cmpVal']

		if status == 0:
			continue

		score = 0.0

		# Non-volatile statuses – only apply when target has no NV status yet
		for sv, sv_score in _NV_STATUS_SCORES.items():
			if status & sv:
				if not op_has_nv and opp.can_have_status(status):
					score = sv_score
				break

		# Volatile statuses – independent; check if opponent doesn't already have it
		for sv, sv_score in _VOLATILE_STATUS_SCORES.items():
			if status & sv:
				if not opp.has_status(sv):
					score = max(score, sv_score)
				break

		if score <= 0.0:
			continue

		# cmpVal = 0 means guaranteed (256/256); otherwise it's the probability out of 256
		prob = (cmp_val / 256.0) if cmp_val != 0 else 1.0
		acc  = _hit_accuracy(move, p, opp)
		result[ATTACK1 + i] = score * prob * acc

	return result


# ═══════════════════════════════════════════════════════════════════════════════
# Layer 4 – Stat changes
# ═══════════════════════════════════════════════════════════════════════════════

def layer_stat_changes(me: PlayerState, op: PlayerState, rng: Generator) -> list[float]:
	"""
	Reward moves that raise our stats or lower the opponent's stats.  Value is
	contextual: ATK boosts are worth more when we have more physical moves; DEF
	boosts are worth more when the opponent has more physical moves; Speed matters
	most when we're currently slower; etc.
	"""
	result = [0.0] * 12
	p   = me.pokemon_on_field
	opp = op.pokemon_on_field

	# Pre-count offensive categories to avoid recomputing per move
	n_phys_us  = sum(1 for m in p.move_set   if m.id != 0 and m.category == MoveCategory.Physical)
	n_spec_us  = sum(1 for m in p.move_set   if m.id != 0 and m.category == MoveCategory.Special)
	n_phys_op  = sum(1 for m in opp.move_set if m.id != 0 and m.category == MoveCategory.Physical)
	n_spec_op  = sum(1 for m in opp.move_set if m.id != 0 and m.category == MoveCategory.Special)

	my_stages = p.stats_upgrade_stages
	op_stages = opp.stats_upgrade_stages

	def _self_buff_value(key: str, nb: int) -> float:
		"""Value of raising OUR stat `key` by `nb` stages (nb > 0)."""
		if my_stages.get(key, 0) >= 6:
			return 0.0
		if key == 'ATK':
			return n_phys_us * 0.12
		if key == 'DEF':
			# Raising our defense is valuable against the opponent's physical moves
			return n_phys_op * 0.08
		if key == 'SPD':
			# Gen 1 Special – valuable for both our specials and opponent's specials
			return (n_spec_us + n_spec_op) * 0.08
		if key == 'SPE':
			# Speed: very valuable if we're currently outsped
			return 0.15 if p.speed <= opp.speed else 0.05
		if key == 'EVD':
			return 0.08
		if key == 'ACC':
			return 0.05
		return 0.04

	def _debuff_value(key: str, nb: int) -> float:
		"""Value of lowering the OPPONENT's stat `key` by |nb| stages (nb < 0 in foe_change)."""
		if op_stages.get(key, 0) <= -6:
			return 0.0
		if key == 'ATK':
			return n_phys_op * 0.10
		if key == 'DEF':
			# Lowering opponent's defense benefits our physical moves
			return n_phys_us * 0.08
		if key == 'SPD':
			return (n_spec_us + n_spec_op) * 0.08
		if key == 'SPE':
			# Speed debuff is especially valuable if they currently outspeed us
			return 0.12 if opp.speed >= p.speed else 0.04
		if key == 'EVD':
			return 0.07
		if key == 'ACC':
			return 0.05
		return 0.04

	for i, move in enumerate(p.move_set):
		if i >= 4:
			break
		if move.id == 0:
			continue

		score = 0.0
		acc   = _hit_accuracy(move, p, opp)

		for change in move.owner_change:
			nb      = change['nb']
			cmp_val = change['cmpVal']
			key     = _get_stat_key(change['stat'])
			if nb <= 0 or key is None:
				continue
			prob   = (cmp_val / 256.0) if cmp_val != 0 else 1.0
			score += _self_buff_value(key, nb) * nb * prob

		for change in move.foe_change:
			nb      = change['nb']
			cmp_val = change['cmpVal']
			key     = _get_stat_key(change['stat'])
			if nb >= 0 or key is None:
				continue  # foe_change debuffs have negative nb
			prob   = (cmp_val / 256.0) if cmp_val != 0 else 1.0
			score += _debuff_value(key, nb) * abs(nb) * prob * 0.75

		if score > 0.0:
			result[ATTACK1 + i] = score * acc

	return result


# ═══════════════════════════════════════════════════════════════════════════════
# Layer 5 – Switch evaluation
# ═══════════════════════════════════════════════════════════════════════════════

def layer_switching(me: PlayerState, op: PlayerState, rng: Generator) -> list[float]:
	"""
	Score each switch target by how much it improves the matchup compared to
	staying in, factoring in: defensive type safety, offensive coverage, HP,
	speed advantage, and the inherent tempo cost of switching.
	"""
	result = [0.0] * 12
	p   = me.pokemon_on_field
	opp = op.pokemon_on_field

	op_idx = op.pokemon_on_field_index

	# Collect opponent's known attacking moves (only revealed moves count)
	op_known_atk: list = []
	for j in range(4):
		if me.is_pkmn_move_discovered(op_idx, j) and j < len(opp.move_set):
			m = opp.move_set[j]
			if m.id != 0 and m.power > 0:
				op_known_atk.append(m)

	def _max_threat_vs(target_pkmn) -> float:
		"""Max (effectiveness × STAB) the active opponent can threaten against target_pkmn."""
		best = 0.0
		for m in op_known_atk:
			eff  = get_attack_damage_multiplier(m.type, target_pkmn.types)
			stab = 1.5 if _is_stab(m, opp) else 1.0
			best = max(best, eff * stab)
		if not op_known_atk:
			# No moves known – estimate from opponent's types as STAB coverage
			for t in opp.types:
				eff  = get_attack_damage_multiplier(t, target_pkmn.types)
				best = max(best, eff * 1.5)
		return best

	# Baseline: how bad is our current matchup?
	current_threat = _max_threat_vs(p)
	current_off    = _best_type_effectiveness(p, opp)

	for i in range(6):
		if i >= len(me.team):
			break
		if i == me.pokemon_on_field_index or me.team[i].health == 0:
			continue

		target   = me.team[i]
		hp_ratio = target.health / max(1, target.max_health)

		# Defensive improvement: candidate takes less from opponent's moves
		cand_threat  = _max_threat_vs(target)
		def_delta    = (current_threat - cand_threat) * 0.10

		# Offensive improvement: candidate hits opponent harder
		cand_off  = _best_type_effectiveness(target, opp)
		off_delta = (cand_off - current_off) * 0.10

		# Speed advantage bonus
		spd_bonus = 0.05 if target.speed > opp.speed else 0.0

		# Tempo penalty: opponent gets a free turn to attack
		tempo = -0.12

		score = def_delta + off_delta + spd_bonus + tempo

		# Scale by health ratio – don't switch into a nearly-fainted Pokémon
		score *= max(0.2, hp_ratio)

		result[SWITCH1 + i] = score

	return result


# ═══════════════════════════════════════════════════════════════════════════════
# Layer 6 – Self-preservation
# ═══════════════════════════════════════════════════════════════════════════════

def layer_self_preservation(me: PlayerState, op: PlayerState, rng: Generator) -> list[float]:
	"""
	Encourage switching when the current matchup is losing, the active Pokémon
	is in danger, or is incapacitated.  Also hard-blocks moves that are immune
	on the current opponent, and tunes Rest usage based on HP level.
	"""
	result = [0.0] * 12
	p   = me.pokemon_on_field
	opp = op.pokemon_on_field

	hp_ratio    = p.health / max(1, p.max_health)
	op_best_ko  = _best_ko_prob(opp, p)
	my_best_ko  = _best_ko_prob(p, opp)

	threatened      = op_best_ko > 0.50
	cannot_threaten = my_best_ko < 0.30
	is_frozen       = p.has_status(StatusChange.Frozen)
	is_asleep       = p.has_status(StatusChange.Asleep)
	very_low_hp     = hp_ratio < 0.10
	low_hp          = hp_ratio < 0.20

	has_healthy_bench = any(
		me.team[i].health > 0 and i != me.pokemon_on_field_index
		and me.team[i].health / max(1, me.team[i].max_health) > 0.35
		for i in range(len(me.team))
	)

	# ── Determine switch boost ──────────────────────────────────────────────
	switch_boost = 0.0

	if threatened and cannot_threaten and has_healthy_bench:
		# We're losing: opponent can likely KO us, we can't KO back
		switch_boost += 0.35

	if (is_frozen or is_asleep) and has_healthy_bench:
		has_rest = any(m.id == AvailableMove.Rest for m in p.move_set if m.id != 0)
		if not has_rest:
			switch_boost += 0.25

	if very_low_hp and has_healthy_bench:
		switch_boost += 0.20
	elif low_hp and threatened and has_healthy_bench:
		switch_boost += 0.15

	if switch_boost > 0.0:
		for i in range(6):
			if i >= len(me.team):
				break
			if me.team[i].health > 0 and i != me.pokemon_on_field_index:
				result[SWITCH1 + i] += switch_boost

	# ── Block immune moves and penalise heavily-resisted ones ───────────────
	for i, move in enumerate(p.move_set):
		if i >= 4:
			break
		if move.id == 0 or move.power == 0:
			continue
		eff = get_attack_damage_multiplier(move.type, opp.types)
		if eff == 0.0:
			# The opponent is immune: treat as −∞ so this move is never chosen
			result[ATTACK1 + i] = -math.inf
		elif eff < 0.5:
			# Heavily resisted
			result[ATTACK1 + i] -= 0.20

	# ── Rest usage tuning ───────────────────────────────────────────────────
	for i, move in enumerate(p.move_set):
		if i >= 4:
			break
		if move.id != AvailableMove.Rest:
			continue
		if hp_ratio < 0.35:
			# Strong incentive to Rest when badly hurt
			result[ATTACK1 + i] += 0.60 * (0.35 - hp_ratio) / 0.35
		elif hp_ratio > 0.75:
			# Wasting a Rest at high HP
			result[ATTACK1 + i] -= 0.30

	return result


# ═══════════════════════════════════════════════════════════════════════════════
# Layer 7 – Priority and speed context
# ═══════════════════════════════════════════════════════════════════════════════

def layer_priority_and_speed(me: PlayerState, op: PlayerState, rng: Generator) -> list[float]:
	"""
	Adjust scores based on speed relationships and move priority:
	- Bonus for priority moves when we don't outspeed but they can secure a KO.
	- Penalty on non-priority moves when we're slower and the opponent can KO us
	  before we attack.
	"""
	result = [0.0] * 12
	p   = me.pokemon_on_field
	opp = op.pokemon_on_field

	we_outspeed  = p.speed > opp.speed
	we_underspec = p.speed < opp.speed
	op_best_ko   = _best_ko_prob(opp, p) if we_underspec else 0.0

	for i, move in enumerate(p.move_set):
		if i >= 4:
			break
		if move.id == 0:
			continue

		ko = _ko_probability(p, opp, move)

		if move.priority > 0:
			if not we_outspeed:
				# Priority ensures we go first – reward the KO potential
				result[ATTACK1 + i] += ko * 0.8
				if op_best_ko > 0.50:
					# Opponent would KO us with non-priority; priority saves us
					result[ATTACK1 + i] += 0.30
		else:
			if we_underspec and op_best_ko > 0.55:
				# We're slower, opponent likely KOs us first – discount
				result[ATTACK1 + i] -= 0.15

	return result


# ═══════════════════════════════════════════════════════════════════════════════
# Layer 8 – Special-case move handling
# ═══════════════════════════════════════════════════════════════════════════════

def layer_special_moves(me: PlayerState, op: PlayerState, rng: Generator) -> list[float]:
	"""
	Handles Gen 1-specific mechanics that the generic layers cannot capture:
	trapping moves, two-turn moves, recharge moves, Hyper Beam no-recharge-on-KO,
	Explosion trade logic, Rest as a heal, Dream Eater sleep requirement,
	Counter reflect logic, OHKO accuracy rule, and misc edge cases.
	"""
	result = [0.0] * 12
	p   = me.pokemon_on_field
	opp = op.pokemon_on_field

	hp_ratio = p.health / max(1, p.max_health)
	op_asleep = opp.has_status(StatusChange.Asleep)

	healthy_bench = sum(
		1 for i in range(len(me.team))
		if me.team[i].health > 0 and i != me.pokemon_on_field_index
	)

	for i, move in enumerate(p.move_set):
		if i >= 4:
			break
		if move.id == 0:
			continue

		move_id = int(move.id)

		# ── Trapping moves (Wrap / Bind / Clamp / Fire Spin) ─────────────────
		# In Gen 1, these prevent the opponent from acting for 2–5 turns.
		# Extremely powerful: opponent effectively skips turns entirely.
		if move_id in _TRAPPING_MOVE_IDS:
			if not opp.wrapped:
				trap_value = 0.50 if p.speed > opp.speed else 0.20
				result[ATTACK1 + i] += trap_value

		# ── Two-turn loading moves ────────────────────────────────────────────
		if move.needs_loading:
			if move.invulnerable_during_loading:
				# Fly / Dig: user is invulnerable while charging – small penalty
				result[ATTACK1 + i] -= 0.08
			else:
				# Solar Beam, Razor Wind, Skull Bash, Sky Attack: vulnerable – bigger penalty
				result[ATTACK1 + i] -= 0.25

		# ── Recharge moves (e.g. Hyper Beam) ─────────────────────────────────
		# In Gen 1, no recharge is required if the target faints.
		if move.needs_recharge:
			ko = _ko_probability(p, opp, move)
			# Penalty scales down as KO probability increases
			result[ATTACK1 + i] += -0.25 * (1.0 - ko)

		# ── Dream Eater – only works on sleeping opponents ────────────────────
		if move_id == int(AvailableMove.Dream_Eater):
			if not op_asleep:
				result[ATTACK1 + i] -= 2.0

		# ── Explosion / Self-Destruct ─────────────────────────────────────────
		# Our Pokémon faints after use; only worthwhile if the trade is good.
		if move.id in (AvailableMove.Explosion, AvailableMove.Self_Destruct):
			ko = _ko_probability(p, opp, move)
			if healthy_bench == 0:
				# Last Pokémon alive: KOing ourselves ends the game – heavy penalty
				# unless it also KOs the opponent
				if ko < 0.80:
					result[ATTACK1 + i] -= 1.5
			elif hp_ratio > 0.40 and ko < 0.65:
				# Good health and unlikely to KO – bad trade
				result[ATTACK1 + i] -= 0.60

		# ── Rest – acts as a heal that also induces 2-turn sleep ─────────────
		# layer_self_preservation already handles the primary HP-based incentive.
		# Add a bonus here for curing an existing non-volatile status condition.
		if move.id == AvailableMove.Rest:
			if p.has_status(StatusChange.Any_non_volatile_status) and hp_ratio < 0.85:
				result[ATTACK1 + i] += 0.25

		# ── Counter (Gen 1: reflects last Normal/Fighting physical hit × 2) ───
		if move_id == int(AvailableMove.Counter):
			last_op = opp.last_used_move
			if (last_op.id != 0
					and last_op.category == MoveCategory.Physical
					and last_op.type in (Type.Normal, Type.Fighting)):
				stored = p.damages_stored
				if stored > 0:
					counter_dmg = stored * 2
					ko_fraction = min(1.0, counter_dmg / max(1, opp.health))
					result[ATTACK1 + i] += ko_fraction * 0.80
				# Even without stored damage the setup is right; modest bonus
				result[ATTACK1 + i] += 0.10
			else:
				result[ATTACK1 + i] -= 0.25

		# ── Mirror Move ───────────────────────────────────────────────────────
		if move_id == int(AvailableMove.Mirror_Move):
			last_op = opp.last_used_move
			if last_op.id == 0 or last_op.power == 0:
				result[ATTACK1 + i] -= 0.50

		# ── Bide ─────────────────────────────────────────────────────────────
		# Risky multi-turn commitment; penalise unless in a very special setup.
		if move_id == int(AvailableMove.Bide):
			result[ATTACK1 + i] -= 0.35

		# ── OHKO moves (Fissure / Horn Drill / Guillotine) ───────────────────
		# In Gen 1 these completely miss if the user is slower than the target.
		if move.id in (AvailableMove.Fissure, AvailableMove.Horn_Drill, AvailableMove.Guillotine):
			if p.speed <= opp.speed:
				result[ATTACK1 + i] = -math.inf

		# ── Leech Seed ────────────────────────────────────────────────────────
		# layer_status_infliction handles this, but if the opponent is Grass-type
		# it's completely immune – block it here.
		if move_id == int(AvailableMove.Leech_Seed):
			if opp.has_status(StatusChange.Leeched):
				result[ATTACK1 + i] -= 1.0  # already leeched; useless

		# ── Disable ──────────────────────────────────────────────────────────
		# Bonus when opponent's last move was their strongest attacking move.
		if move_id == int(AvailableMove.Disable):
			last_op = opp.last_used_move
			if last_op.id != 0 and last_op.power > 0:
				# Disabling a powerful move is worth something
				eff  = get_attack_damage_multiplier(last_op.type, p.types)
				stab = 1.5 if _is_stab(last_op, opp) else 1.0
				result[ATTACK1 + i] += eff * stab * 0.10

	return result


# ═══════════════════════════════════════════════════════════════════════════════
# Framework functions
# ═══════════════════════════════════════════════════════════════════════════════

def execute_layer(layer: Layer, bias: float, me: PlayerState, op: PlayerState, rng: Generator, acc: list[float]):
	for i, f in enumerate(layer(me, op, rng)):
		acc[i] += f * bias
	return acc


def get_move(layers: list[BiasedLayer], me: PlayerState, op: PlayerState, rng: Generator) -> BattleAction:
	scores = [0] * 12
	for b, l in layers:
		execute_layer(l, b, me, op, rng, scores)
	print(scores)
	maxval = max(scores)
	indices = [i for i, k in enumerate(scores) if k == maxval]
	result = result_index_to_action_array[rng.choice(indices)]
	print(repr(result))
	return result


def make_ai(layers: list[BiasedLayer]) -> Callable[[PlayerState, PlayerState, Generator], BattleAction]:
	agg = [(1, layer_invalid)] + layers

	def e(me: PlayerState, op: PlayerState, rng: Generator):
		return get_move(agg, me, op, rng)

	return e


# ═══════════════════════════════════════════════════════════════════════════════
# Convenience constructor
# ═══════════════════════════════════════════════════════════════════════════════

def make_competitive_ai() -> Callable[[PlayerState, PlayerState, Generator], BattleAction]:
	"""
	Build a competitive Gen 1 heuristic AI by combining all scoring layers with
	tuned bias weights.

	Bias rationale:
	  - KO bonus (5.0)           : securing a KO is the highest priority
	  - Priority/speed (1.5)     : move-order awareness is critical in close games
	  - Status infliction (1.5)  : sleep/freeze/paralysis swing entire battles
	  - Self-preservation (1.2)  : defensive play when outmatched
	  - Damage (1.0)             : bread-and-butter offensive baseline
	  - Special moves (1.0)      : mechanics that need explicit handling
	  - Switching (1.0)          : matchup management
	  - Stat changes (0.8)       : longer-term setup, lower immediate urgency
	"""
	return make_ai([
		(1.0, layer_damage),
		(5.0, layer_ko_bonus),
		(1.5, layer_status_infliction),
		(0.8, layer_stat_changes),
		(1.0, layer_switching),
		(1.2, layer_self_preservation),
		(1.5, layer_priority_and_speed),
		(1.0, layer_special_moves),
	])
