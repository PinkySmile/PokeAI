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


def init_battle(self):
	self.battle_finished = False
	self.write(GBAddress(0xFF50), 0x01)
	self.register_a = 0x11
	self.jump(GBAddress(0x00, 0x100))
	sym = self.symbol("DelayFrames")
	instr = [0xCD, sym.address & 0xFF, sym.address >> 8]
	self.run_until(self.find_instr(instr, self.symbol("PlayShootingStar")))

	def to_battle(_):
		t = TrainerClass.LASS + 200
		self.write(self.symbol("wCurOpponent"), t)
		symbol = self.symbol("InitBattle")
		self.register_hl = symbol.address
		self.register_b = symbol.bank
		self.push(self.symbol("Bankswitch").address)
		self.write(self.symbol("wLinkState"), LINK_STATE_BATTLING) # Removes music

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
	global playing_cry
	playing_cry = False
	while not playing_cry:
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


battle = BattleHandler(False, False)
state = battle.state

state.rng.generate_list(9)
state.me.name = "0"
state.op.name = "0"

playing_cry = False
state.me.team = [Pokemon(state, "0", 100, PokemonBase(PokemonSpecies.Pikachu), [Move(AvailableMove.Razor_Wind)], False)]

def set_playing(_):
	global playing_cry
	playing_cry = True

emulator = PyBoyEmulator(has_interface=True, rom=sys.argv[1], sound_volume=0)
emulator.emulator.set_emulation_speed(0)
emulator.hook(emulator.symbol("PlayCry"), set_playing)
for i in PokemonSpecies:
	state.op.team = [Pokemon(state, "0", 100, PokemonBase(i), [Move(AvailableMove.Razor_Wind)], True)]
	init_battle(emulator)
