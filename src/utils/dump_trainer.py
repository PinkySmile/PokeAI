from PokeBattle.Gen1 import load_trainer, BattleState
import sys

s = BattleState()
with open(sys.argv[1], "rb") as fd:
	data = fd.read()
trainer = load_trainer(data, s)
print(f"Trainer loaded: '{trainer[0]}'")
for pkmn in trainer[1]:
	print(pkmn.dump())
