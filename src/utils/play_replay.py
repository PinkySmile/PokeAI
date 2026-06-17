import sys
import os.path
from numpy import int8, array
from PokeBattle.Gen1.YellowEmulator import TrainerClass
from PokeBattle.Gen1.PyBoyEmulator import PyBoyEmulator
from PokeBattle.Gen1.BattleHandler import BattleHandler
from PokeBattle.Gen1.State import BattleState
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
if trainer.isdigit(): trainer = int(trainer)
elif hasattr(TrainerClass, trainer): trainer = getattr(TrainerClass, trainer)
elif trainer == "BLUE": trainer = TrainerClass.RIVAL1
elif trainer == "CHAMPION": trainer = TrainerClass.RIVAL3
elif trainer == "RED": trainer = TrainerClass.NOBODY
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


battle.start()
emulator.init_battle(None, state, fast_forward=args.fast or to_turn > 0, trainer=trainer)


def get_mask(s: BattleState):
	pkmn = s.me.pokemon_on_field
	can_use_struggle = False
	can_no_action = False
	move_mask = [False] * 4
	switch_mask = [int(len(s.me.team) > i and s.me.pokemon_on_field_index != i and s.me.team[i].health > 0) for i in range(6)]
	if pkmn.health == 0:
		pass
	elif s.op.pokemon_on_field.health == 0:
		switch_mask = [False] * 6
		can_no_action = True
	elif pkmn.wrapped:
		can_no_action = True
	else:
		assert len(pkmn.move_set) == 4
		move_mask = [int(m.id != 0 and m.pp != 0 and pkmn.move_disabled != i) for i, m in enumerate(pkmn.move_set)]
		can_use_struggle = int(not any(move_mask))
	result = move_mask + switch_mask + [can_no_action, can_use_struggle]
	if not any(result):
		result[10] = True
	return result

def print_mask(mask):
	print("Attack1",      bool(mask[0]))
	print("Attack2",      bool(mask[1]))
	print("Attack3",      bool(mask[2]))
	print("Attack4",      bool(mask[3]))
	print("Switch1",      bool(mask[4]))
	print("Switch2",      bool(mask[5]))
	print("Switch3",      bool(mask[6]))
	print("Switch4",      bool(mask[7]))
	print("Switch5",      bool(mask[8]))
	print("Switch6",      bool(mask[9]))
	print("NoAction",     bool(mask[10]))
	print("StruggleMove", bool(mask[11]))


turn = 0
while not battle.finished:
	print(f" ---------- TURN {turn + 1:<3} ----------")
	with open(state_folder + f"/turn{turn:03d}.state", "wb") as fd:
		emulator.emulator.save_state(fd)
	battle.save_state(state_folder + f"/turn{turn:03d}.json")
	print_mask(array(get_mask(battle.state), dtype=int8))
	battle.tick()
	emulator.step(state, fast_forward=args.fast or turn < to_turn)
	emulator_state = emulator.get_emulator_basic_state()
	if not args.fast:
		print(" ---- EMULATOR ---- ")
		print(emulator.dump_basic_state(emulator_state[0]))
		print(emulator.dump_basic_state(emulator_state[1]))
		print(" ---- SIMULATOR ---- ")
		print(state.me.name)
		for i, pkmn in enumerate(state.me.team):
			if i == state.me.pokemon_on_field_index:
				print(pkmn.dump(), "(Active)")
			else:
				print(pkmn.dump())
		print(state.op.name)
		for i, pkmn in enumerate(state.op.team):
			if i == state.op.pokemon_on_field_index:
				print(pkmn.dump(), "(Active)")
			else:
				print(pkmn.dump())
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