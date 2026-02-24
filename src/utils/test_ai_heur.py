import random
import time
from HeuristicAI import get_move as ai_heur
from numpy.random import default_rng
from PokeBattle.Gen1.Pokemon import Pokemon, PokemonBase
from PokeBattle.Gen1.Move import Move
from PokeBattle.Gen1.BattleHandler import BattleHandler
from PokeBattle.Gen1.PyBoyEmulator import PyBoyEmulator
from PokeBattle.Gen1.Env import scenario_folder, gen1AI_13, TRAINER_DATA_SIZE
from PokeBattle.Gen1.Team import load_trainer


rng = default_rng(int(time.time()))
emulator = PyBoyEmulator(rom='/home/pinky/pokeyellow-gen-II/pokeyellow.gbc')
battle = BattleHandler(False, False)
state = battle.state
state.rng.generate_list(9)
state.logger = print
fd = open(scenario_folder + "/Koga.scenario", "rb")
data = fd.read()
state.me.name, state.me.team = load_trainer(data[:TRAINER_DATA_SIZE], state)
state.op.name, state.op.team = load_trainer(data[TRAINER_DATA_SIZE:], state)

state.me.team = rng.permutation([Pokemon(
	state,
	p.get_name(False),
	p.level,
	PokemonBase(p.id),
	rng.permutation(list(Move(m.id, 3) for m in p.move_set if m.id))
) for p in state.me.team])
state.op.team = rng.permutation([Pokemon(
	state,
	p.get_name(False),
	p.level,
	PokemonBase(p.id),
	rng.permutation(list(Move(m.id, 3) for m in p.move_set if m.id))
) for p in state.op.team])

emulator.init_battle(None, state)
current_turn = 0
print(' --------- TURN 0 ---------')
battle.start()
while not battle.finished:
	current_turn += 1
	print(f' --------- TURN {current_turn} ---------')
	state.me.next_action = ai_heur(state.me, state.op, rng)
	state.op.next_action = gen1AI_13(state, rng)
	battle.tick()
	emulator.step(state)
