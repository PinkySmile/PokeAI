from GameEngine import PokemonSpecies, AvailableMove
from Env import PokemonYellowBattle, Examples
import gymnasium as gym

p = gym.make('PokemonYellow', render_mode="ansi")
finished = False
observation, info = p.reset(options=Examples.Brock)

def check_obs(obs):
	low = p.observation_space.low
	high = p.observation_space.high
	for i in range(len(low)):
		if low[i] <= obs[i] <= high[i]:
			continue
		print(f"#{i} not in range: {low[i]} <= {obs[i]} <= {high[i]}")

print(observation, info, len(observation))
check_obs(observation)
while not finished:
	observation, reward, finished, truncated, info = p.step(int(input()))
	r = p.render()
	if r:
		print(r)
	print(observation, info, len(observation))
	check_obs(observation)