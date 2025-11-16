import sys
import wave
from PokeBattle.Gen1.Env import load_scenario
from PokeBattle.Gen1.BattleHandler import BattleHandler
from PokeBattle.Gen1.Move import Move, AvailableMove
from PokeBattle.Gen1.Pokemon import Pokemon, PokemonBase, PokemonSpecies
from PokeBattle.Gen1.PyBoyEmulator import PyBoyEmulator
from PokeBattle.Gen1.State import BattleState
from PokeBattle.Gen1.YellowEmulator import TrainerClass, GBAddress, LINK_STATE_BATTLING

op_start_addr_vram = 0x9C0C
me_start_addr_vram = 0x9CC1
first_tile_check = 0x8310
tileset_1_first = 0x0F
tileset_2_first = 0xC0


def init_battle(self, save_state_fd, state: BattleState, fast_forward: bool=False, trainer: TrainerClass|int|None=None):
	try:
		self.battle_finished = False
		if trainer is None:
			n = state.op.name.upper()
			n = n.replace(".", "")
			n = n.replace(" ", "_")
			n = n.replace("~", "_M")
			n = n.replace("`", "_F")
			if hasattr(TrainerClass, n):
				trainer = getattr(TrainerClass, n)
			elif n == "blue":
				trainer = TrainerClass.RIVAL1
			elif n == "champion":
				trainer = TrainerClass.RIVAL3
			else:
				trainer = TrainerClass.NOBODY
		self.fast_forward = fast_forward
		if save_state_fd is None:
			self.write(GBAddress(0xFF50), 0x01)
			self.register_a = 0x11
			self.jump(GBAddress(0x00, 0x100))
			sym = self.symbol("DelayFrames")
			instr = [0xCD, sym.address & 0xFF, sym.address >> 8]
			self.run_until(self.find_instr(instr, self.symbol("PlayShootingStar")))
		else:
			self.load_state(save_state_fd)

		def to_battle(_):
			t = trainer
			if isinstance(t, TrainerClass):
				t += 200
			if t is not None:
				self.write(self.symbol("wCurOpponent"), t)
			symbol = self.symbol("InitBattle")
			self.register_hl = symbol.address
			self.register_b = symbol.bank
			self.push(self.symbol("Bankswitch").address)
			self.write(self.symbol("wLinkState"), LINK_STATE_BATTLING) #-> Removes music

		if trainer == TrainerClass.NOBODY:
			def __(v):
				self.write(self.symbol("wLinkState"), v)
			self.hook_single(self.symbol("GetTrainerInformation"), __, LINK_STATE_BATTLING)
			self.hook_single(self.symbol("DoBattleTransitionAndInitBattleVariables"), __, 0)
			self.hook_single(self.symbol("DoBattleTransitionAndInitBattleVariables.next"), __, LINK_STATE_BATTLING)
			self.hook_single(self.symbol("_InitBattleCommon"), __, 0)
		if trainer == TrainerClass.JESSIE_JAMES:
			self.write(self.symbol("wTrainerNo"), 0x2A)
			trainer = TrainerClass.ROCKET
		#if isinstance(trainer, int) and trainer < 200:
		#	def __(_):
		#		self.write(self.symbol("wCurOpponent"), 200)
		#		self.write(self.symbol("wTrainerNo"), 200)
		#	self.hook_single(self.symbol("_InitBattleCommon"), __)

		gym = [
			TrainerClass.BROCK,
			TrainerClass.MISTY,
			TrainerClass.LT_SURGE,
			TrainerClass.ERIKA,
			TrainerClass.KOGA,
			TrainerClass.BLAINE,
			TrainerClass.SABRINA,
			TrainerClass.GIOVANNI,
			TrainerClass.LORELEI,
			TrainerClass.BRUNO,
			TrainerClass.AGATHA,
			TrainerClass.LANCE
		]
		if trainer in gym:
			self.write(self.symbol("wGymLeaderNo"), 1)
		self.hook_single(GBAddress(0x00, 0x0040), to_battle)
		self.run_until(self.symbol("SlidePlayerAndEnemySilhouettesOnScreen"))
		self.write(self.symbol("wIsInBattle"), 2)

		rng_list = state.rng.list
		self.write(self.symbol("wLinkState"), LINK_STATE_BATTLING)
		self.write(self.symbol("wLinkBattleRandomNumberListIndex"), state.rng.index)
		for i in range(9):
			self.write(self.symbol("wLinkBattleRandomNumberList") + i, rng_list[i])
		self.last_frames = []
		self.copy_battle_data_to_emulator(state.me, self.symbol("wPartyMons"), self.symbol("wPlayerName"),  self.symbol("wPartyCount"),      self.symbol("wPartyMonNicks"))
		self.copy_battle_data_to_emulator(state.op, self.symbol("wEnemyMons"), self.symbol("wTrainerName"), self.symbol("wEnemyPartyCount"), self.symbol("wEnemyMonNicks"))
		self.waiting_text = True
		for i in range(330):
			self.tick()
		fd = wave.open(f"exports/cry_{state.op.team[0].id}.wav", "wb")
		fd.setnchannels(2)
		fd.setsampwidth(1)
		fd.setframerate(self.emulator.sound.sample_rate)
		for i in range(150):
			fd.writeframes(bytes(k + 128 for k in bytes(self.emulator.sound.ndarray)))
			self.tick()
		fd.close()
		#self.wait_for_start_turn()
		self.waiting_text = False
		self.text_buffer = ""
	finally:
		self.fast_forward = False


battle = BattleHandler(False, False)
state = battle.state

state.rng.generate_list(9)
state.me.name = "0"
state.op.name = "Lass"

state.me.team = [Pokemon(state, "0", 100, PokemonBase(PokemonSpecies.Pikachu), [Move(AvailableMove.Razor_Wind)], False)]

emulator = PyBoyEmulator(has_interface=True, rom=sys.argv[1], sound_volume=25)
for i in PokemonSpecies:
	state.op.team = [Pokemon(state, "0", 100, PokemonBase(i), [Move(AvailableMove.Razor_Wind)], True)]
	init_battle(emulator, None, state)
