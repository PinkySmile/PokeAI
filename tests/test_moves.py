import sys
import time
import threading
from argparse import ArgumentParser
from PokeBattle.Gen1.PyBoyEmulator import PyBoyEmulator
from PokeBattle.Gen1.BattleHandler import BattleHandler
from PokeBattle.Gen1.Type import Type
from PokeBattle.Gen1.Move import AvailableMove
from PokeBattle.Gen1.Pokemon import Pokemon, PokemonSpecies
from PokeBattle.Gen1.State import DesyncPolicy, BattleAction
from PokeBattle.Gen1.StatusChange import StatusChange
from PokeBattle.Gen1.YellowEmulator import GBAddress

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


def test_move(emulator, move, random_state, scenario, min_turns=6):
	battle = BattleHandler(False, debug)
	state = battle.state
	if random_state is not None:
		assert len(random_state) == 9
		state.rng.list = random_state
	else:
		state.rng.generate_list(9)
	if debug:
		state.logger = lambda x: print(f'Simulator: {x}')
		emulator.on_text_displayed = lambda x: print(f'Emulator: {x}')
	state.desync = DesyncPolicy.Ignore

	pokemon_data = [0] * 44
	pokemon_data[PACK_SPECIES] = PokemonSpecies.Eevee
	pokemon_data[PACK_CURR_LEVEL] = 5
	pokemon_data[PACK_STATUS] = StatusChange.OK
	pokemon_data[PACK_TYPEA] = Type.Normal
	pokemon_data[PACK_TYPEB] = Type.Normal
	if scenario & 4:
		pokemon_data[PACK_MOVE1] = AvailableMove.Substitute
	elif scenario & 2:
		pokemon_data[PACK_MOVE1] = AvailableMove.Bubble
	else:
		pokemon_data[PACK_MOVE1] = AvailableMove.Constrict
	pokemon_data[PACK_MOVE2] = AvailableMove.Empty
	pokemon_data[PACK_MOVE3] = AvailableMove.Empty
	pokemon_data[PACK_MOVE4] = AvailableMove.Empty
	pokemon_data[PACK_PPS_MOVE1] = 10
	pokemon_data[PACK_PPS_MOVE2] = 0
	pokemon_data[PACK_PPS_MOVE3] = 0
	pokemon_data[PACK_PPS_MOVE4] = 0
	pokemon_data[PACK_CURR_LEVEL_DUP] = 5
	pokemon_data[PACK_HP_HB  + 0] = 999 >> 8
	pokemon_data[PACK_HP_HB  + 1] = 999 & 0xFF
	pokemon_data[PACK_MAX_HP + 0] = 999 >> 8
	pokemon_data[PACK_MAX_HP + 1] = 999 & 0xFF

	if scenario & 1:
		pokemon_data[PACK_ATK + 0] = 999 >> 8
		pokemon_data[PACK_ATK + 1] = 999 & 0xFF
		pokemon_data[PACK_DEF + 0] = 25 >> 8
		pokemon_data[PACK_DEF + 1] = 25 & 0xFF
		pokemon_data[PACK_SPD + 0] = 25 >> 8
		pokemon_data[PACK_SPD + 1] = 25 & 0xFF
		pokemon_data[PACK_SPE + 0] = 999 >> 8
		pokemon_data[PACK_SPE + 1] = 999 & 0xFF
	else:
		pokemon_data[PACK_ATK + 0] = 999 >> 8
		pokemon_data[PACK_ATK + 1] = 999 & 0xFF
		pokemon_data[PACK_DEF + 0] = 999 >> 8
		pokemon_data[PACK_DEF + 1] = 999 & 0xFF
		pokemon_data[PACK_SPD + 0] = 999 >> 8
		pokemon_data[PACK_SPD + 1] = 999 & 0xFF
		pokemon_data[PACK_SPE + 0] = 999 >> 8
		pokemon_data[PACK_SPE + 1] = 999 & 0xFF
	state.op.name = "Player 2"
	state.op.team = [Pokemon(state, "", pokemon_data, True)]

	pokemon_data = [0] * 44
	pokemon_data[PACK_SPECIES] = PokemonSpecies.Pikachu
	pokemon_data[PACK_CURR_LEVEL] = 5
	pokemon_data[PACK_STATUS] = StatusChange.OK
	pokemon_data[PACK_TYPEA] = Type.Electric
	pokemon_data[PACK_TYPEB] = Type.Electric
	pokemon_data[PACK_MOVE1] = move
	pokemon_data[PACK_MOVE2] = AvailableMove.Empty
	pokemon_data[PACK_MOVE3] = AvailableMove.Empty
	pokemon_data[PACK_MOVE4] = AvailableMove.Empty
	pokemon_data[PACK_PPS_MOVE1] = 10
	pokemon_data[PACK_PPS_MOVE2] = 0
	pokemon_data[PACK_PPS_MOVE3] = 0
	pokemon_data[PACK_PPS_MOVE4] = 0
	pokemon_data[PACK_CURR_LEVEL_DUP] = 5
	pokemon_data[PACK_HP_HB  + 0] = 999 >> 8
	pokemon_data[PACK_HP_HB  + 1] = 999 & 0xFF
	pokemon_data[PACK_MAX_HP + 0] = 999 >> 8
	pokemon_data[PACK_MAX_HP + 1] = 999 & 0xFF
	pokemon_data[PACK_ATK + 0] = 300 >> 8
	pokemon_data[PACK_ATK + 1] = 300 & 0xFF
	pokemon_data[PACK_DEF + 0] = 100 >> 8
	pokemon_data[PACK_DEF + 1] = 100 & 0xFF
	pokemon_data[PACK_SPD + 0] = 300 >> 8
	pokemon_data[PACK_SPD + 1] = 300 & 0xFF
	pokemon_data[PACK_SPE + 0] = 300 >> 8
	pokemon_data[PACK_SPE + 1] = 300 & 0xFF
	state.me.name = "Player 1"
	state.me.team = [Pokemon(state, "", pokemon_data, False)]

	battle.start()
	emulator.init_battle(None, state)

	current_turn = 0
	emulator_state = emulator.get_emulator_basic_state()
	if debug:
		print(emulator.dump_basic_state(emulator_state[0]))
		print(emulator.dump_basic_state(emulator_state[1]))
		print(state.me.team[0].dump())
		print(state.op.team[0].dump())
		print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
	f = emulator.compare_basic_states(battle.state, emulator_state)
	if not f[0]:
		state.rng.reset()
		return f[0], [f"On turn {current_turn}: {e}" for e in f[1]], [state.rng.list]
	while True:
		if debug:
			print(f' ---------- TURN {current_turn + 1} ----------')
		if state.me.team[0].move_set[0].pp != 0:
			state.me.next_action = BattleAction.Attack1
		else:
			state.me.next_action = BattleAction.StruggleMove
		if state.op.pokemon_on_field.health == 0:
			state.op.next_action = BattleAction.Switch2
		else:
			state.op.next_action = BattleAction.Attack1
		battle.tick()
		emulator.step(state)
		current_turn += 1
		emulator_state = emulator.get_emulator_basic_state()
		if debug:
			print(emulator.dump_basic_state(emulator_state[0]))
			print(emulator.dump_basic_state(emulator_state[1]))
			print(state.me.team[0].dump())
			print(state.op.team[0].dump())
			print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
		f = emulator.compare_basic_states(battle.state, emulator_state)
		emulator.tick()
		if not f[0] or battle.finished or (current_turn > min_turns and not emulator.waiting):
			state.rng.reset()
			return f[0], [f"On turn {current_turn}: {e}" for e in f[1]], [state.rng.list]


def test_trap_move_turn_skip(emulator, move, random_state, scenario, min_turns=6):
	battle = BattleHandler(False, debug)
	state = battle.state
	if random_state is not None:
		assert len(random_state) == 9
		state.rng.list = random_state
	else:
		state.rng.generate_list(9)
	if debug:
		state.logger = lambda x: print(f'Simulator: {x}')
		emulator.on_text_displayed = lambda x: print(f'Emulator: {x}')
	state.desync = DesyncPolicy.Ignore

	pokemon_data = [0] * 44
	pokemon_data[PACK_SPECIES] = PokemonSpecies.Eevee
	pokemon_data[PACK_CURR_LEVEL] = 5
	pokemon_data[PACK_STATUS] = StatusChange.OK
	pokemon_data[PACK_TYPEA] = Type.Normal
	pokemon_data[PACK_TYPEB] = Type.Normal
	if scenario & 1:
		pokemon_data[PACK_MOVE1] = AvailableMove.Confuse_Ray
	else:
		pokemon_data[PACK_MOVE1] = AvailableMove.Thunder_Wave
	pokemon_data[PACK_MOVE2] = AvailableMove.Constrict
	pokemon_data[PACK_MOVE3] = AvailableMove.Empty
	pokemon_data[PACK_MOVE4] = AvailableMove.Empty
	pokemon_data[PACK_PPS_MOVE1] = 10
	pokemon_data[PACK_PPS_MOVE2] = 10
	pokemon_data[PACK_PPS_MOVE3] = 0
	pokemon_data[PACK_PPS_MOVE4] = 0
	pokemon_data[PACK_CURR_LEVEL_DUP] = 5
	pokemon_data[PACK_HP_HB  + 0] = 999 >> 8
	pokemon_data[PACK_HP_HB  + 1] = 999 & 0xFF
	pokemon_data[PACK_MAX_HP + 0] = 999 >> 8
	pokemon_data[PACK_MAX_HP + 1] = 999 & 0xFF
	pokemon_data[PACK_ATK + 0] = 999 >> 8
	pokemon_data[PACK_ATK + 1] = 999 & 0xFF
	pokemon_data[PACK_DEF + 0] = 999 >> 8
	pokemon_data[PACK_DEF + 1] = 999 & 0xFF
	pokemon_data[PACK_SPD + 0] = 999 >> 8
	pokemon_data[PACK_SPD + 1] = 999 & 0xFF
	pokemon_data[PACK_SPE + 0] = 999 >> 8
	pokemon_data[PACK_SPE + 1] = 999 & 0xFF
	state.op.name = "Player 2"
	state.op.team = [Pokemon(state, "", pokemon_data, True)]

	pokemon_data = [0] * 44
	pokemon_data[PACK_SPECIES] = PokemonSpecies.Pikachu
	pokemon_data[PACK_CURR_LEVEL] = 5
	pokemon_data[PACK_STATUS] = StatusChange.OK
	pokemon_data[PACK_TYPEA] = Type.Electric
	pokemon_data[PACK_TYPEB] = Type.Electric
	pokemon_data[PACK_MOVE1] = move
	pokemon_data[PACK_MOVE2] = AvailableMove.Empty
	pokemon_data[PACK_MOVE3] = AvailableMove.Empty
	pokemon_data[PACK_MOVE4] = AvailableMove.Empty
	pokemon_data[PACK_PPS_MOVE1] = 10
	pokemon_data[PACK_PPS_MOVE2] = 0
	pokemon_data[PACK_PPS_MOVE3] = 0
	pokemon_data[PACK_PPS_MOVE4] = 0
	pokemon_data[PACK_CURR_LEVEL_DUP] = 5
	pokemon_data[PACK_HP_HB  + 0] = 999 >> 8
	pokemon_data[PACK_HP_HB  + 1] = 999 & 0xFF
	pokemon_data[PACK_MAX_HP + 0] = 999 >> 8
	pokemon_data[PACK_MAX_HP + 1] = 999 & 0xFF
	pokemon_data[PACK_ATK + 0] = 300 >> 8
	pokemon_data[PACK_ATK + 1] = 300 & 0xFF
	pokemon_data[PACK_DEF + 0] = 100 >> 8
	pokemon_data[PACK_DEF + 1] = 100 & 0xFF
	pokemon_data[PACK_SPD + 0] = 300 >> 8
	pokemon_data[PACK_SPD + 1] = 300 & 0xFF
	pokemon_data[PACK_SPE + 0] = 300 >> 8
	pokemon_data[PACK_SPE + 1] = 300 & 0xFF
	state.me.name = "Player 1"
	state.me.team = [Pokemon(state, "", pokemon_data, False)]

	battle.start()
	emulator.init_battle(None, state)

	current_turn = 0
	emulator_state = emulator.get_emulator_basic_state()
	if debug:
		print(emulator.dump_basic_state(emulator_state[0]))
		print(emulator.dump_basic_state(emulator_state[1]))
		print(state.me.team[0].dump())
		print(state.op.team[0].dump())
		print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
	f = emulator.compare_basic_states(battle.state, emulator_state)
	if not f[0]:
		state.rng.reset()
		return f[0], [f"On turn {current_turn}: {e}" for e in f[1]], [state.rng.list]
	while True:
		if debug:
			print(f' ---------- TURN {current_turn + 1} ----------')
		state.me.next_action = BattleAction.Attack1
		if state.me.pokemon_on_field.status == 0:
			state.op.next_action = BattleAction.Attack1
		else:
			state.op.next_action = BattleAction.Attack2
		battle.tick()
		emulator.step(state)
		current_turn += 1
		emulator_state = emulator.get_emulator_basic_state()
		if debug:
			print(emulator.dump_basic_state(emulator_state[0]))
			print(emulator.dump_basic_state(emulator_state[1]))
			print(state.me.team[0].dump())
			print(state.op.team[0].dump())
			print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
		f = emulator.compare_basic_states(battle.state, emulator_state)
		if not f[0] or battle.finished or (current_turn > min_turns and not emulator.waiting):
			state.rng.reset()
			return f[0], [f"On turn {current_turn}: {e}" for e in f[1]], [state.rng.list]


def test_bind_switch(emulator, move, random_state, scenario):
	min_turns = 6
	battle = BattleHandler(False, debug)
	state = battle.state
	if random_state is not None:
		assert len(random_state) == 9
		state.rng.list = random_state
	else:
		state.rng.generate_list(9)
	if debug:
		state.logger = lambda x: print(f'Simulator: {x}')
		emulator.on_text_displayed = lambda x: print(f'Emulator: {x}')
	state.desync = DesyncPolicy.Ignore

	pokemon_data = [0] * 44
	pokemon_data[PACK_SPECIES] = PokemonSpecies.Eevee
	pokemon_data[PACK_CURR_LEVEL] = 5
	pokemon_data[PACK_STATUS] = StatusChange.OK
	pokemon_data[PACK_TYPEA] = Type.Normal
	pokemon_data[PACK_TYPEB] = Type.Normal
	if scenario & 1:
		pokemon_data[PACK_MOVE1] = AvailableMove.Substitute
	else:
		pokemon_data[PACK_MOVE1] = AvailableMove.Constrict
	pokemon_data[PACK_MOVE2] = AvailableMove.Empty
	pokemon_data[PACK_MOVE3] = AvailableMove.Empty
	pokemon_data[PACK_MOVE4] = AvailableMove.Empty
	pokemon_data[PACK_PPS_MOVE1] = 10
	pokemon_data[PACK_PPS_MOVE2] = 0
	pokemon_data[PACK_PPS_MOVE3] = 0
	pokemon_data[PACK_PPS_MOVE4] = 0
	pokemon_data[PACK_CURR_LEVEL_DUP] = 5
	pokemon_data[PACK_HP_HB  + 0] = 999 >> 8
	pokemon_data[PACK_HP_HB  + 1] = 999 & 0xFF
	pokemon_data[PACK_MAX_HP + 0] = 999 >> 8
	pokemon_data[PACK_MAX_HP + 1] = 999 & 0xFF

	pokemon_data[PACK_ATK + 0] = 999 >> 8
	pokemon_data[PACK_ATK + 1] = 999 & 0xFF
	pokemon_data[PACK_DEF + 0] = 25  >> 8
	pokemon_data[PACK_DEF + 1] = 25  & 0xFF
	pokemon_data[PACK_SPD + 0] = 25  >> 8
	pokemon_data[PACK_SPD + 1] = 25  & 0xFF
	pokemon_data[PACK_SPE + 0] = 999 >> 8
	pokemon_data[PACK_SPE + 1] = 999 & 0xFF
	state.op.name = "Player 2"
	state.op.team = [Pokemon(state, "", pokemon_data, True), Pokemon(state, "", pokemon_data, True)]

	pokemon_data = [0] * 44
	pokemon_data[PACK_SPECIES] = PokemonSpecies.Pikachu
	pokemon_data[PACK_CURR_LEVEL] = 5
	pokemon_data[PACK_STATUS] = StatusChange.OK
	pokemon_data[PACK_TYPEA] = Type.Electric
	pokemon_data[PACK_TYPEB] = Type.Electric
	pokemon_data[PACK_MOVE1] = move
	pokemon_data[PACK_MOVE2] = AvailableMove.Fire_Blast
	pokemon_data[PACK_MOVE3] = AvailableMove.Empty
	pokemon_data[PACK_MOVE4] = AvailableMove.Empty
	pokemon_data[PACK_PPS_MOVE1] = 10
	pokemon_data[PACK_PPS_MOVE2] = 10
	pokemon_data[PACK_PPS_MOVE3] = 0
	pokemon_data[PACK_PPS_MOVE4] = 0
	pokemon_data[PACK_CURR_LEVEL_DUP] = 5
	pokemon_data[PACK_HP_HB  + 0] = 999 >> 8
	pokemon_data[PACK_HP_HB  + 1] = 999 & 0xFF
	pokemon_data[PACK_MAX_HP + 0] = 999 >> 8
	pokemon_data[PACK_MAX_HP + 1] = 999 & 0xFF
	pokemon_data[PACK_ATK + 0] = 300 >> 8
	pokemon_data[PACK_ATK + 1] = 300 & 0xFF
	pokemon_data[PACK_DEF + 0] = 100 >> 8
	pokemon_data[PACK_DEF + 1] = 100 & 0xFF
	pokemon_data[PACK_SPD + 0] = 300 >> 8
	pokemon_data[PACK_SPD + 1] = 300 & 0xFF
	pokemon_data[PACK_SPE + 0] = 300 >> 8
	pokemon_data[PACK_SPE + 1] = 300 & 0xFF
	state.me.name = "Player 1"
	state.me.team = [Pokemon(state, "", pokemon_data, False)]

	battle.start()
	emulator.init_battle(None, state)

	current_turn = 0
	emulator_state = emulator.get_emulator_basic_state()
	if debug:
		print(emulator.dump_basic_state(emulator_state[0]))
		print(emulator.dump_basic_state(emulator_state[1]))
		print(state.me.team[0].dump())
		print(state.op.team[0].dump())
		print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
	f = emulator.compare_basic_states(battle.state, emulator_state)
	if not f[0]:
		state.rng.reset()
		return f[0], [f"On turn {current_turn}: {e}" for e in f[1]], [state.rng.list]
	while True:
		if debug:
			print(f' ---------- TURN {current_turn + 1} ----------')
		if state.op.pokemon_on_field.health == 0:
			state.me.next_action = BattleAction.Attack1
			if state.op.team[1].health != 0:
				state.op.next_action = BattleAction.Switch2
			else:
				state.op.next_action = BattleAction.Switch1
		elif current_turn % 2 == 1:
			state.me.next_action = BattleAction.Attack2
			if current_turn % 4 == 0:
				if state.op.team[1].health != 0:
					state.op.next_action = BattleAction.Switch2
				else:
					state.op.next_action = BattleAction.Attack1
			else:
				if state.op.team[0].health != 0:
					state.op.next_action = BattleAction.Switch1
				else:
					state.op.next_action = BattleAction.Attack2
		else:
			state.me.next_action = BattleAction.Attack1
			state.op.next_action = BattleAction.Attack1
		battle.tick()
		emulator.step(state)
		current_turn += 1
		emulator_state = emulator.get_emulator_basic_state()
		if debug:
			print(emulator.dump_basic_state(emulator_state[0]))
			print(emulator.dump_basic_state(emulator_state[1]))
			print(state.me.team[0].dump())
			print(state.op.team[0].dump())
			print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
		f = emulator.compare_basic_states(battle.state, emulator_state)
		if not f[0] or battle.finished or (current_turn > min_turns and not emulator.waiting):
			state.rng.reset()
			return f[0], [f"On turn {current_turn}: {e}" for e in f[1]], [state.rng.list]


def test_bind_switch_inverted(emulator, move, random_state, scenario):
	min_turns = 6
	battle = BattleHandler(False, debug)
	state = battle.state
	if random_state is not None:
		assert len(random_state) == 9
		state.rng.list = random_state
	else:
		state.rng.generate_list(9)
	if debug:
		state.logger = lambda x: print(f'Simulator: {x}')
		emulator.on_text_displayed = lambda x: print(f'Emulator: {x}')
	state.desync = DesyncPolicy.Ignore

	pokemon_data = [0] * 44
	pokemon_data[PACK_SPECIES] = PokemonSpecies.Eevee
	pokemon_data[PACK_CURR_LEVEL] = 5
	pokemon_data[PACK_STATUS] = StatusChange.OK
	pokemon_data[PACK_TYPEA] = Type.Normal
	pokemon_data[PACK_TYPEB] = Type.Normal
	if scenario & 1:
		pokemon_data[PACK_MOVE1] = AvailableMove.Substitute
	else:
		pokemon_data[PACK_MOVE1] = AvailableMove.Constrict
	pokemon_data[PACK_MOVE2] = AvailableMove.Empty
	pokemon_data[PACK_MOVE3] = AvailableMove.Empty
	pokemon_data[PACK_MOVE4] = AvailableMove.Empty
	pokemon_data[PACK_PPS_MOVE1] = 10
	pokemon_data[PACK_PPS_MOVE2] = 0
	pokemon_data[PACK_PPS_MOVE3] = 0
	pokemon_data[PACK_PPS_MOVE4] = 0
	pokemon_data[PACK_CURR_LEVEL_DUP] = 5
	pokemon_data[PACK_HP_HB  + 0] = 999 >> 8
	pokemon_data[PACK_HP_HB  + 1] = 999 & 0xFF
	pokemon_data[PACK_MAX_HP + 0] = 999 >> 8
	pokemon_data[PACK_MAX_HP + 1] = 999 & 0xFF

	pokemon_data[PACK_ATK + 0] = 999 >> 8
	pokemon_data[PACK_ATK + 1] = 999 & 0xFF
	pokemon_data[PACK_DEF + 0] = 25  >> 8
	pokemon_data[PACK_DEF + 1] = 25  & 0xFF
	pokemon_data[PACK_SPD + 0] = 25  >> 8
	pokemon_data[PACK_SPD + 1] = 25  & 0xFF
	pokemon_data[PACK_SPE + 0] = 999 >> 8
	pokemon_data[PACK_SPE + 1] = 999 & 0xFF
	state.op.name = "Player 2"
	state.op.team = [Pokemon(state, "", pokemon_data, True)]

	pokemon_data = [0] * 44
	pokemon_data[PACK_SPECIES] = PokemonSpecies.Pikachu
	pokemon_data[PACK_CURR_LEVEL] = 5
	pokemon_data[PACK_STATUS] = StatusChange.OK
	pokemon_data[PACK_TYPEA] = Type.Electric
	pokemon_data[PACK_TYPEB] = Type.Electric
	pokemon_data[PACK_MOVE1] = move
	pokemon_data[PACK_MOVE2] = AvailableMove.Fire_Blast
	pokemon_data[PACK_MOVE3] = AvailableMove.Empty
	pokemon_data[PACK_MOVE4] = AvailableMove.Empty
	pokemon_data[PACK_PPS_MOVE1] = 10
	pokemon_data[PACK_PPS_MOVE2] = 10
	pokemon_data[PACK_PPS_MOVE3] = 0
	pokemon_data[PACK_PPS_MOVE4] = 0
	pokemon_data[PACK_CURR_LEVEL_DUP] = 5
	pokemon_data[PACK_HP_HB  + 0] = 999 >> 8
	pokemon_data[PACK_HP_HB  + 1] = 999 & 0xFF
	pokemon_data[PACK_MAX_HP + 0] = 999 >> 8
	pokemon_data[PACK_MAX_HP + 1] = 999 & 0xFF
	pokemon_data[PACK_ATK + 0] = 300 >> 8
	pokemon_data[PACK_ATK + 1] = 300 & 0xFF
	pokemon_data[PACK_DEF + 0] = 100 >> 8
	pokemon_data[PACK_DEF + 1] = 100 & 0xFF
	pokemon_data[PACK_SPD + 0] = 300 >> 8
	pokemon_data[PACK_SPD + 1] = 300 & 0xFF
	pokemon_data[PACK_SPE + 0] = 300 >> 8
	pokemon_data[PACK_SPE + 1] = 300 & 0xFF
	state.me.name = "Player 1"
	state.me.team = [Pokemon(state, "", pokemon_data, False), Pokemon(state, "", pokemon_data, False)]

	battle.start()
	emulator.init_battle(None, state)

	current_turn = 0
	emulator_state = emulator.get_emulator_basic_state()
	if debug:
		print(emulator.dump_basic_state(emulator_state[0]))
		print(emulator.dump_basic_state(emulator_state[1]))
		print(state.me.team[0].dump())
		print(state.op.team[0].dump())
		print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
	f = emulator.compare_basic_states(battle.state, emulator_state)
	if not f[0]:
		state.rng.reset()
		return f[0], [f"On turn {current_turn}: {e}" for e in f[1]], [state.rng.list]
	while True:
		if debug:
			print(f' ---------- TURN {current_turn + 1} ----------')
		if state.me.pokemon_on_field.health == 0:
			state.op.next_action = BattleAction.Attack1
			if state.me.team[1].health != 0:
				state.me.next_action = BattleAction.Switch2
			else:
				state.me.next_action = BattleAction.Switch1
		elif current_turn % 2 == 1:
			if current_turn % 4 == 3:
				state.me.next_action = BattleAction.Switch1
			else:
				state.me.next_action = BattleAction.Switch2
			state.op.next_action = BattleAction.NoAction
		else:
			state.me.next_action = BattleAction.Attack1
			state.op.next_action = BattleAction.Attack1
		battle.tick()
		emulator.step(state)
		current_turn += 1
		emulator_state = emulator.get_emulator_basic_state()
		if debug:
			print(emulator.dump_basic_state(emulator_state[0]))
			print(emulator.dump_basic_state(emulator_state[1]))
			print(state.me.team[0].dump())
			print(state.op.team[0].dump())
			print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
		f = emulator.compare_basic_states(battle.state, emulator_state)
		if not f[0] or battle.finished or (current_turn > min_turns and not emulator.waiting):
			state.rng.reset()
			return f[0], [f"On turn {current_turn}: {e}" for e in f[1]], [state.rng.list]


def hyper_beam_status_move(emulator: PyBoyEmulator, move: int, random_state: list|None, scenario: int):
	min_turns = 6
	battle = BattleHandler(False, debug)
	state = battle.state
	if random_state is not None:
		assert len(random_state) == 9
		state.rng.list = random_state
	else:
		state.rng.generate_list(9)
	if debug:
		state.logger = lambda x: print(f'Simulator: {x}')
		emulator.on_text_displayed = lambda x: print(f'Emulator: {x}')
	state.desync = DesyncPolicy.Ignore

	pokemon_data = [0] * 44
	pokemon_data[PACK_SPECIES] = PokemonSpecies.Eevee
	pokemon_data[PACK_CURR_LEVEL] = 5
	pokemon_data[PACK_STATUS] = StatusChange.OK
	pokemon_data[PACK_TYPEA] = Type.Normal
	pokemon_data[PACK_TYPEB] = Type.Normal
	pokemon_data[PACK_MOVE1] = AvailableMove.Thunder_Wave
	pokemon_data[PACK_MOVE2] = move
	pokemon_data[PACK_MOVE3] = AvailableMove.Empty
	pokemon_data[PACK_MOVE4] = AvailableMove.Empty
	pokemon_data[PACK_PPS_MOVE1] = 10
	pokemon_data[PACK_PPS_MOVE2] = 10
	pokemon_data[PACK_PPS_MOVE3] = 0
	pokemon_data[PACK_PPS_MOVE4] = 0
	pokemon_data[PACK_CURR_LEVEL_DUP] = 5
	pokemon_data[PACK_HP_HB  + 0] = 999 >> 8
	pokemon_data[PACK_HP_HB  + 1] = 999 & 0xFF
	pokemon_data[PACK_MAX_HP + 0] = 999 >> 8
	pokemon_data[PACK_MAX_HP + 1] = 999 & 0xFF

	pokemon_data[PACK_ATK + 0] = 999 >> 8
	pokemon_data[PACK_ATK + 1] = 999 & 0xFF
	pokemon_data[PACK_DEF + 0] = 25  >> 8
	pokemon_data[PACK_DEF + 1] = 25  & 0xFF
	pokemon_data[PACK_SPD + 0] = 999  >> 8
	pokemon_data[PACK_SPD + 1] = 999 & 0xFF
	pokemon_data[PACK_SPE + 0] = 999 >> 8
	pokemon_data[PACK_SPE + 1] = 999 & 0xFF
	state.op.name = "Player 2"
	state.op.team = [Pokemon(state, "", pokemon_data, True)]

	pokemon_data = [0] * 44
	pokemon_data[PACK_SPECIES] = PokemonSpecies.Pikachu
	pokemon_data[PACK_CURR_LEVEL] = 5
	pokemon_data[PACK_STATUS] = StatusChange.OK
	pokemon_data[PACK_TYPEA] = Type.Electric
	pokemon_data[PACK_TYPEB] = Type.Electric
	pokemon_data[PACK_MOVE1] = AvailableMove.Hyper_Beam
	pokemon_data[PACK_MOVE2] = AvailableMove.Empty
	pokemon_data[PACK_MOVE3] = AvailableMove.Empty
	pokemon_data[PACK_MOVE4] = AvailableMove.Empty
	pokemon_data[PACK_PPS_MOVE1] = 10
	pokemon_data[PACK_PPS_MOVE2] = 0
	pokemon_data[PACK_PPS_MOVE3] = 0
	pokemon_data[PACK_PPS_MOVE4] = 0
	pokemon_data[PACK_CURR_LEVEL_DUP] = 5
	pokemon_data[PACK_HP_HB  + 0] = 999 >> 8
	pokemon_data[PACK_HP_HB  + 1] = 999 & 0xFF
	pokemon_data[PACK_MAX_HP + 0] = 999 >> 8
	pokemon_data[PACK_MAX_HP + 1] = 999 & 0xFF
	pokemon_data[PACK_ATK + 0] = 300 >> 8
	pokemon_data[PACK_ATK + 1] = 300 & 0xFF
	pokemon_data[PACK_DEF + 0] = 100 >> 8
	pokemon_data[PACK_DEF + 1] = 100 & 0xFF
	pokemon_data[PACK_SPD + 0] = 300 >> 8
	pokemon_data[PACK_SPD + 1] = 300 & 0xFF
	pokemon_data[PACK_SPE + 0] = 300 >> 8
	pokemon_data[PACK_SPE + 1] = 300 & 0xFF
	state.me.name = "Player 1"
	state.me.team = [Pokemon(state, "", pokemon_data, False)]

	battle.start()
	emulator.init_battle(None, state)

	current_turn = 0
	emulator_state = emulator.get_emulator_basic_state()
	if debug:
		print(emulator.dump_basic_state(emulator_state[0]))
		print(emulator.dump_basic_state(emulator_state[1]))
		print(state.me.team[0].dump())
		print(state.op.team[0].dump())
		print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
	f = emulator.compare_basic_states(battle.state, emulator_state)
	if not f[0]:
		state.rng.reset()
		return f[0], [f"On turn {current_turn}: {e}" for e in f[1]], [state.rng.list]
	while True:
		if debug:
			print(f' ---------- TURN {current_turn + 1} ----------')
		if state.me.pokemon_on_field.recharging != bool(scenario):
			state.op.next_action = BattleAction.Attack1
		else:
			state.op.next_action = BattleAction.Attack2
		if state.me.team[0].move_set[0].pp != 0:
			state.me.next_action = BattleAction.Attack1
		else:
			state.me.next_action = BattleAction.StruggleMove
		battle.tick()
		emulator.step(state)
		current_turn += 1
		emulator_state = emulator.get_emulator_basic_state()
		if debug:
			print(emulator.dump_basic_state(emulator_state[0]))
			print(emulator.dump_basic_state(emulator_state[1]))
			print(state.me.team[0].dump())
			print(state.op.team[0].dump())
			print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
		f = emulator.compare_basic_states(battle.state, emulator_state)
		if not f[0] or battle.finished or (current_turn > min_turns and not emulator.waiting):
			state.rng.reset()
			return f[0], [f"On turn {current_turn}: {e}" for e in f[1]], [state.rng.list]


def test_mist(emulator: PyBoyEmulator, move: int, random_state: list|None):
	min_turns = 6
	battle = BattleHandler(False, debug)
	state = battle.state
	if random_state is not None:
		assert len(random_state) == 9
		state.rng.list = random_state
	else:
		state.rng.generate_list(9)
	if debug:
		state.logger = lambda x: print(f'Simulator: {x}')
		emulator.on_text_displayed = lambda x: print(f'Emulator: {x}')
	state.desync = DesyncPolicy.Ignore

	pokemon_data = [0] * 44
	pokemon_data[PACK_SPECIES] = PokemonSpecies.Eevee
	pokemon_data[PACK_CURR_LEVEL] = 5
	pokemon_data[PACK_STATUS] = StatusChange.OK
	pokemon_data[PACK_TYPEA] = Type.Normal
	pokemon_data[PACK_TYPEB] = Type.Normal
	pokemon_data[PACK_MOVE1] = move
	pokemon_data[PACK_MOVE2] = AvailableMove.Empty
	pokemon_data[PACK_MOVE3] = AvailableMove.Empty
	pokemon_data[PACK_MOVE4] = AvailableMove.Empty
	pokemon_data[PACK_PPS_MOVE1] = 10
	pokemon_data[PACK_PPS_MOVE2] = 0
	pokemon_data[PACK_PPS_MOVE3] = 0
	pokemon_data[PACK_PPS_MOVE4] = 0
	pokemon_data[PACK_CURR_LEVEL_DUP] = 5
	pokemon_data[PACK_HP_HB  + 0] = 999 >> 8
	pokemon_data[PACK_HP_HB  + 1] = 999 & 0xFF
	pokemon_data[PACK_MAX_HP + 0] = 999 >> 8
	pokemon_data[PACK_MAX_HP + 1] = 999 & 0xFF

	pokemon_data[PACK_ATK + 0] = 999 >> 8
	pokemon_data[PACK_ATK + 1] = 999 & 0xFF
	pokemon_data[PACK_DEF + 0] = 25  >> 8
	pokemon_data[PACK_DEF + 1] = 25  & 0xFF
	pokemon_data[PACK_SPD + 0] = 999  >> 8
	pokemon_data[PACK_SPD + 1] = 999 & 0xFF
	pokemon_data[PACK_SPE + 0] = 999 >> 8
	pokemon_data[PACK_SPE + 1] = 999 & 0xFF
	state.op.name = "Player 2"
	state.op.team = [Pokemon(state, "", pokemon_data, True)]

	pokemon_data = [0] * 44
	pokemon_data[PACK_SPECIES] = PokemonSpecies.Pikachu
	pokemon_data[PACK_CURR_LEVEL] = 5
	pokemon_data[PACK_STATUS] = StatusChange.OK
	pokemon_data[PACK_TYPEA] = Type.Electric
	pokemon_data[PACK_TYPEB] = Type.Electric
	pokemon_data[PACK_MOVE1] = AvailableMove.Mist
	pokemon_data[PACK_MOVE2] = AvailableMove.Empty
	pokemon_data[PACK_MOVE3] = AvailableMove.Empty
	pokemon_data[PACK_MOVE4] = AvailableMove.Empty
	pokemon_data[PACK_PPS_MOVE1] = 10
	pokemon_data[PACK_PPS_MOVE2] = 0
	pokemon_data[PACK_PPS_MOVE3] = 0
	pokemon_data[PACK_PPS_MOVE4] = 0
	pokemon_data[PACK_CURR_LEVEL_DUP] = 5
	pokemon_data[PACK_HP_HB  + 0] = 999 >> 8
	pokemon_data[PACK_HP_HB  + 1] = 999 & 0xFF
	pokemon_data[PACK_MAX_HP + 0] = 999 >> 8
	pokemon_data[PACK_MAX_HP + 1] = 999 & 0xFF
	pokemon_data[PACK_ATK + 0] = 300 >> 8
	pokemon_data[PACK_ATK + 1] = 300 & 0xFF
	pokemon_data[PACK_DEF + 0] = 100 >> 8
	pokemon_data[PACK_DEF + 1] = 100 & 0xFF
	pokemon_data[PACK_SPD + 0] = 300 >> 8
	pokemon_data[PACK_SPD + 1] = 300 & 0xFF
	pokemon_data[PACK_SPE + 0] = 300 >> 8
	pokemon_data[PACK_SPE + 1] = 300 & 0xFF
	state.me.name = "Player 1"
	state.me.team = [Pokemon(state, "", pokemon_data, False)]

	battle.start()
	emulator.init_battle(None, state)

	current_turn = 0
	emulator_state = emulator.get_emulator_basic_state()
	if debug:
		print(emulator.dump_basic_state(emulator_state[0]))
		print(emulator.dump_basic_state(emulator_state[1]))
		print(state.me.team[0].dump())
		print(state.op.team[0].dump())
		print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
	f = emulator.compare_basic_states(battle.state, emulator_state)
	if not f[0]:
		state.rng.reset()
		return f[0], [f"On turn {current_turn}: {e}" for e in f[1]], [state.rng.list]
	while True:
		if debug:
			print(f' ---------- TURN {current_turn + 1} ----------')
		if state.me.pokemon_on_field.move_disabled == 0:
			state.me.next_action = BattleAction.StruggleMove
		else:
			state.me.next_action = BattleAction.Attack1
		state.op.next_action = BattleAction.Attack1
		battle.tick()
		emulator.step(state)
		current_turn += 1
		emulator_state = emulator.get_emulator_basic_state()
		if debug:
			print(emulator.dump_basic_state(emulator_state[0]))
			print(emulator.dump_basic_state(emulator_state[1]))
			print(state.me.team[0].dump())
			print(state.op.team[0].dump())
			print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
		f = emulator.compare_basic_states(battle.state, emulator_state)
		if not f[0] or battle.finished or (current_turn > min_turns and not emulator.waiting):
			state.rng.reset()
			return f[0], [f"On turn {current_turn}: {e}" for e in f[1]], [state.rng.list]


def run_test(test, emulator):
	if debug:
		print(f"Testing {test['name']} ", test.get('args', []))
	elif jobs == 1:
		print(f'{test['name']}: ', end="", flush=True)
	success, errors, extra = test['cb'](emulator, *test.get('args', []))
	if jobs == 1:
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
	AvailableMove.Pound: [
		[204, 110, 122, 190,  50, 241,  10, 146, 180]
	],
	AvailableMove.Sand_Attack: [
		[128,  17,  73,  86, 130,  10, 248,  81, 235]
	],
	AvailableMove.Smokescreen: [
		[113, 153,  60,  60, 238, 124, 239,  30, 124],
		[163, 145,  78, 209, 166, 188, 110, 110, 237],
		[222, 200, 109,  20,  45, 136, 134,  33,  93]
	],
	AvailableMove.Fire_Blast: [
		[248, 118,   7, 111,  14, 103, 172,  64, 130],
		[111,  14, 248, 118,   7, 103, 172,  64, 130]
	],
	AvailableMove.String_Shot: [
		[  4,  63, 161, 224, 216, 223, 104, 116, 239]
	],
	AvailableMove.Clamp: [
		[185, 140,  44,   9, 213, 131, 159, 100, 113],
		[ 29, 229,  85, 215, 221, 195,   7,  38, 118]
	],
	AvailableMove.Wrap: [
		[185, 140,  44,   9, 213, 131, 159, 100, 113],
		[ 29, 229,  85, 215, 221, 195,   7,  38, 118],
		[ 50, 219,  65,  21, 137, 198,  18, 231, 200]
	],
	AvailableMove.Stomp: [
		[240, 189,  56,  27, 100, 178, 159, 227, 132],
		[ 78, 180,  90, 234,   5, 120,  28, 214, 234],
		[  7,  35, 174, 108, 104, 161,  75, 191, 118]
	],
	AvailableMove.Low_Kick: [
		[  9,  26, 245,  52,  74,  71, 220, 133, 230]
	],
	AvailableMove.Submission: [
		[229,   8, 157,  51, 158, 233,  42,  83,  10]
	],
	AvailableMove.Double_Kick: [
		[198,  46,  19, 167, 197, 228, 144,  12, 228],
		[131,  73, 103, 200, 198, 245, 207, 125,  78]
	],
	AvailableMove.Doubleslap: [
		[137,  49, 218, 202,  45, 152, 150, 134,  58]
	],
	AvailableMove.Confuse_Ray: [
		[ 93, 179, 138, 177, 116, 218, 202,   3, 108]
	],
	AvailableMove.Thrash: [
		[142, 172, 241,  22,  11, 130,  32,  83, 116]
	],
	AvailableMove.Psybeam: [
		[132,  84,  53, 217, 113,  25, 203,  81, 173],
		[174, 168, 171,  25, 193, 233, 186, 163,  94]
	],
	AvailableMove.Agility: [
		[236, 181, 249, 127, 145,  72,  32,   7, 197]
	],
	AvailableMove.Metronome: [
		[202,  88, 229, 250,  88, 170, 200,  85, 211],
		[228,  54, 183, 208, 182,  28, 135,  72, 106],
		[ 47, 203,  15,  24, 188,  70, 195, 139, 158],
		[104, 128,  58,  97, 242, 113, 239, 117,  60],
		[112, 168, 199, 244, 243, 224, 167, 192, 168],
		[204, 227,  35, 111, 224, 158, 249, 221,  61],
		[ 86,  36, 122,  77, 156,  69, 118, 128, 227],
		[ 72, 249, 110, 219,  17, 201, 206,  80, 144],
		[ 83, 252, 161,  23, 176, 198, 107, 165, 167],
		[ 63,  54,  78,   9, 244,  23, 234, 245, 195], # Metronome[25](H*) => Sleep -> Mirror Move should fail
		[135, 193, 204,  72,  70, 187, 223, 161, 250],
		[182,  58, 232, 172,  75, 139,  79, 136, 165], # Metronome[27](*)  => Fake issue. Test fails if dying while substitute is up.
		[ 78, 230, 153,  47, 128, 211, 235, 198,   2], # Metronome[28](*)  => Freeze -> Mirror Move should fail
		[172,   0, 174, 212,  51, 152, 214,  45, 117], # Metronome[29](*)  => Fake issue. Test fails if dying while substitute is up.
		None, None, None, None, None, None, None, None,
		None, None, None, None, None, None, None, None,
		None, None, None, None, None, None, None, None,
		None, None, None, None, None, None, None, None,
		None, None, None, None, None, None, None, None,
		None, None, None, None, None, None, None, None,
		None, None, None, None, None, None, None, None,
		None, None, None, None, None, None, None, None
	]
}
binding_moves = [
	AvailableMove.Bind,
	AvailableMove.Wrap,
	AvailableMove.Clamp,
	AvailableMove.Fire_Spin
]
status_moves = [
	AvailableMove.Sing,
	AvailableMove.Supersonic,
	AvailableMove.Leech_Seed,
	AvailableMove.Poisonpowder,
	AvailableMove.Stun_Spore,
	AvailableMove.Sleep_Powder,
	AvailableMove.Thunder_Wave,
	AvailableMove.Toxic,
	AvailableMove.Hypnosis,
	AvailableMove.Confuse_Ray,
	AvailableMove.Glare,
	AvailableMove.Poison_Gas,
	AvailableMove.Lovely_Kiss,
	AvailableMove.Spore
]

# TODO: Add status (PSN, BRN, LCH) + kill test
tests = []
for move_index in range(1, AvailableMove.Struggle + 1):
	for i, rand in enumerate(rand_lists + extra_lists.get(move_index, [])):
		name = AvailableMove(move_index).name
		tests.append({
			'name': f'{name}[{i}](LP)',
			'cb': test_move,
			'args': [move_index, rand, 1],
			'group': name
		})
		tests.append({
			'name': f'{name}[{i}](HP)',
			'cb': test_move,
			'args': [move_index, rand, 0],
			'group': name
		})
		tests.append({
			'name': f'{name}[{i}](LS)',
			'cb': test_move,
			'args': [move_index, rand, 3],
			'group': name
		})
		tests.append({
			'name': f'{name}[{i}](HS)',
			'cb': test_move,
			'args': [move_index, rand, 2],
			'group': name
		})
		tests.append({
			'name': f'{name}[{i}](Sub)',
			'cb': test_move,
			'args': [move_index, rand, 5],
			'group': name
		})
for move_index in binding_moves:
	for i, rand in enumerate(rand_lists + extra_lists.get(move_index, [])):
		name = AvailableMove(move_index).name
		tests.append({
			'name': f'{name}&Switch[{i}](Atk)',
			'cb': test_bind_switch,
			'args': [int(move_index), rand, 0],
			'group': name
		})
		tests.append({
			'name': f'{name}&Switch[{i}](Sub)',
			'cb': test_bind_switch,
			'args': [int(move_index), rand, 1],
			'group': name
		})
		tests.append({
			'name': f'{name}&Switch[{i}](SAtk)',
			'cb': test_bind_switch_inverted,
			'args': [int(move_index), rand, 2],
			'group': name
		})
		tests.append({
			'name': f'{name}&Switch[{i}](SSub)',
			'cb': test_bind_switch_inverted,
			'args': [int(move_index), rand, 3],
			'group': name
		})
		tests.append({
			'name': f'{name}&CFZ[{i}]',
			'cb': test_trap_move_turn_skip,
			'args': [int(move_index), rand, 1],
			'group': name
		})
		tests.append({
			'name': f'{name}&PAR[{i}]',
			'cb': test_trap_move_turn_skip,
			'args': [int(move_index), rand, 0],
			'group': name
		})
for move_index in status_moves:
	for i, rand in enumerate(rand_lists + extra_lists.get(move_index, [])):
		name = move_index.name
		tests.append({
			'name': f'Hyper_Beam&{name}[{i}](Par->Move)',
			'cb': hyper_beam_status_move,
			'args': [int(move_index), rand, 1],
			'group': 'Hyper_Beam'
		})
		tests.append({
			'name': f'Hyper_Beam&{name}[{i}](Move->Par)',
			'cb': hyper_beam_status_move,
			'args': [int(move_index), rand, 0],
			'group': 'Hyper_Beam'
		})
for move_index in range(1, AvailableMove.Struggle + 1):
	for i, rand in enumerate(rand_lists + extra_lists.get(move_index, [])):
		tests.append({
			'name': f'Mist&{AvailableMove(move_index).name}[{i}]',
			'cb': test_mist,
			'args': [move_index, rand],
			'group': 'Mist'
		})

results = []

parser = ArgumentParser(prog=sys.argv[0])
parser.add_argument('-d', '--debug', action='store_true')
parser.add_argument('-v', '--display-emulator', action='store_true')
parser.add_argument('-e', '--emu-debug', action='store_true')
parser.add_argument('-i', '--show-individual', action='store_true')
parser.add_argument('-f', '--show-failure', action='store_true')
parser.add_argument('-t', '--test', nargs='*')
parser.add_argument('-j', '--jobs', default=1, type=int)
parser.add_argument('-o', '--output')
parser.add_argument('-r', '--rom')
args = parser.parse_args()


debug = args.debug
jobs = 1 if debug else int(args.jobs)

tests_ran = 0
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

def run_tests(offset: int, count: int):
	global tests_ran
	emulator = PyBoyEmulator(has_interface=args.display_emulator and offset == 0, sound_volume=25 if debug and offset == 0 else 0, save_frames=False, debug=args.emu_debug, rom=args.rom)
	for test_object in tests_to_run[offset::count]:
		errors, extra = run_test(test_object, emulator)
		tests_ran += 1
		results.append({
			'name': test_object['name'],
			'errors': errors,
			'group': test_object['group'],
			'extra': extra
		})

if jobs == 1:
	run_tests(0, 1)
else:
	threads = []
	for i in range(jobs):
		thread = threading.Thread(target=run_tests, args=[i, jobs])
		thread.start()
		threads.append(thread)
	b = True
	while b:
		b = False
		for thread in threads:
			b = b or thread.is_alive()
		print(f"{tests_ran}/{len(tests_to_run)}\n", end="\033[A")
		time.sleep(0.1)


successes = 0
failures = 0
groups = {}
print()
fd = None if args.output is None else open(args.output, "w")
for r in sorted(results, key=lambda x: len(x['errors']) != 0):
	if r['group'] not in groups:
		groups[r['group']] = [0, 0]
	groups[r['group']][1] += 1
	if not r['errors']:
		successes += 1
		groups[r['group']][0] += 1
		if args.show_individual:
			print(f"{r['name']}: \033[32mPassed\033[0m")
			if fd is not None:
				fd.write(f"{r['name']}: Passed\n")
		continue
	if args.show_individual or args.show_failure:
		print(f"{r['name']} {r['extra']}: \033[31mFailed\033[0m")
		print("\n".join(f"\033[31;1m - {f}\033[0m" for f in r['errors']))
		if fd is not None:
			fd.write(f"{r['name']} {r['extra']}: Failed\n")
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
print(f'{successes} individual test(s) \033[32mpassed\033[0m. {failures} individual test(s) \033[31mfailed\033[0m.')
print(f'{group_succeed} test group(s) \033[32mpassed\033[0m. {group_failed} test group(s) \033[31mfailed\033[0m.')
if fd is not None:
	fd.write(f'{successes} individual test(s) passed. {failures} individual test(s) failed.\n')
	fd.write(f'{group_succeed} test group(s) passed. {group_failed} test group(s) failed.\n')
	fd.close()
exit(0 if failures == 0 else 1)
