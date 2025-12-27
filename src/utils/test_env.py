import traceback

from PokeBattle.Gen1.Env import Examples
import gymnasium as gym
import asyncio
import sys
from PokeBattle.Gen1.State import PlayerState, BattleAction


def get_move(me: PlayerState) -> BattleAction:
	my_moves = me.pokemon_on_field.move_set
	if me.pokemon_on_field.health == 0:
		return BattleAction(BattleAction.Switch1 + next(i for i in range(len(me.team)) if me.team[i].health != 0 and me.pokemon_on_field_index != i))
	if me.pokemon_on_field.wrapped:
		if sum(p.health != 0 for p in me.team) == 1:
			return BattleAction.NoAction
		return BattleAction(BattleAction.Switch1 + next(i for i in range(len(me.team)) if me.team[i].health != 0 and me.pokemon_on_field_index != i))
	if all(m.pp == 0 and i != me.pokemon_on_field.move_disabled for i, m in enumerate(my_moves)):
		return BattleAction.StruggleMove
	return BattleAction.Attack1 + next(i for i, m in enumerate(my_moves) if m.pp != 0)


async def ainput(string: str) -> str:
	sys.stdout.write(f'{string} ')
	sys.stdout.flush()
	return (await asyncio.to_thread(sys.stdin.readline)).rstrip('\n')


def check_obs(p, obs):
	low = p.observation_space.low
	high = p.observation_space.high
	for i in range(len(low)):
		if low[i] <= obs[i] <= high[i]:
			continue
		print(f"#{i} not in range: {low[i]} <= {obs[i]} <= {high[i]}")

auto = len(sys.argv) > 1

async def main():
	p = gym.make('PokemonYellow', render_mode="rgb_array_list", shuffle_teams=True, rom="/home/pinky/pokeyellow-gen-II/pokeyellow.gbc", leak_state=True, skip_frames=300, use_emulator=False)
	finished = False
	params = Examples.Blue3_1
	observation, info = p.reset(options=params)

	print(observation, info, len(observation))
	check_obs(p, observation)
	r = p.render()
	if isinstance(r, str):
		print(r)
	while not finished:
		didit = False
		while not didit:
			if not auto:
				future = asyncio.ensure_future(ainput("Action index:"))
				while not future.done():
					if info['emulator'] is not None:
						info['emulator'].tick()
					await asyncio.sleep(0)
			else:
				action = get_move(info['simulator'].state.me)
				if action >= BattleAction.Attack1 + 10:
					action = action - BattleAction.NoAction + 10
				else:
					action = action - BattleAction.Attack1
			while not didit:
				try:
					if not auto:
						action = future.result()
						if action == "stop":
							info['simulator'].save_replay("interrupted.replay")
							exit(1)
						action = int(future.result())
					if not info['mask'][action]:
						print("Warning: using disabled action")
					observation, reward, finished, truncated, info = p.step(action)
					didit = True
				except InterruptedError:
					raise
				except SystemExit:
					raise
				except KeyboardInterrupt:
					info['simulator'].save_replay("interrupted.replay")
					raise
				except:
					traceback.print_exc()
					break
			info['simulator'].save_replay("interrupted.replay")

		r = p.render()
		if isinstance(r, str):
			print(r)
		print(observation, info, len(observation), reward, finished, truncated)
		check_obs(p, observation)

asyncio.run(main())