from GameEngine import PokemonSpecies, AvailableMove
from Env import PokemonYellowBattle, Examples
import gymnasium as gym


def check_obs(obs):
	low = p.observation_space.low
	high = p.observation_space.high
	for i in range(len(low)):
		if low[i] <= obs[i] <= high[i]:
			continue
		print(f"#{i} not in range: {low[i]} <= {obs[i]} <= {high[i]}")


p = gym.make('PokemonYellow', render_mode="human", shuffle_teams=True)
finished = False
params = Examples.Brock
# params = {
# 	"p1name": "PokeAI",
# 	"p2name": "Opponent",
# 	"p1team": [{
# 		"species": PokemonSpecies.Charmander,
# 		"name": "CHARMANDER",
# 		"level": 10,
# 		"moves": [AvailableMove.Bone_Club, AvailableMove.Water_Gun, AvailableMove.Thundershock]
# 	}],
# 	"p2team": [{
# 		"species": PokemonSpecies.Articuno,
# 		"name": "ARTICUNO",
# 		"level": 5,
# 		"moves": [AvailableMove.Tackle, AvailableMove.Tail_Whip]
# 	},
# 		{
# 			"species": PokemonSpecies.Diglett,
# 			"name": "DIGLETT",
# 			"level": 10,
# 			"moves": [AvailableMove.Tackle, AvailableMove.Tail_Whip]
# 		},
# 		{
# 			"species": PokemonSpecies.Charmander,
# 			"name": "CHARMANDER",
# 			"level": 10,
# 			"moves": [AvailableMove.Tackle, AvailableMove.Tail_Whip]
# 		}
# 	]
# }
observation, info = p.reset(options=params)

print(observation, info, len(observation))
check_obs(observation)
while not finished:
	observation, reward, finished, truncated, info = p.step(int(input()))
	r = p.render()
	if r:
		print(r)
	print(observation, info, len(observation))
	check_obs(observation)
