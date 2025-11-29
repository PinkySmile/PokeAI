import sys
import json
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
p1_tile_start = 0x31
p1_corners = [
	0x9CA1, # X 01, Y 05
	0x9CA7, # X 07, Y 05
	0x9D61, # X 01, Y 0B
	0x9D67, # X 07, Y 0B
]
p2_tile_start = 0
p2_corners = [
	0x9C0C, # X 0C, Y 00
	0x9C12, # X 12, Y 00
	0x9CCC, # X 0C, Y 06
	0x9CD2, # X 12, Y 06
]


def a(_):
	global in_selection
	in_selection = True


def extract_frame_info(self):
	palB = self.read(GBAddress(0xFF47))
	pal1 = self.read(GBAddress(0xFF48))
	if pal1 == 0xF0:
		pal1 = 0xE4 # PyBoy inaccuracy workaround
	pal2 = self.read(GBAddress(0xFF49))
	l = [i - 256 if i >= 160 else i for i in self.emulator.screen.tilemap_position_list[-1]]
	sprites = []
	p1_c = [self.emulator.memory[c] - p1_tile_start for c in p1_corners]
	p2_c = [self.emulator.memory[c] - p2_tile_start for c in p2_corners]
	frame = {
		'palB': [(palB >> 0) & 3, (palB >> 2) & 3, (palB >> 4) & 3, (palB >> 6) & 3],
		'pal1': [(pal1 >> 0) & 3, (pal1 >> 2) & 3, (pal1 >> 4) & 3, (pal1 >> 6) & 3],
		'pal2': [(pal2 >> 0) & 3, (pal2 >> 2) & 3, (pal2 >> 4) & 3, (pal2 >> 6) & 3],
		'duration': 1,
		'scx': l[0],
		'scy': l[1],
		'wx': l[2],
		'wy': l[3]
	}

	tile = self.read(GBAddress(first_tile_check))
	if tile == tileset_1_first:
		frame['tileset'] = 1
	elif tile == tileset_2_first:
		frame['tileset'] = 2
	else:
		frame['tileset'] = 0

	if all(b == (0x7F - p1_tile_start) for b in p1_c):
		frame['p1'] = None
	else:
		frame['p1'] = [0, 0]
		if p1_c[0] != (0x7F - p1_tile_start):
			frame['p1'][0] = -(p1_c[0] // 7)
			frame['p1'][1] = -(p1_c[0] % 7)
		else:
			frame['p1'][0] = (0x30 - p1_c[3]) // 7
			frame['p1'][1] = (0x30 - p1_c[3]) % 7

	if all(b == (0x7F - p2_tile_start) for b in p2_c):
		frame['p2'] = None
	else:
		frame['p2'] = [0, 0]
		if p2_c[0] != (0x7F - p2_tile_start):
			frame['p2'][0] = -(p2_c[0] // 7)
			frame['p2'][1] = -(p2_c[0] % 7)
		else:
			frame['p2'][0] = (0x30 - p2_c[3]) // 7
			frame['p2'][1] = (0x30 - p2_c[3]) % 7

	frame['sprites'] = sprites
	for i in range(40):
		sprite = self.emulator.get_sprite(i)
		if not sprite.on_screen:
			continue
		sprites.append({
			'x': sprite.x,
			'y': sprite.y,
			'id': sprite.tile_identifier - 0x31,
			'pal_num': sprite.attr_palette_number,
			'flip': [sprite.attr_x_flip, sprite.attr_y_flip],
			'prio': sprite.attr_obj_bg_priority
		})
	return frame


def save_frames(data, path):
	s = "{\n"
	i = 0
	for key, value in data.items():
		if i:
			s += ",\n"
		s += f"    {json.dumps(key)}: "
		if key.startswith("frames"):
			s += '[\n            '
			for k, v in enumerate(value):
				if k:
					s += ",\n            "
				content = json.dumps(v)
				f = '"sprites": ['
				index = content.index(f) + len(f)
				x = content[:index]
				if index == len(content) - 2:
					x += "]}"
				else:
					x += "\n    " + "},\n    ".join(content[index:-2].replace("true", "true ").split("}, ")) + "\n]}"
				s += "\n            ".join(x.split("\n"))
			s += '\n    ]'
		else:
			s += json.dumps(value)
		i += 1
	s += "\n}"
	with open(path, "w") as fd:
		fd.write(s)


def step(self, move):
	global in_selection
	in_selection = False
	old_sp = self.register_sp
	self.write(self.symbol("hWhoseTurn"), 0)
	self.emulator.register_file.PC = 0x37
	self.waiting_text = True
	self.write(self.symbol("wTestBattlePlayerSelectedMove"), move)

	path = f"exports/move_{move}.json"
	frames1 = []
	frames2 = []
	last_frame = ''
	last_frame_obj = {}
	data = { 'frames_p1': frames1, 'frames_p2': frames2 }
	f = True
	while not in_selection:
		self.tick()
		current_frame = extract_frame_info(self)
		if current_frame['wy'] >= 144:
			if current_frame['scx'] == -2:
				f = True
			elif current_frame['scx'] == 2:
				f = False
			current_frame['wx'] = int(f)
		if json.dumps(current_frame) != last_frame:
			frames1.append(current_frame)
			last_frame_obj = current_frame
			last_frame = json.dumps(current_frame)
		else:
			last_frame_obj['duration'] += 1

	in_selection = False
	last_frame = ''
	last_frame_obj = {}
	self.write(self.symbol("hWhoseTurn"), 1)
	self.emulator.register_file.PC = 0x37
	self.waiting_text = True
	self.write(self.symbol("wTestBattlePlayerSelectedMove"), move)
	f = True
	while not in_selection:
		self.tick()
		current_frame = extract_frame_info(self)
		if current_frame['wy'] >= 144:
			if current_frame['scx'] == -2:
				f = True
			elif current_frame['scx'] == 2:
				f = False
			current_frame['wx'] = int(f)
		if json.dumps(current_frame) != last_frame:
			frames2.append(current_frame)
			last_frame_obj = current_frame
			last_frame = json.dumps(current_frame)
		else:
			last_frame_obj['duration'] += 1

	save_frames(data, path)
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

	self.turn_started = False
	self.wait_input = False
	self.waiting = False
	while not self.battle_finished and not self.turn_started and not self.wait_input and not self.waiting:
		self.tick()

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

emulator = PyBoyEmulator(has_interface=True, rom=sys.argv[1], sound_volume=0, cgb=False)
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
if len(sys.argv) == 2:
	for i in list(AvailableMove) + extra_anims:
		if not i:
			continue
		step(emulator, i)
else:
	emulator.emulator.set_emulation_speed(1)
	for i in sys.argv[2:]:
		try:
			val = int(i)
		except:
			val = getattr(AvailableMove, i)
		step(emulator, val)
