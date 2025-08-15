# distutils: language = c++

from libcpp cimport bool
from libcpp.pair cimport pair
from libcpp.functional cimport function
from libcpp.vector cimport vector
from libcpp.string cimport string
from cython cimport cast, fused_type, NULL
from cython.operator cimport dereference
from StatusChange cimport StatusChange as __StatusChange, statusToString as __statusToString, STATUS_NONE, STATUS_ASLEEP_FOR_1_TURN, STATUS_ASLEEP_FOR_2_TURN, STATUS_ASLEEP_FOR_3_TURN, STATUS_ASLEEP_FOR_4_TURN, STATUS_ASLEEP_FOR_5_TURN, STATUS_ASLEEP_FOR_6_TURN, STATUS_ASLEEP_FOR_7_TURN, STATUS_ASLEEP, STATUS_POISONED, STATUS_BURNED, STATUS_FROZEN, STATUS_PARALYZED, STATUS_KO, STATUS_BADLY_POISONED, STATUS_ANY_NON_VOLATILE_STATUS, STATUS_LEECHED, STATUS_CONFUSED_FOR_1_TURN, STATUS_CONFUSED_FOR_2_TURN, STATUS_CONFUSED_FOR_3_TURN, STATUS_CONFUSED_FOR_4_TURN, STATUS_CONFUSED, STATUS_FLINCHED
from StatsChange cimport StatsChange as __StatsChange, statToString as __statToString, statToLittleString as __statToLittleString, STATS_ATK, STATS_DEF, STATS_SPD, STATS_SPE, STATS_EVD, STATS_ACC
from Type cimport Type as __Type, typeToString as __typeToString, typeToStringShort as __typeToStringShort, getAttackDamageMultiplier as __getAttackDamageMultiplier, TYPE_NORMAL, TYPE_FIGHTING, TYPE_FLYING, TYPE_POISON, TYPE_GROUND, TYPE_ROCK, TYPE_INVALID, TYPE_BUG, TYPE_GHOST, TYPE_FIRE, TYPE_WATER, TYPE_GRASS, TYPE_ELECTRIC, TYPE_PSYCHIC, TYPE_ICE, TYPE_DRAGON, TYPE_NEUTRAL_PHYSICAL, TYPE_NEUTRAL_SPECIAL
from RandomGenerator cimport RandomGenerator as __RandomGenerator
from Move cimport Move as __Move, AvailableMove as __AvailableMove, availableMoves, Empty as __Empty, Pound as __Pound, Karate_Chop as __Karate_Chop, Double_Slap as __Double_Slap, Comet_Punch as __Comet_Punch, Mega_Punch as __Mega_Punch, Pay_Day as __Pay_Day, Fire_Punch as __Fire_Punch, Ice_Punch as __Ice_Punch, Thunder_Punch as __Thunder_Punch, Scratch as __Scratch, Vise_Grip as __Vise_Grip, Guillotine as __Guillotine, Razor_Wind as __Razor_Wind, Swords_Dance as __Swords_Dance, Cut as __Cut, Gust as __Gust, Wing_Attack as __Wing_Attack, Whirlwind as __Whirlwind, Fly as __Fly, Bind as __Bind, Slam as __Slam, Vine_Whip as __Vine_Whip, Stomp as __Stomp, Double_Kick as __Double_Kick, Mega_Kick as __Mega_Kick, Jump_Kick as __Jump_Kick, Rolling_Kick as __Rolling_Kick, Sand_Attack as __Sand_Attack, Headbutt as __Headbutt, Horn_Attack as __Horn_Attack, Fury_Attack as __Fury_Attack, Horn_Drill as __Horn_Drill, Tackle as __Tackle, Body_Slam as __Body_Slam, Wrap as __Wrap, Take_Down as __Take_Down, Thrash as __Thrash, Double_Edge as __Double_Edge, Tail_Whip as __Tail_Whip, Poison_Sting as __Poison_Sting, Twineedle as __Twineedle, Pin_Missile as __Pin_Missile, Leer as __Leer, Bite as __Bite, Growl as __Growl, Roar as __Roar, Sing as __Sing, Supersonic as __Supersonic, Sonic_Boom as __Sonic_Boom, Disable as __Disable, Acid as __Acid, Ember as __Ember, Flamethrower as __Flamethrower, Mist as __Mist, Water_Gun as __Water_Gun, Hydro_Pump as __Hydro_Pump, Surf as __Surf, Ice_Beam as __Ice_Beam, Blizzard as __Blizzard, Psybeam as __Psybeam, Bubble_Beam as __Bubble_Beam, Aurora_Beam as __Aurora_Beam, Hyper_Beam as __Hyper_Beam, Peck as __Peck, Drill_Peck as __Drill_Peck, Submission as __Submission, Low_Kick as __Low_Kick, Counter as __Counter, Seismic_Toss as __Seismic_Toss, Strength as __Strength, Absorb as __Absorb, Mega_Drain as __Mega_Drain, Leech_Seed as __Leech_Seed, Growth as __Growth, Razor_Leaf as __Razor_Leaf, Solar_Beam as __Solar_Beam, Poison_Powder as __Poison_Powder, Stun_Spore as __Stun_Spore, Sleep_Powder as __Sleep_Powder, Petal_Dance as __Petal_Dance, String_Shot as __String_Shot, Dragon_Rage as __Dragon_Rage, Fire_Spin as __Fire_Spin, Thundershock as __Thundershock, Thunderbolt as __Thunderbolt, Thunder_Wave as __Thunder_Wave, Thunder as __Thunder, Rock_Throw as __Rock_Throw, Earthquake as __Earthquake, Fissure as __Fissure, Dig as __Dig, Toxic as __Toxic, Confusion as __Confusion, Psychic_M as __Psychic_M, Hypnosis as __Hypnosis, Meditate as __Meditate, Agility as __Agility, Quick_Attack as __Quick_Attack, Rage as __Rage, Teleport as __Teleport, Night_Shade as __Night_Shade, Mimic as __Mimic, Screech as __Screech, Double_Team as __Double_Team, Recover as __Recover, Harden as __Harden, Minimize as __Minimize, Smokescreen as __Smokescreen, Confuse_Ray as __Confuse_Ray, Withdraw as __Withdraw, Defense_Curl as __Defense_Curl, Barrier as __Barrier, Light_Screen as __Light_Screen, Haze as __Haze, Reflect as __Reflect, Focus_Energy as __Focus_Energy, Bide as __Bide, Metronome as __Metronome, Mirror_Move as __Mirror_Move, Self_Destruct as __Self_Destruct, Egg_Bomb as __Egg_Bomb, Lick as __Lick, Smog as __Smog, Sludge as __Sludge, Bone_Club as __Bone_Club, Fire_Blast as __Fire_Blast, Waterfall as __Waterfall, Clamp as __Clamp, Swift as __Swift, Skull_Bash as __Skull_Bash, Spike_Cannon as __Spike_Cannon, Constrict as __Constrict, Amnesia as __Amnesia, Kinesis as __Kinesis, Soft_Boiled as __Soft_Boiled, Hi_Jump_Kick as __Hi_Jump_Kick, Glare as __Glare, Dream_Eater as __Dream_Eater, Poison_Gas as __Poison_Gas, Barrage as __Barrage, Leech_Life as __Leech_Life, Lovely_Kiss as __Lovely_Kiss, Sky_Attack as __Sky_Attack, Transform as __Transform, Bubble as __Bubble, Dizzy_Punch as __Dizzy_Punch, Spore as __Spore, Flash as __Flash, Psywave as __Psywave, Splash as __Splash, Acid_Armor as __Acid_Armor, Crabhammer as __Crabhammer, Explosion as __Explosion, Fury_Swipes as __Fury_Swipes, Bonemerang as __Bonemerang, Rest as __Rest, Rock_Slide as __Rock_Slide, Hyper_Fang as __Hyper_Fang, Sharpen as __Sharpen, Conversion as __Conversion, Tri_Attack as __Tri_Attack, Super_Fang as __Super_Fang, Slash as __Slash, Substitute as __Substitute, Struggle as __Struggle, PHYSICAL as __PHYSICAL, SPECIAL as __SPECIAL, STATUS as __STATUS
from Pokemon cimport DataArray, pokemonList as __pokemonList, Pokemon as __Pokemon, pythonLoggerLambda, evalLogger, Rhydon as __Rhydon, Kangaskhan as __Kangaskhan, Nidoran_M as __Nidoran_M, Clefairy as __Clefairy, Spearow as __Spearow, Voltorb as __Voltorb, Nidoking as __Nidoking, Slowbro as __Slowbro, Ivysaur as __Ivysaur, Exeggutor as __Exeggutor, Lickitung as __Lickitung, Exeggcute as __Exeggcute, Grimer as __Grimer, Gengar as __Gengar, Nidoran_F as __Nidoran_F, Nidoqueen as __Nidoqueen, Cubone as __Cubone, Rhyhorn as __Rhyhorn, Lapras as __Lapras, Arcanine as __Arcanine, Mew as __Mew, Gyarados as __Gyarados, Shellder as __Shellder, Tentacool as __Tentacool, Gastly as __Gastly, Scyther as __Scyther, Staryu as __Staryu, Blastoise as __Blastoise, Pinsir as __Pinsir, Tangela as __Tangela, Growlithe as __Growlithe, Onix as __Onix, Fearow as __Fearow, Pidgey as __Pidgey, Slowpoke as __Slowpoke, Kadabra as __Kadabra, Graveler as __Graveler, Chansey as __Chansey, Machoke as __Machoke, Mr_Mime as __Mr_Mime, Hitmonlee as __Hitmonlee, Hitmonchan as __Hitmonchan, Arbok as __Arbok, Parasect as __Parasect, Psyduck as __Psyduck, Drowzee as __Drowzee, Golem as __Golem, Magmar as __Magmar, Electabuzz as __Electabuzz, Magneton as __Magneton, Koffing as __Koffing, Mankey as __Mankey, Seel as __Seel, Diglett as __Diglett, Tauros as __Tauros, Farfetchd as __Farfetchd, Venonat as __Venonat, Dragonite as __Dragonite, Doduo as __Doduo, Poliwag as __Poliwag, Jynx as __Jynx, Moltres as __Moltres, Articuno as __Articuno, Zapdos as __Zapdos, Ditto as __Ditto, Meowth as __Meowth, Krabby as __Krabby, Vulpix as __Vulpix, Ninetales as __Ninetales, Pikachu as __Pikachu, Raichu as __Raichu, Dratini as __Dratini, Dragonair as __Dragonair, Kabuto as __Kabuto, Kabutops as __Kabutops, Horsea as __Horsea, Seadra as __Seadra, Sandshrew as __Sandshrew, Sandslash as __Sandslash, Omanyte as __Omanyte, Omastar as __Omastar, Jigglypuff as __Jigglypuff, Wigglytuff as __Wigglytuff, Eevee as __Eevee, Flareon as __Flareon, Jolteon as __Jolteon, Vaporeon as __Vaporeon, Machop as __Machop, Zubat as __Zubat, Ekans as __Ekans, Paras as __Paras, Poliwhirl as __Poliwhirl, Poliwrath as __Poliwrath, Weedle as __Weedle, Kakuna as __Kakuna, Beedrill as __Beedrill, Dodrio as __Dodrio, Primeape as __Primeape, Dugtrio as __Dugtrio, Venomoth as __Venomoth, Dewgong as __Dewgong, Caterpie as __Caterpie, Metapod as __Metapod, Butterfree as __Butterfree, Machamp as __Machamp, Golduck as __Golduck, Hypno as __Hypno, Golbat as __Golbat, Mewtwo as __Mewtwo, Snorlax as __Snorlax, Magikarp as __Magikarp, Muk as __Muk, Kingler as __Kingler, Cloyster as __Cloyster, Electrode as __Electrode, Clefable as __Clefable, Weezing as __Weezing, Persian as __Persian, Marowak as __Marowak, Haunter as __Haunter, Abra as __Abra, Alakazam as __Alakazam, Pidgeotto as __Pidgeotto, Pidgeot as __Pidgeot, Starmie as __Starmie, Bulbasaur as __Bulbasaur, Venusaur as __Venusaur, Tentacruel as __Tentacruel, Goldeen as __Goldeen, Seaking as __Seaking, Ponyta as __Ponyta, Rapidash as __Rapidash, Rattata as __Rattata, Raticate as __Raticate, Nidorino as __Nidorino, Nidorina as __Nidorina, Geodude as __Geodude, Porygon as __Porygon, Aerodactyl as __Aerodactyl, Magnemite as __Magnemite, Charmander as __Charmander, Squirtle as __Squirtle, Charmeleon as __Charmeleon, Wartortle as __Wartortle, Charizard as __Charizard, Oddish as __Oddish, Gloom as __Gloom, Vileplume as __Vileplume, Bellsprout as __Bellsprout, Weepinbell as __Weepinbell, Victreebel as __Victreebel, Missingno as __Missingno
from Team cimport loadTrainer as __loadTrainer, saveTrainer as __saveTrainer, Trainer
from State cimport BattleAction as __BattleAction, BattleActionToString as __BattleActionToString, PlayerState as __PlayerState, BattleState as __BattleState, pythonCallbackLambda, pythonCallbackLambdaVoid, evalCallback, evalCallbackVoid, NoAction as __NoAction, Attack1 as __Attack1, Attack2 as __Attack2, Attack3 as __Attack3, Attack4 as __Attack4, Switch1 as __Switch1, Switch2 as __Switch2, Switch3 as __Switch3, Switch4 as __Switch4, Switch5 as __Switch5, Switch6 as __Switch6, StruggleMove as __StruggleMove, Run as __Run
from BattleHandler cimport BattleHandler as __BattleHandler

def loadTrainer(data, BattleState state):
	trainer = __loadTrainer(data, state.__instance.rng, state.__instance.battleLogger)
	result = []
	for index in range(trainer.second.size()):
		p = Pokemon()
		p.__instance = new __Pokemon(trainer.second[index])
		result.append(p)
	return (trainer.first.decode('ASCII'), result)

def saveTrainer(trainerPy):
	cdef Trainer trainer

	trainer.first = trainerPy[0]
	for pkmn in trainerPy[1]:
		trainer.second.push_back(dereference((<Pokemon>pkmn).__instance))
	return __saveTrainer(trainer)


cpdef enum Type:
	Normal = TYPE_NORMAL
	Fighting = TYPE_FIGHTING
	Flying = TYPE_FLYING
	Poison = TYPE_POISON
	Ground = TYPE_GROUND
	Rock = TYPE_ROCK
	Invalid = TYPE_INVALID
	Bug = TYPE_BUG
	Ghost = TYPE_GHOST
	Fire = TYPE_FIRE
	Water = TYPE_WATER
	Grass = TYPE_GRASS
	Electric = TYPE_ELECTRIC
	Psychic = TYPE_PSYCHIC
	Ice = TYPE_ICE
	Dragon = TYPE_DRAGON
	Neutral_Physical = TYPE_NEUTRAL_PHYSICAL
	Neutral_Special = TYPE_NEUTRAL_SPECIAL

def typeToString(s: Type):
	return __typeToString(s).decode('ASCII')

def typeToStringShort(s: Type):
	return __typeToStringShort(s).decode('ASCII')

def getAttackDamageMultiplier(__Type m, __Type d):
	return __getAttackDamageMultiplier(m, d)

def getAttackDamageMultiplier(__Type m, d):
	if isinstance(d, tuple):
		return __getAttackDamageMultiplier(m, cast(pair[__Type, __Type], d))
	return __getAttackDamageMultiplier(m, cast(__Type, d))



cpdef enum StatusChange:
	OK = STATUS_NONE
	Asleep_for_1_turn = STATUS_ASLEEP_FOR_1_TURN
	Asleep_for_2_turn = STATUS_ASLEEP_FOR_2_TURN
	Asleep_for_3_turn = STATUS_ASLEEP_FOR_3_TURN
	Asleep_for_4_turn = STATUS_ASLEEP_FOR_4_TURN
	Asleep_for_5_turn = STATUS_ASLEEP_FOR_5_TURN
	Asleep_for_6_turn = STATUS_ASLEEP_FOR_6_TURN
	Asleep_for_7_turn = STATUS_ASLEEP_FOR_7_TURN
	Asleep = STATUS_ASLEEP
	Poisoned = STATUS_POISONED
	Burned = STATUS_BURNED
	Frozen = STATUS_FROZEN
	Paralyzed = STATUS_PARALYZED
	KO = STATUS_KO
	Badly_poisoned = STATUS_BADLY_POISONED
	Any_non_volatile_status = STATUS_ANY_NON_VOLATILE_STATUS
	Leeched = STATUS_LEECHED
	Confused_for_1_turn = STATUS_CONFUSED_FOR_1_TURN
	Confused_for_2_turn = STATUS_CONFUSED_FOR_2_TURN
	Confused_for_3_turn = STATUS_CONFUSED_FOR_3_TURN
	Confused_for_4_turn = STATUS_CONFUSED_FOR_4_TURN
	Confused = STATUS_CONFUSED
	Flinched = STATUS_FLINCHED

def statusToString(s):
	return __statusToString(s).decode('ASCII')



cpdef enum StatsChange:
	Atk = STATS_ATK
	Def = STATS_DEF
	Spd = STATS_SPD
	Spe = STATS_SPE
	Evd = STATS_EVD
	Acc = STATS_ACC

def statToString(s):
	return __statToString(s).decode('ASCII')

def statToLittleString(s):
	return __statToLittleString(s).decode('ASCII')



cdef class RandomGenerator:
	cdef __RandomGenerator *__instance
	cdef bool __allocd

	def __cinit__(self):
		self.__instance = NULL
		self.__allocd = False

	def __dealloc__(self):
		if self.__allocd:
			del self.__instance

	def __init__(self, alloc=True):
		if not alloc:
			self.__instance = new __RandomGenerator()
			self.__allocd = True

	def makeRandomList(self, unsigned int size):
		self.__instance.makeRandomList(size)

	def setList(self, list):
		self.__instance.setList(list)

	def getList(self):
		return self.__instance.getList()

	def getIndex(self):
		return self.__instance.getIndex()

	def setIndex(self, unsigned index):
		return self.__instance.setIndex(index)

	def reset(self):
		return self.__instance.reset()

	def __call__(self):
		return dereference(self.__instance)()


cpdef enum AvailableMove:
	Empty = __Empty
	Pound = __Pound
	Karate_Chop = __Karate_Chop
	Double_Slap = __Double_Slap
	Comet_Punch = __Comet_Punch
	Mega_Punch = __Mega_Punch
	Pay_Day = __Pay_Day
	Fire_Punch = __Fire_Punch
	Ice_Punch = __Ice_Punch
	Thunder_Punch = __Thunder_Punch
	Scratch = __Scratch
	Vise_Grip = __Vise_Grip
	Guillotine = __Guillotine
	Razor_Wind = __Razor_Wind
	Swords_Dance = __Swords_Dance
	Cut = __Cut
	Gust = __Gust
	Wing_Attack = __Wing_Attack
	Whirlwind = __Whirlwind
	Fly = __Fly
	Bind = __Bind
	Slam = __Slam
	Vine_Whip = __Vine_Whip
	Stomp = __Stomp
	Double_Kick = __Double_Kick
	Mega_Kick = __Mega_Kick
	Jump_Kick = __Jump_Kick
	Rolling_Kick = __Rolling_Kick
	Sand_Attack = __Sand_Attack
	Headbutt = __Headbutt
	Horn_Attack = __Horn_Attack
	Fury_Attack = __Fury_Attack
	Horn_Drill = __Horn_Drill
	Tackle = __Tackle
	Body_Slam = __Body_Slam
	Wrap = __Wrap
	Take_Down = __Take_Down
	Thrash = __Thrash
	Double_Edge = __Double_Edge
	Tail_Whip = __Tail_Whip
	Poison_Sting = __Poison_Sting
	Twineedle = __Twineedle
	Pin_Missile = __Pin_Missile
	Leer = __Leer
	Bite = __Bite
	Growl = __Growl
	Roar = __Roar
	Sing = __Sing
	Supersonic = __Supersonic
	Sonic_Boom = __Sonic_Boom
	Disable = __Disable
	Acid = __Acid
	Ember = __Ember
	Flamethrower = __Flamethrower
	Mist = __Mist
	Water_Gun = __Water_Gun
	Hydro_Pump = __Hydro_Pump
	Surf = __Surf
	Ice_Beam = __Ice_Beam
	Blizzard = __Blizzard
	Psybeam = __Psybeam
	Bubble_Beam = __Bubble_Beam
	Aurora_Beam = __Aurora_Beam
	Hyper_Beam = __Hyper_Beam
	Peck = __Peck
	Drill_Peck = __Drill_Peck
	Submission = __Submission
	Low_Kick = __Low_Kick
	Counter = __Counter
	Seismic_Toss = __Seismic_Toss
	Strength = __Strength
	Absorb = __Absorb
	Mega_Drain = __Mega_Drain
	Leech_Seed = __Leech_Seed
	Growth = __Growth
	Razor_Leaf = __Razor_Leaf
	Solar_Beam = __Solar_Beam
	Poison_Powder = __Poison_Powder
	Stun_Spore = __Stun_Spore
	Sleep_Powder = __Sleep_Powder
	Petal_Dance = __Petal_Dance
	String_Shot = __String_Shot
	Dragon_Rage = __Dragon_Rage
	Fire_Spin = __Fire_Spin
	Thundershock = __Thundershock
	Thunderbolt = __Thunderbolt
	Thunder_Wave = __Thunder_Wave
	Thunder = __Thunder
	Rock_Throw = __Rock_Throw
	Earthquake = __Earthquake
	Fissure = __Fissure
	Dig = __Dig
	Toxic = __Toxic
	Confusion = __Confusion
	Psychic_M = __Psychic_M
	Hypnosis = __Hypnosis
	Meditate = __Meditate
	Agility = __Agility
	Quick_Attack = __Quick_Attack
	Rage = __Rage
	Teleport = __Teleport
	Night_Shade = __Night_Shade
	Mimic = __Mimic
	Screech = __Screech
	Double_Team = __Double_Team
	Recover = __Recover
	Harden = __Harden
	Minimize = __Minimize
	Smokescreen = __Smokescreen
	Confuse_Ray = __Confuse_Ray
	Withdraw = __Withdraw
	Defense_Curl = __Defense_Curl
	Barrier = __Barrier
	Light_Screen = __Light_Screen
	Haze = __Haze
	Reflect = __Reflect
	Focus_Energy = __Focus_Energy
	Bide = __Bide
	Metronome = __Metronome
	Mirror_Move = __Mirror_Move
	Self_Destruct = __Self_Destruct
	Egg_Bomb = __Egg_Bomb
	Lick = __Lick
	Smog = __Smog
	Sludge = __Sludge
	Bone_Club = __Bone_Club
	Fire_Blast = __Fire_Blast
	Waterfall = __Waterfall
	Clamp = __Clamp
	Swift = __Swift
	Skull_Bash = __Skull_Bash
	Spike_Cannon = __Spike_Cannon
	Constrict = __Constrict
	Amnesia = __Amnesia
	Kinesis = __Kinesis
	Soft_Boiled = __Soft_Boiled
	Hi_Jump_Kick = __Hi_Jump_Kick
	Glare = __Glare
	Dream_Eater = __Dream_Eater
	Poison_Gas = __Poison_Gas
	Barrage = __Barrage
	Leech_Life = __Leech_Life
	Lovely_Kiss = __Lovely_Kiss
	Sky_Attack = __Sky_Attack
	Transform = __Transform
	Bubble = __Bubble
	Dizzy_Punch = __Dizzy_Punch
	Spore = __Spore
	Flash = __Flash
	Psywave = __Psywave
	Splash = __Splash
	Acid_Armor = __Acid_Armor
	Crabhammer = __Crabhammer
	Explosion = __Explosion
	Fury_Swipes = __Fury_Swipes
	Bonemerang = __Bonemerang
	Rest = __Rest
	Rock_Slide = __Rock_Slide
	Hyper_Fang = __Hyper_Fang
	Sharpen = __Sharpen
	Conversion = __Conversion
	Tri_Attack = __Tri_Attack
	Super_Fang = __Super_Fang
	Slash = __Slash
	Substitute = __Substitute
	Struggle = __Struggle

cpdef enum MoveCategory:
	Physical = __PHYSICAL
	Special = __SPECIAL
	Status = __STATUS

cdef class Move:
	cdef __Move *__instance
	cdef const __Move *__cinstance
	cdef bool __allocd

	def __cinit__(self):
		self.__instance = NULL
		self.__cinstance = NULL
		self.__allocd = False

	def __dealloc__(self):
		if self.__allocd:
			del self.__instance

	def __init__(self, *args):
		if self.__instance:
			del self.__instance
			self.__instance = NULL
		if len(args) == 0:
			return
		if len(args) == 1:
			object = args[0]
			if isinstance(object, int) or isinstance(object, AvailableMove):
				if object < 0 or object > 256:
					raise IndexError("list object out of range")
				self.__instance = new __Move(availableMoves[object])
				self.__cinstance = self.__instance
				self.__allocd = True
				return
			elif isinstance(object, Move):
				self.__instance = new __Move(dereference((<Move>object).__instance))
				self.__cinstance = self.__instance
				self.__allocd = True
				return
		raise TypeError("Invalid argument received")

	def copy(self):
		return Move(self)

	def __assign__(self, Move other):
		self.__instance[0] = other.__instance[0]
		return self

	def getNbRuns(self):
		return self.__cinstance.getNbRuns()

	def getNbHits(self):
		return self.__cinstance.getNbHits()

	def getStatusChange(self):
		cdef const __Move.StatusChangeProb *r = &self.__cinstance.getStatusChange()

		return {
			'status': StatusChange(r.status),
                        'cmpVal': r.cmpVal
		}

	def getOwnerChange(self):
		cdef const vector[__Move.StatsChangeProb] *r = &self.__cinstance.getOwnerChange()
		result = []

		for i in range(r.size()):
			result.append({
				'stat': StatsChange(dereference(r)[i].stat),
				'nb': dereference(r)[i].nb,
				'cmpVal': dereference(r)[i].cmpVal
			})
		return result

	def getFoeChange(self):
		cdef const vector[__Move.StatsChangeProb] *r = &self.__cinstance.getFoeChange()
		result = []

		for i in range(r.size()):
			result.append({
				'stat': StatsChange(dereference(r)[i].stat),
				'nb': dereference(r)[i].nb,
				'cmpVal': dereference(r)[i].cmpVal
			})
		return result

	def needsLoading(self):
		return self.__cinstance.needsLoading()

	def isInvulnerableDuringLoading(self):
		return self.__cinstance.isInvulnerableDuringLoading()

	def needsRecharge(self):
		return self.__cinstance.needsRecharge()

	def getHitCallBackDescription(self):
		return self.__cinstance.getHitCallBackDescription().decode('utf-8')

	def getMissCallBackDescription(self):
		return self.__cinstance.getMissCallBackDescription().decode('utf-8')

	def getCritChance(self):
		return self.__cinstance.getCritChance()

	def getAccuracy(self):
		return self.__cinstance.getAccuracy()

	def getCategory(self):
		return self.__cinstance.getCategory()

	def makesInvulnerable(self):
		return self.__cinstance.makesInvulnerable()

	def getMaxPP(self):
		return self.__cinstance.getMaxPP()

	def getPower(self):
		return self.__cinstance.getPower()

	def getPPUp(self):
		return self.__cinstance.getPPUp()

	def getType(self):
		return self.__cinstance.getType()

	def getPP(self):
		return self.__cinstance.getPP()

	def getID(self):
		return self.__cinstance.getID()

	def getName(self):
		return self.__cinstance.getName().decode('utf-8')

	def getDescription(self):
		return self.__cinstance.getDescription().decode('utf-8')

	def getPriority(self):
		return self.__cinstance.getPriority()

	def isFinished(self):
		return self.__cinstance.isFinished()

	def getHitsLeft(self):
		return self.__cinstance.getHitsLeft()

	def glitch(self):
		if self.__instance == NULL:
			raise TypeError("Instance is read only")
		self.__instance.glitch()

	def setPP(self, unsigned char pp):
		if self.__instance == NULL:
			raise TypeError("Instance is read only")
		self.__instance.setPP(pp)

	def setPPUp(self, unsigned char nb):
		if self.__instance == NULL:
			raise TypeError("Instance is read only")
		self.__instance.setPPUp(nb)

	def setHitsLeft(self, unsigned char nb):
		if self.__instance == NULL:
			raise TypeError("Instance is read only")
		self.__instance.setHitsLeft(nb)

	def reset(self):
		if self.__instance == NULL:
			raise TypeError("Instance is read only")
		self.__instance.reset()

	def attack(self, Pokemon owner, Pokemon target, logger):
		if self.__instance == NULL:
			raise TypeError("Instance is read only")
		self.__instance.attack(dereference(owner.__instance), dereference(target.__instance), pythonLoggerLambda(<void *>logger, &evalLogger))


cpdef enum PokemonSpecies:
	Rhydon = __Rhydon
	Kangaskhan = __Kangaskhan
	Nidoran_M = __Nidoran_M
	Clefairy = __Clefairy
	Spearow = __Spearow
	Voltorb = __Voltorb
	Nidoking = __Nidoking
	Slowbro = __Slowbro
	Ivysaur = __Ivysaur
	Exeggutor = __Exeggutor
	Lickitung = __Lickitung
	Exeggcute = __Exeggcute
	Grimer = __Grimer
	Gengar = __Gengar
	Nidoran_F = __Nidoran_F
	Nidoqueen = __Nidoqueen
	Cubone = __Cubone
	Rhyhorn = __Rhyhorn
	Lapras = __Lapras
	Arcanine = __Arcanine
	Mew = __Mew
	Gyarados = __Gyarados
	Shellder = __Shellder
	Tentacool = __Tentacool
	Gastly = __Gastly
	Scyther = __Scyther
	Staryu = __Staryu
	Blastoise = __Blastoise
	Pinsir = __Pinsir
	Tangela = __Tangela
	Growlithe = __Growlithe
	Onix = __Onix
	Fearow = __Fearow
	Pidgey = __Pidgey
	Slowpoke = __Slowpoke
	Kadabra = __Kadabra
	Graveler = __Graveler
	Chansey = __Chansey
	Machoke = __Machoke
	Mr_Mime = __Mr_Mime
	Hitmonlee = __Hitmonlee
	Hitmonchan = __Hitmonchan
	Arbok = __Arbok
	Parasect = __Parasect
	Psyduck = __Psyduck
	Drowzee = __Drowzee
	Golem = __Golem
	Magmar = __Magmar
	Electabuzz = __Electabuzz
	Magneton = __Magneton
	Koffing = __Koffing
	Mankey = __Mankey
	Seel = __Seel
	Diglett = __Diglett
	Tauros = __Tauros
	Farfetchd = __Farfetchd
	Venonat = __Venonat
	Dragonite = __Dragonite
	Doduo = __Doduo
	Poliwag = __Poliwag
	Jynx = __Jynx
	Moltres = __Moltres
	Articuno = __Articuno
	Zapdos = __Zapdos
	Ditto = __Ditto
	Meowth = __Meowth
	Krabby = __Krabby
	Vulpix = __Vulpix
	Ninetales = __Ninetales
	Pikachu = __Pikachu
	Raichu = __Raichu
	Dratini = __Dratini
	Dragonair = __Dragonair
	Kabuto = __Kabuto
	Kabutops = __Kabutops
	Horsea = __Horsea
	Seadra = __Seadra
	Sandshrew = __Sandshrew
	Sandslash = __Sandslash
	Omanyte = __Omanyte
	Omastar = __Omastar
	Jigglypuff = __Jigglypuff
	Wigglytuff = __Wigglytuff
	Eevee = __Eevee
	Flareon = __Flareon
	Jolteon = __Jolteon
	Vaporeon = __Vaporeon
	Machop = __Machop
	Zubat = __Zubat
	Ekans = __Ekans
	Paras = __Paras
	Poliwhirl = __Poliwhirl
	Poliwrath = __Poliwrath
	Weedle = __Weedle
	Kakuna = __Kakuna
	Beedrill = __Beedrill
	Dodrio = __Dodrio
	Primeape = __Primeape
	Dugtrio = __Dugtrio
	Venomoth = __Venomoth
	Dewgong = __Dewgong
	Caterpie = __Caterpie
	Metapod = __Metapod
	Butterfree = __Butterfree
	Machamp = __Machamp
	Golduck = __Golduck
	Hypno = __Hypno
	Golbat = __Golbat
	Mewtwo = __Mewtwo
	Snorlax = __Snorlax
	Magikarp = __Magikarp
	Muk = __Muk
	Kingler = __Kingler
	Cloyster = __Cloyster
	Electrode = __Electrode
	Clefable = __Clefable
	Weezing = __Weezing
	Persian = __Persian
	Marowak = __Marowak
	Haunter = __Haunter
	Abra = __Abra
	Alakazam = __Alakazam
	Pidgeotto = __Pidgeotto
	Pidgeot = __Pidgeot
	Starmie = __Starmie
	Bulbasaur = __Bulbasaur
	Venusaur = __Venusaur
	Tentacruel = __Tentacruel
	Goldeen = __Goldeen
	Seaking = __Seaking
	Ponyta = __Ponyta
	Rapidash = __Rapidash
	Rattata = __Rattata
	Raticate = __Raticate
	Nidorino = __Nidorino
	Nidorina = __Nidorina
	Geodude = __Geodude
	Porygon = __Porygon
	Aerodactyl = __Aerodactyl
	Magnemite = __Magnemite
	Charmander = __Charmander
	Squirtle = __Squirtle
	Charmeleon = __Charmeleon
	Wartortle = __Wartortle
	Charizard = __Charizard
	Oddish = __Oddish
	Gloom = __Gloom
	Vileplume = __Vileplume
	Bellsprout = __Bellsprout
	Weepinbell = __Weepinbell
	Victreebel = __Victreebel
	Missingno = __Missingno

cdef class PokemonBase:
	cdef __Pokemon.Base *__instance

	def __cinit__(self):
		self.__instance = NULL

	def __dealloc__(self):
		del self.__instance

	def __init__(self, int index):
		if index < 0 or index >= 256:
			raise IndexError("Index out of range")
		if self.__instance != NULL:
			del self.__instance
		self.__instance = new __Pokemon.Base(__pokemonList.at(index))

	@property
	def id(self):
		return self.__instance.id
	@id.setter
	def id(self, v):
		self.__instance.id = v

	@property
	def dexId(self):
		return self.__instance.dexId
	@dexId.setter
	def dexId(self, v):
		self.__instance.dexId = v

	@property
	def name(self):
		o = self.__instance.name
		return o.decode('ASCII')
	@name.setter
	def name(self, v):
		self.__instance.name = v.encode('ASCII')

	@property
	def HP(self):
		return self.__instance.HP
	@HP.setter
	def HP(self, v):
		self.__instance.HP = v

	@property
	def ATK(self):
		return self.__instance.ATK
	@ATK.setter
	def ATK(self, v):
		self.__instance.ATK = v

	@property
	def SPD(self):
		return self.__instance.SPD
	@SPD.setter
	def SPD(self, v):
		self.__instance.SPD = v

	@property
	def SPE(self):
		return self.__instance.SPE
	@SPE.setter
	def SPE(self, v):
		self.__instance.SPE = v

	@property
	def typeA(self):
		return Type(self.__instance.typeA)
	@typeA.setter
	def typeA(self, v):
		self.__instance.typeA = cast(__Type, v)

	@property
	def typeB(self):
		return Type(self.__instance.typeB)
	@typeB.setter
	def typeB(self, v):
		self.__instance.typeB = cast(__Type, v)

	@property
	def catchRate(self):
		return self.__instance.catchRate
	@catchRate.setter
	def catchRate(self, v):
		self.__instance.catchRate = v

	@property
	def baseXpYield(self):
		return self.__instance.baseXpYield
	@baseXpYield.setter
	def baseXpYield(self, v):
		self.__instance.baseXpYield = v

	@property
	def movePool(self):
		return set(AvailableMove(i) for i in self.__instance.movePool)

	@property
	def statsAtLevel(self):
		return [{
                        'HP': r.HP,
			'maxHP': r.maxHP,
			'ATK': r.ATK,
			'DEF': r.DeF,
			'SPD': r.SPD,
			'SPE': r.SPE
		} for r in self.__instance.statsAtLevel]

	def __getattr__(self, name):
		if name == "DEF":
			return self.__instance.DeF
		return super(PokemonBase, self).__getattribute__(name)
	def __setattr__(self, name, value):
		if name == "DEF":
			self.__instance.DeF = value
		else:
			super(PokemonBase, self).__setattr__(name, value)


cdef class Pokemon:
	cdef __Pokemon *__instance
	cdef bool __allocd

	def __cinit__(self):
		self.__instance = NULL
		self.__allocd = False

	def __dealloc__(self):
		if self.__allocd:
			del self.__instance

	def __init__(self, *args):
		cdef DataArray data

		if len(args) == 0:
			return
		if len(args) == 1:
			if isinstance(args[0], Pokemon):
				self.__instance = new __Pokemon(dereference((<Pokemon>args[0]).__instance))
				self.__allocd = True
				return
			if args[0] is None:
				return
		elif len(args) == 4 or len(args) == 3:
			state: BattleState = args[0]
			nickname: string = args[1] if isinstance(args[1], bytes) else args[1].encode('utf-8')
			if len(args[2]) != 44:
				raise TypeError("Invalid data length")
			for i in range(44):
				data[i] = args[2][i]
			enemy: bool = False if len(args) == 3 else args[3]
			self.__instance = new __Pokemon(
				state.__instance.rng,
				state.__instance.battleLogger,
				nickname,
				data,
				enemy
			)
			return
		elif len(args) == 6 or len(args) == 5:
			moveSet: vector[__Move]

			state: BattleState = args[0]
			nickname: string = args[1] if isinstance(args[1], bytes) else args[1].encode('utf-8')
			level: int = args[2]
			base = args[3]
			moveSetPy: [Move] = args[4]
			enemy: bool = False if len(args) == 5 else args[5]

			for move in moveSetPy:
				if not isinstance(move, Move):
					raise TypeError("Moveset must contains Moves only")
				moveSet.push_back(dereference((<Move>move).__instance))
			self.__instance = new __Pokemon(
				state.__instance.rng,
				state.__instance.battleLogger,
				nickname,
				level,
				dereference((<PokemonBase>base).__instance),
				moveSet,
				enemy
			)
			self.__allocd = True
			return
		raise TypeError("Invalid arguments sent to constructor")

	def copy(self):
		return Pokemon(self)

	def setGlobalCritRatio(self, double ratio):
		return self.__instance.setGlobalCritRatio(ratio)

	def setStatus(self, StatusChange status):
		return self.__instance.setStatus(status)

	def setNonVolatileStatus(self, StatusChange status):
		return self.__instance.setNonVolatileStatus(status)

	def setNonVolatileStatus(self, StatusChange status, unsigned duration):
		return self.__instance.setNonVolatileStatus(status, duration)

	def addStatus(self, StatusChange status):
		return self.__instance.addStatus(status)

	def addStatus(self, StatusChange status, unsigned duration):
		return self.__instance.addStatus(status, duration)

	def resetStatsChanges(self):
		self.__instance.resetStatsChanges()

	def changeStat(self, StatsChange stat, char nb):
		return self.__instance.changeStat(stat, nb)

	def useMove(self, Move move, Pokemon target):
		self.__instance.useMove(dereference(move.__instance), dereference(target.__instance))

	def storeDamages(self, bool active):
		return self.__instance.storeDamages(active)

	def hasStatus(self, StatusChange status):
		return self.__instance.hasStatus(status)

	def takeDamage(self, int damage):
		self.__instance.takeDamage(damage)

	def attack(self, unsigned char moveSlot, Pokemon target):
		self.__instance.attack(moveSlot, dereference(target.__instance))

	def calcDamage(self, Pokemon target, unsigned power, Type damageType, MoveCategory category, bool critical, bool randomized = True):
		cdef __Pokemon.DamageResult result = self.__instance.calcDamage(
			dereference(target.__instance),
			power,
			cast(__Type, damageType),
			cast(__Move.MoveCategory, category),
			critical,
			randomized
		)

		return {
			'critical': result.critical,
			'damage': result.damage,
			'affect': result.affect,
			'isVeryEffective': result.isVeryEffective,
			'isNotVeryEffective': result.isNotVeryEffective
                }

	def endTurn(self):
		self.__instance.endTurn()

	def switched(self):
		self.__instance.switched()

	def getPriorityFactor(self, unsigned char moveSlot):
		return self.__instance.getPriorityFactor(moveSlot)

	def setWrapped(self, bool isWrapped):
		self.__instance.setWrapped(isWrapped)

	def setRecharging(self, bool recharging = True):
		self.__instance.setRecharging(recharging)

	def transform(self, Pokemon target):
		self.__instance.transform(dereference(target.__instance))

	def getLearnableMoveSet(self):
		return self.__instance.getLearnableMoveSet()

	def dump(self):
		return self.__instance.dump().decode('ASCII')

	#array[unsigned char, ENCODED_SIZE] encode()
	#nlohmann::json serialize()

	def setInvincible(self, bool invincible):
		self.__instance.setInvincible(invincible)

	def setId(self, unsigned char id, bool recomputeStats = True):
		self.__instance.setId(id, recomputeStats)

	def setNickname(self, const string &nickname):
		self.__instance.setNickname(nickname)

	def setLevel(self, unsigned char level):
		self.__instance.setLevel(level)

	def setMove(self, unsigned char index, Move move):
		self.__instance.setMove(index, dereference(move.__instance))

	def reset(self):
		self.__instance.reset()

	def applyStatusDebuff(self):
		self.__instance.applyStatusDebuff()

	def getRandomGenerator(self):
		r = RandomGenerator(alloc=False)
		r.__instance = &self.__instance.getRandomGenerator()

	def canHaveStatus(self, StatusChange status):
		return self.__instance.canHaveStatus(cast(__StatusChange, status))

	def getStatus(self):
		return self.__instance.getStatus()

	def getNonVolatileStatus(self):
		return self.__instance.getNonVolatileStatus()

	def canGetHit(self):
		return self.__instance.canGetHit()

	def getID(self):
		return self.__instance.getID()

	def getDamagesStored(self):
		return self.__instance.getDamagesStored()

	def getSpeed(self):
		return self.__instance.getSpeed()

	def getLevel(self):
		return self.__instance.getLevel()

	def getAttack(self):
		return self.__instance.getAttack()

	def getSpecial(self):
		return self.__instance.getSpecial()

	def getDefense(self):
		return self.__instance.getDefense()

	def getRawAttack(self):
		return self.__instance.getRawAttack()

	def getRawSpecial(self):
		return self.__instance.getRawSpecial()

	def getRawDefense(self):
		return self.__instance.getRawDefense()

	def getRawSpeed(self):
		return self.__instance.getRawSpeed()

	def getHealth(self):
		return self.__instance.getHealth()

	def getName(self, bool hasEnemy = True):
		return self.__instance.getName(hasEnemy).decode('ASCII')

	def getNickname(self):
		return self.__instance.getNickname().decode('ASCII')

	def getMaxHealth(self):
		return self.__instance.getMaxHealth()

	def getAccuracy(self, accuracy):
		return self.__instance.getAccuracy(accuracy)

	def getEvasion(self, accuracy):
		return self.__instance.getEvasion(accuracy)

	def getAccuracyMul(self):
		return self.__instance.getAccuracyMul()

	def getEvasionMul(self):
		return self.__instance.getEvasionMul()

	def getLastUsedMove(self):
		m = Move()
		m.__cinstance = &self.__instance.getLastUsedMove()
		return m

	def getTypes(self):
		types = self.__instance.getTypes()
		return (Type(types.first), Type(types.second))

	def getMoveSet(self):
		cdef const vector[__Move] *moveSet = &self.__instance.getMoveSet()
		result = []

		for i in range(moveSet.size()):
			m = Move()
			m.__cinstance = &dereference(moveSet)[i]
			result.append(m)
		return result

	def getBaseStats(self):
		cdef __Pokemon.BaseStats r = self.__instance.getBaseStats()

		return {
			'HP': r.HP,
			'maxHP': r.maxHP,
			'ATK': r.ATK,
			'DEF': r.DeF,
			'SPD': r.SPD,
			'SPE': r.SPE
                }

	def getStatsUpgradeStages(self):
		cdef __Pokemon.UpgradableStats r = self.__instance.getStatsUpgradeStages()
		return {
			'ATK': r.ATK,
			'DEF': r.DeF,
			'SPD': r.SPD,
			'SPE': r.SPE,
			'ACC': r.ACC,
			'EVD': r.EVD
                }

	def getSpeciesName(self):
		return self.__instance.getSpeciesName().decode('ASCII')

	def getDvs(self):
		cdef __Pokemon.BaseStats r = self.__instance.getDvs()
		return {
			'HP': r.HP,
			'maxHP': r.maxHP,
			'ATK': r.ATK,
			'DEF': r.DeF,
			'SPD': r.SPD,
			'SPE': r.SPE
                }

	def getStatExps(self):
		cdef __Pokemon.BaseStats r = self.__instance.getStatExps()
		return {
			'HP': r.HP,
			'maxHP': r.maxHP,
			'ATK': r.ATK,
			'DEF': r.DeF,
			'SPD': r.SPD,
			'SPE': r.SPE
                }

	def isEnemy(self):
		return self.__instance.isEnemy()

	def setStatExps(self, dict d):
		cdef __Pokemon.BaseStats statExps

		statExps.HP = d['HP']
		statExps.maxHP = d['maxHP']
		statExps.ATK = d['ATK']
		statExps.DeF = d['DEF']
		statExps.SPD = d['SPD']
		statExps.SPE = d['SPE']
		self.__instance.setStatExps(statExps)

	def setTypes(self, types):
		if not isinstance(types, tuple):
			types = (types, types)
		types[0] = Type(types[0])
		types[1] = Type(types[1])
		self.__instance.setTypes(types)

	#@staticmethod
	#BaseStats makeStats(unsigned char level, const Base &base, const BaseStats &dvs, const BaseStats &evs):
	#	BaseStats r = self.__instance.getStatExps()
	#	return {
	#		'HP': r.HP,
	#		'maxHP': r.maxHP,
	#		'ATK': r.ATK,
	#		'DEF': r.DeF,
	#		'SPD': r.SPD,
	#		'SPE': r.SPE
        #        }


cpdef enum BattleAction:
	NoAction = __NoAction
	Attack1 = __Attack1
	Attack2 = __Attack2
	Attack3 = __Attack3
	Attack4 = __Attack4
	Switch1 = __Switch1
	Switch2 = __Switch2
	Switch3 = __Switch3
	Switch4 = __Switch4
	Switch5 = __Switch5
	Switch6 = __Switch6
	StruggleMove = __StruggleMove
	Run = __Run

def battleActionToString(s):
	return __BattleActionToString(s).decode('ASCII')


cdef class PlayerState:
	cdef __PlayerState *__instance

	@property
	def name(self):
		o = self.__instance.name
		return o.decode('ASCII')
	@name.setter
	def name(self, v):
		self.__instance.name = v.encode('ASCII')

	@property
	def lastAction(self):
		return BattleAction(self.__instance.lastAction)
	@lastAction.setter
	def lastAction(self, v):
		self.__instance.lastAction = cast(__BattleAction, v)

	@property
	def nextAction(self):
		return BattleAction(self.__instance.nextAction)
	@nextAction.setter
	def nextAction(self, v):
		self.__instance.nextAction = cast(__BattleAction, v)

	@property
	def pokemonOnField(self):
		return self.__instance.pokemonOnField
	@pokemonOnField.setter
	def pokemonOnField(self, v):
		self.__instance.pokemonOnField = v

	@property
	def team(self):
		result = []
		for pkmn in range(self.__instance.team.size()):
			p = Pokemon()
			p.__instance = &self.__instance.team[pkmn]
			result.append(p)
		return result
	@team.setter
	def team(self, v):
		cdef vector[__Pokemon] l
		for pkmn in v:
			l.push_back(dereference((<Pokemon>pkmn).__instance))
		self.__instance.team = l

	def isPkmnDiscovered(self, size_t i):
		if i > 6:
			return False
		return self.__instance.discovered[i].first
	def setPkmnDiscovered(self, size_t i, bool v):
		if i < 6:
			self.__instance.discovered[i].first = v

	def isPkmnMoveDiscovered(self, size_t pkmn, size_t move):
		if pkmn > 6 or move > 4:
			return False
		return self.__instance.discovered[pkmn].first and self.__instance.discovered[pkmn].second[move]
	def setPkmnMoveDiscovered(self, size_t pkmn, size_t move, bool v):
		if pkmn < 6 and move < 4:
			self.__instance.discovered[pkmn].second[move] = v

cdef class BattleState:
	cdef __BattleState *__instance
	cdef bool __allocd
	__battleLoggerPy: object
	__onTurnStartPy: object
	__onBattleEndPy: object
	__onBattleStartPy: object

	def __cinit__(self):
		self.__instance = NULL
		self.__allocd = False

	def __dealloc__(self):
		if self.__allocd:
			del self.__instance

	def __init__(self, alloc=True):
		if alloc:
			self.__instance = new __BattleState()
			self.__allocd = True
		self.__battleLoggerPy = None
		self.__onTurnStartPy = None
		self.__onBattleEndPy = None
		self.__onBattleStartPy = None

	@property
	def me(self):
		p = PlayerState()
		p.__instance = &self.__instance.me
		return p

	@property
	def op(self):
		p = PlayerState()
		p.__instance = &self.__instance.op
		return p

	@property
	def rng(self):
		p = RandomGenerator()
		p.__instance = &self.__instance.rng
		return p

	@property
	def battleLogger(self):
		#return self.battleLoggerPy
		raise NotImplementedError("Not implemented")
	@battleLogger.setter
	def battleLogger(self, c):
		if not callable(c):
			raise TypeError("Requires callable")
		self.__battleLoggerPy = c
		self.__instance.battleLogger = pythonLoggerLambda(<void *>self.__battleLoggerPy, &evalLogger)

	@property
	def onTurnStart(self):
		#return self.__instance.onTurnStart
		raise NotImplementedError("Not implemented")
	@onTurnStart.setter
	def onTurnStart(self, c):
		if not callable(c):
			raise TypeError("Requires callable")
		self.__onTurnStartPy = c
		self.__instance.onTurnStart = pythonCallbackLambda(<void*>self.__onTurnStartPy, &evalCallback)

	@property
	def onBattleEnd(self):
		#return self.__instance.onBattleEnd
		raise NotImplementedError("Not implemented")
	@onBattleEnd.setter
	def onBattleEnd(self, c):
		if not callable(c):
			raise TypeError("Requires callable")
		self.__onBattleEndPy = c
		self.__instance.onBattleEnd = pythonCallbackLambdaVoid(<void*>self.__onBattleEndPy, &evalCallbackVoid)

	@property
	def onBattleStart(self):
		#return self.__instance.onBattleStart
		raise NotImplementedError("Not implemented")
	@onBattleStart.setter
	def onBattleStart(self, c):
		if not callable(c):
			raise TypeError("Requires callable")
		self.__onBattleStartPy = c
		self.__instance.onBattleStart = pythonCallbackLambdaVoid(<void*>self.__onBattleStartPy, &evalCallbackVoid)


cdef class BattleHandler:
	cdef __BattleHandler *__instance
	__pyState: BattleState

	def __dealloc__(self):
		del self.__instance

	def __init__(self, bool viewSwapped, bool logMessages):
		self.__instance = new __BattleHandler(viewSwapped, logMessages)
		self.__pyState = None

	def logBattle(self, message: str):
		self.__instance.logBattle(message.encode('ASCII'))

	def getBattleState(self):
		if self.__pyState is None:
			b = BattleState(alloc=False)
			b.__instance = &self.__instance.getBattleState()
			self.__pyState = b
		return self.__pyState

	def tick(self):
		return self.__instance.tick()

	def isFinished(self):
		return self.__instance.isFinished()

	def reset(self):
		return self.__instance.reset()

	def saveReplay(self, path):
		if isinstance(path, str):
			path = path.encode('ASCII')
		return self.__instance.saveReplay(path)

	def loadReplay(self, path):
		if isinstance(path, str):
			path = path.encode('ASCII')
		self.__instance.loadReplay(path)

	def stopReplay(self):
		self.__instance.stopReplay()

	def playingReplay(self):
		return self.__instance.playingReplay()

	@property
	def state(self):
		return self.getBattleState()
