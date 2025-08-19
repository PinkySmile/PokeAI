import os.path
from pyboy import PyBoy
from GameEngine import Pokemon, PokemonSpecies, AvailableMove, BattleHandler, BattleAction, StatusChange, Type, typeToStringShort, typeToString, statusToString, convertString
from argparse import ArgumentParser
import sys

wCurrentMenuItem = 0xCC26
wLinkBattleRandomNumberListIndex = 0xCCDE
wLinkBattleRandomNumberList = 0xD147
wLinkState = 0xD12A
wSerialExchangeNybbleReceiveData = 0xCC3E
LINK_STATE_BATTLING = 4

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

PACK_SPECIES = 0
PACK_HP_HB = 1
PACK_CURR_LEVEL = 3
PACK_STATUS = 4
PACK_TYPEA = 5
PACK_TYPEB = 6
PACK_CATCH_RATE = 7
PACK_MOVE1 = 8
PACK_MOVE2 = 9
PACK_MOVE3 = 10
PACK_MOVE4 = 11
PACK_TID = 12
PACK_EXP = 14
PACK_STAT_EXP_HP = 17
PACK_STAT_EXP_ATK = 19
PACK_STAT_EXP_DEF = 21
PACK_STAT_EXP_SPD = 23
PACK_STAT_EXP_SPE = 25
PACK_DVS_SPD_SPE = 27
PACK_DVS_ATK_DEF = 28
PACK_PPS_MOVE1 = 29
PACK_PPS_MOVE2 = 30
PACK_PPS_MOVE3 = 31
PACK_PPS_MOVE4 = 32
PACK_CURR_LEVEL_DUP = 33
PACK_MAX_HP = 34
PACK_ATK = 36
PACK_DEF = 38
PACK_SPD = 40
PACK_SPE = 42

wPartyMons = 0xD16A
wPartyCount = 0xD162
wPartySpecies = 0xD163
wPartyMonNicks = 0xD2B4
wPlayerName = 0xD157
wEnemyMons = 0xD8A3
wEnemyPartyCount = 0xD89B
wEnemyPartySpecies = 0xD89C
wEnemyMonNicks = 0xD9ED
wTrainerName = 0xD049

wPartyMon1HP = 0xD16B
wPartyMon1MaxHP = 0xD18C
wPartyMon1Moves = 0xD172
wPartyMon1Attack = 0xD18E
wPartyMon1Defense = 0xD190
wPartyMon1Speed = 0xD192
wPartyMon1Special = 0xD194
wEnemyMon1HP = 0xD8A4
wEnemyMon1MaxHP = 0xD8C5
wEnemyMon1Moves = 0xD8AB
wEnemyMon1Attack = 0xD8C7
wEnemyMon1Defense = 0xD8C9
wEnemyMon1Speed = 0xD8CB
wEnemyMon1Special = 0xD8CD

wEnemyMonNick = 0xCFD9
wEnemyMon = 0xCFE4
wEnemyMonSpecies = 0xCFE4
wEnemyMonHP = 0xCFE5
wEnemyMonBoxLevel = 0xCFE7
wEnemyMonStatus = 0xCFE8
wEnemyMonType1 = 0xCFE9
wEnemyMonType2 = 0xCFEA
wEnemyMonCatchRate = 0xCFEB
wEnemyMonMoves = 0xCFEC
wEnemyMonDVs = 0xCFF0
wEnemyMonLevel = 0xCFF2
wEnemyMonStats = 0xCFF3
wEnemyMonMaxHP = 0xCFF3
wEnemyMonAttack = 0xCFF5
wEnemyMonDefense = 0xCFF7
wEnemyMonSpeed = 0xCFF9
wEnemyMonSpecial = 0xCFFB
wEnemyMonPP = 0xCFFD
wEnemyMonUnmodifiedLevel = 0xCD23
wEnemyMonUnmodifiedMaxHP = 0xCD24
wEnemyMonUnmodifiedAttack = 0xCD26
wEnemyMonUnmodifiedDefense = 0xCD28
wEnemyMonUnmodifiedSpeed = 0xCD2A
wEnemyMonUnmodifiedSpecial = 0xCD2C

wBattleMonNick = 0xD008
wBattleMonSpecies = 0xD013
wBattleMonHP = 0xD014
wBattleMonBoxLevel = 0xD016
wBattleMonStatus = 0xD017
wBattleMonType1 = 0xD018
wBattleMonType2 = 0xD019
wBattleMonCatchRate = 0xD01A
wBattleMonMoves = 0xD01B
wBattleMonDVs = 0xD01F
wBattleMonLevel = 0xD021
wBattleMonMaxHP = 0xD022
wBattleMonAttack = 0xD024
wBattleMonDefense = 0xD026
wBattleMonSpeed = 0xD028
wBattleMonSpecial = 0xD02A
wBattleMonPP = 0xD02C
wPlayerMonUnmodifiedLevel = 0xCD0F
wPlayerMonUnmodifiedMaxHP = 0xCD10
wPlayerMonUnmodifiedAttack = 0xCD12
wPlayerMonUnmodifiedDefense = 0xCD14
wPlayerMonUnmodifiedSpeed = 0xCD16
wPlayerMonUnmodifiedSpecial = 0xCD18

assert enemyBaseAddr + nickOffset == wEnemyMonNick
assert enemyBaseAddr + speciesOffset == wEnemyMonSpecies
assert enemyBaseAddr + hpOffset == wEnemyMonHP
assert enemyBaseAddr + boxLevelOffset == wEnemyMonBoxLevel
assert enemyBaseAddr + statusOffset == wEnemyMonStatus
assert enemyBaseAddr + type1Offset == wEnemyMonType1
assert enemyBaseAddr + type2Offset == wEnemyMonType2
assert enemyBaseAddr + catchRateOffset == wEnemyMonCatchRate
assert enemyBaseAddr + movesOffset == wEnemyMonMoves
assert enemyBaseAddr + dvsOffset == wEnemyMonDVs
assert enemyBaseAddr + levelOffset == wEnemyMonLevel
assert enemyBaseAddr + maxHPOffset == wEnemyMonMaxHP
assert enemyBaseAddr + attackOffset == wEnemyMonAttack
assert enemyBaseAddr + defenseOffset == wEnemyMonDefense
assert enemyBaseAddr + speedOffset == wEnemyMonSpeed
assert enemyBaseAddr + specialOffset == wEnemyMonSpecial
assert enemyBaseAddr + ppOffset == wEnemyMonPP

assert playerBaseAddr + nickOffset == wBattleMonNick
assert playerBaseAddr + speciesOffset == wBattleMonSpecies
assert playerBaseAddr + hpOffset == wBattleMonHP
assert playerBaseAddr + boxLevelOffset == wBattleMonBoxLevel
assert playerBaseAddr + statusOffset == wBattleMonStatus
assert playerBaseAddr + type1Offset == wBattleMonType1
assert playerBaseAddr + type2Offset == wBattleMonType2
assert playerBaseAddr + catchRateOffset == wBattleMonCatchRate
assert playerBaseAddr + movesOffset == wBattleMonMoves
assert playerBaseAddr + dvsOffset == wBattleMonDVs
assert playerBaseAddr + levelOffset == wBattleMonLevel
assert playerBaseAddr + maxHPOffset == wBattleMonMaxHP
assert playerBaseAddr + attackOffset == wBattleMonAttack
assert playerBaseAddr + defenseOffset == wBattleMonDefense
assert playerBaseAddr + speedOffset == wBattleMonSpeed
assert playerBaseAddr + specialOffset == wBattleMonSpecial
assert playerBaseAddr + ppOffset == wBattleMonPP


def get_basic_mon_state(emulator, base_address):
	nickname = bytes(emulator.memory[base_address + nickOffset:base_address + nickOffset + 11])
	species = emulator.memory[base_address + speciesOffset]
	hp = int.from_bytes(bytes(emulator.memory[base_address + hpOffset:base_address + hpOffset + 2]), byteorder='big')
	boxLevel = emulator.memory[base_address + boxLevelOffset]
	status = emulator.memory[base_address + statusOffset]
	typeA = emulator.memory[base_address + type1Offset]
	typeB = emulator.memory[base_address + type2Offset]
	moves = emulator.memory[base_address + movesOffset:base_address + movesOffset + 4]
	level = emulator.memory[base_address + levelOffset]
	maxHp = int.from_bytes(bytes(emulator.memory[base_address + maxHPOffset:base_address + maxHPOffset + 2]), byteorder='big')
	attack = int.from_bytes(bytes(emulator.memory[base_address + attackOffset:base_address + attackOffset + 2]), byteorder='big')
	defense = int.from_bytes(bytes(emulator.memory[base_address + defenseOffset:base_address + defenseOffset + 2]), byteorder='big')
	speed = int.from_bytes(bytes(emulator.memory[base_address + speedOffset:base_address + speedOffset + 2]), byteorder='big')
	special = int.from_bytes(bytes(emulator.memory[base_address + specialOffset:base_address + specialOffset + 2]), byteorder='big')
	pps = emulator.memory[base_address + ppOffset:base_address + ppOffset + 4]
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
	}


def get_emulator_basic_state(emulator):
	return (
		get_basic_mon_state(emulator, playerBaseAddr),
		get_basic_mon_state(emulator, enemyBaseAddr),
		emulator.memory[wLinkBattleRandomNumberListIndex],
		emulator.memory[wLinkBattleRandomNumberList:wLinkBattleRandomNumberList+9]
	)


def dump_basic_state(s):
	r = f'?????????? ({s['species'].name: >10}) l{s['level']: >3d}, {typeToStringShort(s['typeA'])}'
	if s['typeA'] != s['typeB']:
		r += f'/{typeToStringShort(s['typeB'])}, '
	else:
		r += "    , "
	r += f'{s['hp']: >3d}/{s['maxHp']: >3d}HP, '
	r += f'{s['attack']: >3d}ATK (???@+0), '
	r += f'{s['defense']: >3d}DEF (???@+0), '
	r += f'{s['special']: >3d}SPE (???@+0), '
	r += f'{s['speed']: >3d}SPD (???@+0), '
	r += f'100%ACC (+0), '
	r += f'100%EVD (+0), '
	r += f'Status: {s['status']} {statusToString(s['status'])}, '
	r += f'Moves: {", ".join(f'{m.name} {s['pps'][i]}/?PP' for i, m in enumerate(s['moves']) if m)}'
	return r


def compare_basic_states(battle_state, emu_state):
	me_b = battle_state.me.team[battle_state.me.pokemonOnField]
	op_b = battle_state.op.team[battle_state.op.pokemonOnField]
	rn_b = battle_state.rng.getIndex()
	rl_b = battle_state.rng.getList()
	me_e = emu_state[0]
	op_e = emu_state[1]
	rn_e = emu_state[2]
	rl_e = emu_state[3]
	errors = []

	if rn_b != rn_e:
		errors.append(f"RNG index b.{rn_b} vs e.{rn_e}")
	if rl_b != rl_e:
		errors.append(f"RNG list b.{rl_b} vs e.{rl_e}")

	if me_b.getTypes()[0] != me_e['typeA']:
		errors.append(f"P1 Type 1 b.{typeToString(me_b.getTypes()[0])} vs e.{typeToString(me_e['typeA'])}")
	if me_b.getTypes()[1] != me_e['typeB']:
		errors.append(f"P1 Type 2 b.{typeToString(me_b.getTypes()[1])} vs e.{typeToString(me_e['typeB'])}")
	if me_b.getNonVolatileStatus() != me_e['status'] and me_e['hp'] != 0:
		errors.append(f"P1 Status b.{me_b.getNonVolatileStatus()} ({StatusChange(me_b.getNonVolatileStatus()).name}) vs e.{me_e['status']} ({StatusChange(me_e['status']).name})")
	if me_b.getHealth() != me_e['hp']:
		errors.append(f"P1 Health b.{me_b.getHealth()} vs e.{me_e['hp']}")
	if me_b.getAttack() != me_e['attack']:
		errors.append(f"P1 Attack b.{me_b.getAttack()} vs e.{me_e['attack']}")
	if me_b.getDefense() != me_e['defense']:
		errors.append(f"P1 Defense b.{me_b.getDefense()} vs e.{me_e['defense']}")
	if me_b.getSpecial() != me_e['special']:
		errors.append(f"P1 Special b.{me_b.getSpecial()} vs e.{me_e['special']}")
	if me_b.getSpeed() != me_e['speed']:
		errors.append(f"P1 Speed b.{me_b.getSpeed()} vs e.{me_e['speed']}")

	if op_b.getTypes()[0] != op_e['typeA']:
		errors.append(f"P2 Type 1 b.{typeToString(op_b.getTypes()[0])} vs e.{typeToString(op_e['typeA'])}")
	if op_b.getTypes()[1] != op_e['typeB']:
		errors.append(f"P2 Type 2 b.{typeToString(op_b.getTypes()[1])} vs e.{typeToString(op_e['typeB'])}")
	if op_b.getNonVolatileStatus() != op_e['status'] and op_e['hp'] != 0:
		errors.append(f"P2 Status b.{op_b.getNonVolatileStatus()} ({StatusChange(op_b.getNonVolatileStatus()).name}) vs e.{op_e['status']} ({StatusChange(op_e['status']).name})")
	if op_b.getHealth() != op_e['hp']:
		errors.append(f"P2 Health b.{op_b.getHealth()} vs e.{op_e['hp']}")
	if op_b.getAttack() != op_e['attack']:
		errors.append(f"P2 Attack b.{op_b.getAttack()} vs e.{op_e['attack']}")
	if op_b.getDefense() != op_e['defense']:
		errors.append(f"P2 Defense b.{op_b.getDefense()} vs e.{op_e['defense']}")
	if op_b.getSpecial() != op_e['special']:
		errors.append(f"P2 Special b.{op_b.getSpecial()} vs e.{op_e['special']}")
	if op_b.getSpeed() != op_e['speed']:
		errors.append(f"P2 Speed b.{op_b.getSpeed()} vs e.{op_e['speed']}")
	return len(errors) == 0, errors


def copy_battle_data_to_emulator(state, teamBaseAddress, nameAddress, speciesArray, nameListAddress):
	emulator.memory[speciesArray] = len(state.team)
	for i, pkmn in enumerate(state.team):
		emulator.memory[speciesArray + i + 1] = pkmn.getID()
		data = pkmn.encode()
		for k, b in enumerate(data):
			emulator.memory[teamBaseAddress + i * len(data) + k] = b
		data = convertString(pkmn.getName(False))
		for j in range(11):
			if j < len(data):
				emulator.memory[nameListAddress + i * 11 + j] = data[j]
			else:
				emulator.memory[nameListAddress + i * 11 + j] = 0x50
	emulator.memory[speciesArray + len(state.team) + 1] = 0xFF
	data = convertString(state.name)
	for j in range(11):
		if j < len(data):
			emulator.memory[nameAddress + j] = data[j]
		else:
			emulator.memory[nameAddress + j] = 0x50


def tickEmulator(count=1):
	step = 1 if not args.fast and turn + 1 >= to_turn else 30
	for i in range(0, count, step):
		if not emulator.tick(step):
			exit(0)


parser = ArgumentParser(prog=sys.argv[0])
parser.add_argument('-t', '--to-turn', default=0)
parser.add_argument('-v', '--volume', default=25)
parser.add_argument('-f', '--fast', action='store_true')
parser.add_argument('-e', '--emu-debug', action='store_true')
parser.add_argument('-s', '--swap-side', action='store_true')
parser.add_argument('replay_file')
args = parser.parse_args()
emulator = PyBoy('pokeyellow.gbc', sound_volume=int(args.volume), sound_emulated=not args.fast, window='SDL2' if not args.fast else 'null', debug=args.emu_debug)

battle = BattleHandler(args.swap_side, False)
state = battle.state
to_turn = int(args.to_turn)
battle.loadReplay(args.replay_file)
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
	state.battleLogger = print
fd = open("pokeyellow_replay.state", "rb")
emulator.load_state(fd)
fd.close()
l = state.rng.getList()
emulator.memory[wLinkBattleRandomNumberListIndex] = 0
for i in range(9):
	emulator.memory[wLinkBattleRandomNumberList + i] = l[i]
emulator.memory[wLinkState] = LINK_STATE_BATTLING

copy_battle_data_to_emulator(state.me, wPartyMons, wPlayerName,  wPartyCount,      wPartyMonNicks)
copy_battle_data_to_emulator(state.op, wEnemyMons, wTrainerName, wEnemyPartyCount, wEnemyMonNicks)
turn = 0
while True:
	if emulator.memory[0x9D64:0x9D6F] == [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7]: # Waiting...!
		break
	if emulator.memory[wBattleMonHP:wBattleMonHP+2] != [0, 0] and emulator.memory[0x9DD0] == 0xE1 and emulator.memory[0x9DD1] == 0xE2:
		break
	if emulator.memory[wBattleMonHP:wBattleMonHP+2] == [0, 0] and emulator.memory[0x9DC1:0x9DD0] == [0x81, 0xB1, 0xA8, 0xAD, 0xA6, 0x7F, 0xAE, 0xB4, 0xB3, 0x7F, 0xB6, 0xA7, 0xA8, 0xA2, 0xA7]: # Bring out which
		break
	tickEmulator()
while not battle.isFinished():
	print(f" ---------- TURN {turn + 1:<3} ----------")
	with open(state_folder + f"/turn{turn:03d}.state", "wb") as fd:
		emulator.save_state(fd)
	battle.saveState(state_folder + f"/turn{turn:03d}.json")
	battle.tick()
	if emulator.memory[0x9D64:0x9D6F] != [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7]: # Waiting...!
		if emulator.memory[wBattleMonHP:wBattleMonHP+2] == [0, 0]:
			tickEmulator(10)
			emulator.memory[wCurrentMenuItem] = state.me.lastAction - BattleAction.Switch1
			tickEmulator(10)
			emulator.button_press('a')
			tickEmulator(10)
			emulator.button_release('a')
			tickEmulator(10)
			emulator.button_press('a')
			tickEmulator(10)
			emulator.button_release('a')
		else:
			if BattleAction.Run == state.me.lastAction:
				emulator.memory[wCurrentMenuItem] = 3
			elif BattleAction.Switch1 <= state.me.lastAction <= BattleAction.Switch6:
				emulator.memory[wCurrentMenuItem] = 2
			else:
				emulator.memory[wCurrentMenuItem] = 0
			tickEmulator(10)
			emulator.button_press('a')
			tickEmulator(10)
			emulator.button_release('a')
			if BattleAction.Switch1 <= state.me.lastAction <= BattleAction.Switch6:
				tickEmulator(10)
				emulator.memory[wCurrentMenuItem] = state.me.lastAction - BattleAction.Switch1
				tickEmulator(30)
				emulator.button_press('a')
				tickEmulator(10)
				emulator.button_release('a')
				tickEmulator(10)
				emulator.button_press('a')
				tickEmulator(10)
				emulator.button_release('a')
			elif state.me.lastAction != BattleAction.StruggleMove:
				emulator.memory[wCurrentMenuItem] = state.me.lastAction - BattleAction.Attack1 + 1
				tickEmulator(10)
				emulator.button_press('a')
				tickEmulator(10)
				emulator.button_release('a')

	isDead = emulator.memory[wBattleMonHP:wBattleMonHP+2] == [0, 0]
	while emulator.memory[0x9D64:0x9D6F] != [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7]: # Waiting...!
		tickEmulator()
	while emulator.memory[wSerialExchangeNybbleReceiveData] == 0xFF:
		emulator.memory[wSerialExchangeNybbleReceiveData] = state.op.lastAction - BattleAction.Attack1
		tickEmulator()
	while (
		emulator.memory[0x9D64:0x9D6F] == [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7] or # Waiting...!
		emulator.memory[0x9D6A:0x9D74] != [0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x7F]
	):
		tickEmulator()
	while True:
		if emulator.memory[0x9C00:0x9C20] == [0x59, 0x5A, 0x58, 0x59, 0x59, 0x5A, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0F, 0x0F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F]:
			break
		if emulator.memory[0x9D64:0x9D6F] == [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7]: # Waiting...!
			break
		if emulator.memory[wBattleMonHP:wBattleMonHP+2] != [0, 0] and emulator.memory[0x9DD0] == 0xE1 and emulator.memory[0x9DD1] == 0xE2:
			break
		if emulator.memory[wBattleMonHP:wBattleMonHP+2] == [0, 0] and emulator.memory[0x9DC1:0x9DD0] == [0x81, 0xB1, 0xA8, 0xAD, 0xA6, 0x7F, 0xAE, 0xB4, 0xB3, 0x7F, 0xB6, 0xA7, 0xA8, 0xA2, 0xA7]: # Bring out which
			break
		tickEmulator()
	emulator_state = get_emulator_basic_state(emulator)
	if not args.fast:
		print(dump_basic_state(emulator_state[0]))
		print(dump_basic_state(emulator_state[1]))
		print(state.me.team[0].dump())
		print(state.op.team[0].dump())
		print(state.rng.getIndex(), emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.getList())), list(map(lambda x: f'{x:02X}', emulator_state[3])))
	f = compare_basic_states(battle.state, emulator_state)
	if f[1]:
		to_turn = 0
		print("Desync detected!")
		print("\n".join(f[1]))
		with open(state_folder + f"/turn{turn:03d}.state", "rb") as fd:
			emulator.load_state(fd)
		battle.loadState(state_folder + f"/turn{turn:03d}.json")
		while True:
			tickEmulator()
	turn += 1