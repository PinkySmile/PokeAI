# cython: language_level=3

from libcpp cimport bool
from libcpp.string cimport string
from libcpp.variant import variant, get


cdef extern from "<variant>" namespace "std" nogil:
	cdef cppclass Event "PkmnCommon::Event":
		size_t index();


cdef extern from "<GameEngine/Event.hpp>" namespace "PkmnCommon":
	struct TextEvent:
		string message;

	ctypedef enum MoveIndex:
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

	struct MoveEvent:
		unsigned moveId;
		bool player;
		bool hideSubstitute;

	struct MoveMissEvent:
		unsigned moveId;
		bool player;

	ctypedef enum SystemAnim:
		SYSANIM_ASLEEP,
		SYSANIM_BURN,
		SYSANIM_FROZEN,
		SYSANIM_POISON,
		SYSANIM_BAD_POISON,
		SYSANIM_CONFUSED,
		SYSANIM_FLINCHED,
		SYSANIM_PARALYZED,
		SYSANIM_CONFUSED_HIT,
		SYSANIM_LEECHED,

		SYSANIM_ATK_DECREASE_BIG,
		SYSANIM_ATK_DECREASE,
		SYSANIM_ATK_INCREASE,
		SYSANIM_ATK_INCREASE_BIG,
		SYSANIM_DEF_DECREASE_BIG,
		SYSANIM_DEF_DECREASE,
		SYSANIM_DEF_INCREASE,
		SYSANIM_DEF_INCREASE_BIG,
		SYSANIM_SPA_DECREASE_BIG,
		SYSANIM_SPA_DECREASE,
		SYSANIM_SPA_INCREASE,
		SYSANIM_SPA_INCREASE_BIG,
		SYSANIM_SPD_DECREASE_BIG,
		SYSANIM_SPD_DECREASE,
		SYSANIM_SPD_INCREASE,
		SYSANIM_SPD_INCREASE_BIG,
		SYSANIM_SPE_DECREASE_BIG,
		SYSANIM_SPE_DECREASE,
		SYSANIM_SPE_INCREASE,
		SYSANIM_SPE_INCREASE_BIG,
		SYSANIM_ACC_DECREASE_BIG,
		SYSANIM_ACC_DECREASE,
		SYSANIM_ACC_INCREASE,
		SYSANIM_ACC_INCREASE_BIG,
		SYSANIM_EVD_DECREASE_BIG,
		SYSANIM_EVD_DECREASE,
		SYSANIM_EVD_INCREASE,
		SYSANIM_EVD_INCREASE_BIG,

		SYSANIM_RECHARGE,
		SYSANIM_SUB_BREAK,
		SYSANIM_WAKE_UP,
		SYSANIM_BACK_TO_SENSE,
		SYSANIM_THAWED,

		SYSANIM_NOW_ASLEEP,
		SYSANIM_NOW_FROZEN,
		SYSANIM_NOW_BURNED,
		SYSANIM_NOW_POISONED,
		SYSANIM_NOW_PARALYZED,
		SYSANIM_NOW_BADLY_POISONED,
		SYSANIM_NOW_CONFUSED


	struct AnimEvent:
		unsigned animId;
		bool isGuaranteed;
		bool player;
		bool turn;

	struct ExtraAnimEvent:
		unsigned moveId;
		unsigned index;
		bool player;

	struct HealthModEvent:
		unsigned newHealth;
		bool player;

	struct SwitchEvent:
		unsigned newPkmnId;
		bool player;

	struct WithdrawEvent:
		bool player;

	struct DeathEvent:
		bool player;

	struct HitEvent:
		bool veryEffective;
		bool notVeryEffective;
		bool player;
		bool hasEffect;

	struct GameStartEvent:
		pass

	struct GameEndEvent:
		bool p1Won;
		bool p2Won;
		bool p1Ran;
		bool p2Ran;

	struct TurnStartEvent:
		pass

	struct StatusClearedEvent:
		bool player;


	TextEvent getTextEvent(const Event &e);
	MoveEvent getMoveEvent(const Event &e);
	AnimEvent getAnimEvent(const Event &e);
	ExtraAnimEvent getExtraAnimEvent(const Event &e);
	HealthModEvent getHealthModEvent(const Event &e);
	SwitchEvent getSwitchEvent(const Event &e);
	WithdrawEvent getWithdrawEvent(const Event &e);
	DeathEvent getDeathEvent(const Event &e);
	HitEvent getHitEvent(const Event &e);
	StatusClearedEvent getStatusClearedEvent(const Event &e);
	TurnStartEvent getTurnStartEvent(const Event &e);
	MoveMissEvent getMoveMissEvent(const Event &e);
	GameStartEvent getGameStartEvent(const Event &e);
	GameEndEvent getGameEndEvent(const Event &e);


cdef inline dict dictFromTextEvent(const TextEvent &value):
	f: bytes = value.message
	return {
		'type': 'TextEvent',
		'message': f.decode('ASCII')
	}


cdef inline dict dictFromMoveEvent(const MoveEvent &value):
	return {
		'type': 'MoveEvent',
		'moveId': value.moveId,
		'player': value.player,
		'hideSubstitute': value.hideSubstitute,
	}


cdef inline dict dictFromMoveMissEvent(const MoveMissEvent &value):
	return {
		'type': 'MoveMissEvent',
		'moveId': value.moveId,
		'player': value.player,
	}


cdef inline dict dictFromAnimEvent(const AnimEvent &value):
	return {
		'type': 'AnimEvent',
		'animId': value.animId,
		'isGuaranteed': value.isGuaranteed,
		'player': value.player,
		'turn': value.turn,
	}


cdef inline dict dictFromExtraAnimEvent(const ExtraAnimEvent &value):
	return {
		'type': 'ExtraAnimEvent',
		'moveId': value.moveId,
		'index': value.index,
		'player': value.player,
	}


cdef inline dict dictFromHealthModEvent(const HealthModEvent &value):
	return {
		'type': 'HealthModEvent',
		'newHealth': value.newHealth,
		'player': value.player,
	}


cdef inline dict dictFromSwitchEvent(const SwitchEvent &value):
	return {
		'type': 'SwitchEvent',
		'newPkmnId': value.newPkmnId,
		'player': value.player,
	}


cdef inline dict dictFromWithdrawEvent(const WithdrawEvent &value):
	return {
		'type': 'WithdrawEvent',
		'player': value.player,
	}


cdef inline dict dictFromDeathEvent(const DeathEvent &value):
	return {
		'type': 'DeathEvent',
		'player': value.player,
	}


cdef inline dict dictFromHitEvent(const HitEvent &value):
	return {
		'type': 'HitEvent',
		'veryEffective': value.veryEffective,
		'notVeryEffective': value.notVeryEffective,
		'player': value.player,
		'hasEffect': value.hasEffect,
	}


cdef inline dict dictFromGameStartEvent(const GameStartEvent &value):
	return {
		'type': 'GameStartEvent',
	}


cdef inline dict dictFromGameEndEvent(const GameEndEvent &value):
	return {
		'type': 'GameEndEvent',
		'p1Won': value.p1Won,
		'p2Won': value.p2Won,
		'p1Ran': value.p1Ran,
		'p2Ran': value.p2Ran,
	}


cdef inline dict dictFromTurnStartEvent(const TurnStartEvent &value):
	return {
		'type': 'TurnStartEvent',
	}


cdef inline dict dictFromStatusClearedEvent(const StatusClearedEvent &value):
	return {
		'type': 'StatusClearedEvent',
		'player': value.player,
	}


cdef inline void evalLogger(void *func_p, const Event &msg) noexcept:
	result: dict | None = None
	index = msg.index()
	if index == 0:
		result = dictFromTextEvent(getTextEvent(msg))
	elif index == 1:
		result = dictFromMoveEvent(getMoveEvent(msg))
	elif index == 2:
		result = dictFromAnimEvent(getAnimEvent(msg))
	elif index == 3:
		result = dictFromExtraAnimEvent(getExtraAnimEvent(msg))
	elif index == 4:
		result = dictFromHealthModEvent(getHealthModEvent(msg))
	elif index == 5:
		result = dictFromSwitchEvent(getSwitchEvent(msg))
	elif index == 6:
		result = dictFromWithdrawEvent(getWithdrawEvent(msg))
	elif index == 7:
		result = dictFromDeathEvent(getDeathEvent(msg))
	elif index == 8:
		result = dictFromHitEvent(getHitEvent(msg))
	elif index == 9:
		result = dictFromStatusClearedEvent(getStatusClearedEvent(msg))
	elif index == 10:
		result = dictFromTurnStartEvent(getTurnStartEvent(msg))
	elif index == 11:
		result = dictFromMoveMissEvent(getMoveMissEvent(msg))
	elif index == 12:
		result = dictFromGameStartEvent(getGameStartEvent(msg))
	elif index == 13:
		result = dictFromGameEndEvent(getGameEndEvent(msg))
	assert result is not None
	(<object> func_p)(result)
