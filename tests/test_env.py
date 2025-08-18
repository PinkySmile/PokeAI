from GameEngine import PokemonSpecies, AvailableMove
from Env import PokemonYellowBattle, Examples
import gymnasium as gym

p = gym.make('PokemonYellow', render_mode="human")
finished = False
print(p.reset(options=Examples.Brock))
while not finished:
	observation, reward, finished, truncated, info = p.step(int(input()))
	r = p.render()
	if r:
		print(r)