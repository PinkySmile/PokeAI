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
# --- Fixing the seed for reproducibility ---
random.seed(args.seed)
np.random.seed(args.seed)
torch.manual_seed(args.seed)
torch.backends.cudnn.deterministic = True
session = onnxruntime.InferenceSession(args.model, providers=['CUDAExecutionProvider', 'CPUExecutionProvider'])
env = gym.make(
	'PokemonYellow',
	args.seed,
	render_mode='human',
	opponent_callback=basic_opponent,
	shuffle_teams=True,
	use_emulator=True
)
next_obs, next_obs_info = env.reset(seed=args.seed, options=start_options)
mask = next_obs_info.get('mask')
terminated = False
while not terminated:
	outputs = session.run(["action"], {
		'observation': [next_obs],
		'action_mask': [mask]
	})
	next_obs, reward, terminated, truncated, infos = env.step(outputs[0][0])
	next_mask = infos.get('mask')
