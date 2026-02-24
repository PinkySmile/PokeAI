from numpy.random import Generator
from PokeBattle.Gen1.State import PlayerState, BattleAction


def get_move(me: PlayerState, op: PlayerState, rng: Generator) -> BattleAction:
	my_moves = me.pokemon_on_field.move_set
	if me.pokemon_on_field.health == 0:
		return BattleAction(BattleAction.Switch1 + next(i for i in range(len(me.team)) if me.team[i].health != 0 and me.pokemon_on_field_index != i))
	if me.pokemon_on_field.wrapped:
		if sum(p.health != 0 for p in me.team) == 1:
			return BattleAction.NoAction
		return BattleAction(BattleAction.Switch1 + next(i for i in range(len(me.team)) if me.team[i].health != 0 and me.pokemon_on_field_index != i))
	if all(m.pp == 0 and i != me.pokemon_on_field.move_disabled for i, m in enumerate(my_moves)):
		return BattleAction.StruggleMove
	return BattleAction.Attack1 + next(i for i, m in enumerate(my_moves) if m.pp != 0)
