# distutils: language = c++

from libcpp cimport bool
from libcpp.vector cimport vector
from cython.operator cimport dereference
from .Pokemon cimport Pokemon as PyPokemon
from ._Pokemon cimport pythonLoggerLambda, evalLogger
from ._Move cimport Move as __Move, AvailableMove as __AvailableMove, availableMoves, Empty as __Empty, Pound as __Pound, Karate_Chop as __Karate_Chop, Doubleslap as __Doubleslap, Comet_Punch as __Comet_Punch, Mega_Punch as __Mega_Punch, Pay_Day as __Pay_Day, Fire_Punch as __Fire_Punch, Ice_Punch as __Ice_Punch, Thunder_Punch as __Thunder_Punch, Scratch as __Scratch, Vise_Grip as __Vise_Grip, Guillotine as __Guillotine, Razor_Wind as __Razor_Wind, Swords_Dance as __Swords_Dance, Cut as __Cut, Gust as __Gust, Wing_Attack as __Wing_Attack, Whirlwind as __Whirlwind, Fly as __Fly, Bind as __Bind, Slam as __Slam, Vine_Whip as __Vine_Whip, Stomp as __Stomp, Double_Kick as __Double_Kick, Mega_Kick as __Mega_Kick, Jump_Kick as __Jump_Kick, Rolling_Kick as __Rolling_Kick, Sand_Attack as __Sand_Attack, Headbutt as __Headbutt, Horn_Attack as __Horn_Attack, Fury_Attack as __Fury_Attack, Horn_Drill as __Horn_Drill, Tackle as __Tackle, Body_Slam as __Body_Slam, Wrap as __Wrap, Take_Down as __Take_Down, Thrash as __Thrash, Double_Edge as __Double_Edge, Tail_Whip as __Tail_Whip, Poison_Sting as __Poison_Sting, Twineedle as __Twineedle, Pin_Missile as __Pin_Missile, Leer as __Leer, Bite as __Bite, Growl as __Growl, Roar as __Roar, Sing as __Sing, Supersonic as __Supersonic, Sonic_Boom as __Sonic_Boom, Disable as __Disable, Acid as __Acid, Ember as __Ember, Flamethrower as __Flamethrower, Mist as __Mist, Water_Gun as __Water_Gun, Hydro_Pump as __Hydro_Pump, Surf as __Surf, Ice_Beam as __Ice_Beam, Blizzard as __Blizzard, Psybeam as __Psybeam, Bubblebeam as __Bubblebeam, Aurora_Beam as __Aurora_Beam, Hyper_Beam as __Hyper_Beam, Peck as __Peck, Drill_Peck as __Drill_Peck, Submission as __Submission, Low_Kick as __Low_Kick, Counter as __Counter, Seismic_Toss as __Seismic_Toss, Strength as __Strength, Absorb as __Absorb, Mega_Drain as __Mega_Drain, Leech_Seed as __Leech_Seed, Growth as __Growth, Razor_Leaf as __Razor_Leaf, Solarbeam as __Solarbeam, Poisonpowder as __Poisonpowder, Stun_Spore as __Stun_Spore, Sleep_Powder as __Sleep_Powder, Petal_Dance as __Petal_Dance, String_Shot as __String_Shot, Dragon_Rage as __Dragon_Rage, Fire_Spin as __Fire_Spin, Thundershock as __Thundershock, Thunderbolt as __Thunderbolt, Thunder_Wave as __Thunder_Wave, Thunder as __Thunder, Rock_Throw as __Rock_Throw, Earthquake as __Earthquake, Fissure as __Fissure, Dig as __Dig, Toxic as __Toxic, Confusion as __Confusion, Psychic_M as __Psychic_M, Hypnosis as __Hypnosis, Meditate as __Meditate, Agility as __Agility, Quick_Attack as __Quick_Attack, Rage as __Rage, Teleport as __Teleport, Night_Shade as __Night_Shade, Mimic as __Mimic, Screech as __Screech, Double_Team as __Double_Team, Recover as __Recover, Harden as __Harden, Minimize as __Minimize, Smokescreen as __Smokescreen, Confuse_Ray as __Confuse_Ray, Withdraw as __Withdraw, Defense_Curl as __Defense_Curl, Barrier as __Barrier, Light_Screen as __Light_Screen, Haze as __Haze, Reflect as __Reflect, Focus_Energy as __Focus_Energy, Bide as __Bide, Metronome as __Metronome, Mirror_Move as __Mirror_Move, Self_Destruct as __Self_Destruct, Egg_Bomb as __Egg_Bomb, Lick as __Lick, Smog as __Smog, Sludge as __Sludge, Bone_Club as __Bone_Club, Fire_Blast as __Fire_Blast, Waterfall as __Waterfall, Clamp as __Clamp, Swift as __Swift, Skull_Bash as __Skull_Bash, Spike_Cannon as __Spike_Cannon, Constrict as __Constrict, Amnesia as __Amnesia, Kinesis as __Kinesis, Softboiled as __Softboiled, Hi_Jump_Kick as __Hi_Jump_Kick, Glare as __Glare, Dream_Eater as __Dream_Eater, Poison_Gas as __Poison_Gas, Barrage as __Barrage, Leech_Life as __Leech_Life, Lovely_Kiss as __Lovely_Kiss, Sky_Attack as __Sky_Attack, Transform as __Transform, Bubble as __Bubble, Dizzy_Punch as __Dizzy_Punch, Spore as __Spore, Flash as __Flash, Psywave as __Psywave, Splash as __Splash, Acid_Armor as __Acid_Armor, Crabhammer as __Crabhammer, Explosion as __Explosion, Fury_Swipes as __Fury_Swipes, Bonemerang as __Bonemerang, Rest as __Rest, Rock_Slide as __Rock_Slide, Hyper_Fang as __Hyper_Fang, Sharpen as __Sharpen, Conversion as __Conversion, Tri_Attack as __Tri_Attack, Super_Fang as __Super_Fang, Slash as __Slash, Substitute as __Substitute, Struggle as __Struggle, PHYSICAL as __PHYSICAL, SPECIAL as __SPECIAL, STATUS as __STATUS
from .StatsChange import StatsChange
from .StatusChange import StatusChange
from .Type import Type

cpdef enum AvailableMove:
	Empty = __Empty
	Pound = __Pound
	Karate_Chop = __Karate_Chop
	Doubleslap = __Doubleslap
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
	Bubblebeam = __Bubblebeam
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
	Solarbeam = __Solarbeam
	Poisonpowder = __Poisonpowder
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
	Softboiled = __Softboiled
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

	def __assign__(self, Move other):
		self.__instance[0] = other.__instance[0]
		return self

	def copy(self):
		return Move(self)

	@property
	def nb_runs(self):
		return self.__cinstance.getNbRuns()

	@property
	def nb_hits(self):
		return self.__cinstance.getNbHits()

	@property
	def status_change(self):
		cdef const __Move.StatusChangeProb *r = &self.__cinstance.getStatusChange()

		return {
			'status': StatusChange(r.status),
                        'cmpVal': r.cmpVal
		}

	@property
	def owner_change(self):
		cdef const vector[__Move.StatsChangeProb] *r = &self.__cinstance.getOwnerChange()
		result = []

		for i in range(r.size()):
			result.append({
				'stat': StatsChange(dereference(r)[i].stat),
				'nb': dereference(r)[i].nb,
				'cmpVal': dereference(r)[i].cmpVal
			})
		return result

	@property
	def foe_change(self):
		cdef const vector[__Move.StatsChangeProb] *r = &self.__cinstance.getFoeChange()
		result = []

		for i in range(r.size()):
			result.append({
				'stat': StatsChange(dereference(r)[i].stat),
				'nb': dereference(r)[i].nb,
				'cmpVal': dereference(r)[i].cmpVal
			})
		return result

	@property
	def needs_loading(self):
		return self.__cinstance.needsLoading()

	@property
	def invulnerable_during_loading(self):
		return self.__cinstance.isInvulnerableDuringLoading()

	@property
	def needs_recharge(self):
		return self.__cinstance.needsRecharge()

	@property
	def hit_callback_description(self):
		return self.__cinstance.getHitCallBackDescription().decode('utf-8')

	@property
	def miss_callback_description(self):
		return self.__cinstance.getMissCallBackDescription().decode('utf-8')

	@property
	def crit_chance(self):
		return self.__cinstance.getCritChance()

	@property
	def accuracy(self):
		return self.__cinstance.getAccuracy()

	@property
	def category(self):
		return MoveCategory(self.__cinstance.getCategory())

	@property
	def makes_invulnerable(self):
		return self.__cinstance.makesInvulnerable()

	@property
	def max_pp(self):
		return self.__cinstance.getMaxPP()

	@property
	def power(self):
		return self.__cinstance.getPower()

	@property
	def pp_up(self):
		return self.__cinstance.getPPUp()

	@property
	def type(self):
		return Type(self.__cinstance.getType())

	@property
	def pp(self):
		return self.__cinstance.getPP()

	@property
	def id(self):
		return AvailableMove(self.__cinstance.getID())

	@property
	def name(self):
		return self.__cinstance.getName().decode('utf-8')

	@property
	def description(self):
		return self.__cinstance.getDescription().decode('utf-8')

	@property
	def priority(self):
		return self.__cinstance.getPriority()

	@property
	def finished(self):
		return self.__cinstance.isFinished()

	@property
	def hits_left(self):
		return self.__cinstance.getHitsLeft()

	@pp.setter
	def pp(self, unsigned char pp):
		if self.__instance == NULL:
			raise TypeError("Instance is read only")
		self.__instance.setPP(pp)

	@pp_up.setter
	def pp_up(self, unsigned char nb):
		if self.__instance == NULL:
			raise TypeError("Instance is read only")
		self.__instance.setPPUp(nb)

	@hits_left.setter
	def hits_left(self, unsigned char nb):
		if self.__instance == NULL:
			raise TypeError("Instance is read only")
		self.__instance.setHitsLeft(nb)

	def glitch(self):
		if self.__instance == NULL:
			raise TypeError("Instance is read only")
		self.__instance.glitch()

	def reset(self):
		if self.__instance == NULL:
			raise TypeError("Instance is read only")
		self.__instance.reset()

	def attack(self, PyPokemon owner, PyPokemon target, logger):
		if self.__instance == NULL:
			raise TypeError("Instance is read only")
		self.__instance.attack(dereference(owner.__instance), dereference(target.__instance), pythonLoggerLambda(<void *>logger, &evalLogger))