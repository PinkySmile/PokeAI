from GameEngine import RandomGenerator, PokemonBase, Pokemon, PokemonSpecies, Move, AvailableMove, Type, BattleHandler, BattleAction

battle = BattleHandler(False, False)
battle.state.rng.makeRandomList(9)
pika_base = PokemonBase(PokemonSpecies.Pikachu)
moveList = [
	Move(AvailableMove.Pound),
	Move(AvailableMove.Thunderbolt),
	Move(AvailableMove.Transform)
]

battle.state.me.team = [Pokemon(battle.state.rng, battle.state, "Pika", 100, pika_base, moveList, False)]
battle.state.me.name = "Gamer"
battle.state.op.team = [Pokemon(battle.state.rng, battle.state, "Pika", 100, pika_base, moveList, True)]
battle.state.op.name = "Not gamer"
battle.state.battleLogger = lambda msg: print(f'[BATTLE]: {msg}')
battle.state.onTurnStart = lambda: print("Turn start!")
battle.state.onBattleEnd = lambda: print("Battle end!")
battle.state.onBattleStart = lambda: print("Battle start!")

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