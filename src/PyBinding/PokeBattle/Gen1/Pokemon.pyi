from .RandomGenerator import RandomGenerator
from .StatsChange import StatsChange
from .StatusChange import StatusChange
from .Type import Type
from typing import TYPE_CHECKING

if TYPE_CHECKING:
	from .Move import Move, MoveCategory, AvailableMove


class PokemonSpecies(int):
	Rhydon: PokemonSpecies
	Kangaskhan: PokemonSpecies
	Nidoran_M: PokemonSpecies
	Clefairy: PokemonSpecies
	Spearow: PokemonSpecies
	Voltorb: PokemonSpecies
	Nidoking: PokemonSpecies
	Slowbro: PokemonSpecies
	Ivysaur: PokemonSpecies
	Exeggutor: PokemonSpecies
	Lickitung: PokemonSpecies
	Exeggcute: PokemonSpecies
	Grimer: PokemonSpecies
	Gengar: PokemonSpecies
	Nidoran_F: PokemonSpecies
	Nidoqueen: PokemonSpecies
	Cubone: PokemonSpecies
	Rhyhorn: PokemonSpecies
	Lapras: PokemonSpecies
	Arcanine: PokemonSpecies
	Mew: PokemonSpecies
	Gyarados: PokemonSpecies
	Shellder: PokemonSpecies
	Tentacool: PokemonSpecies
	Gastly: PokemonSpecies
	Scyther: PokemonSpecies
	Staryu: PokemonSpecies
	Blastoise: PokemonSpecies
	Pinsir: PokemonSpecies
	Tangela: PokemonSpecies
	Growlithe: PokemonSpecies
	Onix: PokemonSpecies
	Fearow: PokemonSpecies
	Pidgey: PokemonSpecies
	Slowpoke: PokemonSpecies
	Kadabra: PokemonSpecies
	Graveler: PokemonSpecies
	Chansey: PokemonSpecies
	Machoke: PokemonSpecies
	Mr_Mime: PokemonSpecies
	Hitmonlee: PokemonSpecies
	Hitmonchan: PokemonSpecies
	Arbok: PokemonSpecies
	Parasect: PokemonSpecies
	Psyduck: PokemonSpecies
	Drowzee: PokemonSpecies
	Golem: PokemonSpecies
	Magmar: PokemonSpecies
	Electabuzz: PokemonSpecies
	Magneton: PokemonSpecies
	Koffing: PokemonSpecies
	Mankey: PokemonSpecies
	Seel: PokemonSpecies
	Diglett: PokemonSpecies
	Tauros: PokemonSpecies
	Farfetchd: PokemonSpecies
	Venonat: PokemonSpecies
	Dragonite: PokemonSpecies
	Doduo: PokemonSpecies
	Poliwag: PokemonSpecies
	Jynx: PokemonSpecies
	Moltres: PokemonSpecies
	Articuno: PokemonSpecies
	Zapdos: PokemonSpecies
	Ditto: PokemonSpecies
	Meowth: PokemonSpecies
	Krabby: PokemonSpecies
	Vulpix: PokemonSpecies
	Ninetales: PokemonSpecies
	Pikachu: PokemonSpecies
	Raichu: PokemonSpecies
	Dratini: PokemonSpecies
	Dragonair: PokemonSpecies
	Kabuto: PokemonSpecies
	Kabutops: PokemonSpecies
	Horsea: PokemonSpecies
	Seadra: PokemonSpecies
	Sandshrew: PokemonSpecies
	Sandslash: PokemonSpecies
	Omanyte: PokemonSpecies
	Omastar: PokemonSpecies
	Jigglypuff: PokemonSpecies
	Wigglytuff: PokemonSpecies
	Eevee: PokemonSpecies
	Flareon: PokemonSpecies
	Jolteon: PokemonSpecies
	Vaporeon: PokemonSpecies
	Machop: PokemonSpecies
	Zubat: PokemonSpecies
	Ekans: PokemonSpecies
	Paras: PokemonSpecies
	Poliwhirl: PokemonSpecies
	Poliwrath: PokemonSpecies
	Weedle: PokemonSpecies
	Kakuna: PokemonSpecies
	Beedrill: PokemonSpecies
	Dodrio: PokemonSpecies
	Primeape: PokemonSpecies
	Dugtrio: PokemonSpecies
	Venomoth: PokemonSpecies
	Dewgong: PokemonSpecies
	Caterpie: PokemonSpecies
	Metapod: PokemonSpecies
	Butterfree: PokemonSpecies
	Machamp: PokemonSpecies
	Golduck: PokemonSpecies
	Hypno: PokemonSpecies
	Golbat: PokemonSpecies
	Mewtwo: PokemonSpecies
	Snorlax: PokemonSpecies
	Magikarp: PokemonSpecies
	Muk: PokemonSpecies
	Kingler: PokemonSpecies
	Cloyster: PokemonSpecies
	Electrode: PokemonSpecies
	Clefable: PokemonSpecies
	Weezing: PokemonSpecies
	Persian: PokemonSpecies
	Marowak: PokemonSpecies
	Haunter: PokemonSpecies
	Abra: PokemonSpecies
	Alakazam: PokemonSpecies
	Pidgeotto: PokemonSpecies
	Pidgeot: PokemonSpecies
	Starmie: PokemonSpecies
	Bulbasaur: PokemonSpecies
	Venusaur: PokemonSpecies
	Tentacruel: PokemonSpecies
	Goldeen: PokemonSpecies
	Seaking: PokemonSpecies
	Ponyta: PokemonSpecies
	Rapidash: PokemonSpecies
	Rattata: PokemonSpecies
	Raticate: PokemonSpecies
	Nidorino: PokemonSpecies
	Nidorina: PokemonSpecies
	Geodude: PokemonSpecies
	Porygon: PokemonSpecies
	Aerodactyl: PokemonSpecies
	Magnemite: PokemonSpecies
	Charmander: PokemonSpecies
	Squirtle: PokemonSpecies
	Charmeleon: PokemonSpecies
	Wartortle: PokemonSpecies
	Charizard: PokemonSpecies
	Oddish: PokemonSpecies
	Gloom: PokemonSpecies
	Vileplume: PokemonSpecies
	Bellsprout: PokemonSpecies
	Weepinbell: PokemonSpecies
	Victreebel: PokemonSpecies
	Missingno: PokemonSpecies

	@property
	def name(self) -> str: ...


class PokemonBase:
	def __init__(self, index: int): ...

	@property
	def id(self) -> PokemonSpecies: ...
	@id.setter
	def id(self, v: PokemonSpecies|int): ...

	@property
	def dex_id(self) -> int: ...
	@dex_id.setter
	def dex_id(self, v: int): ...

	@property
	def name(self) -> str: ...
	@name.setter
	def name(self, v: str): ...

	@property
	def HP(self) -> int: ...
	@HP.setter
	def HP(self, v: int): ...

	@property
	def ATK(self) -> int: ...
	@ATK.setter
	def ATK(self, v: int): ...

	@property
	def DEF(self) -> int: ...
	@DEF.setter
	def DEF(self, v: int): ...

	@property
	def SPD(self) -> int: ...
	@SPD.setter
	def SPD(self, v: int): ...

	@property
	def SPE(self) -> int: ...
	@SPE.setter
	def SPE(self, v: int): ...

	@property
	def typeA(self) -> Type: ...
	@typeA.setter
	def typeA(self, v: int): ...

	@property
	def typeB(self) -> Type: ...
	@typeB.setter
	def typeB(self, v: int): ...

	@property
	def types(self) -> tuple[Type, Type]: ...
	@types.setter
	def types(self, v: int|tuple[int, int]): ...

	@property
	def catch_rate(self) -> int: ...
	@catch_rate.setter
	def catch_rate(self, v: int): ...

	@property
	def base_xp_yield(self) -> int: ...
	@base_xp_yield.setter
	def base_xp_yield(self, v: int): ...

	@property
	def move_pool(self) -> list[AvailableMove]: ...

	@property
	def stats_at_level(self) -> list[dict]: ...

	def __getattr__(self, name): ...
	def __setattr__(self, name, value): ...


class Pokemon:
	def __init__(self, *args): ...
	def copy(self) -> Pokemon: ...
	def set_global_crit_ratio(self, ratio: float): ...
	def add_status(self, status: StatusChange, duration: int = 0): ...
	def reset_stats_changes(self): ...

	@property
	def status(self) -> StatusChange: ...
	@status.setter
	def status(self, status: StatusChange): ...

	@property
	def recharging(self) -> bool: ...
	@recharging.setter
	def recharging(self, recharging: bool): ...

	@property
	def non_volatile_status(self) -> StatusChange: ...
	@non_volatile_status.setter
	def non_volatile_status(self, status : StatusChange): ...

	@property
	def learnable_move_set(self) -> list[AvailableMove]: ...

	def change_stat(self, stat: StatsChange, nb: int): ...
	def use_move(self, move: Move, target: Pokemon): ...
	def store_damages(self, active: bool): ...
	def has_status(self, status: StatusChange): ...
	def heal(self, health: int): ...
	def take_damage(self, target: Pokemon, damage: int, ignore_substitute: bool, swap_side: bool): ...
	def use_move_slot(self, move_slot: int, target: Pokemon): ...
	def calc_damage(self, target: Pokemon, power: int, type: Type, category: MoveCategory, critical: bool, randomized: bool, half_defense: bool, swap_side: bool): ...
	def end_turn(self): ...
	def switched(self): ...
	def get_priority_factor(self, move_slot: int): ...
	def transform(self, target: Pokemon): ...

	def dump(self) -> str: ...
	def encode(self) -> list[int]: ...
	def set_invincible(self, invincible: bool): ...
	def set_move(self, index: int, move: Move): ...
	def reset(self): ...
	def apply_status_debuff(self): ...
	def can_have_status(self, status: StatusChange): ...

	def get_accuracy(self, accuracy_byte) -> int: ...
	def get_evasion(self, accuracy_byte) -> int: ...

	@property
	def random_generator(self) -> RandomGenerator: ...

	@property
	def can_get_hit(self) -> bool: ...

	@property
	def id(self) -> int: ...
	@id.setter
	def id(self, id: int): ...
	def set_id(self, id: int, recompute_stats: bool = True): ...

	@property
	def wrapped(self) -> bool: ...
	@wrapped.setter
	def wrapped(self, is_wrapped: bool): ...

	@property
	def level(self) -> int: ...
	@level.setter
	def level(self, level: int): ...

	@property
	def damages_stored(self) -> int: ...

	@property
	def attack(self) -> int: ...
	@property
	def raw_attack(self) -> int: ...

	@property
	def defense(self) -> int: ...
	@property
	def raw_defense(self) -> int: ...

	@property
	def speed(self) -> int: ...
	@property
	def raw_speed(self) -> int: ...

	@property
	def special(self) -> int: ...
	@property
	def raw_special(self) -> int: ...

	@property
	def health(self) -> int: ...
	@property
	def max_health(self) -> int: ...

	@property
	def substitute(self) -> int|None: ...

	@property
	def name(self) -> str: ...
	def get_name(self, has_enemy: bool = True) -> str: ...

	@property
	def nickname(self) -> str: ...
	@nickname.setter
	def nickname(self, nickname: str): ...

	@property
	def accuracy_mul(self) -> float: ...

	@property
	def evasion_mul(self) -> float: ...

	@property
	def last_used_move(self) -> Move: ...

	@property
	def types(self) -> tuple[Type, Type]: ...
	@types.setter
	def types(self, types: int|tuple[int, int]): ...

	@property
	def move_set(self) -> list[Move]: ...

	@property
	def base_stats(self) -> dict: ...

	@property
	def stats_upgrade_stages(self) -> dict: ...

	@property
	def species_name(self) -> str: ...

	@property
	def dvs(self) -> dict: ...

	@property
	def stat_exps(self) -> dict: ...
	@stat_exps.setter
	def stat_exps(self, d: dict): ...

	@property
	def enemy(self) -> bool: ...
