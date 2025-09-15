from typing import Callable

from .Pokemon import Pokemon
from .RandomGenerator import RandomGenerator


class DesyncPolicy(int):
	Miss: DesyncPolicy
	Throw: DesyncPolicy
	Invert: DesyncPolicy
	Ignore: DesyncPolicy

	@property
	def name(self) -> str: ...


class BattleAction(int):
	EmptyAction: BattleAction
	Attack1: BattleAction
	Attack2: BattleAction
	Attack3: BattleAction
	Attack4: BattleAction
	Switch1: BattleAction
	Switch2: BattleAction
	Switch3: BattleAction
	Switch4: BattleAction
	Switch5: BattleAction
	Switch6: BattleAction
	NoAction: BattleAction
	StruggleMove: BattleAction
	Run: BattleAction

	@property
	def name(self) -> str: ...


def battle_action_to_string(action: int) -> str: ...


class PlayerState:
	@property
	def name(self) -> str: ...
	@name.setter
	def name(self, v: str): ...

	@property
	def last_action(self) -> BattleAction: ...
	@last_action.setter
	def last_action(self, v: BattleAction): ...

	@property
	def next_action(self) -> BattleAction: ...
	@next_action.setter
	def next_action(self, v: BattleAction): ...

	@property
	def pokemon_on_field_index(self) -> int: ...
	@pokemon_on_field_index.setter
	def pokemon_on_field_index(self, v: int): ...

	@property
	def pokemon_on_field(self) -> Pokemon: ...

	@property
	def team(self) -> list[Pokemon]: ...
	@team.setter
	def team(self, v: list[Pokemon]): ...

	def is_pkmn_discovered(self, pkmn: int) -> bool: ...
	def set_pkmn_discovered(self, pkmn: int, v: bool): ...

	def is_pkmn_move_discovered(self, pkmn: int, move: int): ...
	def set_pkmn_move_discovered(self, pkmn: int, move: int, v: bool): ...


class BattleState:
	def __init__(self, alloc=True): ...
	def copy(self) -> BattleState: ...

	@property
	def me(self) -> PlayerState: ...

	@property
	def op(self) -> PlayerState: ...

	@property
	def rng(self) -> RandomGenerator: ...

	@property
	def desync(self) -> DesyncPolicy: ...
	@desync.setter
	def desync(self, c: DesyncPolicy): ...

	@property
	def logger(self) -> Callable[[str], None]: ...
	@logger.setter
	def logger(self, c: Callable[[str], None]): ...

	@property
	def on_turn_start(self) -> Callable[[], bool]: ...
	@on_turn_start.setter
	def on_turn_start(self, c: Callable[[], bool]): ...

	@property
	def on_battle_end(self) -> Callable[[], None]: ...
	@on_battle_end.setter
	def on_battle_end(self, c: Callable[[], None]): ...

	@property
	def on_battle_start(self) -> Callable[[], None]: ...
	@on_battle_start.setter
	def on_battle_start(self, c: Callable[[], None]): ...
