from PokeBattle.Gen1.State import BattleAction

import OpenRouterAPI
from PokeBattle.Gen1.Team import load_trainer
from PokeBattle.Gen1.BattleHandler import BattleHandler
from PokeBattle.Gen1.Env import gen1AI
import numpy


battle = BattleHandler(False, False)
state = battle.state
state.rng.generate_list(9)
name1, team1 = load_trainer(open("../../cmake-build-debug/Brock_p1.pkmns", "rb").read(), state)
name2, team2 = load_trainer(open("../../cmake-build-debug/Brock_op.pkmns", "rb").read(), state)
state.me.name = name1
state.me.team = team1
state.op.name = name2
state.op.team = team2
def f(g):
	print(g)
	messages.append(g)
messages = []
state.logger = f
while not battle.finished:
	state.me.next_action = OpenRouterAPI.get_move(state, messages)
	state.op.next_action = gen1AI(state.op, state.me, [1], numpy.random.default_rng())
	battle.tick()
battle.save_state("replay.replay")