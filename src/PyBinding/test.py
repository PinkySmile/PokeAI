from GameEngine import RandomGenerator, PokemonBase, Pokemon, PokemonSpecies, Move, AvailableMove, Type, BattleHandler, BattleAction, StatusChange, MoveCategory, getAttackDamageMultiplier
from argparse import ArgumentParser
import random
import sys

parser = ArgumentParser(prog=sys.argv[0])
parser.add_argument('-f', '--team-files')
parser.add_argument('-t', '--team-size')
parser.add_argument('-m', '--move-list-size')
parser.add_argument('-p', '--print-battle-logs', action='store_true')
parser.add_argument('-n', '--number-of-runs')
parser.add_argument('-d', '--display-state')
args = parser.parse_args()


def gen1AI(me, op, categories):
	mepkmn = me.team[me.pokemonOnField]
	if mepkmn.getHealth() == 0:
		return BattleAction(BattleAction.Switch1 + next(i for i in range(len(me.team)) if me.team[i].getHealth()))
	moves = mepkmn.getMoveSet()
	scores = [1000 for _ in moves]
	oppkmn = op.team[op.pokemonOnField]
	if 1 in categories and oppkmn.hasStatus(StatusChange.Any_non_volatile_status):
		for i in range(len(scores)):
			if (moves[i].getStatusChange()['status'] & StatusChange.Any_non_volatile_status) and moves[i].getCategory() == MoveCategory.Status:
				scores[i] /= 8
	if 2 in categories and me.lastAction in [BattleAction.Switch1, BattleAction.Switch2, BattleAction.Switch3, BattleAction.Switch4, BattleAction.Switch5, BattleAction.Switch6]:
		priority = [int(i) for i in [
			AvailableMove.Meditate, AvailableMove.Sharpen,
			AvailableMove.Defense_Curl, AvailableMove.Harden, AvailableMove.Withdraw,
			AvailableMove.Growth,
			AvailableMove.Double_Team, AvailableMove.Minimize,
			AvailableMove.Pay_Day,
			AvailableMove.Swift,
			AvailableMove.Growl,
			AvailableMove.Leer, AvailableMove.Tail_Whip,
			AvailableMove.String_Shot,
			AvailableMove.Flash, AvailableMove.Kinesis, AvailableMove.Sand_Attack, AvailableMove.SmokeScreen,
			AvailableMove.Conversion,
			AvailableMove.Haze,
			AvailableMove.Swords_Dance,
			AvailableMove.Acid_Armor, AvailableMove.Barrier,
			AvailableMove.Agility,
			AvailableMove.Amnesia,
			AvailableMove.Recover, AvailableMove.Rest, AvailableMove.Softboiled,
			AvailableMove.Transform,
			AvailableMove.Screech,
			AvailableMove.Light_Screen,
			AvailableMove.Reflect,
		]]
		for i in range(len(scores)):
			if moves[i].getID() in priority:
				scores[i] *= 2
	if 3 in categories:
		for i in range(len(scores)):
			if moves[i].getCategory() != MoveCategory.Status:
				mul = getAttackDamageMultiplier(moves[i].getType(), oppkmn.getTypes())
				scores[i] *= mul
		priority = [int(i) for i in [
			AvailableMove.Super_Fang,
			AvailableMove.Dragon_Rage, AvailableMove.Psywave, AvailableMove.Night_Shade, AvailableMove.Seismic_Toss,
			AvailableMove.Fly
		]]
		for k, p in enumerate(priority):
			for i in range(len(scores)):
				if moves[i].getID() == p:
					scores[i] *= 2 + k  / 10
	if 4 in categories:
		if mepkmn.getHealth() / mepkmn.getMaxHealth() < 0.1 and random.randint(0, 63) < 5:
			for i in range(6):
				index = (me.pokemonOnField + i) % 6
				if len(me.team) <= index:
					continue
				if me.team[index].getHealth() == 0:
					continue
				return BattleAction(BattleAction.Switch1 + index)
	for i in range(0, len(scores)):
		if moves[i].getID() == 0:
			scores[i] = -1000
	best = [0]
	for i in range(1, len(scores)):
		if scores[best[0]] > scores[i]:
			best = [i]
		elif scores[best[0]] == scores[i]:
			best.append(i)
	return BattleAction(BattleAction.Attack1 + random.choice(best))

def level0(me, _):
	if me.team[me.pokemonOnField].getHealth() == 0:
		return BattleAction(BattleAction.Switch1 + next(i for i in range(len(me.team)) if me.team[i].getHealth()))
	moves = me.team[me.pokemonOnField].getMoveSet()
	move = 0
	for i in range(1, len(moves)):
		if int(moves[i].getID()) == 0:
			continue
		if moves[move].getPower() > moves[i].getPower():
			move = i
		elif moves[move].getPower() < moves[i].getPower():
			continue
		elif moves[move].getStatusChange()['status'] and not moves[move].getStatusChange()['status']:
			move = i
	return BattleAction(BattleAction.Attack1 + move)

def level1(me, _):
	if me.team[me.pokemonOnField].getHealth() == 0:
		return BattleAction(BattleAction.Switch1 + next(i for i in range(len(me.team)) if me.team[i].getHealth()))
	moves = me.team[me.pokemonOnField].getMoveSet()
	valid_moves = [i for i in range(len(moves)) if moves[i].getID() != 0]
	return BattleAction(BattleAction.Attack1 + random.choice(valid_moves))

def level2(me, op):
	return gen1AI(me, op, [])

battle = BattleHandler(False, False)
battle.state.rng.makeRandomList(9)
clef_base = PokemonBase(PokemonSpecies.Clefable)
clef_moveList = [
	Move(AvailableMove.Pound),
	Move(AvailableMove.Thunderbolt),
	Move(AvailableMove.Fire_Blast)
]
cloy_base = PokemonBase(PokemonSpecies.Cloyster)
cloy_moveList = [
	Move(AvailableMove.Explosion),
	Move(AvailableMove.Tackle),
	Move(AvailableMove.Hydro_Pump)
]

battle.state.me.name = "Gamer"
battle.state.me.team = [Pokemon(battle.state.rng, battle.state, "", 100, clef_base, clef_moveList, False)]
battle.state.op.name = "Not gamer"
battle.state.op.team = [Pokemon(battle.state.rng, battle.state, "", 100, cloy_base, cloy_moveList, True)]
battle.state.battleLogger = print

def step():
	print(" ---------------------- ")
	print(f"{battle.state.me.name}'s (P1) team:")
	print(f"{"\n".join(pkmn.dump() for pkmn in battle.state.me.team)}")
	print(f"{battle.state.op.name}'s (P2) team:")
	print(f"{"\n".join(pkmn.dump() for pkmn in battle.state.op.team)}")
	print(" ---------------------- ")
	if not battle.isFinished():
		while True:
			try:
				battle.state.me.nextAction = getattr(BattleAction, input("Enter action: "))
				break
			except AttributeError:
				pass
		battle.state.op.nextAction = level2(battle.state.op, battle.state.me)

step()
while not battle.tick():
	step()
step()
battle.saveReplay("test.replay")