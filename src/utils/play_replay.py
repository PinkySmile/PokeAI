import sys
import os.path

from PokeBattle.Gen1.YellowEmulator import PkmnYellowEmulator
from pyboy import PyBoy
from PokeBattle.Gen1.PyBoyEmulator import PyBoyEmulator
from PokeBattle.Gen1.Pokemon import Pokemon, PokemonSpecies
from PokeBattle.Gen1.Move import AvailableMove
from PokeBattle.Gen1.BattleHandler import BattleHandler
from PokeBattle.Gen1.State import BattleAction, BattleState
from PokeBattle.Gen1.StatusChange import StatusChange, status_to_string
from PokeBattle.Gen1.Type import Type, type_to_string_short, type_to_string
from PokeBattle.Gen1.EmulatorGameHandle import EmulatorGameHandle
from argparse import ArgumentParser

wLinkBattleRandomNumberListIndex = 0xCCDE
wLinkBattleRandomNumberList = 0xD147

enemyBaseAddr = 0xCFD9
playerBaseAddr = 0xD008
nickOffset = 0
speciesOffset = nickOffset + 11
hpOffset = speciesOffset + 1
boxLevelOffset = hpOffset + 2
statusOffset = boxLevelOffset + 1
type1Offset = statusOffset + 1
type2Offset = type1Offset + 1
catchRateOffset = type2Offset + 1
movesOffset = catchRateOffset + 1
dvsOffset = movesOffset + 4
levelOffset = dvsOffset + 2
maxHPOffset = levelOffset + 1
attackOffset = maxHPOffset + 2
defenseOffset = attackOffset + 2
speedOffset = defenseOffset + 2
specialOffset = speedOffset + 2
ppOffset = specialOffset + 2


wPlayerSubstituteHP = 0xCCD7
wEnemySubstituteHP = 0xCCD8
wPlayerBattleStatus1 = 0xD061
wEnemyBattleStatus1 = 0xD066

HAS_SUBSTITUTE_UP =  4

wEnemyMonNick = 0xCFD9
wEnemyMon = 0xCFE4
wEnemyMonUnmodifiedMaxHP = 0xCD24

wBattleMonNick = 0xD008
wPlayerMonUnmodifiedMaxHP = 0xCD10


def get_basic_mon_state(emulator: PkmnYellowEmulator, base_address, base_stats, sub_addr, status1_addr):
	nickname = bytes(emulator.read_range(base_address + nickOffset, base_address + nickOffset + 11))
	species = emulator.read(base_address + speciesOffset)
	hp = int.from_bytes(bytes(emulator.read_range(base_address + hpOffset, base_address + hpOffset + 2)), byteorder='big')
	boxLevel = emulator.read(base_address + boxLevelOffset)
	status = emulator.read(base_address + statusOffset)
	typeA = emulator.read(base_address + type1Offset)
	typeB = emulator.read(base_address + type2Offset)
	moves = emulator.read_range(base_address + movesOffset, base_address + movesOffset + 4)
	level = emulator.read(base_address + levelOffset)
	maxHp = int.from_bytes(bytes(emulator.read_range(base_address + maxHPOffset, base_address + maxHPOffset + 2)), byteorder='big')
	attack = int.from_bytes(bytes(emulator.read_range(base_address + attackOffset, base_address + attackOffset + 2)), byteorder='big')
	defense = int.from_bytes(bytes(emulator.read_range(base_address + defenseOffset, base_address + defenseOffset + 2)), byteorder='big')
	speed = int.from_bytes(bytes(emulator.read_range(base_address + speedOffset, base_address + speedOffset + 2)), byteorder='big')
	special = int.from_bytes(bytes(emulator.read_range(base_address + specialOffset, base_address + specialOffset + 2)), byteorder='big')
	pps = emulator.read_range(base_address + ppOffset, base_address + ppOffset + 4)
	baseMaxHp = int.from_bytes(bytes(emulator.read_range(base_stats, base_stats + 2)), byteorder='big')
	baseAttack = int.from_bytes(bytes(emulator.read_range(base_stats + 2, base_stats + 4)), byteorder='big')
	baseDefense = int.from_bytes(bytes(emulator.read_range(base_stats + 4, base_stats + 6)), byteorder='big')
	baseSpeed = int.from_bytes(bytes(emulator.read_range(base_stats + 6, base_stats + 8)), byteorder='big')
	baseSpecial = int.from_bytes(bytes(emulator.read_range(base_stats + 8, base_stats + 10)), byteorder='big')
	return {
		'nickname': nickname,
		'species': PokemonSpecies(species),
		'hp': hp,
		'boxLevel': boxLevel,
		'status': StatusChange(status),
		'typeA': Type(typeA),
		'typeB': Type(typeB),
		'moves': list(map(AvailableMove, moves)),
		'level': level,
		'maxHp': maxHp,
		'attack': attack,
		'defense': defense,
		'speed': speed,
		'special': special,
		'pps': pps,
		'baseMaxHp': baseMaxHp,
		'baseAttack': baseAttack,
		'baseDefense': baseDefense,
		'baseSpeed': baseSpeed,
		'baseSpecial': baseSpecial,
		'substitute': emulator.read(sub_addr),
		'status_flags': emulator.read_range(status1_addr, status1_addr + 3)
	}


def get_emulator_basic_state(emulator: PyBoyEmulator):
	return (
		get_basic_mon_state(emulator, playerBaseAddr, wPlayerMonUnmodifiedMaxHP, wPlayerSubstituteHP, wPlayerBattleStatus1),
		get_basic_mon_state(emulator, enemyBaseAddr, wEnemyMonUnmodifiedMaxHP, wEnemySubstituteHP, wEnemyBattleStatus1),
		emulator.read(wLinkBattleRandomNumberListIndex),
		emulator.read_range(wLinkBattleRandomNumberList, wLinkBattleRandomNumberList+9)
	)


def dump_basic_state(s):
	r = f'?????????? ({s['species'].name: >10}) l{s['level']: >3d}, {type_to_string_short(s['typeA'])}'
	if s['typeA'] != s['typeB']:
		r += f'/{type_to_string_short(s['typeB'])}, '
	else:
		r += "    , "
	r += f'{s['hp']: >3d}/{s['maxHp']: >3d}HP, '
	r += f'{s['attack']: >3d}ATK ({s['baseAttack']: >3d}@+?), '
	r += f'{s['defense']: >3d}DEF ({s['baseDefense']: >3d}@+?), '
	r += f'{s['special']: >3d}SPE ({s['baseSpecial']: >3d}@+?), '
	r += f'{s['speed']: >3d}SPD ({s['baseSpeed']: >3d}@+?), '
	r += f'???%ACC (+?), '
	r += f'???%EVD (+?), '
	r += f'Status: {s['status']} {status_to_string(s['status'])}, '
	if s['status_flags'][1] & (1 << HAS_SUBSTITUTE_UP):
		r += f"Sub {s['substitute']}HP, "
	r += f'Moves: {", ".join(f'{m.name: >12} {s['pps'][i]: >2}/??PP' for i, m in enumerate(s['moves']) if m)}'
	return r


def compare_basic_states(battle_state: BattleState, emu_state):
	me_b = battle_state.me.pokemon_on_field
	op_b = battle_state.op.pokemon_on_field
	rn_b = battle_state.rng.index
	rl_b = battle_state.rng.list
	me_e = emu_state[0]
	op_e = emu_state[1]
	rn_e = emu_state[2]
	rl_e = emu_state[3]
	errors = []

	if rn_b != rn_e:
		errors.append(f"RNG index b.{rn_b} vs e.{rn_e}")
	if rl_b != rl_e:
		errors.append(f"RNG list b.{rl_b} vs e.{rl_e}")

	if me_b.types[0] != me_e['typeA']:
		errors.append(f"P1 Type 1 b.{type_to_string(me_b.types[0])} vs e.{type_to_string(me_e['typeA'])}")
	if me_b.types[1] != me_e['typeB']:
		errors.append(f"P1 Type 2 b.{type_to_string(me_b.types[1])} vs e.{type_to_string(me_e['typeB'])}")
	if me_b.non_volatile_status != me_e['status'] and me_b.health:
		errors.append(f"P1 Status b.{me_b.non_volatile_status} ({StatusChange(me_b.non_volatile_status).name}) vs e.{me_e['status']} ({StatusChange(me_e['status']).name})")
	if me_b.health != me_e['hp']:
		errors.append(f"P1 Health b.{me_b.health} vs e.{me_e['hp']}")
	has_substitute = ((me_e['status_flags'][1] & (1 << HAS_SUBSTITUTE_UP)) != 0)
	if (me_b.substitute is not None) != has_substitute:
		errors.append(f"P1 Has substitute b.{me_b.substitute is not None} vs e.{has_substitute}")
	if me_b.substitute is not None and has_substitute and me_b.substitute != me_e['substitute']:
		errors.append(f"P1 Substitute health b.{me_b.substitute} vs e.{me_e['substitute']}")
	if me_b.attack != me_e['attack']:
		errors.append(f"P1 Attack b.{me_b.attack} vs e.{me_e['attack']}")
	if me_b.defense != me_e['defense']:
		errors.append(f"P1 Defense b.{me_b.defense} vs e.{me_e['defense']}")
	if me_b.special != me_e['special']:
		errors.append(f"P1 Special b.{me_b.special} vs e.{me_e['special']}")
	if me_b.speed != me_e['speed']:
		errors.append(f"P1 Speed b.{me_b.speed} vs e.{me_e['speed']}")
	pps = [m.pp for m in me_b.move_set]
	if pps != me_e['pps']:
		errors.append(f"P1 PPs b.{pps} vs e.{me_e['pps']}")

	if op_b.types[0] != op_e['typeA']:
		errors.append(f"P2 Type 1 b.{type_to_string(op_b.types[0])} vs e.{type_to_string(op_e['typeA'])}")
	if op_b.types[1] != op_e['typeB']:
		errors.append(f"P2 Type 2 b.{type_to_string(op_b.types[1])} vs e.{type_to_string(op_e['typeB'])}")
	if op_b.non_volatile_status != op_e['status'] and op_b.health:
		errors.append(f"P2 Status b.{op_b.non_volatile_status} ({StatusChange(op_b.non_volatile_status).name}) vs e.{op_e['status']} ({StatusChange(op_e['status']).name})")
	if op_b.health != op_e['hp']:
		errors.append(f"P2 Health b.{op_b.health} vs e.{op_e['hp']}")
	has_substitute = ((op_e['status_flags'][1] & (1 << HAS_SUBSTITUTE_UP)) != 0)
	if (op_b.substitute is not None) != has_substitute:
		errors.append(f"P2 Has substitute b.{op_b.substitute is not None} vs e.{has_substitute}")
	if op_b.substitute is not None and has_substitute and op_b.substitute != op_e['substitute']:
		errors.append(f"P2 Substitute health b.{op_b.substitute} vs e.{op_e['substitute']}")
	if op_b.attack != op_e['attack']:
		errors.append(f"P2 Attack b.{op_b.attack} vs e.{op_e['attack']}")
	if op_b.defense != op_e['defense']:
		errors.append(f"P2 Defense b.{op_b.defense} vs e.{op_e['defense']}")
	if op_b.special != op_e['special']:
		errors.append(f"P2 Special b.{op_b.special} vs e.{op_e['special']}")
	if op_b.speed != op_e['speed']:
		errors.append(f"P2 Speed b.{op_b.speed} vs e.{op_e['speed']}")
	return len(errors) == 0, errors


parser = ArgumentParser(prog=sys.argv[0])
parser.add_argument('-t', '--to-turn', default=0)
parser.add_argument('-v', '--volume', default=25)
parser.add_argument('-f', '--fast', action='store_true')
parser.add_argument('-e', '--emu-debug', action='store_true')
parser.add_argument('-s', '--swap-side', action='store_true')
parser.add_argument('replay_file')
args = parser.parse_args()
emulator = PyBoyEmulator(has_interface=not args.fast, sound_volume=int(args.volume), save_frames=False, debug=args.emu_debug)

battle = BattleHandler(args.swap_side, False)
state = battle.state
to_turn = int(args.to_turn)
battle.load_replay(args.replay_file)
state_folder = os.path.basename(args.replay_file) + "_states"
os.system(f"rm -rf {state_folder}")
os.mkdir(state_folder)
if args.swap_side:
	tmp_team = [Pokemon(p) for p in state.me.team]
	tmp_name = state.me.name
	state.me.name = state.op.name
	state.me.team = state.op.team
	state.op.name = tmp_name
	state.op.team = tmp_team
print(f"{state.me.name}'s team (P1):")
print("\n".join(pkmn.dump() for pkmn in state.me.team))
print()
print(f"{state.op.name}'s team (P2):")
print("\n".join(pkmn.dump() for pkmn in state.op.team))
print()
if not args.fast:
	state.logger = print


with open("pokeyellow_replay.state", "rb") as fd:
	emulator.init_battle(fd, state)

turn = 0
while not battle.finished:
	print(f" ---------- TURN {turn + 1:<3} ----------")
	with open(state_folder + f"/turn{turn:03d}.state", "wb") as fd:
		emulator.emulator.save_state(fd)
	battle.save_state(state_folder + f"/turn{turn:03d}.json")
	battle.tick()
	emulator.step(state)
	emulator_state = get_emulator_basic_state(emulator.emulator)
	if not args.fast:
		print(dump_basic_state(emulator_state[0]))
		print(dump_basic_state(emulator_state[1]))
		print(state.me.team[0].dump())
		print(state.op.team[0].dump())
		print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
	f = compare_basic_states(battle.state, emulator_state)
	if f[1]:
		to_turn = 0
		print("Desync detected!")
		print("\n".join(f[1]))
		with open(state_folder + f"/turn{turn:03d}.state", "rb") as fd:
			emulator.emulator.load_state(fd)
		battle.load_state(state_folder + f"/turn{turn:03d}.json")
		while not args.fast and emulator.emulator.tick():
			break
	turn += 1