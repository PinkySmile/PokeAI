import json
import OpenRouterAPI
from PokeBattle.Gen1.Team import load_trainer
from PokeBattle.Gen1.BattleHandler import BattleHandler
from PokeBattle.Gen1.Env import gen1AI
import numpy
from argparse import ArgumentParser


parser = ArgumentParser()
parser.add_argument('-m', '--model', default="openrouter/sonoma-dusk-alpha")
parser.add_argument('--manual', action='store_true')
parser.add_argument('-s', '--swap-side', action='store_true')
parser.add_argument('-d', '--debug', action='store_true')
parser.add_argument('scenario')
parser.add_argument('replay')
parser.add_argument('log')
args = parser.parse_args()

TRAINER_DATA_SIZE = 11 * 7 + 44 * 6 + 1
battle = BattleHandler(args.swap_side, args.debug)
state = battle.state
state.rng.generate_list(9)
data = open(args.scenario, "rb").read()
name1, team1 = load_trainer(data[:TRAINER_DATA_SIZE], state)
name2, team2 = load_trainer(data[TRAINER_DATA_SIZE:], state)
state.me.name = name1
state.me.team = team1
state.op.name = name2
state.op.team = team2
def f(g):
	print(g)
	messages.append(g)
messages = []
state.logger = f
battle.start()
while not battle.finished:
	if args.swap_side:
		if not args.manual:
			state.op.next_action = OpenRouterAPI.get_move(state, args.model, messages)
		else:
			state.op.next_action = OpenRouterAPI.get_move_manual(state, messages)
		state.me.next_action = gen1AI(state.op, state.me, [1], numpy.random.default_rng())
	else:
		if not args.manual:
			state.me.next_action = OpenRouterAPI.get_move(state, args.model, messages)
		else:
			state.me.next_action = OpenRouterAPI.get_move_manual(state, messages)
		state.op.next_action = gen1AI(state.op, state.me, [1], numpy.random.default_rng())
	messages.clear()
	battle.tick()
battle.save_replay(args.replay)
json.dump(OpenRouterAPI.msgs, open(args.log, "w"), indent=4)