from gymnasium import Env, register
from gymnasium.spaces import Discrete, Box
from numpy import array, int16, float32
from pyboy import PyBoy
from typing import Any
from GameEngine import BattleHandler, BattleState, BattleAction, PokemonSpecies, AvailableMove, convertString, typeToStringShort, statusToString, Pokemon, PokemonBase, Move, loadTrainer as __loadTrainer
from scipy.stats import truncate

wBattleMonPP = 0xD02C

wLinkBattleRandomNumberListIndex = 0xCCDE
wLinkBattleRandomNumberList = 0xD147
wLinkState = 0xD12A
wSerialExchangeNybbleReceiveData = 0xCC3E
LINK_STATE_BATTLING = 4
wCurrentMenuItem = 0xCC26

wPartyMons = 0xD16A
wPartyCount = 0xD162
wPlayerName = 0xD157
wPartyMonNicks = 0xD2B4

wEnemyMons = 0xD8A3
wEnemyPartyCount = 0xD89B
wEnemyMonNicks = 0xD9ED
wTrainerName = 0xD049

wBattleMonHP = 0xD014

t_waiting = [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7] # Waiting...!
t_bring_out_which = [0x81, 0xB1, 0xA8, 0xAD, 0xA6, 0x7F, 0xAE, 0xB4, 0xB3, 0x7F, 0xB6, 0xA7, 0xA8, 0xA2, 0xA7] # Bring out which


class Examples:
	Brock={
		"sprite": 0,
		"p1name": "Ash",
		"p2name": "Brock",
		"p1team": [
			{
				"name": "PIKACHU",
				"level": 11,
				"species": PokemonSpecies.Pikachu,
				"moves": [AvailableMove.Thundershock, AvailableMove.Quick_Attack, AvailableMove.Tail_Whip, AvailableMove.Thunder_Wave]
			},
			{
				"name": "MANKEY",
				"level": 8,
				"species": PokemonSpecies.Mankey,
				"moves": [AvailableMove.Scratch, AvailableMove.Leer]
			},
			{
				"name": "NIDORAN~",
				"level": 8,
				"species": PokemonSpecies.Nidoran_M,
				"moves": [AvailableMove.Tackle, AvailableMove.Leer, AvailableMove.Horn_Attack]
			},
			{
				"name": "PIDGEY",
				"level": 7,
				"species": PokemonSpecies.Pidgey,
				"moves": [AvailableMove.Gust, AvailableMove.Sand_Attack]
			}
		],
		"p2team": [
			{
				"name": "GEODUDE",
				"level": 10,
				"species": PokemonSpecies.Geodude,
				"moves": [AvailableMove.Tackle]
			},
			{
				"name": "ONIX",
				"level": 12,
				"species": PokemonSpecies.Geodude,
				"moves": [AvailableMove.Bide, AvailableMove.Bind, AvailableMove.Screech, AvailableMove.Tackle]
			}
		]
	}
	Misty={
		"sprite": 0,
		"p1name": "Ash",
		"p2name": "Misty",
		"p1team": [
			{
				"name": "PIKACHU",
				"level": 15,
				"species": PokemonSpecies.Pikachu,
				"moves": [AvailableMove.Thundershock, AvailableMove.Quick_Attack, AvailableMove.Double_Team, AvailableMove.Thunder_Wave]
			},
			{
				"name": "MANKEY",
				"level": 9,
				"species": PokemonSpecies.Mankey,
				"moves": [AvailableMove.Scratch, AvailableMove.Leer, AvailableMove.Low_Kick]
			},
			{
				"name": "NIDORAN~",
				"level": 9,
				"species": PokemonSpecies.Nidoran_M,
				"moves": [AvailableMove.Tackle, AvailableMove.Leer, AvailableMove.Horn_Attack]
			},
			{
				"name": "PIDGEY",
				"level": 8,
				"species": PokemonSpecies.Pidgey,
				"moves": [AvailableMove.Gust, AvailableMove.Sand_Attack]
			}
		],
		"p2team": [
			{
				"name": "STARYU",
				"level": 18,
				"species": PokemonSpecies.Staryu,
				"moves": [AvailableMove.Tackle, AvailableMove.Water_Gun]
			},
			{
				"name": "STARMIE",
				"level": 21,
				"species": PokemonSpecies.Starmie,
				"moves": [AvailableMove.Bubble_Beam, AvailableMove.Harden, AvailableMove.Water_Gun, AvailableMove.Tackle]
			}
		]
	}
	LtSurge={
		"sprite": 0,
		"p1name": "Ash",
		"p2name": "LT. Surge",
		"p1team": [
			{
				"name": "PIKACHU",
				"level": 22,
				"species": PokemonSpecies.Pikachu,
				"moves": [AvailableMove.Thundershock, AvailableMove.Slam, AvailableMove.Double_Team, AvailableMove.Thunder_Wave]
			},
			{
				"name": "MANKEY",
				"level": 15,
				"species": PokemonSpecies.Mankey,
				"moves": [AvailableMove.Scratch, AvailableMove.Leer, AvailableMove.Low_Kick, AvailableMove.Karate_Chop]
			},
			{
				"name": "NIDORINO",
				"level": 16,
				"species": PokemonSpecies.Nidorino,
				"moves": [AvailableMove.Tackle, AvailableMove.Leer, AvailableMove.Horn_Attack, AvailableMove.Double_Kick]
			},
			{
				"name": "PIDGEY",
				"level": 15,
				"species": PokemonSpecies.Pidgey,
				"moves": [AvailableMove.Gust, AvailableMove.Sand_Attack, AvailableMove.Quick_Attack]
			}
		],
		"p2team": [
			{
				"name": "RAICHU",
				"level": 28,
				"species": PokemonSpecies.Raichu,
				"moves": [AvailableMove.Growl, AvailableMove.Mega_Kick, AvailableMove.Mega_Punch, AvailableMove.Thunderbolt]
			}
		]
	}


def basic_opponent(state):
	pkmn = state.op.team[state.op.pokemonOnField]
	if pkmn.getHealth() == 0:
		return BattleAction.Switch1 + state.op.pokemonOnField
	for i, move in pkmn.getMoveSet():
		if move.getID() != 0 and move.getPP() != 0:
			return BattleAction.Attack1 + i
	return BattleAction.StruggleMove


def loadTrainer(path):
	state = BattleState()
	with open(path, "rb") as fd:
		data = fd.read()
	name, team = __loadTrainer(data, state)
	return name, [{
		"name": p.getName(False),
		"level": p.getLevel(),
		"species": p.getID(),
		"moves": [m.getID() for m in p.getMoveSet()]
	} for p in team]


class PokemonYellowBattle(Env):
	metadata = {
		'render_modes': ["human", "ansi", "rgb_array_list"],
		'render_fps': 180
	}
	action_space = Discrete(11)
	observation_space = Box(
		#                  HP1,MHP1, HP2,MHP2, T1, T2
		low= array([  0,   0,   0,   0,  0,  0], dtype=float32),
		high=array([999, 999, 999, 999, 26, 26], dtype=float32),
		shape=(6,),
		dtype=float32
	)

	def __init__(self, render_mode=None, episode_trigger=None, opponent_callback=basic_opponent):
		self.battle = BattleHandler(False, False)
		self.op = opponent_callback
		self.max_turns = -1
		self.current_turn = 0
		self.render_mode = render_mode
		self.last_frames = []
		self.episode_id = 0
		self.episode_trigger = episode_trigger
		self.recording = False
		self.last_state = None
		if self.render_mode == "human":
			self.emulator = PyBoy('pokeyellow.gbc', sound_volume=25, window='SDL2')
		elif self.render_mode == "rgb_array_list":
			self.emulator = PyBoy('pokeyellow.gbc', sound_volume=0, window='null')
		else:
			self.emulator = None
		self.messages = []
		if self.render_mode == "ansi":
			self.battle.state.battleLogger = lambda x: self.messages.append(x)


	def copy_battle_data_to_emulator(self, state, team_base_address, name_address, species_array, name_list_address):
		self.emulator.memory[species_array] = len(state.team)
		for i, pkmn in enumerate(state.team):
			self.emulator.memory[species_array + i + 1] = pkmn.getID()
			data = pkmn.encode()
			for k, b in enumerate(data):
				self.emulator.memory[team_base_address + i * len(data) + k] = b
			data = convertString(pkmn.getName(False))
			for j in range(11):
				if j < len(data):
					self.emulator.memory[name_list_address + i * 11 + j] = data[j]
				else:
					self.emulator.memory[name_list_address + i * 11 + j] = 0x50
		self.emulator.memory[species_array + len(state.team) + 1] = 0xFF
		data = convertString(state.name)
		for j in range(11):
			if j < len(data):
				self.emulator.memory[name_address + j] = data[j]
			else:
				self.emulator.memory[name_address + j] = 0x50


	def tick_emulator(self, count=1):
		step = 1 if self.render_mode == "human" or self.render_mode == "rgb_array_list" else 30
		for i in range(0, count, step):
			if not self.emulator.tick(step):
				exit(0)
			self.last_frames.append(self.emulator.screen.ndarray[:, :, :3].copy())


	def wait_for_start_turn(self):
		while True:
			if self.emulator.memory[0x9C00:0x9C20] == [0x59, 0x5A, 0x58, 0x59, 0x59, 0x5A, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0F, 0x0F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F]:
				break
			if self.emulator.memory[0x9D64:0x9D6F] == t_waiting:
				break
			if self.emulator.memory[wBattleMonHP:wBattleMonHP+2] != [0, 0] and self.emulator.memory[0x9DD0] == 0xE1 and self.emulator.memory[0x9DD1] == 0xE2:
				break
			if self.emulator.memory[wBattleMonHP:wBattleMonHP+2] == [0, 0] and self.emulator.memory[0x9DC1:0x9DD0] == t_bring_out_which:
				break
			self.tick_emulator()


	def step_emulator(self, state):
		if self.emulator is None or not self.recording:
			return

		if self.emulator.memory[0x9D64:0x9D6F] != t_waiting:
			if self.emulator.memory[wBattleMonHP:wBattleMonHP+2] == [0, 0]:
				self.tick_emulator(10)
				self.emulator.memory[wCurrentMenuItem] = state.me.lastAction - BattleAction.Switch1
				self.tick_emulator(10)
				self.emulator.button_press('a')
				self.tick_emulator(10)
				self.emulator.button_release('a')
				self.tick_emulator(10)
				self.emulator.button_press('a')
				self.tick_emulator(10)
				self.emulator.button_release('a')
			else:
				if BattleAction.Run == state.me.lastAction:
					self.emulator.memory[wCurrentMenuItem] = 3
				elif BattleAction.Switch1 <= state.me.lastAction <= BattleAction.Switch6:
					self.emulator.memory[wCurrentMenuItem] = 2
				else:
					self.emulator.memory[wCurrentMenuItem] = 0
				self.tick_emulator(10)
				self.emulator.button_press('a')
				self.tick_emulator(10)
				self.emulator.button_release('a')
				if BattleAction.Switch1 <= state.me.lastAction <= BattleAction.Switch6:
					self.tick_emulator(10)
					self.emulator.memory[wCurrentMenuItem] = state.me.lastAction - BattleAction.Switch1
					self.tick_emulator(30)
					self.emulator.button_press('a')
					self.tick_emulator(10)
					self.emulator.button_release('a')
					self.tick_emulator(10)
					self.emulator.button_press('a')
					self.tick_emulator(10)
					self.emulator.button_release('a')
				elif state.me.lastAction != BattleAction.StruggleMove:
					self.emulator.memory[wCurrentMenuItem] = state.me.lastAction - BattleAction.Attack1 + 1
					self.tick_emulator(10)
					self.emulator.button_press('a')
					self.tick_emulator(10)
					self.emulator.button_release('a')
		isDead = self.emulator.memory[wBattleMonHP:wBattleMonHP+2] == [0, 0]
		while self.emulator.memory[0x9D64:0x9D6F] != t_waiting:
			self.tick_emulator()
		while self.emulator.memory[wSerialExchangeNybbleReceiveData] == 0xFF:
			self.emulator.memory[wSerialExchangeNybbleReceiveData] = state.op.lastAction - BattleAction.Attack1
			self.tick_emulator()
		while (
			self.emulator.memory[0x9D64:0x9D6F] == t_waiting or
			# When switching from death, screen flashes, so we also wait during the flash
			(isDead and self.emulator.memory[0x9D64:0x9D6F] == [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1])
		):
			self.tick_emulator()
		self.wait_for_start_turn()


	def init_emulator(self, state):
		if self.emulator is None or not self.recording:
			return

		with open("pokeyellow_replay.state", "rb")as fd:
			self.emulator.load_state(fd)
		l = state.rng.getList()
		self.emulator.memory[wLinkBattleRandomNumberListIndex] = 0
		for i in range(9):
			self.emulator.memory[wLinkBattleRandomNumberList + i] = l[i]
		self.last_frames = []
		self.emulator.memory[wLinkState] = LINK_STATE_BATTLING
		self.copy_battle_data_to_emulator(state.me, wPartyMons, wPlayerName,  wPartyCount,      wPartyMonNicks)
		self.copy_battle_data_to_emulator(state.op, wEnemyMons, wTrainerName, wEnemyPartyCount, wEnemyMonNicks)
		self.wait_for_start_turn()


	def make_observation(self, state):
		ob = array([
			state.me.team[state.me.pokemonOnField].getHealth(),
			state.me.team[state.me.pokemonOnField].getMaxHealth(),
			state.op.team[state.op.pokemonOnField].getHealth(),
			state.op.team[state.op.pokemonOnField].getMaxHealth(),
			state.op.team[state.op.pokemonOnField].getTypes()[0],
			state.op.team[state.op.pokemonOnField].getTypes()[1],
		], dtype=float32)
		moveMask = [int(m.getID() != 0 and m.getPP() != 0) for m in state.me.team[state.me.pokemonOnField].getMoveSet()]
		switchMask = [int(len(state.me.team) > i and state.me.pokemonOnField != i) for i in range(6)]
		canUseStruggle = int(not any(moveMask))
		self.last_state = (ob, {
			'mask': moveMask + switchMask + [canUseStruggle]
		})
		return self.last_state


	def compute_reward(self, old, new):
		if new[0][2] == 0:
			return 100
		return (new[0][0] - new[0][2]) / (1 + abs(new[0][1] - new[0][3])) * 2


	def step(self, action):
		old = self.last_state
		state = self.battle.state
		if action == 10:
			state.me.nextAction = BattleAction.StruggleMove
		else:
			state.me.nextAction = BattleAction.Attack1 + action
		state.op.nextAction = self.op(state)
		self.battle.tick()
		observation, info = self.make_observation(state)
		self.step_emulator(state)
		# self.spec.max_episode_steps
		return observation, self.compute_reward(old, self.last_state), self.battle.isFinished(), False, info


	def reset(self, seed=None, options=None):
		super().reset(seed=seed)
		self.battle.reset()
		self.current_turn = 0
		self.episode_id += 1
		if self.episode_trigger:
			self.recording = self.episode_trigger(self.episode_id)
		else:
			self.recording = True
		state = self.battle.state
		if options is None:
			self.battle.reset()
		else:
			state.me.name = options["p1name"]
			state.op.name = options["p2name"]
			state.me.team = [Pokemon(self.battle.state, p["name"], p["level"], PokemonBase(p["species"]), [Move(m) for m in p["moves"]]) for p in options["p1team"]]
			state.op.team = [Pokemon(self.battle.state, p["name"], p["level"], PokemonBase(p["species"]), [Move(m) for m in p["moves"]]) for p in options["p2team"]]
		state.rng.setList([self.np_random.integers(low=0, high=255) for _ in range(9)])
		self.init_emulator(state)
		return self.make_observation(state)


	@staticmethod
	def serialize_mon(s, i, op):
		r = f'?????????? ({s.getSpeciesName(): >10}) l{s.getLevel():03d}, {typeToStringShort(s.getTypes()[0])}'
		if s.getTypes()[0] != s.getTypes()[1]:
			r += f'/{typeToStringShort(s.getTypes()[1])}'
		if not op.isPkmnDiscovered(i):
			r += " (Not revealed yet)"
		r += "\n - "
		r += f'{s.getHealth():03d}/{s.getMaxHealth():03d}HP, '
		r += f'{s.getAttack():03d}ATK (???@+0), '
		r += f'{s.getDefense():03d}DEF (???@+0), '
		r += f'{s.getSpecial():03d}SPE (???@+0), '
		r += f'{s.getSpeed(): >3d}SPD (???@+0), '
		r += f'100%ACC (+0), '
		r += f'100%EVD (+0)'
		r += f'\n - Status: 0x{s.getNonVolatileStatus():02X} {statusToString(s.getNonVolatileStatus())}, '
		r += f'\n - Moves: {", ".join(f'{m.getName()} {m.getPP():02d}/{m.getMaxPP():02d}PP{"" if op.isPkmnMoveDiscovered(i, _i) else " (Not yet revealed)"}' for _i, m in enumerate(s.getMoveSet()) if m)}'
		return r


	def render(self):
		if self.render_mode == 'ansi':
			state = self.battle.state
			messages = "\n".join(self.messages)
			p1 = state.me.name + "'s team (P1)\n" + "\n".join(self.serialize_mon(s, i, state.op) for i, s in enumerate(state.me.team))
			p2 = state.op.name + "'s team (P2)\n" + "\n".join(self.serialize_mon(s, i, state.me) for i, s in enumerate(state.op.team))
			return messages + "\n" + p1 + "\n" + p2
		if self.render_mode == 'rgb_array_list':
			old = self.last_frames
			self.last_frames = []
			return old
		return None


	def close(self):
		if self.emulator is not None:
			self.emulator.stop(False)

register('PokemonYellow', PokemonYellowBattle)