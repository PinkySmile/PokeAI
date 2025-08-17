from GameEngine import PokemonSpecies, AvailableMove
from Env import PokemonYellowBattle
import gymnasium as gym

p = gym.make('PokemonYellow', render_mode="human")
print(p.reset(options={
	"p1name": "test",
	"p2name": "test2",
	"p1team": [{
		"species": PokemonSpecies.Charmander,
		"name": "CHARMANDER",
		"level": 5,
		"moves": [AvailableMove.Tackle]
	}],
	"p2team": [{
		"species": PokemonSpecies.Squirtle,
		"name": "SQUIRTLE",
		"level": 5,
		"moves": [AvailableMove.Tackle]
	}]
}))
print(p.step(0))
print(p.step(0))
print(p.render())
print(p.reset(options={
	"p1name": "Simon",
	"p2name": "Andgel",
	"p1team": [{
		"species": PokemonSpecies.Charizard,
		"name": "CHARIZARD",
		"level": 62,
		"moves": [AvailableMove.Fire_Blast, AvailableMove.Wing_Attack]
	}],
	"p2team": [{
		"species": PokemonSpecies.Clefable,
		"name": "CLEFABLE",
		"level": 57,
		"moves": [AvailableMove.Thunder, AvailableMove.Ice_Beam]
	}]
}))
print(p.step(0))
print(p.step(0))
print(p.render())