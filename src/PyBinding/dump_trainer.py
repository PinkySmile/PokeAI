from GameEngine import loadTrainer, BattleState
import sys

s = BattleState()
with open(sys.argv[1], "rb") as fd:
	data = fd.read()
trainer = loadTrainer(data, s)
print(f"Trainer loaded: '{trainer[0]}'")
for pkmn in trainer[1]:
	print(pkmn.dump())