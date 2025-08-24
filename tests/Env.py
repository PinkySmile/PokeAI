import os
from gymnasium import Env, register
from gymnasium.spaces import Discrete, Box
from numpy import array, int16, float32, int8
from Emulator import Emulator
from GameEngine import Type, BattleHandler, BattleState, BattleAction, StatusChange, MoveCategory, PokemonSpecies, AvailableMove, convertString, typeToStringShort, getAttackDamageMultiplier, statusToString, Pokemon, PokemonBase, Move, loadTrainer as __loadTrainer

banned_moves = [ # These moves aren't implemented properly in the engine
	# Invul turn 1
	AvailableMove.Dig,
	AvailableMove.Fly,

	# Repeat moves
	AvailableMove.Rage,
	AvailableMove.Thrash,
	AvailableMove.Petal_Dance,

	# Some attacking moves
	AvailableMove.Hyper_Beam,
	AvailableMove.Submission,
	AvailableMove.Psybeam,
	AvailableMove.Mega_Kick,
	AvailableMove.Hydro_Pump,
	AvailableMove.Blizzard,
	AvailableMove.Fire_Blast,
	AvailableMove.Confusion,

	# Screens
	AvailableMove.Light_Screen,
	AvailableMove.Reflect,

	# Misc
	AvailableMove.Counter,
	AvailableMove.Transform,
	AvailableMove.Dream_Eater,
	AvailableMove.Mimic,
	AvailableMove.Disable,
	AvailableMove.Agility,
	AvailableMove.Metronome,
]


def gen1AI(me, op, categories, random):
	mepkmn = me.team[me.pokemonOnField]
	if mepkmn.getHealth() == 0:
		return BattleAction(BattleAction.Switch1 + next(i for i in range(len(me.team)) if me.team[i].getHealth()))
	moves = mepkmn.getMoveSet()
	scores = [1000 for _ in moves]
	oppkmn = op.team[op.pokemonOnField]
	if 1 in categories and oppkmn.hasStatus(StatusChange.Any_non_volatile_status):
		for i in range(len(scores)):
			if (moves[i].getStatusChange()['status'] & StatusChange.Any_non_volatile_status) and moves[i].getCategory() == MoveCategory.Status:
				scores[i] /= 8
	if 2 in categories and me.lastAction in [BattleAction.Switch1, BattleAction.Switch2, BattleAction.Switch3, BattleAction.Switch4, BattleAction.Switch5, BattleAction.Switch6]:
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
			AvailableMove.Flash, AvailableMove.Kinesis, AvailableMove.Sand_Attack, AvailableMove.SmokeScreen,
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
			if moves[i].getID() in priority:
				scores[i] *= 2
	if 3 in categories:
		for i in range(len(scores)):
			if moves[i].getCategory() != MoveCategory.Status:
				mul = getAttackDamageMultiplier(moves[i].getType(), oppkmn.getTypes())
				scores[i] *= mul
		priority = [int(i) for i in [
			AvailableMove.Super_Fang,
			AvailableMove.Dragon_Rage, AvailableMove.Psywave, AvailableMove.Night_Shade, AvailableMove.Seismic_Toss,
			AvailableMove.Fly
		]]
		for k, p in enumerate(priority):
			for i in range(len(scores)):
				if moves[i].getID() == p:
					scores[i] *= 2 + k  / 10
	if 4 in categories:
		if mepkmn.getHealth() / mepkmn.getMaxHealth() < 0.1 and random.integers(low=0, high=63) < 5:
			for i in range(6):
				index = (me.pokemonOnField + i) % 6
				if len(me.team) <= index:
					continue
				if me.team[index].getHealth() == 0:
					continue
				return BattleAction(BattleAction.Switch1 + index)
	for i in range(0, len(scores)):
		if moves[i].getID() == 0:
			scores[i] = -1000
	best = [0]
	for i in range(1, len(scores)):
		if scores[best[0]] < scores[i]:
			best = [i]
		elif scores[best[0]] == scores[i]:
			best.append(i)
	return BattleAction(BattleAction.Attack1 + best[random.integers(low=0, high=len(best))])


def basic_opponent(state, rng):
	pkmn = state.op.team[state.op.pokemonOnField]
	if pkmn.getHealth() == 0:
		return BattleAction.Switch1 + state.op.pokemonOnField + 1
	for i, move in enumerate(pkmn.getMoveSet()):
		if move.getID() != 0 and move.getPP() != 0:
			return BattleAction.Attack1 + i
	return BattleAction.StruggleMove


def gen1AI_1(state, rng):
	return gen1AI(state.op, state.me, [1], rng)


def gen1AI_13(state, rng):
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
				"moves": [AvailableMove.Bubble_Beam, AvailableMove.Harden, AvailableMove.Water_Gun, AvailableMove.Tackle]
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


def loadTrainer(path):
	state = BattleState()
	with open(path, "rb") as fd:
		data = fd.read()
	name, team = __loadTrainer(data, state)
	return name, [{
		"name": p.getName(False),
		"level": p.getLevel(),
		"species": p.getID(),
		"moves": [m.getID() for m in p.getMoveSet()]
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
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
			# Nor, Fgt, Fly, Psn, Gnd, Roc, Bug, Gst, Fre, Wtr, Grs, Elc, Psy, Ice, Dgn
			  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			# SLP, PSN, BRN, FRZ, PAR
			  1,   1,   1,   1,   1,
			# Move1ID, Move1PP, Move2ID, Move2PP, Move3ID, Move3PP, Move4ID, Move4PP
			  164,     40,      164,     40,      164,     40,      164,     40,
		], dtype=float32),
		shape=(494,),
		dtype=float32
	)

	def __init__(self, render_mode=None, episode_trigger=None, opponent_callback=basic_opponent, replay_folder=None, shuffle_teams=False):
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
		self.last_state = None
		self.replay_folder = replay_folder
		if self.render_mode == "human":
			self.emulator = Emulator(has_interface=True, sound_volume=25, save_frames=False)
		elif self.render_mode == "rgb_array_list":
			self.emulator = Emulator(has_interface=False, sound_volume=0, save_frames=True)
		else:
			self.emulator = None
		self.messages = []
		if self.render_mode == "ansi":
			self.battle.state.battleLogger = lambda x: self.messages.append(x)


	def step_emulator(self, state):
		if self.emulator is None or not self.recording:
			return
		self.emulator.step(state)


	def init_emulator(self, state):
		if self.emulator is None or not self.recording:
			return
		with open("pokeyellow_replay.state", "rb") as fd:
			self.emulator.init_battle(fd, state)


	@staticmethod
	def observe_pokemon_on_field(me_state, op_state):
		pokemon = me_state.team[me_state.pokemonOnField]
		boosts = pokemon.getStatsUpgradeStages()
		status = pokemon.getStatus()
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
		types = pokemon.getTypes()
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
		move_set = pokemon.getMoveSet()
		move_set += [None] * (4 - len(move_set))
		moves = [
			-1 if op_state is not None and not op_state.isPkmnMoveDiscovered(me_state.pokemonOnField, k) else i
			for k, s in enumerate(move_set) for i in ([s.getID(), s.getPP()] if s is not None else [-10, -10])
		]
		return [
			pokemon.getHealth(), pokemon.getMaxHealth(),
			pokemon.getAttack(), pokemon.getDefense(), pokemon.getSpeed(), pokemon.getSpecial(),
			*types,
			*status,
			boosts['ATK'] + 6, boosts['DEF'] + 6, boosts['SPD'] + 6, boosts['SPE'] + 6,
			boosts['ACC'] + 6, boosts['EVD'] + 6,
			*moves
		]


	@staticmethod
	def observe_pokemon(pokemon, slot, op_state):
		if op_state and not op_state.isPkmnDiscovered(slot):
			return [-1] * 34
		status = pokemon.getNonVolatileStatus()
		status = [
			bool(status & StatusChange.Asleep),
			bool(status & StatusChange.Poisoned),
			bool(status & StatusChange.Burned),
			bool(status & StatusChange.Frozen),
			bool(status & StatusChange.Paralyzed),
		]
		types = pokemon.getTypes()
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
		move_set = pokemon.getMoveSet()
		move_set += [None] * (4 - len(move_set))
		moves = [
			-1 if op_state is not None and not op_state.isPkmnMoveDiscovered(slot, k) else i
			for k, s in enumerate(move_set) for i in ([s.getID(), s.getPP()] if s is not None else [-1, -1])
		]
		return [
			pokemon.getHealth(), pokemon.getMaxHealth(),
			pokemon.getRawAttack(), pokemon.getRawDefense(), pokemon.getRawSpeed(), pokemon.getRawSpecial(),
			*types, *status, *moves
		]


	def make_observation(self, state):
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
		pkmn = state.me.team[state.me.pokemonOnField]
		if pkmn.getHealth() == 0:
			moveMask = [False, False, False, False]
			canUseStruggle = False
		else:
			moveMask = [int(m.getID() != 0 and m.getPP() != 0) for m in pkmn.getMoveSet()] + [False for _ in range(len(pkmn.getMoveSet()), 4)]
			canUseStruggle = int(not any(moveMask))
		switchMask = [int(len(state.me.team) > i and state.me.pokemonOnField != i and state.me.team[i].getHealth() > 0) for i in range(6)]
		result = moveMask + switchMask + [canUseStruggle]
		if not any(result):
			result = [1] * self.action_space.n
		self.last_state = (ob, {
			'mask': array(result, dtype=int8)
		})
		return self.last_state


	def compute_reward(self, old, new):
		if all(f.getHealth() == 0 for f in new.op.team):
			return 100 / self.current_turn
		return sum(p.getHealth() for p in new.me.team) / sum(p.getMaxHealth() for p in new.me.team) - sum(p.getHealth() for p in new.op.team) / sum(p.getMaxHealth() for p in new.op.team)


	def step(self, action):
		state = self.battle.state
		if action == 10:
			state.me.nextAction = BattleAction.StruggleMove
		else:
			state.me.nextAction = BattleAction.Attack1 + action
		state.op.nextAction = self.op(state, self.np_random)
		old = state.copy()
		if state.me.nextAction == BattleAction.StruggleMove:
			assert state.me.team[state.me.pokemonOnField].getHealth() != 0
			assert all(m.getPP() == 0 or m.getID() == 0 for m in state.me.team[state.me.pokemonOnField].getMoveSet())
		if BattleAction.Attack1 <= state.me.nextAction <= BattleAction.Attack4:
			assert state.me.team[state.me.pokemonOnField].getHealth() != 0
			assert state.me.nextAction - BattleAction.Attack1 < len(state.me.team[state.me.pokemonOnField].getMoveSet())
			assert state.me.team[state.me.pokemonOnField].getMoveSet()[state.me.nextAction - BattleAction.Attack1].getID() != 0
			assert state.me.team[state.me.pokemonOnField].getMoveSet()[state.me.nextAction - BattleAction.Attack1].getPP() != 0
		if BattleAction.Switch1 <= state.me.nextAction <= BattleAction.Switch6:
			assert state.me.nextAction - BattleAction.Switch1 != state.me.pokemonOnField
			assert state.me.nextAction - BattleAction.Switch1 < len(state.me.team)
			assert state.me.team[state.me.nextAction - BattleAction.Switch1].getHealth() != 0
		self.battle.tick()
		self.current_turn += 1
		if self.battle.isFinished() and self.replay_folder:
			self.battle.saveReplay(os.path.join(self.replay_folder, f"episode-{self.episode_id}.replay"))
		observation, info = self.make_observation(state)
		self.step_emulator(state)
		# self.spec.max_episode_steps
		return observation, self.compute_reward(old, state), self.battle.isFinished(), False, info


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
			self.battle.reset()
			if self.shuffle_teams:
				state.me.team = self.np_random.permutation([Pokemon(
					self.battle.state,
					p.getName(False),
					p.getLevel(),
					PokemonBase(p.getID()),
					self.np_random.permutation(list(Move(m.getID()) for m in p.getMoveSet() if m.getID()))
				) for p in state.me.team])
				state.op.team = self.np_random.permutation([Pokemon(
					self.battle.state,
					p.getName(False),
					p.getLevel(),
					PokemonBase(p.getID()),
					self.np_random.permutation(list(Move(m.getID()) for m in p.getMoveSet() if m.getID()))
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
		state.rng.setList([self.np_random.integers(low=0, high=255) for _ in range(9)])

		self.init_emulator(state)
		return self.make_observation(state)


	@staticmethod
	def serialize_mon(s, i, op):
		r = f'?????????? ({s.getSpeciesName(): >10}) l{s.getLevel():03d}, {typeToStringShort(s.getTypes()[0])}'
		if s.getTypes()[0] != s.getTypes()[1]:
			r += f'/{typeToStringShort(s.getTypes()[1])}'
		if not op.isPkmnDiscovered(i):
			r += " (Not revealed yet)"
		r += "\n - "
		r += f'{s.getHealth():03d}/{s.getMaxHealth():03d}HP, '
		r += f'{s.getAttack():03d}ATK (???@+0), '
		r += f'{s.getDefense():03d}DEF (???@+0), '
		r += f'{s.getSpecial():03d}SPE (???@+0), '
		r += f'{s.getSpeed(): >3d}SPD (???@+0), '
		r += f'100%ACC (+0), '
		r += f'100%EVD (+0)'
		r += f'\n - Status: 0x{s.getNonVolatileStatus():02X} {statusToString(s.getNonVolatileStatus())}, '
		r += f'\n - Moves: {", ".join(f'{m.getName()} {m.getPP():02d}/{m.getMaxPP():02d}PP{"" if op.isPkmnMoveDiscovered(i, _i) else " (Not yet revealed)"}' for _i, m in enumerate(s.getMoveSet()) if m)}'
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
			self.emulator.stop(False)


register('PokemonYellow', PokemonYellowBattle)