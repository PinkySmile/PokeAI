import os
import json
import argparse
import random

import numpy as np
import onnxruntime
import gymnasium as gym
import torch

import PokeBattle.Gen1.Env
from pathlib import Path
from PokeBattle.Gen1.Env import Examples, basic_opponent, load_scenario

parser = argparse.ArgumentParser()
parser.add_argument('--seed', type=int, default=42, help='Seed used for the training')
parser.add_argument('--scenario', type=str, default="simple", help="Pokemon battle scenario")
parser.add_argument("model")
args = parser.parse_args()

# Fixing the seed for reproducibility
random.seed(args.seed)
np.random.seed(args.seed)
torch.manual_seed(args.seed)
torch.backends.cudnn.deterministic = True
# session = onnxruntime.InferenceSession(args.model, providers=['CUDAExecutionProvider', 'CPUExecutionProvider'])

sess_options = onnxruntime.SessionOptions()
sess_options.add_session_config_entry('session.random_seed', str(args.seed))
sess_options.intra_op_num_threads = 1
sess_options.inter_op_num_threads = 1
sess_options.graph_optimization_level = onnxruntime.GraphOptimizationLevel.ORT_DISABLE_ALL

session = onnxruntime.InferenceSession(args.model, sess_options=sess_options, providers=['CPUExecutionProvider'])

if os.path.exists(f"ai/scenarios/{args.scenario}.json"):
	with open(f"ai/scenarios/{args.scenario}.json") as fd:
		j = json.load(fd)
	ai = getattr(PokeBattle.Gen1.Env, j["ai"]) if "ai" in j else basic_opponent
	start_options = load_scenario(f"ai/scenarios/{j["scenario"]}", ai)
else:
	try:
		start_options = getattr(PokeBattle.Gen1.Env.Examples, args.scenario)
	except AttributeError:
		print(f"Cannot find scenario {args.scenario}")
		print("Valid scenarios are:")
		for elem in filter(lambda k: not k.startswith('__'), Examples.__dict__.keys()):
			print(f" - {elem}")
		for elem in os.listdir("ai/scenarios"):
			p = Path(elem)
			if p.suffix == ".json":
				print(f" - {p.stem}")
		exit(1)

env = gym.make(
	'PokemonYellow',
	args.seed,
	render_mode='human',
	opponent_callback=basic_opponent,
	shuffle_teams=True,
	use_emulator=True
)
next_obs, next_obs_info = env.reset(seed=args.seed, options=start_options)
next_mask = next_obs_info.get('mask')

# print(next_obs.sum())
# print(next_mask)
terminated = False
prev_obs = None
while not terminated:
	obs_array = np.array([next_obs], dtype=np.float32)
	mask_array = np.array([next_mask], dtype=bool)

	outputs = session.run(["action"], {
		'observation': [next_obs],
		'action_mask': [next_mask]
	})
	# print(f"action={outputs[0][0]}, mask={next_mask}, obs_changed={not np.array_equal(next_obs, prev_obs)}")
	# print(next_obs.sum())
	prev_obs = next_obs.copy()
	next_obs, reward, terminated, truncated, infos = env.step(outputs[0][0])
	next_mask = infos.get('mask')
