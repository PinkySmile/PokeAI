import os
from collections.abc import Callable

from PokeBattle.Gen1 import RandomGenerator
from gymnasium import Env, register
from gymnasium.spaces import Discrete, Box
from numpy import array, int16, float32, int8
from numpy.random import Generator
from torch.fx.experimental.migrate_gradual_types.operation import op_eq

from .PyBoyEmulator import PyBoyEmulator
from .BattleHandler import BattleHandler
from .Move import AvailableMove, MoveCategory, Move
from .State import BattleAction, BattleState, PlayerState
from .StatusChange import StatusChange, status_to_string
from .Pokemon import PokemonSpecies, Pokemon, PokemonBase
from .Team import load_trainer as _load_trainer
from .Type import Type, type_to_string_short, get_attack_damage_multiplier

banned_moves = [ # These moves aren't implemented properly in the engine
	# Can roll other moves from the list that aren't implemented
	AvailableMove.Metronome,

	# Confusion side effect
	AvailableMove.Confusion,
	AvailableMove.Psybeam,

	# Misc
	AvailableMove.Leech_Seed,
	AvailableMove.Disable,
	AvailableMove.Mimic,
	AvailableMove.Dream_Eater,
	AvailableMove.Transform,
]


def gen1AI(me: PlayerState, op: PlayerState, categories: list[int], random: Generator):
	me_pkmn = me.pokemon_on_field
	if me_pkmn.health == 0:
		return BattleAction(BattleAction.Switch1 + next(i for i in range(len(me.team)) if me.team[i].health))
	moves = me_pkmn.move_set
	scores = [1000 for _ in moves]
	op_pkmn = op.pokemon_on_field
	if 1 in categories and op_pkmn.has_status(StatusChange.Any_non_volatile_status):
		for i in range(len(scores)):
			if (moves[i].status_change['status'] & StatusChange.Any_non_volatile_status) and moves[i].category == MoveCategory.Status:
				scores[i] /= 8
	if 2 in categories and me.last_action in [BattleAction.Switch1, BattleAction.Switch2, BattleAction.Switch3, BattleAction.Switch4, BattleAction.Switch5, BattleAction.Switch6]:
		priority = [int(i) for i in [
			AvailableMove.Meditate, AvailableMove.Sharpen,
			AvailableMove.Defense_Curl, AvailableMove.Harden, AvailableMove.Withdraw,
			AvailableMove.Growth,
			AvailableMove.Double_Team, AvailableMove.Minimize,
			AvailableMove.Pay_Day,
			AvailableMove.Swift,
			AvailableMove.Growl,
			AvailableMove.Leer, AvailableMove.Tail_Whip,
			AvailableMove.String_Shot,
			AvailableMove.Flash, AvailableMove.Kinesis, AvailableMove.Sand_Attack, AvailableMove.Smokescreen,
			AvailableMove.Conversion,
			AvailableMove.Haze,
			AvailableMove.Swords_Dance,
			AvailableMove.Acid_Armor, AvailableMove.Barrier,
			AvailableMove.Agility,
			AvailableMove.Amnesia,
			AvailableMove.Recover, AvailableMove.Rest, AvailableMove.Softboiled,
			AvailableMove.Transform,
			AvailableMove.Screech,
			AvailableMove.Light_Screen,
			AvailableMove.Reflect,
		]]
		for i in range(len(scores)):
			if moves[i].id in priority:
				scores[i] *= 2
	if 3 in categories:
		for i in range(len(scores)):
			if moves[i].category != MoveCategory.Status:
				mul = get_attack_damage_multiplier(moves[i].type, op_pkmn.types)
				scores[i] *= mul
		priority = [int(i) for i in [
			AvailableMove.Super_Fang,
			AvailableMove.Dragon_Rage, AvailableMove.Psywave, AvailableMove.Night_Shade, AvailableMove.Seismic_Toss,
			AvailableMove.Fly
		]]
		for k, p in enumerate(priority):
			for i in range(len(scores)):
				if moves[i].id == p:
					scores[i] *= 2 + k  / 10
	if 4 in categories:
		if me_pkmn.health / me_pkmn.max_health < 0.1 and random.integers(low=0, high=63) < 5:
			for i in range(6):
				index = (me.pokemon_on_field_index + i) % 6
				if len(me.team) <= index:
					continue
				if me.team[index].health == 0:
					continue
				return BattleAction(BattleAction.Switch1 + index)
	for i in range(0, len(scores)):
		if moves[i].id == 0:
			scores[i] = -1000
	best = [0]
	for i in range(1, len(scores)):
		if scores[best[0]] < scores[i]:
			best = [i]
		elif scores[best[0]] == scores[i]:
			best.append(i)
	return BattleAction(BattleAction.Attack1 + best[random.integers(low=0, high=len(best))])


def basic_opponent(state: BattleState, rng: Generator):
	pkmn = state.op.pokemon_on_field
	if pkmn.health == 0:
		return BattleAction.Switch1 + state.op.pokemon_on_field_index + 1
	for i, move in enumerate(pkmn.move_set):
		if move.id != 0 and move.pp != 0:
			return BattleAction.Attack1 + i
	return BattleAction.StruggleMove


def gen1AI_1(state: BattleState, rng: Generator):
	return gen1AI(state.op, state.me, [1], rng)


def gen1AI_13(state: BattleState, rng: Generator):
	return gen1AI(state.op, state.me, [1, 3], rng)


class Examples:
	Brock={
		"ai": gen1AI_1,
		"sprite": 0,
		"p1name": "Ash",
		"p2name": "Brock",
		"p1team": [
			{
				"name": "PIKACHU",
				"level": 11,
				"species": PokemonSpecies.Pikachu,
				"moves": [AvailableMove.Thundershock, AvailableMove.Quick_Attack, AvailableMove.Tail_Whip, AvailableMove.Thunder_Wave]
			},
			{
				"name": "MANKEY",
				"level": 8,
				"species": PokemonSpecies.Mankey,
				"moves": [AvailableMove.Scratch, AvailableMove.Leer]
			},
			{
				"name": "NIDORAN~",
				"level": 8,
				"species": PokemonSpecies.Nidoran_M,
				"moves": [AvailableMove.Tackle, AvailableMove.Leer, AvailableMove.Horn_Attack]
			},
			{
				"name": "PIDGEY",
				"level": 9,
				"species": PokemonSpecies.Pidgey,
				"moves": [AvailableMove.Gust, AvailableMove.Sand_Attack]
			}
		],
		"p2team": [
			{
				"name": "GEODUDE",
				"level": 10,
				"species": PokemonSpecies.Geodude,
				"moves": [AvailableMove.Tackle]
			},
			{
				"name": "ONIX",
				"level": 12,
				"species": PokemonSpecies.Onix,
				"moves": [AvailableMove.Bide, AvailableMove.Bind, AvailableMove.Screech, AvailableMove.Tackle]
			}
		]
	}
	Misty={
		"ai": gen1AI_13,
		"sprite": 0,
		"p1name": "Ash",
		"p2name": "Misty",
		"p1team": [
			{
				"name": "PIKACHU",
				"level": 15,
				"species": PokemonSpecies.Pikachu,
				"moves": [AvailableMove.Thundershock, AvailableMove.Quick_Attack, AvailableMove.Double_Team, AvailableMove.Thunder_Wave]
			},
			{
				"name": "MANKEY",
				"level": 9,
				"species": PokemonSpecies.Mankey,
				"moves": [AvailableMove.Scratch, AvailableMove.Leer, AvailableMove.Low_Kick]
			},
			{
				"name": "NIDORAN~",
				"level": 9,
				"species": PokemonSpecies.Nidoran_M,
				"moves": [AvailableMove.Tackle, AvailableMove.Leer, AvailableMove.Horn_Attack]
			},
			{
				"name": "PIDGEY",
				"level": 8,
				"species": PokemonSpecies.Pidgey,
				"moves": [AvailableMove.Gust, AvailableMove.Sand_Attack]
			}
		],
		"p2team": [
			{
				"name": "STARYU",
				"level": 18,
				"species": PokemonSpecies.Staryu,
				"moves": [AvailableMove.Tackle, AvailableMove.Water_Gun]
			},
			{
				"name": "STARMIE",
				"level": 21,
				"species": PokemonSpecies.Starmie,
				"moves": [AvailableMove.Bubblebeam, AvailableMove.Harden, AvailableMove.Water_Gun, AvailableMove.Tackle]
			}
		]
	}
	LtSurge={
		"ai": gen1AI_1,
		"sprite": 0,
		"p1name": "Ash",
		"p2name": "LT. Surge",
		"p1team": [
			{
				"name": "PIKACHU",
				"level": 22,
				"species": PokemonSpecies.Pikachu,
				"moves": [AvailableMove.Thundershock, AvailableMove.Slam, AvailableMove.Double_Team, AvailableMove.Thunder_Wave]
			},
			{
				"name": "MANKEY",
				"level": 15,
				"species": PokemonSpecies.Mankey,
				"moves": [AvailableMove.Scratch, AvailableMove.Leer, AvailableMove.Low_Kick, AvailableMove.Karate_Chop]
			},
			{
				"name": "NIDORINO",
				"level": 16,
				"species": PokemonSpecies.Nidorino,
				"moves": [AvailableMove.Tackle, AvailableMove.Leer, AvailableMove.Horn_Attack, AvailableMove.Double_Kick]
			},
			{
				"name": "PIDGEY",
				"level": 15,
				"species": PokemonSpecies.Pidgey,
				"moves": [AvailableMove.Gust, AvailableMove.Sand_Attack, AvailableMove.Quick_Attack]
			}
		],
		"p2team": [
			{
				"name": "RAICHU",
				"level": 28,
				"species": PokemonSpecies.Raichu,
				"moves": [AvailableMove.Growl, AvailableMove.Mega_Kick, AvailableMove.Mega_Punch, AvailableMove.Thunderbolt]
			}
		]
	}
	Erika={
		"ai": gen1AI_13,
		"sprite": 0,
		"p1name": "Ash",
		"p2name": "Erika",
		"p1team": [
			{
				"name": "PIKACHU",
				"level": 28,
				"species": PokemonSpecies.Pikachu,
				"moves": [AvailableMove.Thunderbolt, AvailableMove.Slam, AvailableMove.Double_Team, AvailableMove.Thunder_Wave]
			},
			{
				"name": "MANKEY",
				"level": 23,
				"species": PokemonSpecies.Mankey,
				"moves": [AvailableMove.Fury_Swipes, AvailableMove.Leer, AvailableMove.Low_Kick, AvailableMove.Karate_Chop]
			},
			{
				"name": "NIDOKING",
				"level": 26,
				"species": PokemonSpecies.Nidoking,
				"moves": [AvailableMove.Thrash, AvailableMove.Leer, AvailableMove.Horn_Attack, AvailableMove.Double_Kick]
			},
			{
				"name": "PIDGEOTTO",
				"level": 25,
				"species": PokemonSpecies.Pidgeotto,
				"moves": [AvailableMove.Gust, AvailableMove.Sand_Attack, AvailableMove.Quick_Attack, AvailableMove.Whirlwind]
			}
		],
		"p2team": [
			{
				"name": "TANGELA",
				"level": 30,
				"species": PokemonSpecies.Tangela,
				"moves": [AvailableMove.Bind, AvailableMove.Constrict, AvailableMove.Mega_Drain, AvailableMove.Vine_Whip]
			},
			{
				"name": "WEEPINBELL",
				"level": 32,
				"species": PokemonSpecies.Weepinbell,
				"moves": [AvailableMove.Acid, AvailableMove.Razor_Leaf, AvailableMove.Sleep_Powder, AvailableMove.Stun_Spore]
			},
			{
				"name": "GLOOM",
				"level": 32,
				"species": PokemonSpecies.Gloom,
				"moves": [AvailableMove.Acid, AvailableMove.Petal_Dance, AvailableMove.Sleep_Powder, AvailableMove.Stun_Spore]
			}
		]
	}
	Sabrina={
		"ai": gen1AI_1,
		"sprite": 0,
		"p1name": "Ash",
		"p2name": "Sabrina",
		"p1team": [
			{
				"name": "PIKACHU",
				"level": 46,
				"species": PokemonSpecies.Pikachu,
				"moves": [AvailableMove.Thunderbolt, AvailableMove.Surf, AvailableMove.Body_Slam, AvailableMove.Thunder_Wave]
			},
			{
				"name": "PRIMEAPE",
				"level": 41,
				"species": PokemonSpecies.Primeape,
				"moves": [AvailableMove.Seismic_Toss, AvailableMove.Rock_Slide, AvailableMove.Hyper_Beam, AvailableMove.Body_Slam]
			},
			{
				"name": "NIDOKING",
				"level": 40,
				"species": PokemonSpecies.Nidoking,
				"moves": [AvailableMove.Earthquake, AvailableMove.Thunderbolt, AvailableMove.Blizzard, AvailableMove.Substitute]
			},
			{
				"name": "PIDGEOT",
				"level": 41,
				"species": PokemonSpecies.Pidgeot,
				"moves": [AvailableMove.Double_Edge, AvailableMove.Hyper_Beam, AvailableMove.Agility, AvailableMove.Wing_Attack]
			}
		],
		"p2team": [
			{
				"name": "ABRA",
				"level": 50,
				"species": PokemonSpecies.Abra,
				"moves": [AvailableMove.Flash]
			},
			{
				"name": "KADABRA",
				"level": 50,
				"species": PokemonSpecies.Kadabra,
				"moves": [AvailableMove.Kinesis, AvailableMove.Psychic_M, AvailableMove.Psywave, AvailableMove.Recover]
			},
			{
				"name": "ALAKAZAM",
				"level": 50,
				"species": PokemonSpecies.Alakazam,
				"moves": [AvailableMove.Psychic_M, AvailableMove.Psywave, AvailableMove.Recover, AvailableMove.Reflect]
			}
		]
	}
	Blaine={
		"ai": gen1AI_1,
		"sprite": 0,
		"p1name": "Ash",
		"p2name": "Blaine",
		"p1team": [
			{
				"name": "PIKACHU",
				"level": 55,
				"species": PokemonSpecies.Pikachu,
				"moves": [AvailableMove.Thunderbolt, AvailableMove.Surf, AvailableMove.Body_Slam, AvailableMove.Thunder_Wave]
			},
			{
				"name": "PRIMEAPE",
				"level": 45,
				"species": PokemonSpecies.Primeape,
				"moves": [AvailableMove.Seismic_Toss, AvailableMove.Rock_Slide, AvailableMove.Hyper_Beam, AvailableMove.Body_Slam]
			},
			{
				"name": "NIDOKING",
				"level": 45,
				"species": PokemonSpecies.Nidoking,
				"moves": [AvailableMove.Earthquake, AvailableMove.Thunderbolt, AvailableMove.Blizzard, AvailableMove.Substitute]
			},
			{
				"name": "PIDGEOT",
				"level": 47,
				"species": PokemonSpecies.Pidgeot,
				"moves": [AvailableMove.Double_Edge, AvailableMove.Hyper_Beam, AvailableMove.Agility, AvailableMove.Wing_Attack]
			}
		],
		"p2team": [
			{
				"name": "NINETALES",
				"level": 48,
				"species": PokemonSpecies.Ninetales,
				"moves": [AvailableMove.Confuse_Ray, AvailableMove.Flamethrower, AvailableMove.Quick_Attack, AvailableMove.Tail_Whip]
			},
			{
				"name": "RAPIDASH",
				"level": 50,
				"species": PokemonSpecies.Rapidash,
				"moves": [AvailableMove.Fire_Spin, AvailableMove.Growl, AvailableMove.Stomp, AvailableMove.Take_Down]
			},
			{
				"name": "ARCANINE",
				"level": 54,
				"species": PokemonSpecies.Arcanine,
				"moves": [AvailableMove.Fire_Blast, AvailableMove.Flamethrower, AvailableMove.Reflect, AvailableMove.Take_Down]
			}
		]
	}
	Giovanni={
		"ai": gen1AI_13,
		"sprite": 0,
		"p1name": "Ash",
		"p2name": "Giovanni",
		"p1team": [
			{
				"name": "PIKACHU",
				"level": 60,
				"species": PokemonSpecies.Pikachu,
				"moves": [AvailableMove.Thunderbolt, AvailableMove.Surf, AvailableMove.Body_Slam, AvailableMove.Thunder_Wave]
			},
			{
				"name": "PRIMEAPE",
				"level": 49,
				"species": PokemonSpecies.Primeape,
				"moves": [AvailableMove.Seismic_Toss, AvailableMove.Rock_Slide, AvailableMove.Hyper_Beam, AvailableMove.Body_Slam]
			},
			{
				"name": "NIDOKING",
				"level": 47,
				"species": PokemonSpecies.Nidoking,
				"moves": [AvailableMove.Earthquake, AvailableMove.Thunderbolt, AvailableMove.Blizzard, AvailableMove.Substitute]
			},
			{
				"name": "PIDGEOT",
				"level": 46,
				"species": PokemonSpecies.Pidgeot,
				"moves": [AvailableMove.Double_Edge, AvailableMove.Hyper_Beam, AvailableMove.Agility, AvailableMove.Wing_Attack]
			}
		],
		"p2team": [
			{
				"name": "DUGTRIO",
				"level": 50,
				"species": PokemonSpecies.Dugtrio,
				"moves": [AvailableMove.Dig, AvailableMove.Earthquake, AvailableMove.Fissure, AvailableMove.Sand_Attack]
			},
			{
				"name": "PERSIAN",
				"level": 53,
				"species": PokemonSpecies.Persian,
				"moves": [AvailableMove.Double_Team, AvailableMove.Fury_Swipes, AvailableMove.Screech, AvailableMove.Slash]
			},
			{
				"name": "NIDOQUEEN",
				"level": 53,
				"species": PokemonSpecies.Nidoqueen,
				"moves": [AvailableMove.Double_Kick, AvailableMove.Earthquake, AvailableMove.Tail_Whip, AvailableMove.Thunder]
			},
			{
				"name": "NIDOKING",
				"level": 55,
				"species": PokemonSpecies.Nidoking,
				"moves": [AvailableMove.Double_Kick, AvailableMove.Earthquake, AvailableMove.Leer, AvailableMove.Thunder]
			},
			{
				"name": "RHYDON",
				"level": 55,
				"species": PokemonSpecies.Rhydon,
				"moves": [AvailableMove.Earthquake, AvailableMove.Fury_Attack, AvailableMove.Horn_Drill, AvailableMove.Rock_Slide]
			}
		]
	}


def load_trainer(path: str):
	state = BattleState()
	with open(path, "rb") as fd:
		data = fd.read()
	name, team = _load_trainer(data, state)
	return name, [{
		"name": p.name,
		"level": p.level,
		"species": p.id,
		"moves": [m.id for m in p.move_set]
	} for p in team]


class PokemonYellowBattle(Env):
	metadata = {
		'render_modes': ["human", "ansi", "rgb_array_list"],
		'render_fps': 10
	}
	action_space = Discrete(11)
	observation_space = Box(
		low= array([
			# Offset 0
			# My pokémon on field
			# HP,  MaxHP,
			  0,   0,
			# ATK, DEF, SPD, SPE
			  0,   0,   0,   0,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			# SLP, PSN, BRN, FRZ, PAR, TOX, LEE, CFZ
			  0,   0,   0,   0,   0,   0,   0,   0,
			# Upgrade stages
			# ATK, DEF, SPD, SPE, ACC, EVD
			  0,   0,   0,   0,   0,   0,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  0,       0,       -10,     -10,     -10,     -10,     -10,     -10,

			# Offset 43
			# Opponent pokémon on field
			# HP,  MaxHP,
			  0,   0,
			# ATK, DEF, SPD, SPE
			  0,   0,   0,   0,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			# SLP, PSN, BRN, FRZ, PAR, TOX, LEE, CFZ
			  0,   0,   0,   0,   0,   0,   0,   0,
			# Upgrade stages
			# ATK, DEF, SPD, SPE, ACC, EVD
			  0,   0,   0,   0,   0,   0,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  -1,      -1,      -10,     -10,     -10,     -10,     -10,     -10,

			# Offset 86
			# Pokémon 1 in team
			# HP,  MaxHP,
			  0,   0,
			# ATK, DEF, SPD, SPE
			  0,   0,   0,   0,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			# SLP, PSN, BRN, FRZ, PAR
			  0,   0,   0,   0,   0,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  0,       0,       -10,     -10,     -10,     -10,     -10,     -10,
			# Offset 120
			# Pokémon 2 in team
			# HP,  MaxHP,
			 -10,  -10,
			# ATK, DEF, SPD, SPE
			  -10, -10, -10, -10,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
			# SLP, PSN, BRN, FRZ, PAR
			  -10, -10, -10, -10, -10,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  -10,     -10,     -10,     -10,     -10,     -10,     -10,     -10,
			# Offset 154
			# Pokémon 3 in team
			# HP,  MaxHP,
			  -10,  -10,
			# ATK, DEF, SPD, SPE
			  -10, -10, -10, -10,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
			# SLP, PSN, BRN, FRZ, PAR
			  -10, -10, -10, -10, -10,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  -10,     -10,     -10,     -10,     -10,     -10,     -10,     -10,
			# Offset 188
			# Pokémon 4 in team
			# HP,  MaxHP,
			  -10,  -10,
			# ATK, DEF, SPD, SPE
			  -10, -10, -10, -10,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
			# SLP, PSN, BRN, FRZ, PAR
			  -10, -10, -10, -10, -10,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  -10,     -10,       -10,   -10,     -10,     -10,     -10,     -10,
			# Offset 222
			# Pokémon 5 in team
			# HP,  MaxHP,
			  -10,  -10,
			# ATK, DEF, SPD, SPE
			  -10, -10, -10, -10,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
			# SLP, PSN, BRN, FRZ, PAR
			  -10, -10, -10, -10, -10,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  -10,     -10,     -10,     -10,     -10,     -10,     -10,     -10,
			# Offset 256
			# Pokémon 6 in team
			# HP,  MaxHP,
			  -10,  -10,
			# ATK, DEF, SPD, SPE
			  -10, -10, -10, -10,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
			# SLP, PSN, BRN, FRZ, PAR
			  -10, -10, -10, -10, -10,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  -10,     -10,     -10,     -10,     -10,     -10,     -10,     -10,


			# Offset 290
			# Pokémon 1 in opponent's team
			# HP,  MaxHP,
			  0,   0,
			# ATK, DEF, SPD, SPE
			  0,   0,   0,   0,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			# SLP, PSN, BRN, FRZ, PAR
			  0,   0,   0,   0,   0,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  -10,    -10,      -10,     -10,     -10,     -10,     -10,     -10,
			# Offset 324
			# Pokémon 2 in opponent's team
			# HP,  MaxHP,
			 -10,  -10,
			# ATK, DEF, SPD, SPE
			  -10, -10, -10, -10,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
			# SLP, PSN, BRN, FRZ, PAR
			  -10, -10, -10, -10, -10,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  -10,     -10,     -10,     -10,     -10,     -10,     -10,     -10,
			# Offset 358
			# Pokémon 3 in opponent's team
			# HP,  MaxHP,
			  -10,  -10,
			# ATK, DEF, SPD, SPE
			  -10, -10, -10, -10,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
			# SLP, PSN, BRN, FRZ, PAR
			  -10, -10, -10, -10, -10,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  -10,     -10,     -10,     -10,     -10,     -10,     -10,     -10,
			# Offset 392
			# Pokémon 4 in opponent's team
			# HP,  MaxHP,
			  -10,  -10,
			# ATK, DEF, SPD, SPE
			  -10, -10, -10, -10,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
			# SLP, PSN, BRN, FRZ, PAR
			  -10, -10, -10, -10, -10,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  -10,     -10,       -10,   -10,     -10,     -10,     -10,     -10,
			# Offset 426
			# Pokémon 5 in opponent's team
			# HP,  MaxHP,
			  -10,  -10,
			# ATK, DEF, SPD, SPE
			  -10, -10, -10, -10,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
			# SLP, PSN, BRN, FRZ, PAR
			  -10, -10, -10, -10, -10,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  -10,     -10,     -10,     -10,     -10,     -10,     -10,     -10,
			# Offset 460
			# Pokémon 6 in opponent's team
			# HP,  MaxHP,
			  -10,  -10,
			# ATK, DEF, SPD, SPE
			  -10, -10, -10, -10,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
			# SLP, PSN, BRN, FRZ, PAR
			  -10, -10, -10, -10, -10,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  -10,     -10,     -10,     -10,     -10,     -10,     -10,     -10,
			# Offset 494
		], dtype=float32),
		high=array([
			# My pokémon on field
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR, TOX, LEE, CFZ
			  1,   1,   1,   1,   1,   1,   1,   1,
			# Upgrade stages
			# ATK, DEF, SPD, SPE, ACC, EVD
			  12,  12,  12,  12,  12,  12,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
			
			# My opponent on field
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR, TOX, LEE, CFZ
			  1,   1,   1,   1,   1,   1,   1,   1,
			# Upgrade stages
			# ATK, DEF, SPD, SPE, ACC, EVD
			  12,  12,  12,  12,  12,  12,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,

			# Pokémon 1 in team
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
			# Pokémon 2 in team
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
			# Pokémon 3 in team
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
			# Pokémon 4 in team
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
			# Pokémon 5 in team
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
			# Pokémon 6 in team
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,

			# Pokémon 1 in opponent's team
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
			# Pokémon 2 in opponent's team
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
			# Pokémon 3 in opponent's team
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
			# Pokémon 4 in opponent's team
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
			# Pokémon 5 in opponent's team
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
			# Pokémon 6 in opponent's team
			# HP,  MaxHP,
			  999, 999,
			# ATK, DEF, SPD, SPE
			  999, 999, 999, 999,
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fir, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
		], dtype=float32),
		shape=(494,),
		dtype=float32
	)


	battle: BattleHandler
	op: Callable
	base_op: Callable
	max_turns: int
	current_turn: int
	render_mode: str|None
	last_frames: list
	episode_id: int
	episode_trigger: Callable|None
	recording: bool
	shuffle_teams: bool
	replay_folder: str
	emulator: PyBoyEmulator|None
	messages: list


	def __init__(self, render_mode: str|None=None, episode_trigger: Callable[[], bool]=None, opponent_callback: Callable[[BattleState, Generator], BattleAction]=basic_opponent, replay_folder: str|None=None, shuffle_teams: bool=False):
		self.battle = BattleHandler(False, False)
		self.op = opponent_callback
		self.base_op = opponent_callback
		self.max_turns = -1
		self.current_turn = 0
		self.render_mode = render_mode
		self.last_frames = []
		self.episode_id = 0
		self.episode_trigger = episode_trigger
		self.recording = False
		self.shuffle_teams = shuffle_teams
		self.replay_folder = replay_folder
		if self.render_mode == "human":
			self.emulator = PyBoyEmulator(has_interface=True, sound_volume=25, save_frames=False)
			self.emulator.on_text_displayed = print
		elif self.render_mode == "rgb_array_list":
			self.emulator = PyBoyEmulator(has_interface=False, sound_volume=0, save_frames=True)
		else:
			self.emulator = None
		self.messages = []
		if self.render_mode == "ansi":
			self.battle.state.logger = lambda x: self.messages.append(x)


	def step_emulator(self, state: BattleState):
		if self.emulator is None or not self.recording:
			return
		self.emulator.step(state)


	def init_emulator(self, state: BattleState):
		if self.emulator is None or not self.recording:
			return
		with open("pokeyellow_replay.state", "rb") as fd:
			self.emulator.init_battle(fd, state)


	@staticmethod
	def observe_pokemon_on_field(me_state: PlayerState, op_state: PlayerState|None):
		pokemon = me_state.pokemon_on_field
		boosts = pokemon.stats_upgrade_stages
		status = pokemon.status
		status = [
			bool(status & StatusChange.Asleep),
			bool(status & StatusChange.Poisoned),
			bool(status & StatusChange.Burned),
			bool(status & StatusChange.Frozen),
			bool(status & StatusChange.Paralyzed),
			bool(status & StatusChange.Badly_poisoned),
			bool(status & StatusChange.Leeched),
			bool(status & StatusChange.Confused)
		]
		types = pokemon.types
		types = [
			types[0] == Type.Normal   or types[1] == Type.Normal,
			types[0] == Type.Fighting or types[1] == Type.Fighting,
			types[0] == Type.Flying   or types[1] == Type.Flying,
			types[0] == Type.Poison   or types[1] == Type.Poison,
			types[0] == Type.Ground   or types[1] == Type.Ground,
			types[0] == Type.Rock     or types[1] == Type.Rock,
			types[0] == Type.Bug      or types[1] == Type.Bug,
			types[0] == Type.Ghost    or types[1] == Type.Ghost,
			types[0] == Type.Fire     or types[1] == Type.Fire,
			types[0] == Type.Water    or types[1] == Type.Water,
			types[0] == Type.Grass    or types[1] == Type.Grass,
			types[0] == Type.Electric or types[1] == Type.Electric,
			types[0] == Type.Psychic  or types[1] == Type.Psychic,
			types[0] == Type.Ice      or types[1] == Type.Ice,
			types[0] == Type.Dragon   or types[1] == Type.Dragon,
		]
		move_set = pokemon.move_set
		move_set += [None] * (4 - len(move_set))
		moves = [
			-1 if op_state is not None and not op_state.is_pkmn_move_discovered(me_state.pokemon_on_field_index, k) else i
			for k, s in enumerate(move_set) for i in ([s.id, s.pp] if s is not None else [-10, -10])
		]
		return [
			pokemon.health, pokemon.max_health,
			pokemon.attack, pokemon.defense, pokemon.speed, pokemon.special,
			*types,
			*status,
			boosts['ATK'] + 6, boosts['DEF'] + 6, boosts['SPD'] + 6, boosts['SPE'] + 6,
			boosts['ACC'] + 6, boosts['EVD'] + 6,
			*moves
		]


	@staticmethod
	def observe_pokemon(pokemon: Pokemon, slot: int, op_state: PlayerState|None):
		if op_state and not op_state.is_pkmn_discovered(slot):
			return [-1] * 34
		status = pokemon.non_volatile_status
		status = [
			bool(status & StatusChange.Asleep),
			bool(status & StatusChange.Poisoned),
			bool(status & StatusChange.Burned),
			bool(status & StatusChange.Frozen),
			bool(status & StatusChange.Paralyzed),
		]
		types = pokemon.types
		types = [
			types[0] == Type.Normal   or types[1] == Type.Normal,
			types[0] == Type.Fighting or types[1] == Type.Fighting,
			types[0] == Type.Flying   or types[1] == Type.Flying,
			types[0] == Type.Poison   or types[1] == Type.Poison,
			types[0] == Type.Ground   or types[1] == Type.Ground,
			types[0] == Type.Rock     or types[1] == Type.Rock,
			types[0] == Type.Bug      or types[1] == Type.Bug,
			types[0] == Type.Ghost    or types[1] == Type.Ghost,
			types[0] == Type.Fire     or types[1] == Type.Fire,
			types[0] == Type.Water    or types[1] == Type.Water,
			types[0] == Type.Grass    or types[1] == Type.Grass,
			types[0] == Type.Electric or types[1] == Type.Electric,
			types[0] == Type.Psychic  or types[1] == Type.Psychic,
			types[0] == Type.Ice      or types[1] == Type.Ice,
			types[0] == Type.Dragon   or types[1] == Type.Dragon,
		]
		move_set = pokemon.move_set
		move_set += [None] * (4 - len(move_set))
		moves = [
			-1 if op_state is not None and not op_state.is_pkmn_move_discovered(slot, k) else i
			for k, s in enumerate(move_set) for i in ([s.id, s.pp] if s is not None else [-1, -1])
		]
		return [
			pokemon.health,     pokemon.max_health,
			pokemon.raw_attack, pokemon.raw_defense, pokemon.raw_speed, pokemon.raw_special,
			*types, *status, *moves
		]


	def make_observation(self, state: BattleState):
		me_team = state.me.team
		op_team = state.op.team
		ob = self.observe_pokemon_on_field(state.me, None)
		ob += self.observe_pokemon_on_field(state.op, state.me)
		for i in range(6):
			if len(me_team) > i:
				ob += self.observe_pokemon(me_team[i], i, None)
			else:
				ob += [-10] * 34
		for i in range(6):
			if len(op_team) > i:
				ob += self.observe_pokemon(op_team[i], i, state.me)
			else:
				ob += [-10] * 34
		ob = array(ob, dtype=float32)
		pkmn = state.me.pokemon_on_field
		if pkmn.health == 0:
			move_mask = [False, False, False, False]
			can_use_struggle = False
		else:
			move_mask = [int(m.id != 0 and m.pp != 0) for m in pkmn.move_set] + [False for _ in range(len(pkmn.move_set), 4)]
			can_use_struggle = int(not any(move_mask))
		switch_mask = [int(len(state.me.team) > i and state.me.pokemon_on_field_index != i and state.me.team[i].health > 0) for i in range(6)]
		result = move_mask + switch_mask + [can_use_struggle]
		if not any(result):
			result = [1] * self.action_space.n
		return ob, { 'mask': array(result, dtype=int8) }


	def compute_reward(self, old: BattleState, new: BattleState):
		if all(f.health == 0 for f in new.op.team):
			return 100 / self.current_turn
		return sum(p.health for p in new.me.team) / sum(p.max_health for p in new.me.team) - sum(p.health for p in new.op.team) / sum(p.max_health for p in new.op.team)


	def step(self, action):
		state: BattleState = self.battle.state
		if action == 10:
			state.me.next_action = BattleAction.StruggleMove
		else:
			state.me.next_action = BattleAction.Attack1 + action
		state.op.next_action = self.op(state, self.np_random)
		old = state.copy()
		if state.me.next_action == BattleAction.StruggleMove:
			assert state.me.pokemon_on_field.health != 0
			assert all(m.pp == 0 or m.id == 0 for m in state.me.pokemon_on_field.move_set)
		if BattleAction.Attack1 <= state.me.next_action <= BattleAction.Attack4:
			assert state.me.pokemon_on_field.health != 0
			assert state.me.next_action - BattleAction.Attack1 < len(state.me.pokemon_on_field.move_set)
			assert state.me.pokemon_on_field.move_set[state.me.next_action - BattleAction.Attack1].id != 0
			assert state.me.pokemon_on_field.move_set[state.me.next_action - BattleAction.Attack1].pp != 0
		if BattleAction.Switch1 <= state.me.next_action <= BattleAction.Switch6:
			assert state.me.next_action - BattleAction.Switch1 != state.me.pokemon_on_field_index
			assert state.me.next_action - BattleAction.Switch1 < len(state.me.team)
			assert state.me.team[state.me.next_action - BattleAction.Switch1].health != 0
		self.battle.tick()
		self.current_turn += 1
		if self.battle.finished and self.replay_folder:
			self.battle.save_replay(os.path.join(self.replay_folder, f"episode-{self.episode_id}.replay"))
		observation, info = self.make_observation(state)
		self.step_emulator(state)
		# self.spec.max_episode_steps
		return observation, self.compute_reward(old, state), self.battle.finished, False, info


	def reset(self, seed=None, options=None):
		super().reset(seed=seed)
		self.battle.reset()
		self.current_turn = 0
		self.episode_id += 1
		if self.episode_trigger:
			self.recording = self.episode_trigger(self.episode_id)
		else:
			self.recording = True
		state = self.battle.state
		if options is None:
			if self.shuffle_teams:
				state.me.team = self.np_random.permutation([Pokemon(
					self.battle.state,
					p.get_name(False),
					p.level,
					PokemonBase(p.id),
					self.np_random.permutation(list(Move(m.id) for m in p.move_set if m.id))
				) for p in state.me.team])
				state.op.team = self.np_random.permutation([Pokemon(
					self.battle.state,
					p.get_name(False),
					p.level,
					PokemonBase(p.id),
					self.np_random.permutation(list(Move(m.id) for m in p.move_set if m.id))
				) for p in state.op.team])
		else:
			self.op = options.get("ai", self.base_op)
			state.me.name = options["p1name"]
			state.op.name = options["p2name"]
			for p in options["p1team"]:
				for m in p["moves"]:
					if m in banned_moves:
						print(f"Warning: {AvailableMove(m).name} in team 1, on {p["name"]} doesn't work and has been replaced with Pound")
			for p in options["p2team"]:
				for m in p["moves"]:
					if m in banned_moves:
						print(f"Warning: {AvailableMove(m).name} in team 2, on {p["name"]} doesn't work and has been replaced with Pound")
			state.me.team = [Pokemon(self.battle.state, p["name"], p["level"], PokemonBase(p["species"]), [Move(AvailableMove.Pound if m in banned_moves else m) for m in p["moves"]]) for p in options["p1team"]]
			state.op.team = [Pokemon(self.battle.state, p["name"], p["level"], PokemonBase(p["species"]), [Move(AvailableMove.Pound if m in banned_moves else m) for m in p["moves"]]) for p in options["p2team"]]
		state.rng.list = [self.np_random.integers(low=0, high=255) for _ in range(9)]

		self.init_emulator(state)
		return self.make_observation(state)


	@staticmethod
	def serialize_mon(s: Pokemon, i: int, op: PlayerState):
		r = f'?????????? ({s.species_name: >10}) l{s.level:03d}, {type_to_string_short(s.types[0])}'
		if s.types[0] != s.types[1]:
			r += f'/{type_to_string_short(s.types[1])}'
		if not op.is_pkmn_discovered(i):
			r += " (Not revealed yet)"
		if op.pokemon_on_field_index == i:
			r += " (Active)"
		r += "\n - "
		r += f'{s.health:03d}/{s.max_health:03d}HP, '
		r += f'{s.attack:03d}ATK (???@+0), '
		r += f'{s.defense:03d}DEF (???@+0), '
		r += f'{s.special:03d}SPE (???@+0), '
		r += f'{s.speed: >3d}SPD (???@+0), '
		r += f'100%ACC (+0), '
		r += f'100%EVD (+0)'
		r += f'\n - Status: 0x{s.non_volatile_status:02X} {status_to_string(s.non_volatile_status)}, '
		r += f'\n - Moves: {", ".join(f'{m.name} {m.pp:02d}/{m.max_pp:02d}PP{"" if op.is_pkmn_move_discovered(i, _i) else " (Not yet revealed)"}' for _i, m in enumerate(s.move_set) if m)}'
		return r


	def render(self):
		if self.render_mode == 'ansi':
			state = self.battle.state
			messages = "\n".join(self.messages)
			self.messages = []
			p1 = state.me.name + "'s team (P1)\n" + "\n".join(self.serialize_mon(s, i, state.op) for i, s in enumerate(state.me.team))
			p2 = state.op.name + "'s team (P2)\n" + "\n".join(self.serialize_mon(s, i, state.me) for i, s in enumerate(state.op.team))
			return messages + "\n" + p1 + "\n" + p2
		if self.render_mode == 'rgb_array_list':
			return self.emulator.get_last_frames()
		return None


	def close(self):
		if self.emulator is not None:
			self.emulator.stop()


register('PokemonYellow', PokemonYellowBattle)