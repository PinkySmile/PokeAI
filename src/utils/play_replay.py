import sys
import os.path
from PokeBattle.Gen1.YellowEmulator import TrainerClass
from PokeBattle.Gen1.PyBoyEmulator import PyBoyEmulator
from PokeBattle.Gen1.BattleHandler import BattleHandler
from argparse import ArgumentParser


parser = ArgumentParser(prog=sys.argv[0])
parser.add_argument('-t', '--to-turn', default=0)
parser.add_argument('-v', '--volume', default=25)
parser.add_argument('-f', '--fast', action='store_true')
parser.add_argument('-e', '--emu-debug', action='store_true')
parser.add_argument('-r', '--rom')
parser.add_argument('--trainer')
parser.add_argument('replay_file')
args = parser.parse_args()
emulator = PyBoyEmulator(has_interface=not args.fast, sound_volume=int(args.volume), save_frames=False, debug=args.emu_debug, rom=args.rom)

trainer = args.trainer
if trainer is None: trainer = ""
trainer = trainer.upper()
trainer = trainer.replace(".", "")
trainer = trainer.replace(" ", "_")
trainer = trainer.replace("&", "_")
trainer = trainer.replace("~", "_M")
trainer = trainer.replace("`", "_F")
if hasattr(TrainerClass, trainer): trainer = getattr(TrainerClass, trainer)
elif trainer == "blue": trainer = TrainerClass.RIVAL1
elif trainer == "champion": trainer = TrainerClass.RIVAL3
elif trainer == "red": trainer = TrainerClass.NOBODY
elif trainer != "":
	print("Invalid trainer class. Valid trainers are:")
	for f in dir(TrainerClass):
		if f.upper() != f:
			continue
		print(f' - {" ".join(map(lambda g: g.capitalize(), f.replace("_M", '~').replace("_F", '`').split('_')))}')
	exit(1)
else: trainer = None

battle = BattleHandler(False, False)
state = battle.state
to_turn = int(args.to_turn)
battle.load_replay(args.replay_file)
state_folder = os.path.basename(args.replay_file) + "_states"
os.system(f"rm -rf {state_folder}")
os.mkdir(state_folder)
print(f"{state.me.name}'s team (P1):")
print("\n".join(pkmn.dump() for pkmn in state.me.team))
print()
print(f"{state.op.name}'s team (P2):")
print("\n".join(pkmn.dump() for pkmn in state.op.team))
print()
if not args.fast:
	state.logger = print


#with open("pokeyellow_replay.state", "rb") as fd:
battle.start()
emulator.init_battle(None, state, fast_forward=args.fast or to_turn > 0, trainer=trainer)

turn = 0
while not battle.finished:
	print(f" ---------- TURN {turn + 1:<3} ----------")
	with open(state_folder + f"/turn{turn:03d}.state", "wb") as fd:
		emulator.emulator.save_state(fd)
	battle.save_state(state_folder + f"/turn{turn:03d}.json")
	battle.tick()
	emulator.step(state, fast_forward=args.fast or turn < to_turn)
	emulator_state = emulator.get_emulator_basic_state()
	if not args.fast:
		print(emulator.dump_basic_state(emulator_state[0]))
		print(emulator.dump_basic_state(emulator_state[1]))
		print(state.me.pokemon_on_field.dump())
		print(state.op.pokemon_on_field.dump())
		print(state.rng.index, emulator_state[2], list(map(lambda x: f'{x:02X}', state.rng.list)), list(map(lambda x: f'{x:02X}', emulator_state[3])))
	f = emulator.compare_basic_states(battle.state, emulator_state)
	if f[1]:
		to_turn = 0
		print("Desync detected!")
		print("\n".join(f[1]))
		with open(state_folder + f"/turn{turn:03d}.state", "rb") as fd:
			emulator.emulator.load_state(fd)
		battle.load_state(state_folder + f"/turn{turn:03d}.json")
		while not args.fast and emulator.emulator.tick():
			pass
	turn += 1