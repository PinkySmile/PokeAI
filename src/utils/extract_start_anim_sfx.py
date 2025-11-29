import sys
import json
import wave
from PokeBattle.Gen1.Env import load_scenario
from PokeBattle.Gen1.BattleHandler import BattleHandler
from PokeBattle.Gen1.Move import Move, AvailableMove
from PokeBattle.Gen1.Pokemon import Pokemon, PokemonBase, PokemonSpecies
from PokeBattle.Gen1.PyBoyEmulator import PyBoyEmulator
from PokeBattle.Gen1.State import BattleAction
from PokeBattle.Gen1.YellowEmulator import TrainerClass, GBAddress, LINK_STATE_BATTLING


first_tile_check = 0x8310
tileset_1_first = 0x0F
tileset_2_first = 0xC0


def extract_frame_info(self):
	sprites = []
	frame = { 'duration': 1 }
	tile = self.read(GBAddress(first_tile_check))
	if tile == tileset_1_first:
		frame['tileset'] = 1
	elif tile == tileset_2_first:
		frame['tileset'] = 2
	else:
		frame['tileset'] = 0
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
	for i in range(165):
		self.tick()
	fd = wave.open(f"exports/start.wav", "wb")
	fd.setnchannels(2)
	fd.setsampwidth(1)
	fd.setframerate(self.emulator.sound.sample_rate)
	frames1 = []
	last_frame = ''
	last_frame_obj = {}
	playing_cry = False
	data = { 'frames_p1': frames1, 'frames_p2': [] }
	while not playing_cry:
		fd.writeframes(bytes(k + 128 for k in bytes(self.emulator.sound.ndarray)))
		self.tick()
		current_frame = extract_frame_info(self)
		if json.dumps(current_frame) != last_frame:
			frames1.append(current_frame)
			last_frame_obj = current_frame
			last_frame = json.dumps(current_frame)
		else:
			last_frame_obj['duration'] += 1

	fd.close()
	save_frames(data, "exports/start.json")
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

playing_cry = False

def set_playing(_):
	global playing_cry
	playing_cry = True

emulator = PyBoyEmulator(has_interface=True, rom=sys.argv[1], sound_volume=0, cgb=False)
state.rng.generate_list(9)
emulator.emulator.set_emulation_speed(0)
emulator.hook(emulator.symbol("PlayCry"), set_playing)
init_battle(emulator)
