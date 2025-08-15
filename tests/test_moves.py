from pyboy import PyBoy
from GameEngine import RandomGenerator, PokemonBase, Pokemon, PokemonSpecies, Move, AvailableMove, Type, BattleHandler, BattleAction, StatusChange, MoveCategory, getAttackDamageMultiplier, typeToStringShort, typeToString, statusToString
from argparse import ArgumentParser
import sys
import threading
import traceback

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
wEnemyMons = 0xD8A3

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


def get_basic_mon_state(base_address):
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


def get_emulator_basic_state():
	return get_basic_mon_state(playerBaseAddr), get_basic_mon_state(enemyBaseAddr), emulator.memory[wLinkBattleRandomNumberListIndex], emulator.memory[wLinkBattleRandomNumberList:wLinkBattleRandomNumberList+9]


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
		errors.append(f"RNG index {rn_b} vs {rn_e}")
	if rl_b != rl_e:
		errors.append(f"RNG list {rl_b} vs {rl_e}")

	if me_b.getTypes()[0] != me_e['typeA']:
		errors.append(f"P1 Type 1 {typeToString(me_b.getTypes()[0])} vs {typeToString(me_e['typeA'])}")
	if me_b.getTypes()[1] != me_e['typeB']:
		errors.append(f"P1 Type 2 {typeToString(me_b.getTypes()[1])} vs {typeToString(me_e['typeB'])}")
	if me_b.getNonVolatileStatus() != me_e['status']:
		errors.append(f"P1 Status {me_b.getNonVolatileStatus()} ({StatusChange(me_b.getNonVolatileStatus()).name}) vs {me_e['status']} ({StatusChange(me_e['status']).name})")
	if me_b.getHealth() != me_e['hp']:
		errors.append(f"P1 Health {me_b.getHealth()} vs {me_e['hp']}")
	if me_b.getAttack() != me_e['attack']:
		errors.append(f"P1 Attack {me_b.getAttack()} vs {me_e['attack']}")
	if me_b.getDefense() != me_e['defense']:
		errors.append(f"P1 Defense {me_b.getDefense()} vs {me_e['defense']}")
	if me_b.getSpecial() != me_e['special']:
		errors.append(f"P1 Special {me_b.getSpecial()} vs {me_e['special']}")
	if me_b.getSpeed() != me_e['speed']:
		errors.append(f"P1 Speed {me_b.getSpeed()} vs {me_e['speed']}")

	if op_b.getTypes()[0] != op_e['typeA']:
		errors.append(f"P2 Type 1 {typeToString(op_b.getTypes()[0])} vs {typeToString(op_e['typeA'])}")
	if op_b.getTypes()[1] != op_e['typeB']:
		errors.append(f"P2 Type 2 {typeToString(op_b.getTypes()[1])} vs {typeToString(op_e['typeB'])}")
	if op_b.getNonVolatileStatus() != op_e['status']:
		errors.append(f"P2 Status {op_b.getNonVolatileStatus()} ({StatusChange(op_b.getNonVolatileStatus()).name}) vs {op_e['status']} ({StatusChange(op_e['status']).name})")
	if op_b.getHealth() != op_e['hp']:
		errors.append(f"P2 Health {op_b.getHealth()} vs {op_e['hp']}")
	if op_b.getAttack() != op_e['attack']:
		errors.append(f"P2 Attack {op_b.getAttack()} vs {op_e['attack']}")
	if op_b.getDefense() != op_e['defense']:
		errors.append(f"P2 Defense {op_b.getDefense()} vs {op_e['defense']}")
	if op_b.getSpecial() != op_e['special']:
		errors.append(f"P2 Special {op_b.getSpecial()} vs {op_e['special']}")
	if op_b.getSpeed() != op_e['speed']:
		errors.append(f"P2 Speed {op_b.getSpeed()} vs {op_e['speed']}")
	return len(errors) == 0, errors


def test_move(move, random_state, low_stats):
	battle = BattleHandler(False, debug)
	state = battle.state
	if random_state is not None:
		assert len(random_state) == 9
		state.rng.setList(random_state)
	else:
		state.rng.makeRandomList(9)
	if debug:
		state.battleLogger = print
	fd = open("pokeyellow.state", "rb")
	emulator.load_state(fd)
	fd.close()
	l = state.rng.getList()
	emulator.memory[wLinkBattleRandomNumberListIndex] = 0
	for i in range(9):
		emulator.memory[wLinkBattleRandomNumberList + i] = l[i]
	emulator.memory[wLinkState] = LINK_STATE_BATTLING

	# Move list: Constrict, -, -, -; 999/999 HP; 25 DEF; 25 SPE; 25 SPD
	emulator.memory[wEnemyMonMoves + 0]   = emulator.memory[wEnemyMon1Moves + 0]   = AvailableMove.Constrict
	emulator.memory[wEnemyMonMoves + 1]   = emulator.memory[wEnemyMon1Moves + 1]   = AvailableMove.Empty
	emulator.memory[wEnemyMonMoves + 2]   = emulator.memory[wEnemyMon1Moves + 2]   = AvailableMove.Empty
	emulator.memory[wEnemyMonMoves + 3]   = emulator.memory[wEnemyMon1Moves + 3]   = AvailableMove.Empty
	emulator.memory[wEnemyMonHP + 0]      = emulator.memory[wEnemyMon1HP + 0]      = 999 >> 8
	emulator.memory[wEnemyMonHP + 1]      = emulator.memory[wEnemyMon1HP + 1]      = 999 & 0xFF
	emulator.memory[wEnemyMonMaxHP + 0]   = emulator.memory[wEnemyMon1MaxHP + 0]   = emulator.memory[wEnemyMonUnmodifiedMaxHP + 0]   = 999 >> 8
	emulator.memory[wEnemyMonMaxHP + 1]   = emulator.memory[wEnemyMon1MaxHP + 1]   = emulator.memory[wEnemyMonUnmodifiedMaxHP + 1]   = 999 & 0xFF
	if low_stats:
		emulator.memory[wEnemyMonDefense + 0] = emulator.memory[wEnemyMon1Defense + 0] = emulator.memory[wEnemyMonUnmodifiedDefense + 0] = 25 >> 8
		emulator.memory[wEnemyMonDefense + 1] = emulator.memory[wEnemyMon1Defense + 1] = emulator.memory[wEnemyMonUnmodifiedDefense + 1] = 25 & 0xFF
		emulator.memory[wEnemyMonSpecial + 0] = emulator.memory[wEnemyMon1Special + 0] = emulator.memory[wEnemyMonUnmodifiedSpecial + 0] = 25 >> 8
		emulator.memory[wEnemyMonSpecial + 1] = emulator.memory[wEnemyMon1Special + 1] = emulator.memory[wEnemyMonUnmodifiedSpecial + 1] = 25 & 0xFF
		emulator.memory[wEnemyMonSpeed + 0]   = emulator.memory[wEnemyMon1Speed + 0]   = emulator.memory[wEnemyMonUnmodifiedSpeed + 0]   = 25 >> 8
		emulator.memory[wEnemyMonSpeed + 1]   = emulator.memory[wEnemyMon1Speed + 1]   = emulator.memory[wEnemyMonUnmodifiedSpeed + 1]   = 25 & 0xFF
	else:
		emulator.memory[wEnemyMonDefense + 0] = emulator.memory[wEnemyMon1Defense + 0] = emulator.memory[wEnemyMonUnmodifiedDefense + 0] = 999 >> 8
		emulator.memory[wEnemyMonDefense + 1] = emulator.memory[wEnemyMon1Defense + 1] = emulator.memory[wEnemyMonUnmodifiedDefense + 1] = 999 & 0xFF
		emulator.memory[wEnemyMonSpecial + 0] = emulator.memory[wEnemyMon1Special + 0] = emulator.memory[wEnemyMonUnmodifiedSpecial + 0] = 999 >> 8
		emulator.memory[wEnemyMonSpecial + 1] = emulator.memory[wEnemyMon1Special + 1] = emulator.memory[wEnemyMonUnmodifiedSpecial + 1] = 999 & 0xFF
		emulator.memory[wEnemyMonSpeed + 0]   = emulator.memory[wEnemyMon1Speed + 0]   = emulator.memory[wEnemyMonUnmodifiedSpeed + 0]   = 999 >> 8
		emulator.memory[wEnemyMonSpeed + 1]   = emulator.memory[wEnemyMon1Speed + 1]   = emulator.memory[wEnemyMonUnmodifiedSpeed + 1]   = 999 & 0xFF

	# Move list: <move>, -, -, -; 300 ATK; 300 DEF; 300 SPE; 300 SPD
	emulator.memory[wBattleMonMoves + 0]   = emulator.memory[wPartyMon1Moves + 0]   = move
	emulator.memory[wBattleMonMoves + 1]   = emulator.memory[wPartyMon1Moves + 1]   = AvailableMove.Empty
	emulator.memory[wBattleMonMoves + 2]   = emulator.memory[wPartyMon1Moves + 2]   = AvailableMove.Empty
	emulator.memory[wBattleMonMoves + 3]   = emulator.memory[wPartyMon1Moves + 3]   = AvailableMove.Empty
	emulator.memory[wBattleMonAttack + 0]  = emulator.memory[wPartyMon1Attack + 0]  = emulator.memory[wPlayerMonUnmodifiedAttack + 0]  = 300 >> 8
	emulator.memory[wBattleMonAttack + 1]  = emulator.memory[wPartyMon1Attack + 1]  = emulator.memory[wPlayerMonUnmodifiedAttack + 1]  = 300 & 0xFF
	emulator.memory[wBattleMonDefense + 0] = emulator.memory[wPartyMon1Defense + 0] = emulator.memory[wPlayerMonUnmodifiedDefense + 0] = 300 >> 8
	emulator.memory[wBattleMonDefense + 1] = emulator.memory[wPartyMon1Defense + 1] = emulator.memory[wPlayerMonUnmodifiedDefense + 1] = 300 & 0xFF
	emulator.memory[wBattleMonSpecial + 0] = emulator.memory[wPartyMon1Special + 0] = emulator.memory[wPlayerMonUnmodifiedSpecial + 0] = 300 >> 8
	emulator.memory[wBattleMonSpecial + 1] = emulator.memory[wPartyMon1Special + 1] = emulator.memory[wPlayerMonUnmodifiedSpecial + 1] = 300 & 0xFF
	emulator.memory[wBattleMonSpeed + 0]   = emulator.memory[wPartyMon1Speed + 0]   = emulator.memory[wPlayerMonUnmodifiedSpeed + 0]   = 300 >> 8
	emulator.memory[wBattleMonSpeed + 1]   = emulator.memory[wPartyMon1Speed + 1]   = emulator.memory[wPlayerMonUnmodifiedSpeed + 1]   = 300 & 0xFF

	state.me.team = [Pokemon(state, "", emulator.memory[wPartyMons:wPartyMons+44])]
	state.op.team = [Pokemon(state, "", emulator.memory[wEnemyMons:wEnemyMons+44])]

	starting_state = get_emulator_basic_state()
	if debug:
		print(dump_basic_state(starting_state[0]))
		print(dump_basic_state(starting_state[1]))
		print(state.me.team[0].dump())
		print(state.op.team[0].dump())
		print(starting_state[2], state.rng.getIndex(), starting_state[3], state.rng.getList())

	emulator.button_press('a')
	emulator.tick()
	emulator.button_release('a')
	emulator.tick(10)
	emulator.button_press('a')
	emulator.tick()
	emulator.button_release('a')
	emulator.tick()
	emulator.button_press('b')
	while emulator.memory[wSerialExchangeNybbleReceiveData] == 0xFF:
		emulator.memory[wSerialExchangeNybbleReceiveData] = 0
		if not emulator.tick():
			exit(0)
	while emulator.memory[0x9D64:0x9D6F] != [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7]: # Waiting...!
		if not emulator.tick(1 if debug else 30):
			exit(0)
	while emulator.memory[0x9D64:0x9D6F] == [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7]: # Waiting...!
		if not emulator.tick(1 if debug else 30):
			exit(0)
	state.me.nextAction = BattleAction.Attack1
	state.op.nextAction = BattleAction.Attack1
	battle.tick()
	while True:
		if not emulator.tick(1 if debug else 30):
			exit(0)
		if emulator.memory[0x9DD0] == 0xE1 and emulator.memory[0x9DD1] == 0xE2:
			break
		if emulator.memory[0x9DC1:0x9DCC] == [0x80, 0x92, 0x87, 0x7F, 0xAB, 0xAE, 0xB2, 0xB3, 0x7F, 0xB3, 0xAE]: # "ASH lost to"
			break
		if emulator.memory[0x9DC1:0x9DCD] == [0x80, 0x92, 0x87, 0x7F, 0xA3, 0xA4, 0xA5, 0xA4, 0xA0, 0xB3, 0xA4, 0xA3]: # "ASH defeated"
			break
		if emulator.memory[0x9D64:0x9D6F] == [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7]: # Waiting...!
			# For multi turn moves, we do 2 turns
			state.me.nextAction = BattleAction.Attack1
			state.op.nextAction = BattleAction.Attack1
			battle.tick()
			while emulator.memory[wSerialExchangeNybbleReceiveData] == 0xFF:
				emulator.memory[wSerialExchangeNybbleReceiveData] = 0
				if not emulator.tick():
					exit(0)
			while emulator.memory[0x9D64:0x9D6F] == [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7]: # Waiting...!
				if not emulator.tick(1 if debug else 30):
					exit(0)
	emulator.button_release('b')
	ending_state = get_emulator_basic_state()
	if debug:
		print(dump_basic_state(ending_state[0]))
		print(dump_basic_state(ending_state[1]))
		print(state.me.team[0].dump())
		print(state.op.team[0].dump())
		print(ending_state[2], state.rng.getIndex(), ending_state[3], state.rng.getList())
	f = compare_basic_states(battle.state, ending_state)
	battle.saveReplay('test.replay')
	state.rng.reset()
	return f[0], f[1], [state.rng.getList()]


def run_test(test):
	if debug:
		print(f"Testing {test['name']} ", test.get('args', []))
	else:
		print(f'{test['name']}: ', end="", flush=True)
	success, errors, extra = test['cb'](*test.get('args', []))
	if not success:
		if debug:
			print(errors)
		print("\033[31mFailed\033[0m")
	else:
		print("\033[32mPassed\033[0m")
	return errors, extra

rand_lists = [
	[0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00], #0
	[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF], #1
	[0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF], #2
	[0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80], #3
	[0xA0, 0x40, 0xA0, 0x40, 0xA0, 0x40, 0xA0, 0x40, 0xA0], #4
	[0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF], #5
	None, None, None, None, None, None, None, None, None, None
]
extra_lists = {
	AvailableMove.Sand_Attack: [
		[128,  17,  73,  86, 130,  10, 248,  81, 235]
	],
	AvailableMove.Smokescreen: [
		[113, 153,  60,  60, 238, 124, 239,  30, 124],
		[163, 145,  78, 209, 166, 188, 110, 110, 237],
		[222, 200, 109,  20,  45, 136, 134,  33,  93]
	],
	AvailableMove.Fire_Blast: [
		[248, 118, 7, 111, 14, 103, 172, 64, 130],
		[111, 14, 248, 118, 7, 103, 172, 64, 130]
	]
}

tests = []
for move_index in range(1, AvailableMove.Struggle + 1):
	for i, rand in enumerate(rand_lists + extra_lists.get(move_index, [])):
		name = AvailableMove(move_index).name
		tests.append({
			'name': f'{name}[{i}](Low)',
			'cb': test_move,
			'args': [move_index, rand, True],
			'group': name
		})
		tests.append({
			'name': f'{name}[{i}](High)',
			'cb': test_move,
			'args': [move_index, rand, False],
			'group': name
		})

results = []

parser = ArgumentParser(prog=sys.argv[0])
parser.add_argument('-d', '--debug', action='store_true')
parser.add_argument('-e', '--emu-debug', action='store_true')
parser.add_argument('-s', '--show-individual', action='store_true')
parser.add_argument('-t', '--test', nargs='*')
parser.add_argument('-o', '--output')
args = parser.parse_args()

debug = args.debug
emulator = PyBoy('pokeyellow.gbc', sound_volume=25, sound_emulated=debug, window='SDL2' if debug else 'null', debug=args.emu_debug)

if args.test is None:
	tests_to_run = tests
else:
	tests_to_run = []
	for requested in args.test:
		l = [t for t in tests if t['name'].startswith(requested)]
		if not l:
			print(f"Cannot find any test starting with '{requested}'")
			exit(1)
		tests_to_run += l


for test_object in tests_to_run:
	errors, extra = run_test(test_object)
	results.append({
		'name': test_object['name'],
		'errors': errors,
		'group': test_object['group'],
		'extra': extra
	})
	if not emulator.tick():
		exit(0)


success = 0
failures = 0
groups = {}
print()
fd = None if args.output is None else open(args.output, "w")
for r in sorted(results, key=lambda x: len(x['errors']) != 0):
	if r['group'] not in groups:
		groups[r['group']] = [0, 0]
	groups[r['group']][1] += 1
	if not r['errors']:
		success += 1
		groups[r['group']][0] += 1
		if args.show_individual:
			print(f"{r['name']}: \033[32mPassed\033[0m")
			if fd is not None:
				fd.write(f"{r['name']}: Passed\n")
		continue
	if args.show_individual:
		print(f"{r['name']} {r['extra']}: \033[31mFailed\033[0m")
		print("\n".join(f"\033[31;1m - {f}\033[0m" for f in r['errors']))
		if fd is not None:
			fd.write(f"{r['name']}: Failed\n")
			fd.write("\n".join(f" - {f}" for f in r['errors']) + "\n")
	failures += 1
group_failed = 0
group_succeed = 0
for group, v in sorted(groups.items(), key=lambda f: 1 - f[1][0] / f[1][1]):
	print(f"{group}: ", end="")
	if v[0] == v[1]:
		group_succeed += 1
		print('\033[32m', end="")
	else:
		group_failed += 1
		print('\033[31m', end="")
	print(f'{v[0]}\033[0m/{v[1]}')
	if fd is not None:
		fd.write(f"{group}: {v[0]}/{v[1]}\n")
print(f'{success} individual test(s) \033[32mpassed\033[0m. {failures} individual test(s) \033[31mfailed\033[0m.')
print(f'{group_succeed} test group(s) \033[32mpassed\033[0m. {group_failed} test group(s) \033[31mfailed\033[0m.')
if fd is not None:
	fd.write(f'{success} individual test(s) passed. {failures} individual test(s) failed.\n')
	fd.write(f'{group_succeed} test group(s) passed. {group_failed} test group(s) failed.\n')
	fd.close()
exit(0 if failures == 0 else 1)