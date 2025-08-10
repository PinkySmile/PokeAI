from GameEngine import loadTrainer, RandomGenerator
import sys

r = RandomGenerator()
with open(sys.argv[1], "rb") as fd:
	data = fd.read()
trainer = loadTrainer(data, r, print)
print(f"Trainer loaded: '{trainer[0]}'")
for pkmn in trainer[1]:
	print(pkmn.dump())