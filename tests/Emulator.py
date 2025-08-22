from pyboy import PyBoy
from GameEngine import BattleAction, AvailableMove, convertString


wEnemyMonNick = 0xCFD9
wEnemyMon = 0xCFE4
wEnemyMonSpecies = 0xCFE4
wEnemyMonHP = 0xCFE5
wEnemyMonBoxLevel = 0xCFE7
wEnemyMonStatus = 0xCFE8
wEnemyMonType1 = 0xCFE9
wEnemyMonType2 = 0xCFEA
wEnemyMonCatchRate = 0xCFEB
wEnemyMonMoves = 0xCFEC
wEnemyMonDVs = 0xCFF0
wEnemyMonLevel = 0xCFF2
wEnemyMonStats = 0xCFF3
wEnemyMonMaxHP = 0xCFF3
wEnemyMonAttack = 0xCFF5
wEnemyMonDefense = 0xCFF7
wEnemyMonSpeed = 0xCFF9
wEnemyMonSpecial = 0xCFFB
wEnemyMonPP = 0xCFFD
wEnemyMonUnmodifiedLevel = 0xCD23
wEnemyMonUnmodifiedMaxHP = 0xCD24
wEnemyMonUnmodifiedAttack = 0xCD26
wEnemyMonUnmodifiedDefense = 0xCD28
wEnemyMonUnmodifiedSpeed = 0xCD2A
wEnemyMonUnmodifiedSpecial = 0xCD2C

wBattleMonNick = 0xD008
wBattleMonSpecies = 0xD013
wBattleMonHP = 0xD014
wBattleMonBoxLevel = 0xD016
wBattleMonStatus = 0xD017
wBattleMonType1 = 0xD018
wBattleMonType2 = 0xD019
wBattleMonCatchRate = 0xD01A
wBattleMonMoves = 0xD01B
wBattleMonDVs = 0xD01F
wBattleMonLevel = 0xD021
wBattleMonMaxHP = 0xD022
wBattleMonAttack = 0xD024
wBattleMonDefense = 0xD026
wBattleMonSpeed = 0xD028
wBattleMonSpecial = 0xD02A
wBattleMonPP = 0xD02C
wPlayerMonUnmodifiedLevel = 0xCD0F
wPlayerMonUnmodifiedMaxHP = 0xCD10
wPlayerMonUnmodifiedAttack = 0xCD12
wPlayerMonUnmodifiedDefense = 0xCD14
wPlayerMonUnmodifiedSpeed = 0xCD16
wPlayerMonUnmodifiedSpecial = 0xCD18

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

t_health_holder = [0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x7F]
t_oak_lab = [0x59, 0x5A, 0x58, 0x59, 0x59, 0x5A, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0F, 0x0F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F]
t_waiting = [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7] # Waiting...!
t_bring_out_which = [0x81, 0xB1, 0xA8, 0xAD, 0xA6, 0x7F, 0xAE, 0xB4, 0xB3, 0x7F, 0xB6, 0xA7, 0xA8, 0xA2, 0xA7] # Bring out which

ROM_PATH='pokeyellow.gbc'


class Emulator:
	def __init__(self, has_interface=True, sound_volume=25, save_frames=False, debug=False):
		self.has_interface = has_interface
		self.save_frames = save_frames
		self.last_frames = []
		self.emulator = PyBoy(ROM_PATH, sound_volume=sound_volume, window='SDL2' if has_interface else 'null', debug=debug)


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


	def sync_battle_state(self, state):
		me = state.me.team[state.me.pokemonOnField]
		op = state.op.team[state.op.pokemonOnField]

		data = convertString(me.getName(False))
		moves = me.getMoveSet()
		for i in range(11):
			if i < len(data):
				self.emulator.memory[wBattleMonNick + i] = data[i]
			else:
				self.emulator.memory[wBattleMonNick + i] = 0x50
		self.emulator.memory[wBattleMonLevel] = me.getLevel()
		self.emulator.memory[wBattleMonBoxLevel] = me.getLevel()
		self.emulator.memory[wPlayerMonUnmodifiedLevel] = me.getLevel()
		self.emulator.memory[wBattleMonStatus] = me.getNonVolatileStatus()
		self.emulator.memory[wBattleMonType1] = me.getTypes()[0]
		self.emulator.memory[wBattleMonType2] = me.getTypes()[1]
		self.emulator.memory[wBattleMonSpecies] = me.getID()
		self.emulator.memory[wBattleMonHP      + 0] = me.getHealth() >> 8
		self.emulator.memory[wBattleMonHP      + 1] = me.getHealth() & 0xFF
		self.emulator.memory[wBattleMonMaxHP   + 0] = me.getMaxHealth() >> 8
		self.emulator.memory[wBattleMonMaxHP   + 1] = me.getMaxHealth() & 0xFF
		self.emulator.memory[wBattleMonAttack  + 0] = me.getAttack() >> 8
		self.emulator.memory[wBattleMonAttack  + 1] = me.getAttack() & 0xFF
		self.emulator.memory[wBattleMonDefense + 0] = me.getDefense() >> 8
		self.emulator.memory[wBattleMonDefense + 1] = me.getDefense() & 0xFF
		self.emulator.memory[wBattleMonSpeed   + 0] = me.getSpeed() >> 8
		self.emulator.memory[wBattleMonSpeed   + 1] = me.getSpeed() & 0xFF
		self.emulator.memory[wBattleMonSpecial + 0] = me.getSpecial() >> 8
		self.emulator.memory[wBattleMonSpecial + 1] = me.getSpecial() & 0xFF
		self.emulator.memory[wPlayerMonUnmodifiedMaxHP   + 0] = me.getMaxHealth() >> 8
		self.emulator.memory[wPlayerMonUnmodifiedMaxHP   + 1] = me.getMaxHealth() & 0xFF
		self.emulator.memory[wPlayerMonUnmodifiedAttack  + 0] = me.getRawAttack() >> 8
		self.emulator.memory[wPlayerMonUnmodifiedAttack  + 1] = me.getRawAttack() & 0xFF
		self.emulator.memory[wPlayerMonUnmodifiedDefense + 0] = me.getRawDefense() >> 8
		self.emulator.memory[wPlayerMonUnmodifiedDefense + 1] = me.getRawDefense() & 0xFF
		self.emulator.memory[wPlayerMonUnmodifiedSpeed   + 0] = me.getRawSpeed() >> 8
		self.emulator.memory[wPlayerMonUnmodifiedSpeed   + 1] = me.getRawSpeed() & 0xFF
		self.emulator.memory[wPlayerMonUnmodifiedSpecial + 0] = me.getRawSpecial() >> 8
		self.emulator.memory[wPlayerMonUnmodifiedSpecial + 1] = me.getRawSpecial() & 0xFF
		for i in range(4):
			if i < len(moves):
				self.emulator.memory[wBattleMonMoves + i] = moves[i].getID()
				self.emulator.memory[wBattleMonPP    + i] = moves[i].getPP()
			else:
				self.emulator.memory[wBattleMonMoves + i] = AvailableMove.Empty
				self.emulator.memory[wBattleMonPP    + i] = 0

		data = convertString(op.getName(False))
		moves = op.getMoveSet()
		for i in range(11):
			if i < len(data):
				self.emulator.memory[wEnemyMonNick + i] = data[i]
			else:
				self.emulator.memory[wEnemyMonNick + i] = 0x50
		self.emulator.memory[wEnemyMonLevel] = op.getLevel()
		self.emulator.memory[wEnemyMonBoxLevel] = op.getLevel()
		self.emulator.memory[wEnemyMonUnmodifiedLevel] = op.getLevel()
		self.emulator.memory[wEnemyMonStatus] = op.getNonVolatileStatus()
		self.emulator.memory[wEnemyMonType1] = op.getTypes()[0]
		self.emulator.memory[wEnemyMonType2] = op.getTypes()[1]
		self.emulator.memory[wEnemyMonSpecies] = op.getID()
		self.emulator.memory[wEnemyMonHP      + 0] = op.getHealth() >> 8
		self.emulator.memory[wEnemyMonHP      + 1] = op.getHealth() & 0xFF
		self.emulator.memory[wEnemyMonMaxHP   + 0] = op.getMaxHealth() >> 8
		self.emulator.memory[wEnemyMonMaxHP   + 1] = op.getMaxHealth() & 0xFF
		self.emulator.memory[wEnemyMonAttack  + 0] = op.getAttack() >> 8
		self.emulator.memory[wEnemyMonAttack  + 1] = op.getAttack() & 0xFF
		self.emulator.memory[wEnemyMonDefense + 0] = op.getDefense() >> 8
		self.emulator.memory[wEnemyMonDefense + 1] = op.getDefense() & 0xFF
		self.emulator.memory[wEnemyMonSpeed   + 0] = op.getSpeed() >> 8
		self.emulator.memory[wEnemyMonSpeed   + 1] = op.getSpeed() & 0xFF
		self.emulator.memory[wEnemyMonSpecial + 0] = op.getSpecial() >> 8
		self.emulator.memory[wEnemyMonSpecial + 1] = op.getSpecial() & 0xFF
		self.emulator.memory[wEnemyMonUnmodifiedMaxHP   + 0] = op.getMaxHealth() >> 8
		self.emulator.memory[wEnemyMonUnmodifiedMaxHP   + 1] = op.getMaxHealth() & 0xFF
		self.emulator.memory[wEnemyMonUnmodifiedAttack  + 0] = op.getRawAttack() >> 8
		self.emulator.memory[wEnemyMonUnmodifiedAttack  + 1] = op.getRawAttack() & 0xFF
		self.emulator.memory[wEnemyMonUnmodifiedDefense + 0] = op.getRawDefense() >> 8
		self.emulator.memory[wEnemyMonUnmodifiedDefense + 1] = op.getRawDefense() & 0xFF
		self.emulator.memory[wEnemyMonUnmodifiedSpeed   + 0] = op.getRawSpeed() >> 8
		self.emulator.memory[wEnemyMonUnmodifiedSpeed   + 1] = op.getRawSpeed() & 0xFF
		self.emulator.memory[wEnemyMonUnmodifiedSpecial + 0] = op.getRawSpecial() >> 8
		self.emulator.memory[wEnemyMonUnmodifiedSpecial + 1] = op.getRawSpecial() & 0xFF
		for i in range(4):
			if i < len(moves):
				self.emulator.memory[wEnemyMonMoves + i] = moves[i].getID()
				self.emulator.memory[wEnemyMonPP    + i] = moves[i].getPP()
			else:
				self.emulator.memory[wEnemyMonMoves + i] = AvailableMove.Empty
				self.emulator.memory[wEnemyMonPP    + i] = 0



	def tick(self, count=1):
		step = 1 if self.has_interface else 30
		for i in range(0, count, step):
			if not self.emulator.tick(step):
				raise InterruptedError()
			if self.save_frames:
				self.last_frames.append(self.emulator.screen.ndarray[:, :, :3].copy())


	def wait_for_start_turn(self):
		while True:
			if self.emulator.memory[0x9C00:0x9C20] == t_oak_lab:
				return
			if self.emulator.memory[0x9D64:0x9D6F] == t_waiting:
				return
			if self.emulator.memory[wBattleMonHP:wBattleMonHP+2] != [0, 0] and self.emulator.memory[0x9DD0] == 0xE1 and self.emulator.memory[0x9DD1] == 0xE2:
				return
			if self.emulator.memory[wBattleMonHP:wBattleMonHP+2] == [0, 0] and self.emulator.memory[0x9DC1:0x9DD0] == t_bring_out_which:
				return
			self.tick()


	def select_action(self, action):
		if self.emulator.memory[wBattleMonHP:wBattleMonHP+2] == [0, 0]:
			assert BattleAction.Switch1 <= action <= BattleAction.Switch6
			self.tick(10)
			while self.emulator.memory[wCurrentMenuItem] != action - BattleAction.Switch1:
				self.emulator.button('up', 5)
				self.tick(10)
			self.emulator.button('a', 5)
			self.tick(10)
			return
		if BattleAction.Run == action:
			self.emulator.button('right', 5)
			self.emulator.button('down', 5)
		elif BattleAction.Switch1 <= action <= BattleAction.Switch6:
			self.emulator.button('right', 5)
			self.emulator.button('up', 5)
		else:
			self.emulator.button('left', 5)
			self.emulator.button('up', 5)
		self.tick(5)
		self.emulator.button('a', 5)
		self.tick(20)
		if BattleAction.Switch1 <= action <= BattleAction.Switch6:
			while self.emulator.memory[wCurrentMenuItem] != action - BattleAction.Switch1:
				self.emulator.button('up', 5)
				self.tick(10)
			self.emulator.button('a', 5)
			self.tick(10)
			self.emulator.button('a', 5)
			self.tick(10)
		elif BattleAction.Attack1 <= action <= BattleAction.Attack4:
			while self.emulator.memory[wCurrentMenuItem] != action - BattleAction.Attack1 + 1 and self.emulator.memory[0x9D64:0x9D6F] != t_waiting:
				self.emulator.button('up', 5)
				self.tick(10)
			self.emulator.button('a', 5)
			self.tick(10)


	def step(self, state):
		if self.emulator.memory[0x9D64:0x9D6F] != t_waiting:
			self.select_action(state.me.lastAction)
		while self.emulator.memory[0x9D64:0x9D6F] != t_waiting:
			self.tick()
		while self.emulator.memory[wSerialExchangeNybbleReceiveData] == 0xFF:
			self.emulator.memory[wSerialExchangeNybbleReceiveData] = state.op.lastAction - BattleAction.Attack1
			self.tick()
		while self.emulator.memory[0x9D64:0x9D6F] == t_waiting or self.emulator.memory[0x9D6A:0x9D74] != t_health_holder:
			self.tick()
		self.wait_for_start_turn()


	def init_battle(self, save_state_fd, state, sync_data=False):
		self.emulator.load_state(save_state_fd)
		l = state.rng.getList()
		self.emulator.memory[wLinkBattleRandomNumberListIndex] = 0
		for i in range(9):
			self.emulator.memory[wLinkBattleRandomNumberList + i] = l[i]
		self.last_frames = []
		self.emulator.memory[wLinkState] = LINK_STATE_BATTLING
		self.copy_battle_data_to_emulator(state.me, wPartyMons, wPlayerName,  wPartyCount,      wPartyMonNicks)
		self.copy_battle_data_to_emulator(state.op, wEnemyMons, wTrainerName, wEnemyPartyCount, wEnemyMonNicks)
		if sync_data:
			self.sync_battle_state(state)
		return self.wait_for_start_turn()


	def get_last_frames(self):
		old = self.last_frames
		self.last_frames = []
		return old


	def stop(self, save):
		self.emulator.stop(save)


	def is_finished(self):
		return self.emulator.memory[0x9C00:0x9C20] == t_oak_lab
