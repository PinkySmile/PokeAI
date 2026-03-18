import sys
import wave
from PokeBattle.Gen1.Env import load_scenario
from PokeBattle.Gen1.BattleHandler import BattleHandler
from PokeBattle.Gen1.Move import Move, AvailableMove
from PokeBattle.Gen1.Pokemon import Pokemon, PokemonBase, PokemonSpecies
from PokeBattle.Gen1.PyBoyEmulator import PyBoyEmulator
from PokeBattle.Gen1.State import BattleAction
from PokeBattle.Gen1.YellowEmulator import TrainerClass, GBAddress, LINK_STATE_BATTLING

op_start_addr_vram = 0x9C0C
me_start_addr_vram = 0x9CC1
first_tile_check = 0x8310
tileset_1_first = 0x0F
tileset_2_first = 0xC0
in_selection = False

def a(_):
	global in_selection
	in_selection = True


def step(self, move):
	global in_selection
	in_selection = False
	old_sp = self.register_sp
	self.write(self.symbol("hWhoseTurn"), 0)
	self.emulator.register_file.PC = 0x37
	self.waiting_text = True
	self.write(self.symbol("wTestBattlePlayerSelectedMove"), move)

	if move == AvailableMove.Growl or move == AvailableMove.Roar:
		path = f"exports/move_{move}/{state.me.team[0].id}.wav"
	else:
		path = f"exports/move_{move}.wav"
	fd = wave.open(path, "wb")
	fd.setnchannels(2)
	fd.setsampwidth(1)
	fd.setframerate(self.emulator.sound.sample_rate)

	self.turn_started = False
	self.wait_input = False
	self.waiting = False
	while not in_selection:
		fd.writeframes(bytes(k + 128 for k in bytes(self.emulator.sound.ndarray)))
		self.tick()
	for i in range(30):
		fd.writeframes(bytes(k + 128 for k in bytes(self.emulator.sound.ndarray)))
		self.tick()
	fd.close()
	self.register_sp = old_sp

	self.waiting_text = False
	self.text_buffer = ""


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
	self.wait_for_start_turn()

	self.waiting_text = False
	self.text_buffer = ""



STATUS_AFFECTED_ANIM = 0xA7
ENEMY_HUD_SHAKE_ANIM = 0xA8
SHRINKING_SQUARE_ANIM = 0xB0
BURN_PSN_ANIM = 0xBA
SLP_PLAYER_ANIM = 0xBC
SLP_ANIM = 0xBD
CONF_PLAYER_ANIM = 0xBE
CONF_ANIM = 0xBF
SLIDE_DOWN_ANIM = 0xC0
HIDEPIC_ANIM = 0xC8
extra_anims = [
	STATUS_AFFECTED_ANIM,
	ENEMY_HUD_SHAKE_ANIM,
	SHRINKING_SQUARE_ANIM,
	BURN_PSN_ANIM,
	SLP_PLAYER_ANIM,
	SLP_ANIM,
	CONF_PLAYER_ANIM,
	CONF_ANIM,
	SLIDE_DOWN_ANIM,
	HIDEPIC_ANIM
]

battle = BattleHandler(False, False)
state = battle.state

state.me.name = "0"
state.op.name = "0"

state.me.team = [Pokemon(state, "0", 100, PokemonBase(PokemonSpecies.Pikachu), [Move(AvailableMove.Pound)], False)]
state.op.team = [Pokemon(state, "0", 100, PokemonBase(PokemonSpecies.Pikachu), [Move(AvailableMove.Pound)], False)]

emulator = PyBoyEmulator(has_interface=True, rom=sys.argv[1], sound_volume=0)
state.rng.generate_list(9)
emulator.emulator.set_emulation_speed(0)
init_battle(emulator)
emulator.press_button('left', 5)
emulator.press_button('up', 5)
emulator.tick(10)
emulator.press_button('a', 5)
emulator.tick(20)
emulator.hook(emulator.symbol("MoveSelectionMenu"), a)
dbg_sym = emulator.symbol("Func_3d4f5.asm_3d4fd") + 2
bank_sw = emulator.symbol("Bankswitch")
emulator.write(GBAddress(0x00, 0x37), 0xFB)
emulator.write(GBAddress(0x00, 0x38), 0x21)
emulator.write(GBAddress(0x00, 0x39), dbg_sym.address & 0xFF)
emulator.write(GBAddress(0x00, 0x3A), dbg_sym.address >> 8)
emulator.write(GBAddress(0x00, 0x3B), 0x06)
emulator.write(GBAddress(0x00, 0x3C), dbg_sym.bank)
emulator.write(GBAddress(0x00, 0x3D), 0xC3)
emulator.write(GBAddress(0x00, 0x3E), bank_sw.address & 0xFF)
emulator.write(GBAddress(0x00, 0x3F), bank_sw.address >> 8)
for i in list(AvailableMove) + extra_anims:
	if not i:
		continue
	if i == AvailableMove.Growl:
		continue
	if i == AvailableMove.Roar:
		continue
	step(emulator, i)
for i in PokemonSpecies:
	state.me.team = [Pokemon(state, "0", 100, PokemonBase(i), [Move(AvailableMove.Razor_Wind)], True)]
	init_battle(emulator)
	step(emulator, AvailableMove.Growl)
	step(emulator, AvailableMove.Roar)