from pyboy import PyBoy
from pyboy.plugins.game_wrapper_pokemon_gen1 import GameWrapperPokemonGen1
from GameEngine import RandomGenerator, PokemonBase, Pokemon, PokemonSpecies, Move, AvailableMove, Type, BattleHandler, BattleAction, StatusChange, MoveCategory, getAttackDamageMultiplier
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

assert enemyBaseAddr + nickOffset == 0xCFD9 # wEnemyMonNick
assert enemyBaseAddr + speciesOffset == 0xCFE4 # wEnemyMonSpecies
assert enemyBaseAddr + hpOffset == 0xCFE5 # wEnemyMonHP
assert enemyBaseAddr + boxLevelOffset == 0xCFE7 # wEnemyMonBoxLevel
assert enemyBaseAddr + statusOffset == 0xCFE8 # wEnemyMonStatus
assert enemyBaseAddr + type1Offset == 0xCFE9 # wEnemyMonType1
assert enemyBaseAddr + type2Offset == 0xCFEA # wEnemyMonType2
assert enemyBaseAddr + catchRateOffset == 0xCFEB # wEnemyMonCatchRate
assert enemyBaseAddr + movesOffset == 0xCFEC # wEnemyMonMoves
assert enemyBaseAddr + dvsOffset == 0xCFF0 # wEnemyMonDVs
assert enemyBaseAddr + levelOffset == 0xCFF2 # wEnemyMonLevel
assert enemyBaseAddr + maxHPOffset == 0xCFF3 # wEnemyMonMaxHP
assert enemyBaseAddr + attackOffset == 0xCFF5 # wEnemyMonAttack
assert enemyBaseAddr + defenseOffset == 0xCFF7 # wEnemyMonDefense
assert enemyBaseAddr + speedOffset == 0xCFF9 # wEnemyMonSpeed
assert enemyBaseAddr + specialOffset == 0xCFFB # wEnemyMonSpecial
assert enemyBaseAddr + ppOffset == 0xCFFD # wEnemyMonPP

assert playerBaseAddr + nickOffset == 0xD008 # wBattleMonNick
assert playerBaseAddr + speciesOffset == 0xD013 # wBattleMonSpecies
assert playerBaseAddr + hpOffset == 0xD014 # wBattleMonHP
assert playerBaseAddr + boxLevelOffset == 0xD016 # wBattleMonBoxLevel
assert playerBaseAddr + statusOffset == 0xD017 # wBattleMonStatus
assert playerBaseAddr + type1Offset == 0xD018 # wBattleMonType1
assert playerBaseAddr + type2Offset == 0xD019 # wBattleMonType2
assert playerBaseAddr + catchRateOffset == 0xD01A # wBattleMonCatchRate
assert playerBaseAddr + movesOffset == 0xD01B # wBattleMonMoves
assert playerBaseAddr + dvsOffset == 0xD01F # wBattleMonDVs
assert playerBaseAddr + levelOffset == 0xD021 # wBattleMonLevel
assert playerBaseAddr + maxHPOffset == 0xD022 # wBattleMonMaxHP
assert playerBaseAddr + attackOffset == 0xD024 # wBattleMonAttack
assert playerBaseAddr + defenseOffset == 0xD026 # wBattleMonDefense
assert playerBaseAddr + speedOffset == 0xD028 # wBattleMonSpeed
assert playerBaseAddr + specialOffset == 0xD02A # wBattleMonSpecial
assert playerBaseAddr + ppOffset == 0xD02C # wBattleMonPP

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

def get_input():
	while b:
		try:
			v = input("Enter address[ value]: ")
			try:
				v = v.split(" ")
				addr = int(v[0], 0)
				value = None
				if len(v) != 1:
					value = int(v[1], 0)
				sem.acquire()
				if value is None:
					value = emulator.memory[addr]
					print(f'${addr:04X} -> ${value:02X} ({value})')
				else:
					emulator.memory[addr] = value
					print(f'${addr:04X} set to ${value:02X}')
				sem.release()
			except:
				traceback.print_exc()
		except KeyboardInterrupt | EOFError:
			sem.acquire()
			emulator.stop(False)
			sem.release()
			return
		except:
			pass

def get_mon_state(base_address):
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

def get_emulator_state():
	return get_mon_state(playerBaseAddr), get_mon_state(enemyBaseAddr), emulator.memory[wLinkBattleRandomNumberListIndex]

def compare_states(emu_state, battle_state):
	pass

def test_move(move, random_state=None):
	print(f"Testing {move.name}")
	battle = BattleHandler(False, False)
	state = battle.state
	if random_state:
		state.rng.setList(random_state)
	else:
		state.rng.makeRandomList(9)
	fd = open("pokeyellow.state", "rb")
	emulator.load_state(fd)
	fd.close()
	l = state.rng.getList()
	emulator.memory[wLinkBattleRandomNumberListIndex] = 0
	for i in range(9):
		emulator.memory[wLinkBattleRandomNumberList + i] = l[i]
	emulator.memory[wEnemyMonMoves] = AvailableMove.Tail_Whip
	emulator.memory[wLinkState] = LINK_STATE_BATTLING
	emulator.memory[wBattleMonMoves + 0] = move
	emulator.memory[wBattleMonMoves + 1] = AvailableMove.Empty
	emulator.memory[wBattleMonMoves + 2] = AvailableMove.Empty
	emulator.memory[wBattleMonMoves + 3] = AvailableMove.Empty
	emulator.memory[wEnemyMonHP + 0] = 999 >> 8
	emulator.memory[wEnemyMonHP + 1] = 999 & 0xFF
	emulator.memory[wEnemyMonMaxHP + 0] = 999 >> 8
	emulator.memory[wEnemyMonMaxHP + 1] = 999 & 0xFF
	emulator.memory[wEnemyMonDefense + 0] = 25 >> 8
	emulator.memory[wEnemyMonDefense + 1] = 25 & 0xFF
	emulator.memory[wEnemyMonSpecial + 0] = 25 >> 8
	emulator.memory[wEnemyMonSpecial + 1] = 25 & 0xFF
	emulator.memory[wBattleMonAttack + 0] = 300 >> 8
	emulator.memory[wBattleMonAttack + 1] = 300 & 0xFF
	emulator.memory[wBattleMonDefense + 0] = 300 >> 8
	emulator.memory[wBattleMonDefense + 1] = 300 & 0xFF
	emulator.memory[wBattleMonSpeed + 0] = 300 >> 8
	emulator.memory[wBattleMonSpeed + 1] = 300 & 0xFF
	emulator.memory[wBattleMonSpecial + 0] = 300 >> 8
	emulator.memory[wBattleMonSpecial + 1] = 300 & 0xFF
	starting_state = get_emulator_state()
	print(starting_state)
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
		emulator.tick()
	while True:
		if not emulator.tick(1 if debug else 30):
			break
		if emulator.memory[0x9DD0] == 0xE1 and emulator.memory[0x9DD1] == 0xE2:
			break
		if emulator.memory[0x9DC1:0x9DCC] == [0x80, 0x92, 0x87, 0x7F, 0xAB, 0xAE, 0xB2, 0xB3, 0x7F, 0xB3, 0xAE]: # "ASH lost to"
			break
	emulator.button_release('b')
	ending_state = get_emulator_state()
	print(ending_state)
	return compare_states(battle.state, ending_state)

debug = False
if len(sys.argv) > 1:
	debug = True

emulator = PyBoy('pokeyellow.gbc', sound_volume=25, sound_emulated=debug, window='SDL2' if debug else 'null', debug=True)
sem = threading.Semaphore()
thread = threading.Thread(target=get_input)
b = True
for i in range(1, AvailableMove.Struggle + 1):
	rand_lists = [
		[0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
		[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF],
		[0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF],
		[0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80],
		[0xA0, 0x40, 0xA0, 0x40, 0xA0, 0x40, 0xA0, 0x40, 0xA0],
	]
	for rand in rand_lists:
		test_move(AvailableMove(i), rand)
		if not emulator.tick():
			exit(0)
#thread.start()
#while b:
#	sem.acquire()
#	b = emulator.tick()
#	sem.release()
#	time.sleep(0.01)
#thread.join()