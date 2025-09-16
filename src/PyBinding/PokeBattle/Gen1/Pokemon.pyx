# distutils: language = c++

from libcpp cimport bool
from libcpp.pair cimport pair
from libcpp.vector cimport vector
from cython cimport cast
from cython.operator cimport dereference
from .Move cimport Move as PyMove
from .State cimport BattleState as PyBattleState
from .RandomGenerator cimport RandomGenerator as PyRandomGenerator
from ._Type cimport Type as __Type
from ._Move cimport Move as __Move, MoveCategory
from ._StatsChange cimport StatsChange as __StatsChange
from ._StatusChange cimport StatusChange as __StatusChange
from ._Pokemon cimport DataArray, pokemonList as __pokemonList, Pokemon as __Pokemon, pythonLoggerLambda, evalLogger, Rhydon as __Rhydon, Kangaskhan as __Kangaskhan, Nidoran_M as __Nidoran_M, Clefairy as __Clefairy, Spearow as __Spearow, Voltorb as __Voltorb, Nidoking as __Nidoking, Slowbro as __Slowbro, Ivysaur as __Ivysaur, Exeggutor as __Exeggutor, Lickitung as __Lickitung, Exeggcute as __Exeggcute, Grimer as __Grimer, Gengar as __Gengar, Nidoran_F as __Nidoran_F, Nidoqueen as __Nidoqueen, Cubone as __Cubone, Rhyhorn as __Rhyhorn, Lapras as __Lapras, Arcanine as __Arcanine, Mew as __Mew, Gyarados as __Gyarados, Shellder as __Shellder, Tentacool as __Tentacool, Gastly as __Gastly, Scyther as __Scyther, Staryu as __Staryu, Blastoise as __Blastoise, Pinsir as __Pinsir, Tangela as __Tangela, Growlithe as __Growlithe, Onix as __Onix, Fearow as __Fearow, Pidgey as __Pidgey, Slowpoke as __Slowpoke, Kadabra as __Kadabra, Graveler as __Graveler, Chansey as __Chansey, Machoke as __Machoke, Mr_Mime as __Mr_Mime, Hitmonlee as __Hitmonlee, Hitmonchan as __Hitmonchan, Arbok as __Arbok, Parasect as __Parasect, Psyduck as __Psyduck, Drowzee as __Drowzee, Golem as __Golem, Magmar as __Magmar, Electabuzz as __Electabuzz, Magneton as __Magneton, Koffing as __Koffing, Mankey as __Mankey, Seel as __Seel, Diglett as __Diglett, Tauros as __Tauros, Farfetchd as __Farfetchd, Venonat as __Venonat, Dragonite as __Dragonite, Doduo as __Doduo, Poliwag as __Poliwag, Jynx as __Jynx, Moltres as __Moltres, Articuno as __Articuno, Zapdos as __Zapdos, Ditto as __Ditto, Meowth as __Meowth, Krabby as __Krabby, Vulpix as __Vulpix, Ninetales as __Ninetales, Pikachu as __Pikachu, Raichu as __Raichu, Dratini as __Dratini, Dragonair as __Dragonair, Kabuto as __Kabuto, Kabutops as __Kabutops, Horsea as __Horsea, Seadra as __Seadra, Sandshrew as __Sandshrew, Sandslash as __Sandslash, Omanyte as __Omanyte, Omastar as __Omastar, Jigglypuff as __Jigglypuff, Wigglytuff as __Wigglytuff, Eevee as __Eevee, Flareon as __Flareon, Jolteon as __Jolteon, Vaporeon as __Vaporeon, Machop as __Machop, Zubat as __Zubat, Ekans as __Ekans, Paras as __Paras, Poliwhirl as __Poliwhirl, Poliwrath as __Poliwrath, Weedle as __Weedle, Kakuna as __Kakuna, Beedrill as __Beedrill, Dodrio as __Dodrio, Primeape as __Primeape, Dugtrio as __Dugtrio, Venomoth as __Venomoth, Dewgong as __Dewgong, Caterpie as __Caterpie, Metapod as __Metapod, Butterfree as __Butterfree, Machamp as __Machamp, Golduck as __Golduck, Hypno as __Hypno, Golbat as __Golbat, Mewtwo as __Mewtwo, Snorlax as __Snorlax, Magikarp as __Magikarp, Muk as __Muk, Kingler as __Kingler, Cloyster as __Cloyster, Electrode as __Electrode, Clefable as __Clefable, Weezing as __Weezing, Persian as __Persian, Marowak as __Marowak, Haunter as __Haunter, Abra as __Abra, Alakazam as __Alakazam, Pidgeotto as __Pidgeotto, Pidgeot as __Pidgeot, Starmie as __Starmie, Bulbasaur as __Bulbasaur, Venusaur as __Venusaur, Tentacruel as __Tentacruel, Goldeen as __Goldeen, Seaking as __Seaking, Ponyta as __Ponyta, Rapidash as __Rapidash, Rattata as __Rattata, Raticate as __Raticate, Nidorino as __Nidorino, Nidorina as __Nidorina, Geodude as __Geodude, Porygon as __Porygon, Aerodactyl as __Aerodactyl, Magnemite as __Magnemite, Charmander as __Charmander, Squirtle as __Squirtle, Charmeleon as __Charmeleon, Wartortle as __Wartortle, Charizard as __Charizard, Oddish as __Oddish, Gloom as __Gloom, Vileplume as __Vileplume, Bellsprout as __Bellsprout, Weepinbell as __Weepinbell, Victreebel as __Victreebel, Missingno as __Missingno

from .Move import Move, AvailableMove
from .RandomGenerator import RandomGenerator
from .StatusChange import StatusChange
from .Type import Type


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
		return PokemonSpecies(self.__instance.id)
	@id.setter
	def id(self, v):
		self.__instance.id = v

	@property
	def dex_id(self):
		return self.__instance.dexId
	@dex_id.setter
	def dex_id(self, v):
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
	def types(self):
		return (Type(self.__instance.typeA), Type(self.__instance.typeB))
	@types.setter
	def types(self, v):
		if isinstance(v, tuple):
			self.__instance.typeA = cast(__Type, v[0])
			self.__instance.typeB = cast(__Type, v[1])
		else:
			self.__instance.typeA = cast(__Type, v)
			self.__instance.typeB = cast(__Type, v)

	@property
	def catch_rate(self):
		return self.__instance.catchRate
	@catch_rate.setter
	def catch_rate(self, v):
		self.__instance.catchRate = v

	@property
	def base_xp_yield(self):
		return self.__instance.baseXpYield
	@base_xp_yield.setter
	def base_xp_yield(self, v):
		self.__instance.baseXpYield = v

	@property
	def move_pool(self):
		return set(AvailableMove(i) for i in self.__instance.movePool)

	@property
	def stats_at_level(self):
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
			state: PyBattleState = args[0]
			nickname: bytes = args[1] if isinstance(args[1], bytes) else args[1].encode('utf-8')
			if len(args[2]) != 44:
				raise TypeError("Invalid data length")
			for i in range(44):
				data[i] = args[2][i]
			enemy: bool = False if len(args) == 3 else args[3]
			self.__instance = new __Pokemon(
				dereference(state.__instance),
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
			moveSetPy = args[4]
			enemy: bool = False if len(args) == 5 else args[5]

			for move in moveSetPy:
				if not isinstance(move, PyMove):
					raise TypeError("Moveset must contains Moves only")
				moveSet.push_back(dereference((<PyMove>move).__instance))
			self.__instance = new __Pokemon(
				dereference(state.__instance),
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

	def set_global_crit_ratio(self, double ratio):
		return self.__instance.setGlobalCritRatio(ratio)

	def add_status(self, int status, unsigned duration = 0):
		if duration == 0:
			return self.__instance.addStatus(cast(__StatusChange, status))
		return self.__instance.addStatus(cast(__StatusChange, status), duration)

	def reset_stats_changes(self):
		self.__instance.resetStatsChanges()

	@property
	def status(self):
		return StatusChange(self.__instance.getStatus())
	@status.setter
	def status(self, int status):
		self.__instance.setStatus(cast(__StatusChange, status))

	@property
	def recharging(self):
		return self.__instance.isRecharging()
	@recharging.setter
	def recharging(self, bool recharging):
		self.__instance.setRecharging(recharging)

	@property
	def non_volatile_status(self):
		return self.__instance.getNonVolatileStatus()
	@non_volatile_status.setter
	def non_volatile_status(self, int status):
		self.__instance.setNonVolatileStatus(cast(__StatusChange, status))

	def change_stat(self, int stat, char nb):
		return self.__instance.changeStat(cast(__StatsChange, stat), nb)

	def use_move(self, PyMove move, Pokemon target):
		self.__instance.useMove(dereference(move.__instance), dereference(target.__instance))

	def store_damages(self, bool active):
		return self.__instance.storeDamages(active)

	def has_status(self, int status):
		return self.__instance.hasStatus(cast(__StatusChange, status))

	def heal(self, unsigned short health):
		self.__instance.heal(health)

	def take_damage(self, Pokemon target, unsigned short damage, bool ignoreSubstitute, bool swapSide):
		self.__instance.takeDamage(dereference(target.__instance), damage, ignoreSubstitute, swapSide)

	def use_move_slot(self, unsigned char moveSlot, Pokemon target):
		self.__instance.attack(moveSlot, dereference(target.__instance))

	def calc_damage(self, Pokemon target, unsigned power, int damageType, int category, bool critical, bool randomized, bool halfDefense, bool swapSide):
		cdef __Pokemon.DamageResult result = self.__instance.calcDamage(
			dereference(target.__instance),
			power,
			cast(__Type, damageType),
			cast(__Move.MoveCategory, category),
			critical,
			randomized,
			halfDefense,
			swapSide
		)
		return {
			'critical': result.critical,
			'damage': result.damage,
			'affect': result.affect,
			'isVeryEffective': result.isVeryEffective,
			'isNotVeryEffective': result.isNotVeryEffective
		}

	def end_turn(self):
		self.__instance.endTurn()

	def switched(self):
		self.__instance.switched()

	def get_priority_factor(self, unsigned char moveSlot):
		return self.__instance.getPriorityFactor(moveSlot)

	def set_wrapped(self, bool isWrapped):
		self.__instance.setWrapped(isWrapped)

	def transform(self, Pokemon target):
		self.__instance.transform(dereference(target.__instance))

	@property
	def learnable_move_set(self):
		return self.__instance.getLearnableMoveSet()

	def dump(self):
		return self.__instance.dump().decode('ASCII')

	def encode(self):
		cdef DataArray data = self.__instance.encode()
		result = []

		for i in range(44):
			result.append(data[i])
		return result

	#nlohmann::json serialize()

	def set_invincible(self, bool invincible):
		self.__instance.setInvincible(invincible)

	def set_move(self, unsigned char index, PyMove move):
		self.__instance.setMove(index, dereference(move.__instance))

	def reset(self):
		self.__instance.reset()

	def apply_status_debuff(self):
		self.__instance.applyStatusDebuff()

	def can_have_status(self, int status):
		return self.__instance.canHaveStatus(cast(__StatusChange, status))

	@property
	def random_generator(self):
		r = <PyRandomGenerator>RandomGenerator(alloc=False)
		r.__instance = &self.__instance.getRandomGenerator()
		return r

	@property
	def can_get_hit(self):
		return self.__instance.canGetHit()

	@property
	def id(self):
		return self.__instance.getID()
	@id.setter
	def id(self, unsigned char id):
		self.__instance.setID(id, True)
	def set_id(self, unsigned char id, bool recomputeStats = True):
		self.__instance.setID(id, recomputeStats)

	@property
	def level(self):
		return self.__instance.getLevel()
	@level.setter
	def level(self, unsigned char level):
		self.__instance.setLevel(level)

	@property
	def damages_stored(self):
		return self.__instance.getDamagesStored()

	@property
	def attack(self):
		return self.__instance.getAttack()
	@property
	def raw_attack(self):
		return self.__instance.getRawAttack()

	@property
	def defense(self):
		return self.__instance.getDefense()
	@property
	def raw_defense(self):
		return self.__instance.getRawDefense()

	@property
	def speed(self):
		return self.__instance.getSpeed()
	@property
	def raw_speed(self):
		return self.__instance.getRawSpeed()

	@property
	def special(self):
		return self.__instance.getSpecial()
	@property
	def raw_special(self):
		return self.__instance.getRawSpecial()

	@property
	def health(self):
		return self.__instance.getHealth()
	@property
	def max_health(self):
		return self.__instance.getMaxHealth()

	@property
	def substitute(self):
		if self.__instance.hasSubstitute():
			return self.__instance.getSubstituteHealth()
		return None

	def get_name(self, bool hasEnemy = True):
		return self.__instance.getName(hasEnemy).decode('ASCII')

	@property
	def name(self):
		return self.__instance.getName(True).decode('ASCII')

	@property
	def nickname(self):
		return self.__instance.getNickname().decode('ASCII')
	@nickname.setter
	def nickname(self, nickname):
		self.__instance.setNickname(nickname.encode('ASCII'))

	def get_accuracy(self, accuracy):
		return self.__instance.getAccuracy(accuracy)

	def get_evasion(self, accuracy):
		return self.__instance.getEvasion(accuracy)

	@property
	def accuracy_mul(self):
		return self.__instance.getAccuracyMul()

	@property
	def evasion_mul(self):
		return self.__instance.getEvasionMul()

	@property
	def last_used_move(self):
		m = <PyMove>Move()
		m.__cinstance = &self.__instance.getLastUsedMove()
		return m

	@property
	def types(self):
		cdef pair[__Type, __Type] types = self.__instance.getTypes()
		return (Type(types.first), Type(types.second))
	@types.setter
	def types(self, types):
		if not isinstance(types, tuple):
			types = (types, types)
		types[0] = Type(types[0])
		types[1] = Type(types[1])
		self.__instance.setTypes(types)

	@property
	def move_set(self):
		cdef const vector[__Move] *moveSet = &self.__instance.getMoveSet()
		result = []
		for i in range(moveSet.size()):
			m = <PyMove>Move()
			m.__cinstance = &dereference(moveSet)[i]
			result.append(m)
		return result

	@property
	def base_stats(self):
		cdef __Pokemon.BaseStats r = self.__instance.getBaseStats()
		return {
			'HP': r.HP,
			'maxHP': r.maxHP,
			'ATK': r.ATK,
			'DEF': r.DeF,
			'SPD': r.SPD,
			'SPE': r.SPE
                }

	@property
	def stats_upgrade_stages(self):
		cdef __Pokemon.UpgradableStats r = self.__instance.getStatsUpgradeStages()
		return {
			'ATK': r.ATK,
			'DEF': r.DeF,
			'SPD': r.SPD,
			'SPE': r.SPE,
			'ACC': r.ACC,
			'EVD': r.EVD
                }

	@property
	def species_name(self):
		return self.__instance.getSpeciesName().decode('ASCII')

	@property
	def dvs(self):
		cdef __Pokemon.BaseStats r = self.__instance.getDvs()
		return {
			'HP': r.HP,
			'maxHP': r.maxHP,
			'ATK': r.ATK,
			'DEF': r.DeF,
			'SPD': r.SPD,
			'SPE': r.SPE
                }

	@property
	def stat_exps(self):
		cdef __Pokemon.BaseStats r = self.__instance.getStatExps()
		return {
			'HP': r.HP,
			'maxHP': r.maxHP,
			'ATK': r.ATK,
			'DEF': r.DeF,
			'SPD': r.SPD,
			'SPE': r.SPE
                }
	@stat_exps.setter
	def stat_exps(self, dict d):
		cdef __Pokemon.BaseStats statExps
		statExps.HP = d['HP']
		statExps.maxHP = d['maxHP']
		statExps.ATK = d['ATK']
		statExps.DeF = d['DEF']
		statExps.SPD = d['SPD']
		statExps.SPE = d['SPE']
		self.__instance.setStatExps(statExps)

	@property
	def enemy(self):
		return self.__instance.isEnemy()


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
