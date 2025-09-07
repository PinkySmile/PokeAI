from .Type import Type
from typing import TYPE_CHECKING, Callable

if TYPE_CHECKING:
	from .Pokemon import Pokemon

class AvailableMove(int):
	Empty: AvailableMove
	Pound: AvailableMove
	Karate_Chop: AvailableMove
	Doubleslap: AvailableMove
	Comet_Punch: AvailableMove
	Mega_Punch: AvailableMove
	Pay_Day: AvailableMove
	Fire_Punch: AvailableMove
	Ice_Punch: AvailableMove
	Thunder_Punch: AvailableMove
	Scratch: AvailableMove
	Vise_Grip: AvailableMove
	Guillotine: AvailableMove
	Razor_Wind: AvailableMove
	Swords_Dance: AvailableMove
	Cut: AvailableMove
	Gust: AvailableMove
	Wing_Attack: AvailableMove
	Whirlwind: AvailableMove
	Fly: AvailableMove
	Bind: AvailableMove
	Slam: AvailableMove
	Vine_Whip: AvailableMove
	Stomp: AvailableMove
	Double_Kick: AvailableMove
	Mega_Kick: AvailableMove
	Jump_Kick: AvailableMove
	Rolling_Kick: AvailableMove
	Sand_Attack: AvailableMove
	Headbutt: AvailableMove
	Horn_Attack: AvailableMove
	Fury_Attack: AvailableMove
	Horn_Drill: AvailableMove
	Tackle: AvailableMove
	Body_Slam: AvailableMove
	Wrap: AvailableMove
	Take_Down: AvailableMove
	Thrash: AvailableMove
	Double_Edge: AvailableMove
	Tail_Whip: AvailableMove
	Poison_Sting: AvailableMove
	Twineedle: AvailableMove
	Pin_Missile: AvailableMove
	Leer: AvailableMove
	Bite: AvailableMove
	Growl: AvailableMove
	Roar: AvailableMove
	Sing: AvailableMove
	Supersonic: AvailableMove
	Sonic_Boom: AvailableMove
	Disable: AvailableMove
	Acid: AvailableMove
	Ember: AvailableMove
	Flamethrower: AvailableMove
	Mist: AvailableMove
	Water_Gun: AvailableMove
	Hydro_Pump: AvailableMove
	Surf: AvailableMove
	Ice_Beam: AvailableMove
	Blizzard: AvailableMove
	Psybeam: AvailableMove
	Bubblebeam: AvailableMove
	Aurora_Beam: AvailableMove
	Hyper_Beam: AvailableMove
	Peck: AvailableMove
	Drill_Peck: AvailableMove
	Submission: AvailableMove
	Low_Kick: AvailableMove
	Counter: AvailableMove
	Seismic_Toss: AvailableMove
	Strength: AvailableMove
	Absorb: AvailableMove
	Mega_Drain: AvailableMove
	Leech_Seed: AvailableMove
	Growth: AvailableMove
	Razor_Leaf: AvailableMove
	Solarbeam: AvailableMove
	Poisonpowder: AvailableMove
	Stun_Spore: AvailableMove
	Sleep_Powder: AvailableMove
	Petal_Dance: AvailableMove
	String_Shot: AvailableMove
	Dragon_Rage: AvailableMove
	Fire_Spin: AvailableMove
	Thundershock: AvailableMove
	Thunderbolt: AvailableMove
	Thunder_Wave: AvailableMove
	Thunder: AvailableMove
	Rock_Throw: AvailableMove
	Earthquake: AvailableMove
	Fissure: AvailableMove
	Dig: AvailableMove
	Toxic: AvailableMove
	Confusion: AvailableMove
	Psychic_M: AvailableMove
	Hypnosis: AvailableMove
	Meditate: AvailableMove
	Agility: AvailableMove
	Quick_Attack: AvailableMove
	Rage: AvailableMove
	Teleport: AvailableMove
	Night_Shade: AvailableMove
	Mimic: AvailableMove
	Screech: AvailableMove
	Double_Team: AvailableMove
	Recover: AvailableMove
	Harden: AvailableMove
	Minimize: AvailableMove
	Smokescreen: AvailableMove
	Confuse_Ray: AvailableMove
	Withdraw: AvailableMove
	Defense_Curl: AvailableMove
	Barrier: AvailableMove
	Light_Screen: AvailableMove
	Haze: AvailableMove
	Reflect: AvailableMove
	Focus_Energy: AvailableMove
	Bide: AvailableMove
	Metronome: AvailableMove
	Mirror_Move: AvailableMove
	Self_Destruct: AvailableMove
	Egg_Bomb: AvailableMove
	Lick: AvailableMove
	Smog: AvailableMove
	Sludge: AvailableMove
	Bone_Club: AvailableMove
	Fire_Blast: AvailableMove
	Waterfall: AvailableMove
	Clamp: AvailableMove
	Swift: AvailableMove
	Skull_Bash: AvailableMove
	Spike_Cannon: AvailableMove
	Constrict: AvailableMove
	Amnesia: AvailableMove
	Kinesis: AvailableMove
	Softboiled: AvailableMove
	Hi_Jump_Kick: AvailableMove
	Glare: AvailableMove
	Dream_Eater: AvailableMove
	Poison_Gas: AvailableMove
	Barrage: AvailableMove
	Leech_Life: AvailableMove
	Lovely_Kiss: AvailableMove
	Sky_Attack: AvailableMove
	Transform: AvailableMove
	Bubble: AvailableMove
	Dizzy_Punch: AvailableMove
	Spore: AvailableMove
	Flash: AvailableMove
	Psywave: AvailableMove
	Splash: AvailableMove
	Acid_Armor: AvailableMove
	Crabhammer: AvailableMove
	Explosion: AvailableMove
	Fury_Swipes: AvailableMove
	Bonemerang: AvailableMove
	Rest: AvailableMove
	Rock_Slide: AvailableMove
	Hyper_Fang: AvailableMove
	Sharpen: AvailableMove
	Conversion: AvailableMove
	Tri_Attack: AvailableMove
	Super_Fang: AvailableMove
	Slash: AvailableMove
	Substitute: AvailableMove
	Struggle: AvailableMove

	@property
	def name(self) -> str: ...


class MoveCategory(int):
	Physical: MoveCategory
	Special: MoveCategory
	Status: MoveCategory


class Move:
	def __init__(self, *args): ...
	def __assign__(self, other: Move): ...
	def copy(self) -> Move: ...

	@property
	def nb_runs(self) -> tuple[int, int]: ...

	@property
	def nb_hits(self) -> tuple[int, int]: ...

	@property
	def status_change(self) -> dict: ...

	@property
	def owner_change(self) -> list[dict]: ...

	@property
	def foe_change(self) -> list[dict]: ...

	@property
	def needs_loading(self) -> bool: ...

	@property
	def invulnerable_during_loading(self) -> bool: ...

	@property
	def needs_recharge(self) -> bool: ...

	@property
	def hit_callback_description(self) -> str: ...

	@property
	def miss_callback_description(self) -> str: ...

	@property
	def crit_chance(self) -> float: ...

	@property
	def accuracy(self) -> int: ...

	@property
	def category(self) -> MoveCategory: ...

	@property
	def makes_invulnerable(self) -> bool: ...

	@property
	def max_pp(self) -> int: ...

	@property
	def power(self) -> int: ...

	@property
	def pp(self) -> int: ...
	@pp.setter
	def pp(self, pp: int): ...

	@property
	def pp_up(self) -> int: ...
	@pp_up.setter
	def pp_up(self, nb: int): ...

	@property
	def type(self) -> Type: ...

	@property
	def id(self) -> AvailableMove: ...

	@property
	def name(self) -> str: ...

	@property
	def description(self) -> str: ...

	@property
	def priority(self) -> int: ...

	@property
	def finished(self) -> bool: ...

	@property
	def hits_left(self) -> int: ...
	@hits_left.setter
	def hits_left(self, nb: int): ...

	def glitch(self): ...
	def reset(self): ...
	def attack(self, owner: Pokemon, target: Pokemon, logger: Callable[[str], None]): ...