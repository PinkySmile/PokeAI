//
// Created by PinkySmile on 07/08/2025.
//

#ifndef POKEAI_MOVE_H
#define POKEAI_MOVE_H


#include "StatusChange.h"
#include "StatsChange.h"
#include "Type.h"

#ifdef BUILD_LIB
#include "GameEngine/Move.hpp"
namespace PokemonGen1 {
	class Pokemon;
}
typedef PokemonGen1::Pokemon PokemonGen1_Pokemon;
#endif

typedef enum PokemonGen1_AvailableMove
{
	PokemonGen1_Move_None = 0x00,
	PokemonGen1_Move_Pound = 0x01,
	PokemonGen1_Move_Karate_Chop = 0x02,
	PokemonGen1_Move_Doubleslap = 0x03,
	PokemonGen1_Move_Comet_Punch = 0x04,
	PokemonGen1_Move_Mega_Punch = 0x05,
	PokemonGen1_Move_Pay_Day = 0x06,
	PokemonGen1_Move_Fire_Punch = 0x07,
	PokemonGen1_Move_Ice_Punch = 0x08,
	PokemonGen1_Move_Thunder_Punch = 0x09,
	PokemonGen1_Move_Scratch = 0x0A,
	PokemonGen1_Move_Vise_Grip = 0x0B,
	PokemonGen1_Move_Guillotine = 0x0C,
	PokemonGen1_Move_Razor_Wind = 0x0D,
	PokemonGen1_Move_Swords_Dance = 0x0E,
	PokemonGen1_Move_Cut = 0x0F,
	PokemonGen1_Move_Gust = 0x10,
	PokemonGen1_Move_Wing_Attack = 0x11,
	PokemonGen1_Move_Whirlwind = 0x12,
	PokemonGen1_Move_Fly = 0x13,
	PokemonGen1_Move_Bind = 0x14,
	PokemonGen1_Move_Slam = 0x15,
	PokemonGen1_Move_Vine_Whip = 0x16,
	PokemonGen1_Move_Stomp = 0x17,
	PokemonGen1_Move_Double_Kick = 0x18,
	PokemonGen1_Move_Mega_Kick = 0x19,
	PokemonGen1_Move_Jump_Kick = 0x1A,
	PokemonGen1_Move_Rolling_Kick = 0x1B,
	PokemonGen1_Move_Sand_Attack = 0x1C,
	PokemonGen1_Move_Headbutt = 0x1D,
	PokemonGen1_Move_Horn_Attack = 0x1E,
	PokemonGen1_Move_Fury_Attack = 0x1F,
	PokemonGen1_Move_Horn_Drill = 0x20,
	PokemonGen1_Move_Tackle = 0x21,
	PokemonGen1_Move_Body_Slam = 0x22,
	PokemonGen1_Move_Wrap = 0x23,
	PokemonGen1_Move_Take_Down = 0x24,
	PokemonGen1_Move_Thrash = 0x25,
	PokemonGen1_Move_Double_Edge = 0x26,
	PokemonGen1_Move_Tail_Whip = 0x27,
	PokemonGen1_Move_Poison_Sting = 0x28,
	PokemonGen1_Move_Twineedle = 0x29,
	PokemonGen1_Move_Pin_Missile = 0x2A,
	PokemonGen1_Move_Leer = 0x2B,
	PokemonGen1_Move_Bite = 0x2C,
	PokemonGen1_Move_Growl = 0x2D,
	PokemonGen1_Move_Roar = 0x2E,
	PokemonGen1_Move_Sing = 0x2F,
	PokemonGen1_Move_Supersonic = 0x30,
	PokemonGen1_Move_Sonic_Boom = 0x31,
	PokemonGen1_Move_Disable = 0x32,
	PokemonGen1_Move_Acid = 0x33,
	PokemonGen1_Move_Ember = 0x34,
	PokemonGen1_Move_Flamethrower = 0x35,
	PokemonGen1_Move_Mist = 0x36,
	PokemonGen1_Move_Water_Gun = 0x37,
	PokemonGen1_Move_Hydro_Pump = 0x38,
	PokemonGen1_Move_Surf = 0x39,
	PokemonGen1_Move_Ice_Beam = 0x3A,
	PokemonGen1_Move_Blizzard = 0x3B,
	PokemonGen1_Move_Psybeam = 0x3C,
	PokemonGen1_Move_Bubble_Beam = 0x3D,
	PokemonGen1_Move_Aurora_Beam = 0x3E,
	PokemonGen1_Move_Hyper_Beam = 0x3F,
	PokemonGen1_Move_Peck = 0x40,
	PokemonGen1_Move_Drill_Peck = 0x41,
	PokemonGen1_Move_Submission = 0x42,
	PokemonGen1_Move_Low_Kick = 0x43,
	PokemonGen1_Move_Counter = 0x44,
	PokemonGen1_Move_Seismic_Toss = 0x45,
	PokemonGen1_Move_Strength = 0x46,
	PokemonGen1_Move_Absorb = 0x47,
	PokemonGen1_Move_Mega_Drain = 0x48,
	PokemonGen1_Move_Leech_Seed = 0x49,
	PokemonGen1_Move_Growth = 0x4A,
	PokemonGen1_Move_Razor_Leaf = 0x4B,
	PokemonGen1_Move_Solar_Beam = 0x4C,
	PokemonGen1_Move_Poison_Powder = 0x4D,
	PokemonGen1_Move_Stun_Spore = 0x4E,
	PokemonGen1_Move_Sleep_Powder = 0x4F,
	PokemonGen1_Move_Petal_Dance = 0x50,
	PokemonGen1_Move_String_Shot = 0x51,
	PokemonGen1_Move_Dragon_Rage = 0x52,
	PokemonGen1_Move_Fire_Spin = 0x53,
	PokemonGen1_Move_Thundershock = 0x54,
	PokemonGen1_Move_Thunderbolt = 0x55,
	PokemonGen1_Move_Thunder_Wave = 0x56,
	PokemonGen1_Move_Thunder = 0x57,
	PokemonGen1_Move_Rock_Throw = 0x58,
	PokemonGen1_Move_Earthquake = 0x59,
	PokemonGen1_Move_Fissure = 0x5A,
	PokemonGen1_Move_Dig = 0x5B,
	PokemonGen1_Move_Toxic = 0x5C,
	PokemonGen1_Move_Confusion = 0x5D,
	PokemonGen1_Move_Psychic_M = 0x5E,
	PokemonGen1_Move_Hypnosis = 0x5F,
	PokemonGen1_Move_Meditate = 0x60,
	PokemonGen1_Move_Agility = 0x61,
	PokemonGen1_Move_Quick_Attack = 0x62,
	PokemonGen1_Move_Rage = 0x63,
	PokemonGen1_Move_Teleport = 0x64,
	PokemonGen1_Move_Night_Shade = 0x65,
	PokemonGen1_Move_Mimic = 0x66,
	PokemonGen1_Move_Screech = 0x67,
	PokemonGen1_Move_Double_Team = 0x68,
	PokemonGen1_Move_Recover = 0x69,
	PokemonGen1_Move_Harden = 0x6A,
	PokemonGen1_Move_Minimize = 0x6B,
	PokemonGen1_Move_Smokescreen = 0x6C,
	PokemonGen1_Move_Confuse_Ray = 0x6D,
	PokemonGen1_Move_Withdraw = 0x6E,
	PokemonGen1_Move_Defense_Curl = 0x6F,
	PokemonGen1_Move_Barrier = 0x70,
	PokemonGen1_Move_Light_Screen = 0x71,
	PokemonGen1_Move_Haze = 0x72,
	PokemonGen1_Move_Reflect = 0x73,
	PokemonGen1_Move_Focus_Energy = 0x74,
	PokemonGen1_Move_Bide = 0x75,
	PokemonGen1_Move_Metronome = 0x76,
	PokemonGen1_Move_Mirror_Move = 0x77,
	PokemonGen1_Move_Self_Destruct = 0x78,
	PokemonGen1_Move_Egg_Bomb = 0x79,
	PokemonGen1_Move_Lick = 0x7A,
	PokemonGen1_Move_Smog = 0x7B,
	PokemonGen1_Move_Sludge = 0x7C,
	PokemonGen1_Move_Bone_Club = 0x7D,
	PokemonGen1_Move_Fire_Blast = 0x7E,
	PokemonGen1_Move_Waterfall = 0x7F,
	PokemonGen1_Move_Clamp = 0x80,
	PokemonGen1_Move_Swift = 0x81,
	PokemonGen1_Move_Skull_Bash = 0x82,
	PokemonGen1_Move_Spike_Cannon = 0x83,
	PokemonGen1_Move_Constrict = 0x84,
	PokemonGen1_Move_Amnesia = 0x85,
	PokemonGen1_Move_Kinesis = 0x86,
	PokemonGen1_Move_Soft_Boiled = 0x87,
	PokemonGen1_Move_Hi_Jump_Kick = 0x88,
	PokemonGen1_Move_Glare = 0x89,
	PokemonGen1_Move_Dream_Eater = 0x8A,
	PokemonGen1_Move_Poison_Gas = 0x8B,
	PokemonGen1_Move_Barrage = 0x8C,
	PokemonGen1_Move_Leech_Life = 0x8D,
	PokemonGen1_Move_Lovely_Kiss = 0x8E,
	PokemonGen1_Move_Sky_Attack = 0x8F,
	PokemonGen1_Move_Transform = 0x90,
	PokemonGen1_Move_Bubble = 0x91,
	PokemonGen1_Move_Dizzy_Punch = 0x92,
	PokemonGen1_Move_Spore = 0x93,
	PokemonGen1_Move_Flash = 0x94,
	PokemonGen1_Move_Psywave = 0x95,
	PokemonGen1_Move_Splash = 0x96,
	PokemonGen1_Move_Acid_Armor = 0x97,
	PokemonGen1_Move_Crabhammer = 0x98,
	PokemonGen1_Move_Explosion = 0x99,
	PokemonGen1_Move_Fury_Swipes = 0x9A,
	PokemonGen1_Move_Bonemerang = 0x9B,
	PokemonGen1_Move_Rest = 0x9C,
	PokemonGen1_Move_Rock_Slide = 0x9D,
	PokemonGen1_Move_Hyper_Fang = 0x9E,
	PokemonGen1_Move_Sharpen = 0x9F,
	PokemonGen1_Move_Conversion = 0xA0,
	PokemonGen1_Move_Tri_Attack = 0xA1,
	PokemonGen1_Move_Super_Fang = 0xA2,
	PokemonGen1_Move_Slash = 0xA3,
	PokemonGen1_Move_Substitute = 0xA4,
	PokemonGen1_Move_Struggle = 0xA5
} PokemonGen1_AvailableMove;


typedef enum PokemonGen1_MoveCategory {
	PokemonGen1_PHYSICAL,
	PokemonGen1_SPECIAL,
	PokemonGen1_STATUS,
} PokemonGen1_MoveCategory;

#ifdef BUILD_LIB
typedef PokemonGen1::Move::StatusChangeProb PokemonGen1_Move_StatusChangeProb;
typedef PokemonGen1::Move::StatsChangeProb PokemonGen1_Move_StatsChangeProb;
#else
typedef struct PokemonGen1_Move_StatusChangeProb {
	PokemonGen1_StatusChange status;
	unsigned char            cmpVal;
} PokemonGen1_Move_StatusChangeProb;
typedef struct PokemonGen1_Move_StatsChangeProb {
	PokemonGen1_StatsChange stat;
	char                    nb;
	unsigned char           cmpVal;
} PokemonGen1_Move_StatsChangeProb;
#endif
typedef struct PokemonGen1_Move_MinMax {
	unsigned min;
	unsigned max;
} PokemonGen1_Move_MinMax;


TYPEDEF(Move, Move);

GEN1API PokemonGen1_Move *PokemonGen1_Move_create(
	unsigned char id,
	const char *&name,
	PokemonGen1_Type type,
	PokemonGen1_MoveCategory category,
	unsigned int power,
	unsigned char accuracy,
	unsigned char maxpp,
	PokemonGen1_Move_StatusChangeProb statusChange,
	PokemonGen1_Move_StatsChangeProb *ownerChange, size_t ownerChangeCount,
	PokemonGen1_Move_StatsChangeProb *foeChange, size_t foeChangeCount,
	PokemonGen1_Move_MinMax nbHits,
	PokemonGen1_Move_MinMax nbRuns,
	const char *keepGoingMsg,
	char priority,
	double critChance,
	bool needLoading,
	const char *loadingMsg,
	bool invulnerableDuringLoading,
	bool needRecharge,
	bool (*hitCallback)(PokemonGen1_Pokemon *owner, PokemonGen1_Pokemon *target, unsigned damage, bool last, void (*logger)(const char *msg)),
	const char *hitCallBackDescription,
	bool (*missCallback)(PokemonGen1_Pokemon *owner, PokemonGen1_Pokemon *target, bool last, void (*logger)(const char *msg)),
	const char *missCallBackDescription
);
GEN1API PokemonGen1_Move *PokemonGen1_Move_copy(PokemonGen1_Move *object);
GEN1API void PokemonGen1_Move_assign_to(PokemonGen1_Move *object, PokemonGen1_Move *target);

GEN1API PokemonGen1_Move_MinMax PokemonGen1_Move_getNbRuns(const PokemonGen1_Move *object);
GEN1API PokemonGen1_Move_MinMax PokemonGen1_Move_getNbHits(const PokemonGen1_Move *object);
GEN1API PokemonGen1_Move_StatusChangeProb PokemonGen1_Move_getStatusChange(const PokemonGen1_Move *object);
GEN1API const PokemonGen1_Move_StatsChangeProb *PokemonGen1_Move_getOwnerChange(const PokemonGen1_Move *object, size_t *size);
GEN1API const PokemonGen1_Move_StatsChangeProb *PokemonGen1_Move_getFoeChange(const PokemonGen1_Move *object, size_t *size);
GEN1API bool PokemonGen1_Move_needsLoading(const PokemonGen1_Move *object);
GEN1API bool PokemonGen1_Move_isInvulnerableDuringLoading(const PokemonGen1_Move *object);
GEN1API bool PokemonGen1_Move_needsRecharge(const PokemonGen1_Move *object);
GEN1API const char *PokemonGen1_Move_getHitCallBackDescription(const PokemonGen1_Move *object);
GEN1API const char *PokemonGen1_Move_getMissCallBackDescription(const PokemonGen1_Move *object);
GEN1API double PokemonGen1_Move_getCritChance(const PokemonGen1_Move *object);
GEN1API unsigned char PokemonGen1_Move_getAccuracy(const PokemonGen1_Move *object);
GEN1API PokemonGen1_MoveCategory PokemonGen1_Move_getCategory(const PokemonGen1_Move *object);
GEN1API bool PokemonGen1_Move_makesInvulnerable(const PokemonGen1_Move *object);
GEN1API unsigned char PokemonGen1_Move_getMaxPP(const PokemonGen1_Move *object);
GEN1API unsigned int PokemonGen1_Move_getPower(const PokemonGen1_Move *object);
GEN1API unsigned char PokemonGen1_Move_getPPUp(const PokemonGen1_Move *object);
GEN1API PokemonGen1_Type PokemonGen1_Move_getType(const PokemonGen1_Move *object);
GEN1API unsigned char PokemonGen1_Move_getPP(const PokemonGen1_Move *object);
GEN1API unsigned char PokemonGen1_Move_getID(const PokemonGen1_Move *object);
GEN1API const char *PokemonGen1_Move_getName(const PokemonGen1_Move *object);
GEN1API const char *PokemonGen1_Move_getDescription(const PokemonGen1_Move *object);
GEN1API char PokemonGen1_Move_getPriority(const PokemonGen1_Move *object);
GEN1API bool PokemonGen1_Move_isFinished(const PokemonGen1_Move *object);
GEN1API unsigned char PokemonGen1_Move_getHitsLeft(const PokemonGen1_Move *object);

GEN1API void PokemonGen1_Move_glitch(PokemonGen1_Move *object);
GEN1API void PokemonGen1_Move_setPP(PokemonGen1_Move *object, unsigned char pp);
GEN1API void PokemonGen1_Move_setPPUp(PokemonGen1_Move *object, unsigned char nb);
GEN1API void PokemonGen1_Move_setHitsLeft(PokemonGen1_Move *object, unsigned char nb);
GEN1API void PokemonGen1_Move_reset(PokemonGen1_Move *object);

GEN1API bool PokemonGen1_Move_attack(PokemonGen1_Move *object, PokemonGen1_Pokemon *owner, PokemonGen1_Pokemon *target, void (*logger)(const char *msg));


#endif //POKEAI_MOVE_H
