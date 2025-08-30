from pyboy import PyBoy
from GameEngine import BattleAction, AvailableMove, convertString


hWhoseTurn = 0xFFF3
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
wRivalName = 0xD349

wEnemyMons = 0xD8A3
wEnemyPartyCount = 0xD89B
wEnemyMonNicks = 0xD9ED
wTrainerName = 0xD049

t_health_holder = [0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x7F]
t_oak_lab = [0x59, 0x5A, 0x58, 0x59, 0x59, 0x5A, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0F, 0x0F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F]
t_waiting = [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7] # Waiting...!
t_bring_out_which = [0x81, 0xB1, 0xA8, 0xAD, 0xA6, 0x7F, 0xAE, 0xB4, 0xB3, 0x7F, 0xB6, 0xA7, 0xA8, 0xA2, 0xA7] # Bring out which

ROM_PATH='pokeyellow.gbc'

CHAR_START = 0
CHAR_RAM = 1
CHAR_START_ASM = 8
CHAR_FAR = 0x17
CHAR_END = 0x50
CHAR_PLAYER = 0x52
CHAR_RIVAL = 0x53
CHAR_TARGET = 0x59
CHAR_USER = 0x5A
CHAR_LINE = 0x4F
CHAR_DONE = 0x57
CHAR_PROMPT = 0x58

CHAR_INVALID = "?"
CHAR_E_ACCENT = "é"
CHAR_AP_D = "'d"
CHAR_AP_L = "'l"
CHAR_AP_S = "'s"
CHAR_AP_T = "'t"
CHAR_AP_V = "'v"
CHAR_AP_R = "'r"
CHAR_AP_M = "'m"
CHAR_ARR_WHI = "→"
CHAR_ARR_BLA = "►"
CHAR_ARR_DOW = "▼"
CHAR_PK_NUM = "Pₖ"
CHAR_MN_NUM = "mₙ"
CHAR_MAL_NUM = "♂"
CHAR_FEM_NUM = "♀"

Pkmn1CharToASCIIConversionTable = [
	"<CMD 0>",    # 0x00
	"<CMD 1>",    # 0x01
	"<CMD 2>",    # 0x02
	"<CMD 3>",    # 0x03
	"<CMD 4>",    # 0x04
	"<CMD 5>",    # 0x05
	"<CMD 6>",    # 0x06
	"<CMD 7>",    # 0x07
	"<CMD 8>",    # 0x08
	"<CMD 9>",    # 0x09
	"<CMD A>",    # 0x0A
	"<CMD B>",    # 0x0B
	"<CMD C>",    # 0x0C
	"<CMD D>",    # 0x0D
	"<CMD E>",    # 0x0E
	CHAR_INVALID, # 0x0F
	CHAR_INVALID, # 0x10
	CHAR_INVALID, # 0x11
	CHAR_INVALID, # 0x12
	CHAR_INVALID, # 0x13
	CHAR_INVALID, # 0x14
	CHAR_INVALID, # 0x15
	CHAR_INVALID, # 0x16
	CHAR_INVALID, # 0x17
	CHAR_INVALID, # 0x18
	CHAR_INVALID, # 0x19
	CHAR_INVALID, # 0x1A
	CHAR_INVALID, # 0x1B
	CHAR_INVALID, # 0x1C
	CHAR_INVALID, # 0x1D
	CHAR_INVALID, # 0x1E
	CHAR_INVALID, # 0x1F
	CHAR_INVALID, # 0x20
	CHAR_INVALID, # 0x21
	CHAR_INVALID, # 0x22
	CHAR_INVALID, # 0x23
	CHAR_INVALID, # 0x24
	CHAR_INVALID, # 0x25
	CHAR_INVALID, # 0x26
	CHAR_INVALID, # 0x27
	CHAR_INVALID, # 0x28
	CHAR_INVALID, # 0x29
	CHAR_INVALID, # 0x2A
	CHAR_INVALID, # 0x2B
	CHAR_INVALID, # 0x2C
	CHAR_INVALID, # 0x2D
	CHAR_INVALID, # 0x2E
	CHAR_INVALID, # 0x2F
	CHAR_INVALID, # 0x30
	CHAR_INVALID, # 0x31
	CHAR_INVALID, # 0x32
	CHAR_INVALID, # 0x33
	CHAR_INVALID, # 0x34
	CHAR_INVALID, # 0x35
	CHAR_INVALID, # 0x36
	CHAR_INVALID, # 0x37
	CHAR_INVALID, # 0x38
	CHAR_INVALID, # 0x39
	CHAR_INVALID, # 0x3A
	CHAR_INVALID, # 0x3B
	CHAR_INVALID, # 0x3C
	CHAR_INVALID, # 0x3D
	CHAR_INVALID, # 0x3E
	CHAR_INVALID, # 0x3F
	CHAR_INVALID, # 0x40
	CHAR_INVALID, # 0x41
	CHAR_INVALID, # 0x42
	CHAR_INVALID, # 0x43
	CHAR_INVALID, # 0x44
	CHAR_INVALID, # 0x45
	CHAR_INVALID, # 0x46
	CHAR_INVALID, # 0x47
	CHAR_INVALID, # 0x48
	CHAR_INVALID, # 0x49
	CHAR_INVALID, # 0x4A
	CHAR_INVALID, # 0x4B
	CHAR_INVALID, # 0x4C
	CHAR_INVALID, # 0x4D
	CHAR_INVALID, # 0x4E
	CHAR_INVALID, # 0x4F
	CHAR_INVALID, # 0x50 / CHAR_END
	CHAR_INVALID, # 0x51
	CHAR_INVALID, # 0x52 / CHAR_PLAYER
	CHAR_INVALID, # 0x53 / CHAR_RIVAL
	CHAR_INVALID, # 0x54
	CHAR_INVALID, # 0x55
	CHAR_INVALID, # 0x56
	CHAR_INVALID, # 0x57
	CHAR_INVALID, # 0x58
	CHAR_INVALID, # 0x59 / CHAR_TARGET
	CHAR_INVALID, # 0x5A / CHAR_USER
	"PC",         # 0x5B
	"TM",         # 0x5C
	"TRAINER",    # 0x5D
	"ROCKET",     # 0x5E
	CHAR_INVALID, # 0x5F
	CHAR_INVALID, # 0x60
	CHAR_INVALID, # 0x61
	CHAR_INVALID, # 0x62
	CHAR_INVALID, # 0x63
	CHAR_INVALID, # 0x64
	CHAR_INVALID, # 0x65
	CHAR_INVALID, # 0x66
	CHAR_INVALID, # 0x67
	CHAR_INVALID, # 0x68
	CHAR_INVALID, # 0x69
	CHAR_INVALID, # 0x6A
	CHAR_INVALID, # 0x6B
	CHAR_INVALID, # 0x6C
	CHAR_INVALID, # 0x6D
	CHAR_INVALID, # 0x6E
	CHAR_INVALID, # 0x6F
	CHAR_INVALID, # 0x70
	CHAR_INVALID, # 0x71
	CHAR_INVALID, # 0x72
	CHAR_INVALID, # 0x73
	CHAR_INVALID, # 0x74
	CHAR_INVALID, # 0x75
	CHAR_INVALID, # 0x76
	CHAR_INVALID, # 0x77
	CHAR_INVALID, # 0x78
	CHAR_INVALID, # 0x79
	CHAR_INVALID, # 0x7A
	CHAR_INVALID, # 0x7B
	CHAR_INVALID, # 0x7C
	CHAR_INVALID, # 0x7D
	CHAR_INVALID, # 0x7E
	' ',          # 0x7F
	'A',          # 0x80
	'B',          # 0x81
	'C',          # 0x82
	'D',          # 0x83
	'E',          # 0x84
	'F',          # 0x85
	'G',          # 0x86
	'H',          # 0x87
	'I',          # 0x88
	'J',          # 0x89
	'K',          # 0x8A
	'L',          # 0x8B
	'M',          # 0x8C
	'N',          # 0x8D
	'O',          # 0x8E
	'P',          # 0x8F
	'Q',          # 0x90
	'R',          # 0x91
	'S',          # 0x92
	'T',          # 0x93
	'U',          # 0x94
	'V',          # 0x95
	'W',          # 0x96
	'X',          # 0x97
	'Y',          # 0x98
	'Z',          # 0x99
	'(',          # 0x9A
	')',          # 0x9B
	':',          # 0x9C
	';',          # 0x9D
	'[',          # 0x9E
	']',          # 0x9F
	'a',          # 0xA0
	'b',          # 0xA1
	'c',          # 0xA2
	'd',          # 0xA3
	'e',          # 0xA4
	'f',          # 0xA5
	'g',          # 0xA6
	'h',          # 0xA7
	'i',          # 0xA8
	'j',          # 0xA9
	'k',          # 0xAA
	'l',          # 0xAB
	'm',          # 0xAC
	'n',          # 0xAD
	'o',          # 0xAE
	'p',          # 0xAF
	'q',          # 0xB0
	'r',          # 0xB1
	's',          # 0xB2
	't',          # 0xB3
	'u',          # 0xB4
	'v',          # 0xB5
	'w',          # 0xB6
	'x',          # 0xB7
	'y',          # 0xB8
	'z',          # 0xB9
	CHAR_E_ACCENT,# 0xBA
	CHAR_AP_D,    # 0xBB
	CHAR_AP_L,    # 0xBC
	CHAR_AP_S,    # 0xBD
	CHAR_AP_T,    # 0xBE
	CHAR_AP_V,    # 0xBF
	CHAR_INVALID, # 0xC0
	CHAR_INVALID, # 0xC1
	CHAR_INVALID, # 0xC2
	CHAR_INVALID, # 0xC3
	CHAR_INVALID, # 0xC4
	CHAR_INVALID, # 0xC5
	CHAR_INVALID, # 0xC6
	CHAR_INVALID, # 0xC7
	CHAR_INVALID, # 0xC8
	CHAR_INVALID, # 0xC9
	CHAR_INVALID, # 0xCA
	CHAR_INVALID, # 0xCB
	CHAR_INVALID, # 0xCC
	CHAR_INVALID, # 0xCD
	CHAR_INVALID, # 0xCE
	CHAR_INVALID, # 0xCF
	CHAR_INVALID, # 0xD0
	CHAR_INVALID, # 0xD1
	CHAR_INVALID, # 0xD2
	CHAR_INVALID, # 0xD3
	CHAR_INVALID, # 0xD4
	CHAR_INVALID, # 0xD5
	CHAR_INVALID, # 0xD6
	CHAR_INVALID, # 0xD7
	CHAR_INVALID, # 0xD8
	CHAR_INVALID, # 0xD9
	CHAR_INVALID, # 0xDA
	CHAR_INVALID, # 0xDB
	CHAR_INVALID, # 0xDC
	CHAR_INVALID, # 0xDD
	CHAR_INVALID, # 0xDE
	CHAR_INVALID, # 0xDF
	'\'',         # 0xE0
	CHAR_PK_NUM,  # 0xE1
	CHAR_MN_NUM,  # 0xE2
	'-',          # 0xE3
	CHAR_AP_R,    # 0xE4
	CHAR_AP_M,    # 0xE5
	'?',          # 0xE6
	'!',          # 0xE7
	'.',          # 0xE8
	CHAR_INVALID, # 0xE9
	CHAR_INVALID, # 0xEA
	CHAR_INVALID, # 0xEB
	CHAR_ARR_WHI, # 0xEC
	CHAR_ARR_BLA, # 0xED
	CHAR_ARR_DOW, # 0xEE
	CHAR_MAL_NUM, # 0xEF
	'$',          # 0xF0
	'x',          # 0xF1
	'.',          # 0xF2
	'/',          # 0xF3
	',',          # 0xF4
	CHAR_FEM_NUM, # 0xF5
	'0',          # 0xF6
	'1',          # 0xF7
	'2',          # 0xF8
	'3',          # 0xF9
	'4',          # 0xFA
	'5',          # 0xFB
	'6',          # 0xFC
	'7',          # 0xFD
	'8',          # 0xFE
	'9'           # 0xFF
]


def call_text_hook(self):
	if self.on_text_displayed:
		if self.emulator.register_file.HL < 0xc4b9:
			return
		text_address = self.emulator.register_file.D << 8 | self.emulator.register_file.E
		end, txt = self.translate_text_at(text_address)
		if self.waiting_text:
			if self.emulator.register_file.HL == 0xc4b9:
				self.text_buffer = txt
			else:
				self.text_buffer += txt
			if end:
				if self.text_buffer != "!":
					self.on_text_displayed(self.text_buffer)
				self.text_buffer = ""
		else:
			self.text_buffer = ""


class Emulator:
	def __init__(self, has_interface=True, sound_volume=25, save_frames=False, debug=False):
		self.has_interface = has_interface
		self.save_frames = save_frames
		self.last_frames = []
		self.text_buffer = ""
		self.waiting_text = False
		self.on_text_displayed = None
		self.emulator = PyBoy(ROM_PATH, sound_volume=sound_volume, window='SDL2' if has_interface else 'null', debug=debug)
		self.emulator.hook_register(0x00, 0x1723, call_text_hook, self) # 0x3C36


	def translate_text_at(self, address, bank=None):
		s = ""
		end = False
		byte = self.emulator.memory[address] if bank is None else self.emulator.memory[bank, address]
		while byte != CHAR_END:
			if byte == CHAR_LINE:
				s += " "
			elif byte == CHAR_PLAYER:
				end, v = self.translate_text_at(wPlayerName)
				s += v
			elif byte == CHAR_RIVAL:
				end, v = self.translate_text_at(wRivalName)
				s += v
			elif byte == CHAR_TARGET:
				turn = self.emulator.memory[hWhoseTurn]
				end, v = self.translate_text_at(wBattleMonNick if turn else wEnemyMonNick)
				s += v
			elif byte == CHAR_USER:
				turn = self.emulator.memory[hWhoseTurn]
				end, v = self.translate_text_at(wEnemyMonNick if turn else wBattleMonNick)
				s += v
			else:
				s += Pkmn1CharToASCIIConversionTable[byte]
			address += 1
			byte = self.emulator.memory[address] if bank is None else self.emulator.memory[bank, address]
			if byte == CHAR_PROMPT or byte == CHAR_DONE:
				return True, s
		return end, s


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
		self.waiting_text = True
		self.wait_for_start_turn()
		self.waiting_text = False
		self.text_buffer = ""


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
		self.waiting_text = True
		self.wait_for_start_turn()
		self.waiting_text = False
		self.text_buffer = ""


	def get_last_frames(self):
		old = self.last_frames
		self.last_frames = []
		return old


	def stop(self, save):
		self.emulator.stop(save)


	def is_finished(self):
		return self.emulator.memory[0x9C00:0x9C20] == t_oak_lab
