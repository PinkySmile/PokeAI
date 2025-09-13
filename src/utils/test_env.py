import traceback

from PokeBattle.Gen1.Env import Examples
import gymnasium as gym
import asyncio
import sys


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


async def main():
	p = gym.make('PokemonYellow', render_mode="human", shuffle_teams=True, rom="/home/pinky/pokeyellow-gen-II/pokeyellow.gbc")
	finished = False
	params = Examples.Blue3_1
	observation, info = p.reset(options=params)

	print(observation, info, len(observation))
	check_obs(p, observation)
	r = p.render()
	if r:
		print(r)
	while not finished:
		didit = False
		while not didit:
			future = asyncio.ensure_future(ainput("Action index:"))
			while not future.done():
				if info['emulator'] is not None:
					info['emulator'].tick()
				await asyncio.sleep(0)
			info['simulator'].save_replay("interrupted.replay")
			while not didit:
				try:
					if future.result() == "stop":
						info['simulator'].save_replay("interrupted.replay")
						exit(1)
					observation, reward, finished, truncated, info = p.step(int(future.result()))
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
		if r:
			print(r)
		print(observation, info, len(observation))
		check_obs(p, observation)

asyncio.run(main())