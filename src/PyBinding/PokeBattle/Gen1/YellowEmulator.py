import os
import enum
import random
from abc import ABC, abstractmethod
from typing import Callable

from .StatusChange import StatusChange, status_to_string_short
from .Type import Type, type_to_string, type_to_string_short
from .State import BattleAction, BattleState, PlayerState
from .Move import AvailableMove
from .EmulatorGameHandle import EmulatorGameHandle
from .Pokemon import PokemonSpecies


class GBAddress:
	def __init__(self, bank, address=None):
		if address is None:
			self.address = bank
			self.bank = None
		else:
			self.bank = bank
			self.address = address
		if 0xD000 <= self.address < 0xE000 and self.bank == 0:
			self.bank = 1

	def __add__(self, other):
		return GBAddress(self.bank, self.address + other)

	def __sub__(self, other):
		return GBAddress(self.bank, self.address - other)

	def __str__(self):
		if self.bank is not None:
			return f'{self.bank:02X}:{self.address:04X}'
		return f'..:{self.address:04X}'


default_symbols = {
	"PlaceString":                                   GBAddress(0x00, 0x1723),
	"GetTrainerInformation":                         GBAddress(0x00, 0x3563),
	"GetTrainerInformation.linkBattle":              GBAddress(0x00, 0x3594),
	"DelayFrames":                                   GBAddress(0x00, 0x372F),
	"Bankswitch":                                    GBAddress(0x00, 0x3E84),
	"EndOfBattle":                                   GBAddress(0x04, 0x7765),
	"SlidePlayerAndEnemySilhouettesOnScreen":        GBAddress(0x0F, 0x404C),
	"MainInBattleLoop":                              GBAddress(0x0F, 0x4249),
	"MainInBattleLoop.selectEnemyMove":              GBAddress(0x0F, 0x42BC),
	"HandlePlayerMonFainted":                        GBAddress(0x0F, 0x471D),
	"MoveSelectionMenu":                             GBAddress(0x0F, 0x5320),
	"LinkBattleExchangeData":                        GBAddress(0x0F, 0x5777),
	"DoBattleTransitionAndInitBattleVariables":      GBAddress(0x0F, 0x6DB8),
	"DoBattleTransitionAndInitBattleVariables.next": GBAddress(0x0F, 0x6DD3),
	"PlayShootingStar":                              GBAddress(0x10, 0x5A02),
	"InitBattle":                                    GBAddress(0x3D, 0x5FF2),
	"_InitBattleCommon":                             GBAddress(0x3D, 0x60EB),
	"wCurrentMenuItem":                              GBAddress(0xCC26),
	"wSerialExchangeNybbleReceiveData":              GBAddress(0xCC3E),
	"wPlayerSubstituteHP":                           GBAddress(0xCCD7),
	"wEnemySubstituteHP":                            GBAddress(0xCCD8),
	"wLinkBattleRandomNumberListIndex":              GBAddress(0xCCDE),
	"wPlayerMonUnmodifiedLevel":                     GBAddress(0xCD0F),
	"wPlayerMonUnmodifiedMaxHP":                     GBAddress(0xCD10),
	"wPlayerMonUnmodifiedAttack":                    GBAddress(0xCD12),
	"wPlayerMonUnmodifiedDefense":                   GBAddress(0xCD14),
	"wPlayerMonUnmodifiedSpeed":                     GBAddress(0xCD16),
	"wPlayerMonUnmodifiedSpecial":                   GBAddress(0xCD18),
	"wEnemyMonUnmodifiedLevel":                      GBAddress(0xCD23),
	"wEnemyMonUnmodifiedMaxHP":                      GBAddress(0xCD24),
	"wEnemyMonUnmodifiedAttack":                     GBAddress(0xCD26),
	"wEnemyMonUnmodifiedDefense":                    GBAddress(0xCD28),
	"wEnemyMonUnmodifiedSpeed":                      GBAddress(0xCD2A),
	"wEnemyMonUnmodifiedSpecial":                    GBAddress(0xCD2C),
	"wEnemyMonNick":                                 GBAddress(0xCFD9),
	"wEnemyMon":                                     GBAddress(0xCFE4),
	"wEnemyMonSpecies":                              GBAddress(0xCFE4),
	"wEnemyMonHP":                                   GBAddress(0xCFE5),
	"wEnemyMonBoxLevel":                             GBAddress(0xCFE7),
	"wEnemyMonStatus":                               GBAddress(0xCFE8),
	"wEnemyMonType1":                                GBAddress(0xCFE9),
	"wEnemyMonType2":                                GBAddress(0xCFEA),
	"wEnemyMonCatchRate":                            GBAddress(0xCFEB),
	"wEnemyMonMoves":                                GBAddress(0xCFEC),
	"wEnemyMonDVs":                                  GBAddress(0xCFF0),
	"wEnemyMonLevel":                                GBAddress(0xCFF2),
	"wEnemyMonStats":                                GBAddress(0xCFF3),
	"wEnemyMonMaxHP":                                GBAddress(0xCFF3),
	"wEnemyMonAttack":                               GBAddress(0xCFF5),
	"wEnemyMonDefense":                              GBAddress(0xCFF7),
	"wEnemyMonSpeed":                                GBAddress(0xCFF9),
	"wEnemyMonSpecial":                              GBAddress(0xCFFB),
	"wEnemyMonPP":                                   GBAddress(0xCFFD),
	"wBattleMonNick":                                GBAddress(0xD008),
	"wBattleMonSpecies":                             GBAddress(0xD013),
	"wBattleMonHP":                                  GBAddress(0xD014),
	"wBattleMonBoxLevel":                            GBAddress(0xD016),
	"wBattleMonStatus":                              GBAddress(0xD017),
	"wBattleMonType1":                               GBAddress(0xD018),
	"wBattleMonType2":                               GBAddress(0xD019),
	"wBattleMonCatchRate":                           GBAddress(0xD01A),
	"wBattleMonMoves":                               GBAddress(0xD01B),
	"wBattleMonDVs":                                 GBAddress(0xD01F),
	"wBattleMonLevel":                               GBAddress(0xD021),
	"wBattleMonMaxHP":                               GBAddress(0xD022),
	"wBattleMonAttack":                              GBAddress(0xD024),
	"wBattleMonDefense":                             GBAddress(0xD026),
	"wBattleMonSpeed":                               GBAddress(0xD028),
	"wBattleMonSpecial":                             GBAddress(0xD02A),
	"wBattleMonPP":                                  GBAddress(0xD02C),
	"wTrainerName":                                  GBAddress(0xD049),
	"wCurOpponent":                                  GBAddress(0xD058),
	"wGymLeaderNo":                                  GBAddress(0xD05B),
	"wTrainerNo":                                    GBAddress(0xD05C),
	"wPlayerBattleStatus1":                          GBAddress(0xD061),
	"wEnemyBattleStatus1":                           GBAddress(0xD066),
	"wLinkState":                                    GBAddress(0xD12A),
	"wLinkBattleRandomNumberList":                   GBAddress(0xD147),
	"wPlayerName":                                   GBAddress(0xD157),
	"wPartyCount":                                   GBAddress(0xD162),
	"wPartyMons":                                    GBAddress(0xD16A),
	"wPartyMonNicks":                                GBAddress(0xD2B4),
	"wRivalName":                                    GBAddress(0xD349),
	"wEnemyPartyCount":                              GBAddress(0xD89B),
	"wEnemyMons":                                    GBAddress(0xD8A3),
	"wEnemyMonNicks":                                GBAddress(0xD9ED),
	"hWhoseTurn":                                    GBAddress(0xFFF3),
}

class TrainerClass(enum.IntEnum):
	NOBODY        = 0x00
	YOUNGSTER     = 0x01
	BUG_CATCHER   = 0x02
	LASS          = 0x03
	SAILOR        = 0x04
	JR_TRAINER_M  = 0x05
	JR_TRAINER_F  = 0x06
	POKEMANIAC    = 0x07
	SUPER_NERD    = 0x08
	HIKER         = 0x09
	BIKER         = 0x0A
	BURGLAR       = 0x0B
	ENGINEER      = 0x0C
	UNUSED_JUGGLER= 0x0D
	FISHER        = 0x0E
	SWIMMER       = 0x0F
	CUE_BALL      = 0x10
	GAMBLER       = 0x11
	BEAUTY        = 0x12
	PSYCHIC_TR    = 0x13
	ROCKER        = 0x14
	JUGGLER       = 0x15
	TAMER         = 0x16
	BIRD_KEEPER   = 0x17
	BLACKBELT     = 0x18
	RIVAL         = 0x19
	RIVAL1        = 0x19
	PROF_OAK      = 0x1A
	CHIEF         = 0x1B
	SCIENTIST     = 0x1C
	GIOVANNI      = 0x1D
	ROCKET        = 0x1E
	COOLTRAINER_M = 0x1F
	COOLTRAINER_F = 0x20
	BRUNO         = 0x21
	BROCK         = 0x22
	MISTY         = 0x23
	LT_SURGE      = 0x24
	ERIKA         = 0x25
	KOGA          = 0x26
	BLAINE        = 0x27
	SABRINA       = 0x28
	GENTLEMAN     = 0x29
	RIVAL2        = 0x2A
	RIVAL3        = 0x2B
	LORELEI       = 0x2C
	CHANNELER     = 0x2D
	AGATHA        = 0x2E
	LANCE         = 0x2F
	JESSIE_JAMES  = 0x30

LINK_STATE_BATTLING = 4
HAS_SUBSTITUTE_UP =  4

t_health_holder = [0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x7F]
t_oak_lab = [0x59, 0x5A, 0x58, 0x59, 0x59, 0x5A, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0F, 0x0F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F]
t_waiting = [0x96, 0xA0, 0xA8, 0xB3, 0xA8, 0xAD, 0xA6, 0xE8, 0xE8, 0xE8, 0xE7] # Waiting...!
t_bring_out_which = [0x81, 0xB1, 0xA8, 0xAD, 0xA6, 0x7F, 0xAE, 0xB4, 0xB3, 0x7F, 0xB6, 0xA7, 0xA8, 0xA2, 0xA7] # Bring out which

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


nickOffset = 0
speciesOffset = nickOffset + 11
hpOffset = speciesOffset + 1
boxLevelOffset = hpOffset + 2
statusOffset = boxLevelOffset + 1
type1Offset = statusOffset + 1
type2Offset = type1Offset + 1
catchRateOffset = type2Offset + 1
movesOffset = catchRateOffset + 1
dvsOffset = movesOffset + 4
levelOffset = dvsOffset + 2
maxHPOffset = levelOffset + 1
attackOffset = maxHPOffset + 2
defenseOffset = attackOffset + 2
speedOffset = defenseOffset + 2
specialOffset = speedOffset + 2
ppOffset = specialOffset + 2


ROM_PATH = os.path.abspath(os.path.join(__file__, os.path.pardir, 'pokeyellow.gbc'))
SYM_PATH = os.path.abspath(os.path.join(__file__, os.path.pardir, 'pokeyellow.sym'))


class PkmnYellowEmulator(ABC):
	def __init__(self):
		self.last_frames = []
		self.text_buffer = ""
		self.waiting_text = False
		self.fast_forward = False
		self.on_text_displayed = None
		self.battle_finished = True
		self.waiting = False
		self.turn_started = False
		self.wait_input = False
		self.is_dead = False

		def end_battle(_):
			self.battle_finished = True
		def turn_start(_):
			self.turn_started = True
			self.battle_finished = False
			self.waiting = False
			self.wait_input = False
			self.is_dead = False
		def wait_for_opponent(_):
			self.waiting = True
			self.wait_input = False
		def select_move(dead):
			self.wait_input = True
			self.is_dead = dead
			if dead:
				self.turn_started = True
				self.battle_finished = False
				self.waiting = False


		self.hook(self.symbol("EndOfBattle"), end_battle)
		self.hook(self.symbol("MainInBattleLoop"), turn_start)
		self.hook(self.symbol("MoveSelectionMenu"), select_move, False)
		self.hook(self.symbol("HandlePlayerMonFainted"), select_move, True)
		self.hook(self.symbol("LinkBattleExchangeData"), wait_for_opponent)
		self.hook(self.symbol("PlaceString"), self.call_text_hook) # 0x3C36


	def symbol(self, name: str) -> GBAddress:
		return default_symbols[name]


	@property
	@abstractmethod
	def register_a(self):
		pass
	@register_a.setter
	@abstractmethod
	def register_a(self, value):
		pass


	@property
	@abstractmethod
	def register_b(self):
		pass
	@register_b.setter
	@abstractmethod
	def register_b(self, value):
		pass


	@property
	@abstractmethod
	def register_c(self):
		pass
	@register_c.setter
	@abstractmethod
	def register_c(self, value):
		pass


	@property
	@abstractmethod
	def register_d(self):
		pass
	@register_d.setter
	@abstractmethod
	def register_d(self, value):
		pass


	@property
	@abstractmethod
	def register_e(self):
		pass
	@register_e.setter
	@abstractmethod
	def register_e(self, value):
		pass


	@property
	@abstractmethod
	def register_h(self):
		pass
	@register_h.setter
	@abstractmethod
	def register_h(self, value):
		pass


	@property
	@abstractmethod
	def register_l(self):
		pass
	@register_l.setter
	@abstractmethod
	def register_l(self, value):
		pass


	@property
	@abstractmethod
	def register_bc(self):
		pass
	@register_bc.setter
	@abstractmethod
	def register_bc(self, value):
		pass


	@property
	@abstractmethod
	def register_de(self):
		pass
	@register_de.setter
	@abstractmethod
	def register_de(self, value):
		pass


	@property
	@abstractmethod
	def register_hl(self):
		pass
	@register_hl.setter
	@abstractmethod
	def register_hl(self, value):
		pass


	@property
	@abstractmethod
	def register_sp(self):
		pass
	@register_sp.setter
	@abstractmethod
	def register_sp(self, value):
		pass


	@abstractmethod
	def read(self, address: GBAddress):
		pass


	@abstractmethod
	def write(self, address: GBAddress, value: int):
		pass


	@abstractmethod
	def read_range(self, address_l: GBAddress, address_h: GBAddress):
		pass


	@abstractmethod
	def press_button(self, button: str, duration=1):
		pass


	@abstractmethod
	def tick(self, count: int=1):
		pass


	@abstractmethod
	def load_state(self, fd):
		pass


	@abstractmethod
	def stop(self):
		pass


	@abstractmethod
	def jump(self, address: GBAddress):
		pass


	@abstractmethod
	def hook(self, address: GBAddress, callback: Callable, userdata=None):
		pass


	@abstractmethod
	def unhook(self, address: GBAddress):
		pass


	def get_basic_mon_state(self, base_address: GBAddress, base_stats: GBAddress, sub_addr: GBAddress, status1_addr: GBAddress):
		nickname = bytes(self.read_range(base_address + nickOffset, base_address + nickOffset + 11))
		species = self.read(base_address + speciesOffset)
		hp = int.from_bytes(bytes(self.read_range(base_address + hpOffset, base_address + hpOffset + 2)), byteorder='big')
		box_level = self.read(base_address + boxLevelOffset)
		status = self.read(base_address + statusOffset)
		type_a = self.read(base_address + type1Offset)
		type_b = self.read(base_address + type2Offset)
		moves = self.read_range(base_address + movesOffset, base_address + movesOffset + 4)
		level = self.read(base_address + levelOffset)
		max_hp = int.from_bytes(bytes(self.read_range(base_address + maxHPOffset, base_address + maxHPOffset + 2)), byteorder='big')
		attack = int.from_bytes(bytes(self.read_range(base_address + attackOffset, base_address + attackOffset + 2)), byteorder='big')
		defense = int.from_bytes(bytes(self.read_range(base_address + defenseOffset, base_address + defenseOffset + 2)), byteorder='big')
		speed = int.from_bytes(bytes(self.read_range(base_address + speedOffset, base_address + speedOffset + 2)), byteorder='big')
		special = int.from_bytes(bytes(self.read_range(base_address + specialOffset, base_address + specialOffset + 2)), byteorder='big')
		pps = list(map(lambda p: p & 0x3F, self.read_range(base_address + ppOffset, base_address + ppOffset + 4)))
		base_max_hp = int.from_bytes(bytes(self.read_range(base_stats, base_stats + 2)), byteorder='big')
		base_attack = int.from_bytes(bytes(self.read_range(base_stats + 2, base_stats + 4)), byteorder='big')
		base_defense = int.from_bytes(bytes(self.read_range(base_stats + 4, base_stats + 6)), byteorder='big')
		base_speed = int.from_bytes(bytes(self.read_range(base_stats + 6, base_stats + 8)), byteorder='big')
		base_special = int.from_bytes(bytes(self.read_range(base_stats + 8, base_stats + 10)), byteorder='big')
		return {
			'nickname': nickname,
			'species': PokemonSpecies(species),
			'hp': hp,
			'boxLevel': box_level,
			'status': StatusChange(status),
			'type_a': Type(type_a),
			'type_b': Type(type_b),
			'moves': list(map(AvailableMove, moves)),
			'level': level,
			'max_hp': max_hp,
			'attack': attack,
			'defense': defense,
			'speed': speed,
			'special': special,
			'pps': pps,
			'base_max_hp': base_max_hp,
			'base_attack': base_attack,
			'base_defense': base_defense,
			'base_speed': base_speed,
			'base_special': base_special,
			'substitute': self.read(sub_addr),
			'status_flags': self.read_range(status1_addr, status1_addr + 3)
		}


	def get_emulator_basic_state(self):
		return (
			self.get_basic_mon_state(self.symbol("wBattleMonNick"), self.symbol("wPlayerMonUnmodifiedMaxHP"), self.symbol("wPlayerSubstituteHP"), self.symbol("wPlayerBattleStatus1")),
			self.get_basic_mon_state(self.symbol("wEnemyMonNick"), self.symbol("wEnemyMonUnmodifiedMaxHP"), self.symbol("wEnemySubstituteHP"), self.symbol("wEnemyBattleStatus1")),
			self.read(self.symbol("wLinkBattleRandomNumberListIndex")),
			self.read_range(self.symbol("wLinkBattleRandomNumberList"), self.symbol("wLinkBattleRandomNumberList")+9)
		)


	@staticmethod
	def dump_basic_state(s):
		r = f'?????????? ({s['species'].name: >10}) l{s['level']: >3d}, {type_to_string_short(s['type_a'])}'
		if s['type_a'] != s['type_b']:
			r += f'/{type_to_string_short(s['type_b'])}, '
		else:
			r += "    , "
		r += f'{s['hp']: >3d}/{s['max_hp']: >3d}HP, '
		r += f'{s['attack']: >3d}ATK ({s['base_attack']: >3d}@+?), '
		r += f'{s['defense']: >3d}DEF ({s['base_defense']: >3d}@+?), '
		r += f'{s['special']: >3d}SPE ({s['base_special']: >3d}@+?), '
		r += f'{s['speed']: >3d}SPD ({s['base_speed']: >3d}@+?), '
		r += f'???%ACC (+?), '
		r += f'???%EVD (+?), '
		r += f'Status: 0x{s['status']:04d} {status_to_string_short(s['status']): >6}, '
		if s['status_flags'][1] & (1 << HAS_SUBSTITUTE_UP):
			r += f"Sub {s['substitute']}HP, "
		r += f'Moves: {", ".join(f'{m.name: >12} {s['pps'][i]: >2}/??PP' for i, m in enumerate(s['moves']) if m)}'
		return r


	@staticmethod
	def compare_basic_states(battle_state: BattleState, emu_state):
		me_b = battle_state.me.pokemon_on_field
		op_b = battle_state.op.pokemon_on_field
		rn_b = battle_state.rng.index
		rl_b = battle_state.rng.list
		me_e = emu_state[0]
		op_e = emu_state[1]
		rn_e = emu_state[2]
		rl_e = emu_state[3]
		errors = []

		if rn_b != rn_e:
			errors.append(f"RNG index b.{rn_b} vs e.{rn_e}")
		if rl_b != rl_e:
			errors.append(f"RNG list b.{rl_b} vs e.{rl_e}")

		if me_b.types[0] != me_e['type_a']:
			errors.append(f"P1 Type 1 b.{type_to_string(me_b.types[0])} vs e.{type_to_string(me_e['type_a'])}")
		if me_b.types[1] != me_e['type_b']:
			errors.append(f"P1 Type 2 b.{type_to_string(me_b.types[1])} vs e.{type_to_string(me_e['type_b'])}")
		if me_b.non_volatile_status != me_e['status'] and me_b.health:
			errors.append(f"P1 Status b.{me_b.non_volatile_status} ({StatusChange(me_b.non_volatile_status).name}) vs e.{me_e['status']} ({StatusChange(me_e['status']).name})")
		if me_b.health != me_e['hp']:
			errors.append(f"P1 Health b.{me_b.health} vs e.{me_e['hp']}")
		has_substitute = ((me_e['status_flags'][1] & (1 << HAS_SUBSTITUTE_UP)) != 0)
		if (me_b.substitute is not None) != has_substitute:
			errors.append(f"P1 Has substitute b.{me_b.substitute is not None} vs e.{has_substitute}")
		if me_b.substitute is not None and has_substitute and me_b.substitute != me_e['substitute']:
			errors.append(f"P1 Substitute health b.{me_b.substitute} vs e.{me_e['substitute']}")
		if me_b.attack != me_e['attack']:
			errors.append(f"P1 Attack b.{me_b.attack} vs e.{me_e['attack']}")
		if me_b.defense != me_e['defense']:
			errors.append(f"P1 Defense b.{me_b.defense} vs e.{me_e['defense']}")
		if me_b.special != me_e['special']:
			errors.append(f"P1 Special b.{me_b.special} vs e.{me_e['special']}")
		if me_b.speed != me_e['speed']:
			errors.append(f"P1 Speed b.{me_b.speed} vs e.{me_e['speed']}")
		pps = [m.pp for m in me_b.move_set]
		if pps != me_e['pps']:
			errors.append(f"P1 PPs b.{pps} vs e.{me_e['pps']}")

		if op_b.types[0] != op_e['type_a']:
			errors.append(f"P2 Type 1 b.{type_to_string(op_b.types[0])} vs e.{type_to_string(op_e['type_a'])}")
		if op_b.types[1] != op_e['type_b']:
			errors.append(f"P2 Type 2 b.{type_to_string(op_b.types[1])} vs e.{type_to_string(op_e['type_b'])}")
		if op_b.non_volatile_status != op_e['status'] and op_b.health:
			errors.append(f"P2 Status b.{op_b.non_volatile_status} ({StatusChange(op_b.non_volatile_status).name}) vs e.{op_e['status']} ({StatusChange(op_e['status']).name})")
		if op_b.health != op_e['hp']:
			errors.append(f"P2 Health b.{op_b.health} vs e.{op_e['hp']}")
		has_substitute = ((op_e['status_flags'][1] & (1 << HAS_SUBSTITUTE_UP)) != 0)
		if (op_b.substitute is not None) != has_substitute:
			errors.append(f"P2 Has substitute b.{op_b.substitute is not None} vs e.{has_substitute}")
		if op_b.substitute is not None and has_substitute and op_b.substitute != op_e['substitute']:
			errors.append(f"P2 Substitute health b.{op_b.substitute} vs e.{op_e['substitute']}")
		if op_b.attack != op_e['attack']:
			errors.append(f"P2 Attack b.{op_b.attack} vs e.{op_e['attack']}")
		if op_b.defense != op_e['defense']:
			errors.append(f"P2 Defense b.{op_b.defense} vs e.{op_e['defense']}")
		if op_b.special != op_e['special']:
			errors.append(f"P2 Special b.{op_b.special} vs e.{op_e['special']}")
		if op_b.speed != op_e['speed']:
			errors.append(f"P2 Speed b.{op_b.speed} vs e.{op_e['speed']}")
		return len(errors) == 0, errors


	def call_text_hook(self, _=None):
		if self.on_text_displayed is None or self.register_hl < 0xc4b9:
			return
		end, txt = self.translate_text_at(GBAddress(self.register_de))
		if self.waiting_text:
			if self.register_de == 0xc4b9:
				self.text_buffer = txt
			else:
				self.text_buffer += txt
			if end:
				if self.text_buffer != "!":
					self.on_text_displayed(self.text_buffer)
				self.text_buffer = ""
		else:
			self.text_buffer = ""


	def translate_text_at(self, address: GBAddress):
		s = ""
		end = False
		byte = self.read(address)
		while byte != CHAR_END:
			if byte == CHAR_LINE:
				s += " "
			elif byte == CHAR_PLAYER:
				end, v = self.translate_text_at(self.symbol("wPlayerName"))
				s += v
			elif byte == CHAR_RIVAL:
				end, v = self.translate_text_at(self.symbol("wRivalName"))
				s += v
			elif byte == CHAR_TARGET:
				turn = self.read(self.symbol("hWhoseTurn"))
				end, v = self.translate_text_at(self.symbol("wBattleMonNick") if turn else self.symbol("wEnemyMonNick"))
				s += v
			elif byte == CHAR_USER:
				turn = self.read(self.symbol("hWhoseTurn"))
				end, v = self.translate_text_at(self.symbol("wEnemyMonNick") if turn else self.symbol("wBattleMonNick"))
				s += v
			else:
				s += Pkmn1CharToASCIIConversionTable[byte]
			address += 1
			byte = self.read(address)
			if byte == CHAR_PROMPT or byte == CHAR_DONE:
				return True, s
		return end, s


	def copy_battle_data_to_emulator(self, state: PlayerState, team_base_address: GBAddress, name_address: GBAddress, species_array: GBAddress, name_list_address: GBAddress):
		self.write(species_array, len(state.team))
		for i, pkmn in enumerate(state.team):
			self.write(species_array + i + 1, pkmn.id)
			data = pkmn.encode()
			for k, b in enumerate(data):
				self.write(team_base_address + i * len(data) + k, b)
			data = EmulatorGameHandle.convert_string(pkmn.get_name(False))
			for j in range(11):
				if j < len(data):
					self.write(name_list_address + i * 11 + j, data[j])
				else:
					self.write(name_list_address + i * 11 + j, CHAR_END)
		self.write(species_array + len(state.team) + 1, 0xFF)
		data = EmulatorGameHandle.convert_string(state.name)
		for j in range(11):
			if j < len(data):
				self.write(name_address + j, data[j])
			else:
				self.write(name_address + j, CHAR_END)


	def sync_battle_state(self, state: BattleState):
		me = state.me.pokemon_on_field
		op = state.op.pokemon_on_field

		data = EmulatorGameHandle.convert_string(me.get_name(False))
		moves = me.move_set
		for i in range(11):
			if i < len(data):
				self.write(self.symbol("wBattleMonNick") + i, data[i])
			else:
				self.write(self.symbol("wBattleMonNick") + i, CHAR_END)
		self.write(self.symbol("wBattleMonLevel"),           me.level)
		self.write(self.symbol("wBattleMonBoxLevel"),        me.level)
		self.write(self.symbol("wPlayerMonUnmodifiedLevel"), me.level)
		self.write(self.symbol("wBattleMonStatus"),          me.non_volatile_status)
		self.write(self.symbol("wBattleMonType1"),           me.types[0])
		self.write(self.symbol("wBattleMonType2"),           me.types[1])
		self.write(self.symbol("wBattleMonSpecies"),         me.id)
		self.write(self.symbol("wBattleMonHP")      + 0,     me.health >> 8)
		self.write(self.symbol("wBattleMonHP")      + 1,     me.health & 0xFF)
		self.write(self.symbol("wBattleMonMaxHP")   + 0,     me.max_health >> 8)
		self.write(self.symbol("wBattleMonMaxHP")   + 1,     me.max_health & 0xFF)
		self.write(self.symbol("wBattleMonAttack")  + 0,     me.attack >> 8)
		self.write(self.symbol("wBattleMonAttack")  + 1,     me.attack & 0xFF)
		self.write(self.symbol("wBattleMonDefense") + 0,     me.defense >> 8)
		self.write(self.symbol("wBattleMonDefense") + 1,     me.defense & 0xFF)
		self.write(self.symbol("wBattleMonSpeed")   + 0,     me.speed >> 8)
		self.write(self.symbol("wBattleMonSpeed")   + 1,     me.speed & 0xFF)
		self.write(self.symbol("wBattleMonSpecial") + 0,     me.special >> 8)
		self.write(self.symbol("wBattleMonSpecial") + 1,     me.special & 0xFF)
		self.write(self.symbol("wPlayerMonUnmodifiedMaxHP")   + 0, me.max_health >> 8)
		self.write(self.symbol("wPlayerMonUnmodifiedMaxHP")   + 1, me.max_health & 0xFF)
		self.write(self.symbol("wPlayerMonUnmodifiedAttack")  + 0, me.raw_attack >> 8)
		self.write(self.symbol("wPlayerMonUnmodifiedAttack")  + 1, me.raw_attack & 0xFF)
		self.write(self.symbol("wPlayerMonUnmodifiedDefense") + 0, me.raw_defense >> 8)
		self.write(self.symbol("wPlayerMonUnmodifiedDefense") + 1, me.raw_defense & 0xFF)
		self.write(self.symbol("wPlayerMonUnmodifiedSpeed")   + 0, me.raw_speed >> 8)
		self.write(self.symbol("wPlayerMonUnmodifiedSpeed")   + 1, me.raw_speed & 0xFF)
		self.write(self.symbol("wPlayerMonUnmodifiedSpecial") + 0, me.raw_special >> 8)
		self.write(self.symbol("wPlayerMonUnmodifiedSpecial") + 1, me.raw_special & 0xFF)
		for i in range(4):
			if i < len(moves):
				self.write(self.symbol("wBattleMonMoves") + i, moves[i].id)
				self.write(self.symbol("wBattleMonPP")    + i, moves[i].pp)
			else:
				self.write(self.symbol("wBattleMonMoves") + i, AvailableMove.Empty)
				self.write(self.symbol("wBattleMonPP")    + i, 0)

		data = EmulatorGameHandle.convert_string(op.name)
		moves = op.move_set
		for i in range(11):
			if i < len(data):
				self.write(self.symbol("wEnemyMonNick") + i, data[i])
			else:
				self.write(self.symbol("wEnemyMonNick") + i, CHAR_END)
		self.write(self.symbol("wEnemyMonLevel"),           op.level)
		self.write(self.symbol("wEnemyMonBoxLevel"),        op.level)
		self.write(self.symbol("wEnemyMonUnmodifiedLevel"), op.level)
		self.write(self.symbol("wEnemyMonStatus"),          op.non_volatile_status)
		self.write(self.symbol("wEnemyMonType1"),           op.types[0])
		self.write(self.symbol("wEnemyMonType2"),           op.types[1])
		self.write(self.symbol("wEnemyMonSpecies"),         op.id)
		self.write(self.symbol("wEnemyMonHP")      + 0,     op.health >> 8)
		self.write(self.symbol("wEnemyMonHP")      + 1,     op.health & 0xFF)
		self.write(self.symbol("wEnemyMonMaxHP")   + 0,     op.max_health >> 8)
		self.write(self.symbol("wEnemyMonMaxHP")   + 1,     op.max_health & 0xFF)
		self.write(self.symbol("wEnemyMonAttack")  + 0,     op.attack >> 8)
		self.write(self.symbol("wEnemyMonAttack")  + 1,     op.attack & 0xFF)
		self.write(self.symbol("wEnemyMonDefense") + 0,     op.defense >> 8)
		self.write(self.symbol("wEnemyMonDefense") + 1,     op.defense & 0xFF)
		self.write(self.symbol("wEnemyMonSpeed")   + 0,     op.speed >> 8)
		self.write(self.symbol("wEnemyMonSpeed")   + 1,     op.speed & 0xFF)
		self.write(self.symbol("wEnemyMonSpecial") + 0,     op.special >> 8)
		self.write(self.symbol("wEnemyMonSpecial") + 1,     op.special & 0xFF)
		self.write(self.symbol("wEnemyMonUnmodifiedMaxHP")   + 0, op.max_health >> 8)
		self.write(self.symbol("wEnemyMonUnmodifiedMaxHP")   + 1, op.max_health & 0xFF)
		self.write(self.symbol("wEnemyMonUnmodifiedAttack")  + 0, op.raw_attack >> 8)
		self.write(self.symbol("wEnemyMonUnmodifiedAttack")  + 1, op.raw_attack & 0xFF)
		self.write(self.symbol("wEnemyMonUnmodifiedDefense") + 0, op.raw_defense >> 8)
		self.write(self.symbol("wEnemyMonUnmodifiedDefense") + 1, op.raw_defense & 0xFF)
		self.write(self.symbol("wEnemyMonUnmodifiedSpeed")   + 0, op.raw_speed >> 8)
		self.write(self.symbol("wEnemyMonUnmodifiedSpeed")   + 1, op.raw_speed & 0xFF)
		self.write(self.symbol("wEnemyMonUnmodifiedSpecial") + 0, op.raw_special >> 8)
		self.write(self.symbol("wEnemyMonUnmodifiedSpecial") + 1, op.raw_special & 0xFF)
		for i in range(4):
			if i < len(moves):
				self.write(self.symbol("wEnemyMonMoves") + i, moves[i].id)
				self.write(self.symbol("wEnemyMonPP")    + i, moves[i].pp)
			else:
				self.write(self.symbol("wEnemyMonMoves") + i, AvailableMove.Empty)
				self.write(self.symbol("wEnemyMonPP")    + i, 0)


	def wait_for_start_turn(self):
		self.turn_started = False
		self.wait_input = False
		self.waiting = False
		while True:
			if self.battle_finished or self.turn_started or self.wait_input or self.waiting:
				return
			self.tick()


	def select_action(self, action: BattleAction):
		menu_item = self.symbol("wCurrentMenuItem")
		team_length = self.read(self.symbol("wPartyCount"))
		if self.is_dead:
			assert BattleAction.Switch1 <= action <= BattleAction.Switch6
			self.tick(20)
			target = action - BattleAction.Switch1
			while self.read(menu_item) != target:
				diff = (team_length + self.read(menu_item) - target) % team_length
				self.press_button('up' if diff < team_length // 2 else 'down', 5)
				self.tick(10)
			self.press_button('a', 5)
			self.tick(10)
			return
		self.tick(10)
		if BattleAction.Run == action:
			self.press_button('right', 5)
			self.press_button('down', 5)
		elif BattleAction.Switch1 <= action <= BattleAction.Switch6:
			self.press_button('right', 5)
			self.press_button('up', 5)
		else:
			self.press_button('left', 5)
			self.press_button('up', 5)
		self.tick(10)
		self.press_button('a', 5)
		self.tick(20)
		if BattleAction.Switch1 <= action <= BattleAction.Switch6:
			target = action - BattleAction.Switch1
			diff = (team_length + self.read(menu_item) - target) % team_length
			while self.read(menu_item) != target:
				self.press_button('up' if diff < team_length // 2 else 'down', 5)
				self.tick(10)
			self.press_button('a', 5)
			self.tick(10)
			self.press_button('a', 5)
			self.tick(10)
		elif BattleAction.Attack1 <= action <= BattleAction.Attack4:
			target = action - BattleAction.Attack1 + 1
			diff = (4 + self.read(menu_item) - target) % 4
			while self.read(menu_item) != target and not self.waiting:
				self.press_button('up' if diff < 2 else 'down', 5)
				self.tick(10)
			self.press_button('a', 5)
			self.tick(10)


	def step(self, state: BattleState, fast_forward: bool=False):
		try:
			self.fast_forward = fast_forward
			if not self.waiting:
				self.select_action(state.me.last_action)
			while not self.waiting:
				self.tick()
			recv_data = self.symbol("wSerialExchangeNybbleReceiveData")
			while self.read(recv_data) == 0xFF:
				self.write(recv_data, state.op.last_action - BattleAction.Attack1)
				self.tick()
			self.waiting_text = True
			self.wait_for_start_turn()
			self.waiting_text = False
			self.text_buffer = ""
		finally:
			self.fast_forward = False


	def run_until(self, address: GBAddress):
		wait = True
		def target(_):
			nonlocal wait
			wait = False
		self.hook_single(address, target)
		while wait:
			self.tick()


	def push(self, value):
		self.register_sp += 2
		stack = GBAddress(self.register_sp)
		self.write(stack, value & 0xFF)
		self.write(stack + 1, value >> 8)


	def find_instr(self, data: list[int], base: GBAddress):
		ptr = base + len(data)
		buffer = self.read_range(base, ptr)
		while data != buffer and [0xDB] + data[1:] != buffer:
			buffer.pop(0)
			buffer.append(self.read(ptr))
			ptr += 1
		ptr -= len(data)
		return ptr


	def hook_single(self, address: GBAddress, callback: Callable, data=None):
		def stub(f):
			self.unhook(address)
			callback(f)
		self.hook(address, stub, data)


	def init_battle(self, save_state_fd, state: BattleState, sync_data: bool=False, fast_forward: bool=False, trainer: TrainerClass|None=None):
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
				if trainer is not None:
					self.write(self.symbol("wCurOpponent"), trainer + 200)
				symbol = self.symbol("InitBattle")
				self.register_hl = symbol.address
				self.register_b = symbol.bank
				self.push(self.symbol("Bankswitch").address)
				# self.write(self.symbol("wLinkState"), LINK_STATE_BATTLING) -> Removes music

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
			if sync_data:
				self.sync_battle_state(state)
			self.waiting_text = False
			self.text_buffer = ""
		finally:
			self.fast_forward = False


	def get_last_frames(self):
		old = self.last_frames
		self.last_frames = []
		return old


	def is_finished(self):
		return self.read_range(GBAddress(0x9C00), GBAddress(0x9C20)) == t_oak_lab
