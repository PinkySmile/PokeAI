//
// Created by PinkySmile on 06/08/2025.
//

#ifndef POKEAI_POKEMON_H
#define POKEAI_POKEMON_H


#include "c_cpp.h"
#include "Type.h"
#include "Move.h"

#define MAX_POKEMON_ID 256

#ifdef BUILD_LIB
#include "GameEngine/Pokemon.hpp"
typedef PokemonGen1::Pokemon::BaseStats PokemonGen1_Pokemon_BaseStats;
#else
struct PokemonGen1_Pokemon_BaseStats {
	unsigned       HP;
	unsigned       maxHP;
	unsigned short ATK;
	unsigned short DEF;
	unsigned short SPD;
	unsigned short SPE;
};
#endif

typedef struct PokemonGen1_Pokemon_Base {
	unsigned char                 id;
	unsigned char                 dexId;
	const char                    *name;
	unsigned                      HP;
	unsigned short                ATK;
	unsigned short                DEF;
	unsigned short                SPD;
	unsigned short                SPE;
	PokemonGen1_Type              typeA;
	PokemonGen1_Type              typeB;
	unsigned char                 catchRate;
	unsigned int                  baseXpYield;
	PokemonGen1_AvailableMove     *movePool;
	size_t                        movePoolSize;
	PokemonGen1_Pokemon_BaseStats statsAtLevel[256];
} PokemonGen1_Pokemon_Base;

TYPEDEF(Pokemon, Pokemon);



enum PokemonSpecies
{
	PokemonGen1_Species_Rhydon = 0x01,
	PokemonGen1_Species_Kangaskhan = 0x02,
	PokemonGen1_Species_Nidoran_M = 0x03,
	PokemonGen1_Species_Clefairy = 0x04,
	PokemonGen1_Species_Spearow = 0x05,
	PokemonGen1_Species_Voltorb = 0x06,
	PokemonGen1_Species_Nidoking = 0x07,
	PokemonGen1_Species_Slowbro = 0x08,
	PokemonGen1_Species_Ivysaur = 0x09,
	PokemonGen1_Species_Exeggutor = 0x0A,
	PokemonGen1_Species_Lickitung = 0x0B,
	PokemonGen1_Species_Exeggcute = 0x0C,
	PokemonGen1_Species_Grimer = 0x0D,
	PokemonGen1_Species_Gengar = 0x0E,
	PokemonGen1_Species_Nidoran_F = 0x0F,
	PokemonGen1_Species_Nidoqueen = 0x10,
	PokemonGen1_Species_Cubone = 0x11,
	PokemonGen1_Species_Rhyhorn = 0x12,
	PokemonGen1_Species_Lapras = 0x13,
	PokemonGen1_Species_Arcanine = 0x14,
	PokemonGen1_Species_Mew = 0x15,
	PokemonGen1_Species_Gyarados = 0x16,
	PokemonGen1_Species_Shellder = 0x17,
	PokemonGen1_Species_Tentacool = 0x18,
	PokemonGen1_Species_Gastly = 0x19,
	PokemonGen1_Species_Scyther = 0x1A,
	PokemonGen1_Species_Staryu = 0x1B,
	PokemonGen1_Species_Blastoise = 0x1C,
	PokemonGen1_Species_Pinsir = 0x1D,
	PokemonGen1_Species_Tangela = 0x1E,
	PokemonGen1_Species_Growlithe = 0x21,
	PokemonGen1_Species_Onix = 0x22,
	PokemonGen1_Species_Fearow = 0x23,
	PokemonGen1_Species_Pidgey = 0x24,
	PokemonGen1_Species_Slowpoke = 0x25,
	PokemonGen1_Species_Kadabra = 0x26,
	PokemonGen1_Species_Graveler = 0x27,
	PokemonGen1_Species_Chansey = 0x28,
	PokemonGen1_Species_Machoke = 0x29,
	PokemonGen1_Species_Mr_Mime = 0x2A,
	PokemonGen1_Species_Hitmonlee = 0x2B,
	PokemonGen1_Species_Hitmonchan = 0x2C,
	PokemonGen1_Species_Arbok = 0x2D,
	PokemonGen1_Species_Parasect = 0x2E,
	PokemonGen1_Species_Psyduck = 0x2F,
	PokemonGen1_Species_Drowzee = 0x30,
	PokemonGen1_Species_Golem = 0x31,
	PokemonGen1_Species_Magmar = 0x33,
	PokemonGen1_Species_Electabuzz = 0x35,
	PokemonGen1_Species_Magneton = 0x36,
	PokemonGen1_Species_Koffing = 0x37,
	PokemonGen1_Species_Mankey = 0x39,
	PokemonGen1_Species_Seel = 0x3A,
	PokemonGen1_Species_Diglett = 0x3B,
	PokemonGen1_Species_Tauros = 0x3C,
	PokemonGen1_Species_Farfetchd = 0x40,
	PokemonGen1_Species_Venonat = 0x41,
	PokemonGen1_Species_Dragonite = 0x42,
	PokemonGen1_Species_Doduo = 0x46,
	PokemonGen1_Species_Poliwag = 0x47,
	PokemonGen1_Species_Jynx = 0x48,
	PokemonGen1_Species_Moltres = 0x49,
	PokemonGen1_Species_Articuno = 0x4A,
	PokemonGen1_Species_Zapdos = 0x4B,
	PokemonGen1_Species_Ditto = 0x4C,
	PokemonGen1_Species_Meowth = 0x4D,
	PokemonGen1_Species_Krabby = 0x4E,
	PokemonGen1_Species_Vulpix = 0x52,
	PokemonGen1_Species_Ninetales = 0x53,
	PokemonGen1_Species_Pikachu = 0x54,
	PokemonGen1_Species_Raichu = 0x55,
	PokemonGen1_Species_Dratini = 0x58,
	PokemonGen1_Species_Dragonair = 0x59,
	PokemonGen1_Species_Kabuto = 0x5A,
	PokemonGen1_Species_Kabutops = 0x5B,
	PokemonGen1_Species_Horsea = 0x5C,
	PokemonGen1_Species_Seadra = 0x5D,
	PokemonGen1_Species_Sandshrew = 0x60,
	PokemonGen1_Species_Sandslash = 0x61,
	PokemonGen1_Species_Omanyte = 0x62,
	PokemonGen1_Species_Omastar = 0x63,
	PokemonGen1_Species_Jigglypuff = 0x64,
	PokemonGen1_Species_Wigglytuff = 0x65,
	PokemonGen1_Species_Eevee = 0x66,
	PokemonGen1_Species_Flareon = 0x67,
	PokemonGen1_Species_Jolteon = 0x68,
	PokemonGen1_Species_Vaporeon = 0x69,
	PokemonGen1_Species_Machop = 0x6A,
	PokemonGen1_Species_Zubat = 0x6B,
	PokemonGen1_Species_Ekans = 0x6C,
	PokemonGen1_Species_Paras = 0x6D,
	PokemonGen1_Species_Poliwhirl = 0x6E,
	PokemonGen1_Species_Poliwrath = 0x6F,
	PokemonGen1_Species_Weedle = 0x70,
	PokemonGen1_Species_Kakuna = 0x71,
	PokemonGen1_Species_Beedrill = 0x72,
	PokemonGen1_Species_Dodrio = 0x74,
	PokemonGen1_Species_Primeape = 0x75,
	PokemonGen1_Species_Dugtrio = 0x76,
	PokemonGen1_Species_Venomoth = 0x77,
	PokemonGen1_Species_Dewgong = 0x78,
	PokemonGen1_Species_Caterpie = 0x7B,
	PokemonGen1_Species_Metapod = 0x7C,
	PokemonGen1_Species_Butterfree = 0x7D,
	PokemonGen1_Species_Machamp = 0x7E,
	PokemonGen1_Species_Golduck = 0x80,
	PokemonGen1_Species_Hypno = 0x81,
	PokemonGen1_Species_Golbat = 0x82,
	PokemonGen1_Species_Mewtwo = 0x83,
	PokemonGen1_Species_Snorlax = 0x84,
	PokemonGen1_Species_Magikarp = 0x85,
	PokemonGen1_Species_Muk = 0x88,
	PokemonGen1_Species_Kingler = 0x8A,
	PokemonGen1_Species_Cloyster = 0x8B,
	PokemonGen1_Species_Electrode = 0x8D,
	PokemonGen1_Species_Clefable = 0x8E,
	PokemonGen1_Species_Weezing = 0x8F,
	PokemonGen1_Species_Persian = 0x90,
	PokemonGen1_Species_Marowak = 0x91,
	PokemonGen1_Species_Haunter = 0x93,
	PokemonGen1_Species_Abra = 0x94,
	PokemonGen1_Species_Alakazam = 0x95,
	PokemonGen1_Species_Pidgeotto = 0x96,
	PokemonGen1_Species_Pidgeot = 0x97,
	PokemonGen1_Species_Starmie = 0x98,
	PokemonGen1_Species_Bulbasaur = 0x99,
	PokemonGen1_Species_Venusaur = 0x9A,
	PokemonGen1_Species_Tentacruel = 0x9B,
	PokemonGen1_Species_Goldeen = 0x9D,
	PokemonGen1_Species_Seaking = 0x9E,
	PokemonGen1_Species_Ponyta = 0xA3,
	PokemonGen1_Species_Rapidash = 0xA4,
	PokemonGen1_Species_Rattata = 0xA5,
	PokemonGen1_Species_Raticate = 0xA6,
	PokemonGen1_Species_Nidorino = 0xA7,
	PokemonGen1_Species_Nidorina = 0xA8,
	PokemonGen1_Species_Geodude = 0xA9,
	PokemonGen1_Species_Porygon = 0xAA,
	PokemonGen1_Species_Aerodactyl = 0xAB,
	PokemonGen1_Species_Magnemite = 0xAD,
	PokemonGen1_Species_Charmander = 0xB0,
	PokemonGen1_Species_Squirtle = 0xB1,
	PokemonGen1_Species_Charmeleon = 0xB2,
	PokemonGen1_Species_Wartortle = 0xB3,
	PokemonGen1_Species_Charizard = 0xB4,
	PokemonGen1_Species_Oddish = 0xB9,
	PokemonGen1_Species_Gloom = 0xBA,
	PokemonGen1_Species_Vileplume = 0xBB,
	PokemonGen1_Species_Bellsprout = 0xBC,
	PokemonGen1_Species_Weepinbell = 0xBD,
	PokemonGen1_Species_Victreebel = 0xBE,
	PokemonGen1_Species_Missingno = 0xFF
};

GEN1API PokemonGen1_Pokemon_Base *PokemonGen1_getPokemonBase(size_t i);
GEN1API void PokemonGen1_Pokemon_Base_destroy(PokemonGen1_Pokemon_Base *);


#endif //POKEAI_POKEMON_H
