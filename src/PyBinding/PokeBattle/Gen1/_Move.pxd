# cython: language_level=3

from libcpp cimport bool
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.string cimport string
from libcpp.functional cimport function

from ._Type cimport Type
from ._Pokemon cimport Pokemon
from ._StatsChange cimport StatsChange
from ._StatusChange cimport StatusChange


cdef extern from "<array>" namespace "std" nogil:
	cdef cppclass ArrayMoves "array<PokemonGen1::Move, 256>":
		ArrayMoves() except+
		Move &operator[](size_t)

cdef extern from "<GameEngine/Move.hpp>" namespace "PokemonGen1":
	ctypedef enum MoveCategory:
		PHYSICAL,
		SPECIAL,
		STATUS

	cdef cppclass Move:
		struct StatusChangeProb:
			StatusChange status
			unsigned char cmpVal

		struct StatsChangeProb:
			StatsChange stat
			char nb
			unsigned char cmpVal

		#Move(
		#	unsigned char id,
		#	const string &name,
		#	Type type,
		#	MoveCategory category,
		#	unsigned int power,
		#	unsigned char accuracy,
		#	unsigned char maxpp,
		#	StatusChangeProb statusChange,
		#	vector[StatsChangeProb] ownerChange,
		#	vector[StatsChangeProb] foeChange,
		#	pair[unsigned, unsigned] nbHits,
		#	pair[unsigned, unsigned] nbRuns,
		#	const string &keepGoingMsg,
		#	char priority,
		#	double critChance,
		#	bool needLoading,
		#	const string &loadingMsg,
		#	bool invulnerableDuringLoading,
		#	bool needRecharge,
		#	const HitCallback &&hitCallback,
		#	const string &hitCallBackDescription,
		#	const MissCallback &&missCallback,
		#	const string &missCallBackDescription
		#)
		Move(const Move &)
		Move &operator=(const Move &)

		const pair[unsigned int, unsigned int] &getNbRuns()
		const pair[unsigned int, unsigned int] &getNbHits()
		const Move.StatusChangeProb &getStatusChange()
		const vector[Move.StatsChangeProb] &getOwnerChange()
		const vector[Move.StatsChangeProb] &getFoeChange()
		bool needsLoading()
		bool isInvulnerableDuringLoading()
		bool needsRecharge()
		const string &getHitCallBackDescription()
		const string &getMissCallBackDescription()
		double getCritChance()
		unsigned char getAccuracy()
		MoveCategory getCategory()
		bool makesInvulnerable()
		unsigned char getMaxPP()
		unsigned int getPower()
		unsigned char getPPUp()
		Type getType()
		unsigned char getPP()
		unsigned char getID()
		string getName()
		const string &getDescription()
		char getPriority()
		bool isFinished()
		unsigned char getHitsLeft()

		void glitch()
		void setPP(unsigned char pp)
		void setPPUp(unsigned char nb)
		void setHitsLeft(unsigned char nb)
		void reset()

		bool attack(Pokemon &owner, Pokemon &target, const function[void (const string &)] &logger)

	extern const ArrayMoves availableMoves

	ctypedef enum AvailableMove:
		Empty "PokemonGen1::None",
		Pound,
		Karate_Chop,
		Doubleslap,
		Comet_Punch,
		Mega_Punch,
		Pay_Day,
		Fire_Punch,
		Ice_Punch,
		Thunder_Punch,
		Scratch,
		Vise_Grip,
		Guillotine,
		Razor_Wind,
		Swords_Dance,
		Cut,
		Gust,
		Wing_Attack,
		Whirlwind,
		Fly,
		Bind,
		Slam,
		Vine_Whip,
		Stomp,
		Double_Kick,
		Mega_Kick,
		Jump_Kick,
		Rolling_Kick,
		Sand_Attack,
		Headbutt,
		Horn_Attack,
		Fury_Attack,
		Horn_Drill,
		Tackle,
		Body_Slam,
		Wrap,
		Take_Down,
		Thrash,
		Double_Edge,
		Tail_Whip,
		Poison_Sting,
		Twineedle,
		Pin_Missile,
		Leer,
		Bite,
		Growl,
		Roar,
		Sing,
		Supersonic,
		Sonic_Boom,
		Disable,
		Acid,
		Ember,
		Flamethrower,
		Mist,
		Water_Gun,
		Hydro_Pump,
		Surf,
		Ice_Beam,
		Blizzard,
		Psybeam,
		Bubblebeam,
		Aurora_Beam,
		Hyper_Beam,
		Peck,
		Drill_Peck,
		Submission,
		Low_Kick,
		Counter,
		Seismic_Toss,
		Strength,
		Absorb,
		Mega_Drain,
		Leech_Seed,
		Growth,
		Razor_Leaf,
		Solarbeam,
		Poisonpowder,
		Stun_Spore,
		Sleep_Powder,
		Petal_Dance,
		String_Shot,
		Dragon_Rage,
		Fire_Spin,
		Thundershock,
		Thunderbolt,
		Thunder_Wave,
		Thunder,
		Rock_Throw,
		Earthquake,
		Fissure,
		Dig,
		Toxic,
		Confusion,
		Psychic_M,
		Hypnosis,
		Meditate,
		Agility,
		Quick_Attack,
		Rage,
		Teleport,
		Night_Shade,
		Mimic,
		Screech,
		Double_Team,
		Recover,
		Harden,
		Minimize,
		Smokescreen,
		Confuse_Ray,
		Withdraw,
		Defense_Curl,
		Barrier,
		Light_Screen,
		Haze,
		Reflect,
		Focus_Energy,
		Bide,
		Metronome,
		Mirror_Move,
		Self_Destruct,
		Egg_Bomb,
		Lick,
		Smog,
		Sludge,
		Bone_Club,
		Fire_Blast,
		Waterfall,
		Clamp,
		Swift,
		Skull_Bash,
		Spike_Cannon,
		Constrict,
		Amnesia,
		Kinesis,
		Softboiled,
		Hi_Jump_Kick,
		Glare,
		Dream_Eater,
		Poison_Gas,
		Barrage,
		Leech_Life,
		Lovely_Kiss,
		Sky_Attack,
		Transform,
		Bubble,
		Dizzy_Punch,
		Spore,
		Flash,
		Psywave,
		Splash,
		Acid_Armor,
		Crabhammer,
		Explosion,
		Fury_Swipes,
		Bonemerang,
		Rest,
		Rock_Slide,
		Hyper_Fang,
		Sharpen,
		Conversion,
		Tri_Attack,
		Super_Fang,
		Slash,
		Substitute,
		Struggle