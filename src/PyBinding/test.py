from GameEngine import RandomGenerator, PokemonBase, Pokemon, PokemonSpecies, Move, AvailableMove, Type, BattleHandler, BattleAction
from argparse import ArgumentParser
import sys

parser = ArgumentParser(prog=sys.argv[0])
parser.add_argument('-f', '--team-files')
parser.add_argument('-t', '--team-size')
parser.add_argument('-m', '--move-list-size')
parser.add_argument('-p', '--print-battle-logs', action='store_true')
parser.add_argument('-n', '--number-of-runs')
parser.add_argument('-d', '--display-state')
args = parser.parse_args()

battle = BattleHandler(False, False)
battle.state.rng.makeRandomList(9)
pika_base = PokemonBase(PokemonSpecies.Pikachu)
moveList = [
	Move(AvailableMove.Pound),
	Move(AvailableMove.Thunderbolt),
	Move(AvailableMove.Transform)
]

battle.state.me.name = "Gamer"
battle.state.me.team = [Pokemon(battle.state.rng, battle.state, "Pika", 100, pika_base, moveList, False)]
battle.state.op.name = "Not gamer"
battle.state.op.team = [Pokemon(battle.state.rng, battle.state, "Pika", 100, pika_base, moveList, True)]
#battle.state.battleLogger = print

def step():
	print(" ---------------------- ")
	print(f"{battle.state.me.name}'s (P1) team:")
	print(f"{"\n".join(pkmn.dump() for pkmn in battle.state.me.team)}")
	print(f"{battle.state.op.name}'s (P2) team:")
	print(f"{"\n".join(pkmn.dump() for pkmn in battle.state.op.team)}")
	print(" ---------------------- ")
	battle.state.me.nextAction = BattleAction.Attack2
	battle.state.op.nextAction = BattleAction.Attack2

step()
while not battle.tick():
	step()
step()
battle.saveReplay("test.replay")