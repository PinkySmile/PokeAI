//
// Created by PinkySmile on 22/02/2026.
//

#include "ShowdownReader.hpp"

#include <cstring>

#include "Gen1ResultBasedGenerator.hpp"

static const char *ignore[] = {
	"j",
	"l",
	"n",
	"c",
	"t:",
	"gametype",
	"rated",
	"tier",
	"upkeep",
	"inactive",
	"inactiveoff",
	"rule",
	"raw",
	"html",
	"uhtml",
	"badge",
	"clearpoke",
	"poke",
	"teampreview",
	"-hint",
	"-mustrecharge",
	"-transform",
};

std::map<std::string, unsigned> statusNames{
	{ "frz", PkmnCommon::SYSANIM_NOW_FROZEN },
	{ "slp", PkmnCommon::SYSANIM_NOW_ASLEEP },
	{ "brn", PkmnCommon::SYSANIM_NOW_BURNED },
	{ "psn", PkmnCommon::SYSANIM_NOW_POISONED },
	{ "par", PkmnCommon::SYSANIM_NOW_PARALYZED },
	{ "tox", PkmnCommon::SYSANIM_NOW_BADLY_POISONED },
	{ "cfz", PkmnCommon::SYSANIM_NOW_CONFUSED },
};
std::map<std::string, unsigned> pokemonNames{
	{ "Rhydon",      PokemonGen1::Rhydon },
	{ "Kangaskhan",  PokemonGen1::Kangaskhan },
	{ "Nidoran-M",   PokemonGen1::Nidoran_M },
	{ "Nidoran_M",   PokemonGen1::Nidoran_M },
	{ "Clefairy",    PokemonGen1::Clefairy },
	{ "Spearow",     PokemonGen1::Spearow },
	{ "Voltorb",     PokemonGen1::Voltorb },
	{ "Nidoking",    PokemonGen1::Nidoking },
	{ "Slowbro",     PokemonGen1::Slowbro },
	{ "Ivysaur",     PokemonGen1::Ivysaur },
	{ "Exeggutor",   PokemonGen1::Exeggutor },
	{ "Lickitung",   PokemonGen1::Lickitung },
	{ "Exeggcute",   PokemonGen1::Exeggcute },
	{ "Grimer",      PokemonGen1::Grimer },
	{ "Gengar",      PokemonGen1::Gengar },
	{ "Nidoran-F",   PokemonGen1::Nidoran_F },
	{ "Nidoran_F",   PokemonGen1::Nidoran_F },
	{ "Nidoqueen",   PokemonGen1::Nidoqueen },
	{ "Cubone",      PokemonGen1::Cubone },
	{ "Rhyhorn",     PokemonGen1::Rhyhorn },
	{ "Lapras",      PokemonGen1::Lapras },
	{ "Arcanine",    PokemonGen1::Arcanine },
	{ "Mew",         PokemonGen1::Mew },
	{ "Gyarados",    PokemonGen1::Gyarados },
	{ "Shellder",    PokemonGen1::Shellder },
	{ "Tentacool",   PokemonGen1::Tentacool },
	{ "Gastly",      PokemonGen1::Gastly },
	{ "Scyther",     PokemonGen1::Scyther },
	{ "Staryu",      PokemonGen1::Staryu },
	{ "Blastoise",   PokemonGen1::Blastoise },
	{ "Pinsir",      PokemonGen1::Pinsir },
	{ "Tangela",     PokemonGen1::Tangela },
	{ "Growlithe",   PokemonGen1::Growlithe },
	{ "Onix",        PokemonGen1::Onix },
	{ "Fearow",      PokemonGen1::Fearow },
	{ "Pidgey",      PokemonGen1::Pidgey },
	{ "Slowpoke",    PokemonGen1::Slowpoke },
	{ "Kadabra",     PokemonGen1::Kadabra },
	{ "Graveler",    PokemonGen1::Graveler },
	{ "Chansey",     PokemonGen1::Chansey },
	{ "Machoke",     PokemonGen1::Machoke },
	{ "Mr. Mime",    PokemonGen1::Mr_Mime },
	{ "Mr_Mime",     PokemonGen1::Mr_Mime },
	{ "Hitmonlee",   PokemonGen1::Hitmonlee },
	{ "Hitmonchan",  PokemonGen1::Hitmonchan },
	{ "Arbok",       PokemonGen1::Arbok },
	{ "Parasect",    PokemonGen1::Parasect },
	{ "Psyduck",     PokemonGen1::Psyduck },
	{ "Drowzee",     PokemonGen1::Drowzee },
	{ "Golem",       PokemonGen1::Golem },
	{ "Magmar",      PokemonGen1::Magmar },
	{ "Electabuzz",  PokemonGen1::Electabuzz },
	{ "Magneton",    PokemonGen1::Magneton },
	{ "Koffing",     PokemonGen1::Koffing },
	{ "Mankey",      PokemonGen1::Mankey },
	{ "Seel",        PokemonGen1::Seel },
	{ "Diglett",     PokemonGen1::Diglett },
	{ "Tauros",      PokemonGen1::Tauros },
	{ "Farfetch'd",     PokemonGen1::Farfetchd },
	{ "Farfetch\u2019d", PokemonGen1::Farfetchd },
	{ "Farfetchd",      PokemonGen1::Farfetchd },
	{ "Venonat",     PokemonGen1::Venonat },
	{ "Dragonite",   PokemonGen1::Dragonite },
	{ "Doduo",       PokemonGen1::Doduo },
	{ "Poliwag",     PokemonGen1::Poliwag },
	{ "Jynx",        PokemonGen1::Jynx },
	{ "Moltres",     PokemonGen1::Moltres },
	{ "Articuno",    PokemonGen1::Articuno },
	{ "Zapdos",      PokemonGen1::Zapdos },
	{ "Ditto",       PokemonGen1::Ditto },
	{ "Meowth",      PokemonGen1::Meowth },
	{ "Krabby",      PokemonGen1::Krabby },
	{ "Vulpix",      PokemonGen1::Vulpix },
	{ "Ninetales",   PokemonGen1::Ninetales },
	{ "Pikachu",     PokemonGen1::Pikachu },
	{ "Raichu",      PokemonGen1::Raichu },
	{ "Dratini",     PokemonGen1::Dratini },
	{ "Dragonair",   PokemonGen1::Dragonair },
	{ "Kabuto",      PokemonGen1::Kabuto },
	{ "Kabutops",    PokemonGen1::Kabutops },
	{ "Horsea",      PokemonGen1::Horsea },
	{ "Seadra",      PokemonGen1::Seadra },
	{ "Sandshrew",   PokemonGen1::Sandshrew },
	{ "Sandslash",   PokemonGen1::Sandslash },
	{ "Omanyte",     PokemonGen1::Omanyte },
	{ "Omastar",     PokemonGen1::Omastar },
	{ "Jigglypuff",  PokemonGen1::Jigglypuff },
	{ "Wigglytuff",  PokemonGen1::Wigglytuff },
	{ "Eevee",       PokemonGen1::Eevee },
	{ "Flareon",     PokemonGen1::Flareon },
	{ "Jolteon",     PokemonGen1::Jolteon },
	{ "Vaporeon",    PokemonGen1::Vaporeon },
	{ "Machop",      PokemonGen1::Machop },
	{ "Zubat",       PokemonGen1::Zubat },
	{ "Ekans",       PokemonGen1::Ekans },
	{ "Paras",       PokemonGen1::Paras },
	{ "Poliwhirl",   PokemonGen1::Poliwhirl },
	{ "Poliwrath",   PokemonGen1::Poliwrath },
	{ "Weedle",      PokemonGen1::Weedle },
	{ "Kakuna",      PokemonGen1::Kakuna },
	{ "Beedrill",    PokemonGen1::Beedrill },
	{ "Dodrio",      PokemonGen1::Dodrio },
	{ "Primeape",    PokemonGen1::Primeape },
	{ "Dugtrio",     PokemonGen1::Dugtrio },
	{ "Venomoth",    PokemonGen1::Venomoth },
	{ "Dewgong",     PokemonGen1::Dewgong },
	{ "Caterpie",    PokemonGen1::Caterpie },
	{ "Metapod",     PokemonGen1::Metapod },
	{ "Butterfree",  PokemonGen1::Butterfree },
	{ "Machamp",     PokemonGen1::Machamp },
	{ "Golduck",     PokemonGen1::Golduck },
	{ "Hypno",       PokemonGen1::Hypno },
	{ "Golbat",      PokemonGen1::Golbat },
	{ "Mewtwo",      PokemonGen1::Mewtwo },
	{ "Snorlax",     PokemonGen1::Snorlax },
	{ "Magikarp",    PokemonGen1::Magikarp },
	{ "Muk",         PokemonGen1::Muk },
	{ "Kingler",     PokemonGen1::Kingler },
	{ "Cloyster",    PokemonGen1::Cloyster },
	{ "Electrode",   PokemonGen1::Electrode },
	{ "Clefable",    PokemonGen1::Clefable },
	{ "Weezing",     PokemonGen1::Weezing },
	{ "Persian",     PokemonGen1::Persian },
	{ "Marowak",     PokemonGen1::Marowak },
	{ "Haunter",     PokemonGen1::Haunter },
	{ "Abra",        PokemonGen1::Abra },
	{ "Alakazam",    PokemonGen1::Alakazam },
	{ "Pidgeotto",   PokemonGen1::Pidgeotto },
	{ "Pidgeot",     PokemonGen1::Pidgeot },
	{ "Starmie",     PokemonGen1::Starmie },
	{ "Bulbasaur",   PokemonGen1::Bulbasaur },
	{ "Venusaur",    PokemonGen1::Venusaur },
	{ "Tentacruel",  PokemonGen1::Tentacruel },
	{ "Goldeen",     PokemonGen1::Goldeen },
	{ "Seaking",     PokemonGen1::Seaking },
	{ "Ponyta",      PokemonGen1::Ponyta },
	{ "Rapidash",    PokemonGen1::Rapidash },
	{ "Rattata",     PokemonGen1::Rattata },
	{ "Raticate",    PokemonGen1::Raticate },
	{ "Nidorino",    PokemonGen1::Nidorino },
	{ "Nidorina",    PokemonGen1::Nidorina },
	{ "Geodude",     PokemonGen1::Geodude },
	{ "Porygon",     PokemonGen1::Porygon },
	{ "Aerodactyl",  PokemonGen1::Aerodactyl },
	{ "Magnemite",   PokemonGen1::Magnemite },
	{ "Charmander",  PokemonGen1::Charmander },
	{ "Squirtle",    PokemonGen1::Squirtle },
	{ "Charmeleon",  PokemonGen1::Charmeleon },
	{ "Wartortle",   PokemonGen1::Wartortle },
	{ "Charizard",   PokemonGen1::Charizard },
	{ "Oddish",      PokemonGen1::Oddish },
	{ "Gloom",       PokemonGen1::Gloom },
	{ "Vileplume",   PokemonGen1::Vileplume },
	{ "Bellsprout",  PokemonGen1::Bellsprout },
	{ "Weepinbell",  PokemonGen1::Weepinbell },
	{ "Victreebel",  PokemonGen1::Victreebel },
	{ "MissingNo.",  PokemonGen1::Missingno },
};
std::map<std::string, unsigned> movesNames{
	{ "Pound",         PokemonGen1::Pound },
	{ "Karate Chop",   PokemonGen1::Karate_Chop },
	{ "Doubleslap",    PokemonGen1::Doubleslap },
	{ "Comet Punch",   PokemonGen1::Comet_Punch },
	{ "Mega Punch",    PokemonGen1::Mega_Punch },
	{ "Pay Day",       PokemonGen1::Pay_Day },
	{ "Fire Punch",    PokemonGen1::Fire_Punch },
	{ "Ice Punch",     PokemonGen1::Ice_Punch },
	{ "Thunder Punch", PokemonGen1::Thunder_Punch },
	{ "Scratch",       PokemonGen1::Scratch },
	{ "Vise Grip",     PokemonGen1::Vise_Grip },
	{ "Guillotine",    PokemonGen1::Guillotine },
	{ "Razor Wind",    PokemonGen1::Razor_Wind },
	{ "Swords Dance",  PokemonGen1::Swords_Dance },
	{ "Cut",           PokemonGen1::Cut },
	{ "Gust",          PokemonGen1::Gust },
	{ "Wing Attack",   PokemonGen1::Wing_Attack },
	{ "Whirlwind",     PokemonGen1::Whirlwind },
	{ "Fly",           PokemonGen1::Fly },
	{ "Bind",          PokemonGen1::Bind },
	{ "Slam",          PokemonGen1::Slam },
	{ "Vine Whip",     PokemonGen1::Vine_Whip },
	{ "Stomp",         PokemonGen1::Stomp },
	{ "Double Kick",   PokemonGen1::Double_Kick },
	{ "Mega Kick",     PokemonGen1::Mega_Kick },
	{ "Jump Kick",     PokemonGen1::Jump_Kick },
	{ "Rolling Kick",  PokemonGen1::Rolling_Kick },
	{ "Sand Attack",   PokemonGen1::Sand_Attack },
	{ "Headbutt",      PokemonGen1::Headbutt },
	{ "Horn Attack",   PokemonGen1::Horn_Attack },
	{ "Fury Attack",   PokemonGen1::Fury_Attack },
	{ "Horn Drill",    PokemonGen1::Horn_Drill },
	{ "Tackle",        PokemonGen1::Tackle },
	{ "Body Slam",     PokemonGen1::Body_Slam },
	{ "Wrap",          PokemonGen1::Wrap },
	{ "Take Down",     PokemonGen1::Take_Down },
	{ "Thrash",        PokemonGen1::Thrash },
	{ "Double-Edge",   PokemonGen1::Double_Edge },
	{ "Double Edge",   PokemonGen1::Double_Edge },
	{ "Tail Whip",     PokemonGen1::Tail_Whip },
	{ "Poison Sting",  PokemonGen1::Poison_Sting },
	{ "Twineedle",     PokemonGen1::Twineedle },
	{ "Pin Missile",   PokemonGen1::Pin_Missile },
	{ "Leer",          PokemonGen1::Leer },
	{ "Bite",          PokemonGen1::Bite },
	{ "Growl",         PokemonGen1::Growl },
	{ "Roar",          PokemonGen1::Roar },
	{ "Sing",          PokemonGen1::Sing },
	{ "Supersonic",    PokemonGen1::Supersonic },
	{ "Sonic Boom",    PokemonGen1::Sonic_Boom },
	{ "Disable",       PokemonGen1::Disable },
	{ "Acid",          PokemonGen1::Acid },
	{ "Ember",         PokemonGen1::Ember },
	{ "Flamethrower",  PokemonGen1::Flamethrower },
	{ "Mist",          PokemonGen1::Mist },
	{ "Water Gun",     PokemonGen1::Water_Gun },
	{ "Hydro Pump",    PokemonGen1::Hydro_Pump },
	{ "Surf",          PokemonGen1::Surf },
	{ "Ice Beam",      PokemonGen1::Ice_Beam },
	{ "Blizzard",      PokemonGen1::Blizzard },
	{ "Psybeam",       PokemonGen1::Psybeam },
	{ "Bubblebeam",    PokemonGen1::Bubblebeam },
	{ "Bubble Beam",   PokemonGen1::Bubblebeam },
	{ "Aurora Beam",   PokemonGen1::Aurora_Beam },
	{ "Hyper Beam",    PokemonGen1::Hyper_Beam },
	{ "Peck",          PokemonGen1::Peck },
	{ "Drill Peck",    PokemonGen1::Drill_Peck },
	{ "Submission",    PokemonGen1::Submission },
	{ "Low Kick",      PokemonGen1::Low_Kick },
	{ "Counter",       PokemonGen1::Counter },
	{ "Seismic Toss",  PokemonGen1::Seismic_Toss },
	{ "Strength",      PokemonGen1::Strength },
	{ "Absorb",        PokemonGen1::Absorb },
	{ "Mega Drain",    PokemonGen1::Mega_Drain },
	{ "Leech Seed",    PokemonGen1::Leech_Seed },
	{ "Growth",        PokemonGen1::Growth },
	{ "Razor Leaf",    PokemonGen1::Razor_Leaf },
	{ "Solarbeam",     PokemonGen1::Solarbeam },
	{ "Solar Beam",    PokemonGen1::Solarbeam },
	{ "Poisonpowder",  PokemonGen1::Poisonpowder },
	{ "Stun Spore",    PokemonGen1::Stun_Spore },
	{ "Sleep Powder",  PokemonGen1::Sleep_Powder },
	{ "Petal Dance",   PokemonGen1::Petal_Dance },
	{ "String Shot",   PokemonGen1::String_Shot },
	{ "Dragon Rage",   PokemonGen1::Dragon_Rage },
	{ "Fire Spin",     PokemonGen1::Fire_Spin },
	{ "Thundershock",  PokemonGen1::Thundershock },
	{ "Thunderbolt",   PokemonGen1::Thunderbolt },
	{ "Thunder Wave",  PokemonGen1::Thunder_Wave },
	{ "Thunder",       PokemonGen1::Thunder },
	{ "Rock Throw",    PokemonGen1::Rock_Throw },
	{ "Earthquake",    PokemonGen1::Earthquake },
	{ "Fissure",       PokemonGen1::Fissure },
	{ "Dig",           PokemonGen1::Dig },
	{ "Toxic",         PokemonGen1::Toxic },
	{ "Confusion",     PokemonGen1::Confusion },
	{ "Psychic",       PokemonGen1::Psychic_M },
	{ "Hypnosis",      PokemonGen1::Hypnosis },
	{ "Meditate",      PokemonGen1::Meditate },
	{ "Agility",       PokemonGen1::Agility },
	{ "Quick Attack",  PokemonGen1::Quick_Attack },
	{ "Rage",          PokemonGen1::Rage },
	{ "Teleport",      PokemonGen1::Teleport },
	{ "Night Shade",   PokemonGen1::Night_Shade },
	{ "Mimic",         PokemonGen1::Mimic },
	{ "Screech",       PokemonGen1::Screech },
	{ "Double Team",   PokemonGen1::Double_Team },
	{ "Recover",       PokemonGen1::Recover },
	{ "Harden",        PokemonGen1::Harden },
	{ "Minimize",      PokemonGen1::Minimize },
	{ "Smokescreen",   PokemonGen1::Smokescreen },
	{ "Confuse Ray",   PokemonGen1::Confuse_Ray },
	{ "Withdraw",      PokemonGen1::Withdraw },
	{ "Defense Curl",  PokemonGen1::Defense_Curl },
	{ "Barrier",       PokemonGen1::Barrier },
	{ "Light Screen",  PokemonGen1::Light_Screen },
	{ "Haze",          PokemonGen1::Haze },
	{ "Reflect",       PokemonGen1::Reflect },
	{ "Focus Energy",  PokemonGen1::Focus_Energy },
	{ "Bide",          PokemonGen1::Bide },
	{ "Metronome",     PokemonGen1::Metronome },
	{ "Mirror Move",   PokemonGen1::Mirror_Move },
	{ "Self-Destruct", PokemonGen1::Self_Destruct },
	{ "Selfdestruct",  PokemonGen1::Self_Destruct },
	{ "Egg Bomb",      PokemonGen1::Egg_Bomb },
	{ "Lick",          PokemonGen1::Lick },
	{ "Smog",          PokemonGen1::Smog },
	{ "Sludge",        PokemonGen1::Sludge },
	{ "Bone Club",     PokemonGen1::Bone_Club },
	{ "Fire Blast",    PokemonGen1::Fire_Blast },
	{ "Waterfall",     PokemonGen1::Waterfall },
	{ "Clamp",         PokemonGen1::Clamp },
	{ "Swift",         PokemonGen1::Swift },
	{ "Skull Bash",    PokemonGen1::Skull_Bash },
	{ "Spike Cannon",  PokemonGen1::Spike_Cannon },
	{ "Constrict",     PokemonGen1::Constrict },
	{ "Amnesia",       PokemonGen1::Amnesia },
	{ "Kinesis",       PokemonGen1::Kinesis },
	{ "Soft-Boiled",   PokemonGen1::Softboiled },
	{ "Softboiled",    PokemonGen1::Softboiled },
	{ "Hi Jump Kick",   PokemonGen1::Hi_Jump_Kick },
	{ "High Jump Kick", PokemonGen1::Hi_Jump_Kick },
	{ "Glare",         PokemonGen1::Glare },
	{ "Dream Eater",   PokemonGen1::Dream_Eater },
	{ "Poison Gas",    PokemonGen1::Poison_Gas },
	{ "Barrage",       PokemonGen1::Barrage },
	{ "Leech Life",    PokemonGen1::Leech_Life },
	{ "Lovely Kiss",   PokemonGen1::Lovely_Kiss },
	{ "Sky Attack",    PokemonGen1::Sky_Attack },
	{ "Transform",     PokemonGen1::Transform },
	{ "Bubble",        PokemonGen1::Bubble },
	{ "Dizzy Punch",   PokemonGen1::Dizzy_Punch },
	{ "Spore",         PokemonGen1::Spore },
	{ "Flash",         PokemonGen1::Flash },
	{ "Psywave",       PokemonGen1::Psywave },
	{ "Splash",        PokemonGen1::Splash },
	{ "Acid Armor",    PokemonGen1::Acid_Armor },
	{ "Crabhammer",    PokemonGen1::Crabhammer },
	{ "Explosion",     PokemonGen1::Explosion },
	{ "Fury Swipes",   PokemonGen1::Fury_Swipes },
	{ "Bonemerang",    PokemonGen1::Bonemerang },
	{ "Rest",          PokemonGen1::Rest },
	{ "Rock Slide",    PokemonGen1::Rock_Slide },
	{ "Hyper Fang",    PokemonGen1::Hyper_Fang },
	{ "Sharpen",       PokemonGen1::Sharpen },
	{ "Conversion",    PokemonGen1::Conversion },
	{ "Tri Attack",    PokemonGen1::Tri_Attack },
	{ "Super Fang",    PokemonGen1::Super_Fang },
	{ "Slash",         PokemonGen1::Slash },
	{ "Substitute",    PokemonGen1::Substitute },
	{ "Struggle",      PokemonGen1::Struggle },
};

static std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> result;
	auto it = s.begin();

	result.reserve(std::ranges::count(s.begin(), s.end(), delim));
	do {
		auto found = std::find(it, s.end(), delim);

		result.emplace_back(it, found);
		it = found;
		if (it == s.end())
			break;
		it++;
	} while (true);
	return result;
}

// Parse the HP value from Showdown HP strings:
//   "74/100"         -> 74
//   "74/100 par"     -> 74
//   "0 fnt"          -> 0
//   "100/100"        -> 100
static unsigned parseHP(const std::string &hpStr)
{
	auto slashPos = hpStr.find('/');
	if (slashPos == std::string::npos) {
		// "0 fnt" format – parse up to first space
		auto spacePos = hpStr.find(' ');
		return std::stoul(hpStr.substr(0, spacePos == std::string::npos ? hpStr.size() : spacePos));
	}
	return std::stoul(hpStr.substr(0, slashPos));
}

// Returns the portion after the slash, stripped of any status suffix.
static unsigned parseMaxHP(const std::string &hpStr)
{
	auto slashPos = hpStr.find('/');
	if (slashPos == std::string::npos)
		return 100;
	std::string after = hpStr.substr(slashPos + 1);
	auto spacePos = after.find(' ');
	return std::stoul(after.substr(0, spacePos == std::string::npos ? after.size() : spacePos));
}

static std::unique_ptr<IResultBasedGenerator> makeRenderer(const std::string &id)
{
	if (id == "1")
		return std::make_unique<Gen1ResultBasedGenerator>();
	throw std::invalid_argument("Invalid generation: \"" + id + "\"");
}

// Map a stat name and signed amount to the appropriate SYSANIM constant.
// Positive amount = boost, negative = unboost.
static unsigned getStatAnim(const std::string &stat, int amount)
{
	static const std::map<std::string, unsigned> bases = {
		{ "atk",      PkmnCommon::SYSANIM_ATK_DECREASE_BIG },
		{ "def",      PkmnCommon::SYSANIM_DEF_DECREASE_BIG },
		{ "spa",      PkmnCommon::SYSANIM_SPA_DECREASE_BIG },
		{ "spd",      PkmnCommon::SYSANIM_SPD_DECREASE_BIG },
		{ "spe",      PkmnCommon::SYSANIM_SPE_DECREASE_BIG },
		{ "accuracy", PkmnCommon::SYSANIM_ACC_DECREASE_BIG },
		{ "evasion",  PkmnCommon::SYSANIM_EVD_DECREASE_BIG },
	};
	auto it = bases.find(stat);
	if (it == bases.end())
		return 0;
	unsigned base = it->second;
	// Layout: base+0=DEC_BIG, base+1=DEC, base+2=INC, base+3=INC_BIG
	if (amount >= 2)  return base + 3;
	if (amount == 1)  return base + 2;
	if (amount == -1) return base + 1;
	return base; // amount <= -2
}

ShowdownReader::ShowdownReader(PkmnRenderer::GameState &state) :
	_state(state)
{
}

void ShowdownReader::feed(std::vector<PkmnCommon::Event> &output, const std::string &line)
{
	output.clear();
	if (line == "|")
		this->_processChunk(output);
	else
		this->_chunk.push_back(line);
}

void ShowdownReader::_processChunk(std::vector<PkmnCommon::Event> &output)
{
	bool first = !this->_converter;
	bool start = false;

	for (auto &line : this->_chunk) {
		if (line.empty() || line[0] != '|')
			continue;

		std::vector<std::string> vals = split(line, '|');

		if (vals.front().empty())
			vals.erase(vals.begin());
		if (vals.empty())
			continue;

		auto &op = vals.front();
		if (op.empty())
			continue;

		for (auto i : ignore)
			if (op == i)
				goto done;

		if (first) {
			if (op == "gen" && !this->_converter)
				this->_converter = makeRenderer(line.substr(5));
			else if (op == "player" && vals.size() >= 3 && !vals.at(2).empty()) {
				auto &p = vals.at(1);
				auto &name = vals.at(2);
				auto &pstate = p == "p1" ? this->_state.p1 : this->_state.p2;

				pstate.name[32] = 0;
				strncpy(pstate.name, name.c_str(), sizeof(pstate.name) - 1);
			}
		} else {
			if (op == "teamsize") {
				auto &p = vals.at(1);
				auto &num = vals.at(2);
				auto &pstate = p == "p1" ? this->_state.p1 : this->_state.p2;
				auto size = std::stoul(num);

				for (size_t i = 0; i < 6; i++)
					pstate.team[i].id = i < size ? 1 : 0;
			} else if (op == "start") {
				start = true;
			} else if (op == "switch") {
				auto target   = split(vals.at(1), ':');
				auto nameDual = split(vals.at(2), ',');
				auto &hpStr   = vals.at(3);
				unsigned current   = parseHP(hpStr);
				unsigned maxHealth = parseMaxHP(hpStr);
				// POKEMON field: "pXa: Nickname" — the in-battle slot identifier.
				// DETAILS field: "Species" or "Species, LLevel" — the actual species.
				auto nickname = target[1].substr(1); // strip leading space
				auto &species = nameDual[0];          // actual species from DETAILS
				bool isP1     = target[0].starts_with("p1");
				auto &pstate  = isP1 ? this->_state.p1 : this->_state.p2;
				auto &s       = isP1 ? this->_pstate.first : this->_pstate.second;
				// Track slots by nickname so the same Pokemon coming back is recognised.
				auto it       = std::ranges::find(s.allocatedNames, nickname);
				unsigned index;

				if (it == s.allocatedNames.end()) {
					s.allocatedNames.emplace_back(nickname);
					index = s.allocatedNames.size() - 1;
					if (nameDual.size() >= 2 && nameDual[1].size() >= 2 && nameDual[1][1] == 'L')
						pstate.team[index].level = std::stoul(nameDual[1].substr(2));
					else
						pstate.team[index].level = 100;
					pstate.team[index].name[32] = 0;
					strncpy(pstate.team[index].name, nickname.c_str(), sizeof(pstate.team[index].name) - 1);
					pstate.team[index].maxHp = maxHealth;
					pstate.team[index].hp    = current;
				} else
					index = it - s.allocatedNames.begin();

				if (!pokemonNames.contains(species))
					throw std::invalid_argument("Unknown species \"" + species + "\"");
				pstate.team[index].id = pokemonNames[species];

				if (!start) {
					if (isP1) {
						if (s.hp != 0)
							this->_events.emplace_back(PkmnCommon::WithdrawEvent{true}, this->_pstate);
						this->_events.emplace_back(PkmnCommon::SwitchEvent{index, true}, this->_pstate);
					} else {
						if (s.hp != 0)
							this->_events.emplace_back(PkmnCommon::WithdrawEvent{false}, this->_pstate);
						this->_events.emplace_back(PkmnCommon::SwitchEvent{index, false}, this->_pstate);
					}
				} else {
					pstate.spriteId = pstate.team[index].id;
					pstate.active   = index;
				}
				s.onField = index;
				s.hp      = current;
			} else if (op == "move") {
				auto user  = split(vals.at(1), ':');
				auto &move = vals.at(2);
				bool isP1  = user[0].starts_with("p1");

				if (!movesNames.contains(move))
					throw std::invalid_argument("Unknown move \"" + move + "\"");

				// Reset move context for each new move
				this->_moveCtx = {};
				this->_moveCtx.lastMoveId   = movesNames[move];
				this->_moveCtx.attackerIsP1 = isP1;

				this->_events.emplace_back(
					PkmnCommon::MoveEvent{movesNames[move], isP1, false},
					this->_pstate
				);
			} else if (op == "-status") {
				auto target = split(vals.at(1), ':');
				auto &type  = vals.at(2);
				bool p1     = target[0].starts_with("p1");
				bool silent = vals.size() > 3 && vals.at(3) == "[silent]";

				if (!statusNames.contains(type))
					throw std::invalid_argument("Unknown status \"" + type + "\"");
				(p1 ? this->_pstate.first : this->_pstate.second).status = statusNames[type];
				if (!silent)
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						statusNames[type],
						false,
						p1,
						false
					}, this->_pstate);
			} else if (op == "-curestatus") {
				auto target = split(vals.at(1), ':');
				auto &type  = vals.at(2);
				bool p1     = target[0].starts_with("p1");

				(p1 ? this->_pstate.first : this->_pstate.second).status = 0;

				if (type == "slp")
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						PkmnCommon::SYSANIM_WAKE_UP, true, p1, p1
					}, this->_pstate);
				else if (type == "frz")
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						PkmnCommon::SYSANIM_THAWED, true, p1, p1
					}, this->_pstate);
				else
					// Other status cures (par/brn/psn/tox) treated as Haze-like full clear
					this->_events.emplace_back(PkmnCommon::StatusClearedEvent{p1}, this->_pstate);
			} else if (op == "-crit") {
				this->_moveCtx.hasCrit = true;
			} else if (op == "-supereffective") {
				this->_moveCtx.isSuperEffective = true;
			} else if (op == "-resisted") {
				this->_moveCtx.isNotVeryEffective = true;
			} else if (op == "-damage") {
				auto target = split(vals.at(1), ':');
				auto &hpStr = vals.at(2);
				bool p1     = target[0].starts_with("p1");
				unsigned newHP = parseHP(hpStr);

				// Check for [from] source
				std::string fromSource;
				if (vals.size() > 3 && vals.at(3).starts_with("[from]"))
					fromSource = vals.at(3).size() > 7 ? vals.at(3).substr(7) : "";

				(p1 ? this->_pstate.first : this->_pstate.second).hp = newHP;

				if (fromSource == "psn" || fromSource == "tox") {
					// Poison/toxic upkeep damage
					bool badPoison = (fromSource == "tox") ||
					                 hpStr.find("tox") != std::string::npos;
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						badPoison ? PkmnCommon::SYSANIM_BAD_POISON : PkmnCommon::SYSANIM_POISON,
						true, p1, p1
					}, this->_pstate);
					this->_events.emplace_back(PkmnCommon::HealthModEvent{newHP, p1, true}, this->_pstate);
				} else if (fromSource == "brn") {
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						PkmnCommon::SYSANIM_BURN, true, p1, p1
					}, this->_pstate);
					this->_events.emplace_back(PkmnCommon::HealthModEvent{newHP, p1, true}, this->_pstate);
				} else if (fromSource == "confusion") {
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						PkmnCommon::SYSANIM_CONFUSED_HIT, true, p1, p1
					}, this->_pstate);
					this->_events.emplace_back(PkmnCommon::HealthModEvent{newHP, p1, true}, this->_pstate);
				} else if (!fromSource.empty()) {
					// Recoil, burn, leech, etc. — just HP change, no extra animation
					this->_events.emplace_back(PkmnCommon::HealthModEvent{newHP, p1, true}, this->_pstate);
				} else {
					// Normal move damage
					if (!this->_moveCtx.firstHitEmitted) {
						this->_events.emplace_back(PkmnCommon::HitEvent{
							this->_moveCtx.isSuperEffective,
							this->_moveCtx.isNotVeryEffective,
							p1,
							true
						}, this->_pstate);
						this->_moveCtx.firstHitEmitted = true;
					}
					this->_events.emplace_back(PkmnCommon::HealthModEvent{newHP, p1, true}, this->_pstate);
					if (this->_moveCtx.hasCrit) {
						this->_events.emplace_back(PkmnCommon::TextEvent{"Critical hit!"}, this->_pstate);
						this->_moveCtx.hasCrit = false; // emit crit text only once
					}
				}
			} else if (op == "-heal") {
				auto target = split(vals.at(1), ':');
				auto &hpStr = vals.at(2);
				bool p1     = target[0].starts_with("p1");
				unsigned newHP = parseHP(hpStr);
				bool silent = vals.size() > 3 && vals.at(3) == "[silent]";

				(p1 ? this->_pstate.first : this->_pstate.second).hp = newHP;
				this->_events.emplace_back(
					PkmnCommon::HealthModEvent{newHP, p1, !silent},
					this->_pstate
				);
			} else if (op == "faint") {
				auto target = split(vals.at(1), ':');
				bool p1     = target[0].starts_with("p1");

				(p1 ? this->_pstate.first : this->_pstate.second).hp = 0;
				this->_events.emplace_back(PkmnCommon::DeathEvent{p1}, this->_pstate);
			} else if (op == "cant") {
				auto target = split(vals.at(1), ':');
				auto &reason = vals.at(2);
				bool p1     = target[0].starts_with("p1");

				static const std::map<std::string, unsigned> cantAnims = {
					{ "slp",      PkmnCommon::SYSANIM_ASLEEP },
					{ "frz",      PkmnCommon::SYSANIM_FROZEN },
					{ "par",      PkmnCommon::SYSANIM_PARALYZED },
					{ "recharge", PkmnCommon::SYSANIM_RECHARGE },
				};
				auto it = cantAnims.find(reason);
				if (it != cantAnims.end())
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						it->second, true, p1, p1
					}, this->_pstate);
				// Unknown cant reasons are silently ignored
			} else if (op == "-miss") {
				this->_events.emplace_back(
					PkmnCommon::MoveMissEvent{this->_moveCtx.lastMoveId, this->_moveCtx.attackerIsP1},
					this->_pstate
				);
			} else if (op == "-fail") {
				this->_events.emplace_back(
					PkmnCommon::MoveMissEvent{this->_moveCtx.lastMoveId, this->_moveCtx.attackerIsP1},
					this->_pstate
				);
			} else if (op == "-immune") {
				this->_events.emplace_back(
					PkmnCommon::MoveMissEvent{this->_moveCtx.lastMoveId, this->_moveCtx.attackerIsP1},
					this->_pstate
				);
			} else if (op == "-nothing") {
				this->_events.emplace_back(PkmnCommon::TextEvent{"No effect!"}, this->_pstate);
			} else if (op == "-boost") {
				auto target = split(vals.at(1), ':');
				auto &stat  = vals.at(2);
				int amount  = std::stoi(vals.at(3));
				bool p1     = target[0].starts_with("p1");
				unsigned animId = getStatAnim(stat, amount);

				if (animId != 0)
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						animId, true, p1, p1
					}, this->_pstate);
			} else if (op == "-unboost") {
				auto target = split(vals.at(1), ':');
				auto &stat  = vals.at(2);
				int amount  = std::stoi(vals.at(3));
				bool p1     = target[0].starts_with("p1");
				unsigned animId = getStatAnim(stat, -amount);

				if (animId != 0)
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						animId, false, p1, !p1
					}, this->_pstate);
			} else if (op == "-start") {
				// vals: ["-start", "pXa: Name", "condition", ...]
				auto target   = split(vals.at(1), ':');
				auto &cond    = vals.at(2);
				bool p1       = target[0].starts_with("p1");

				if (cond == "Substitute") {
					(p1 ? this->_state.p1 : this->_state.p2).substitute = true;
				} else if (cond == "confusion") {
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						PkmnCommon::SYSANIM_NOW_CONFUSED, true, p1, p1
					}, this->_pstate);
				}
				// Reflect, Light Screen, typechange: no intermediary event
			} else if (op == "-end") {
				auto target = split(vals.at(1), ':');
				auto &cond  = vals.at(2);
				bool p1     = target[0].starts_with("p1");

				if (cond == "Substitute") {
					(p1 ? this->_state.p1 : this->_state.p2).substitute = false;
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						PkmnCommon::SYSANIM_SUB_BREAK, true, p1, p1
					}, this->_pstate);
				} else if (cond == "confusion") {
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						PkmnCommon::SYSANIM_BACK_TO_SENSE, true, p1, p1
					}, this->_pstate);
				}
			} else if (op == "-activate") {
				auto target = split(vals.at(1), ':');
				auto &cond  = vals.at(2);

				if (cond == "confusion") {
					bool p1 = target[0].starts_with("p1");
					this->_events.emplace_back(PkmnCommon::AnimEvent{
						PkmnCommon::SYSANIM_CONFUSED, true, p1, p1
					}, this->_pstate);
				}
				// "-activate|Substitute|[damage]": sub absorbed damage, no event
			} else if (op == "-hitcount") {
				unsigned n  = std::stoul(vals.at(2));
				std::string text = "Hit " + std::to_string(n) + " time" + (n == 1 ? "!" : "s!");

				this->_events.emplace_back(PkmnCommon::TextEvent{text}, this->_pstate);
			} else if (op == "-prepare") {
				auto user   = split(vals.at(1), ':');
				auto &move  = vals.at(2);
				bool p1     = user[0].starts_with("p1");

				if (!movesNames.contains(move))
					throw std::invalid_argument("Unknown move in -prepare: \"" + move + "\"");
				this->_events.emplace_back(
					PkmnCommon::ExtraAnimEvent{movesNames[move], 0, p1},
					this->_pstate
				);
			} else if (op == "-message") {
				// Check for forfeit message
				if (!this->_gameEnded && vals.size() >= 2) {
					auto &msg = vals.at(1);
					if (msg.ends_with(" forfeited.")) {
						std::string loserName = msg.substr(0, msg.size() - 11);
						bool loserIsP1 = (loserName == this->_state.p1.name);

						this->_gameEnded = true;
						this->_events.emplace_back(PkmnCommon::GameEndEvent{
							!loserIsP1, loserIsP1, loserIsP1, !loserIsP1
						}, this->_pstate);
					}
				}
			} else if (op == "player") {
				// Late player events (reconnect/disconnect) — ignore
			} else if (op == "win") {
				if (!this->_gameEnded && vals.size() >= 2) {
					auto &winner = vals.at(1);
					bool p1Won = (winner == this->_state.p1.name);
					this->_gameEnded = true;
					this->_events.emplace_back(PkmnCommon::GameEndEvent{
						p1Won, !p1Won, false, false
					}, this->_pstate);
				}
			} else if (op == "turn") {
				if (!start)
					this->_events.emplace_back(PkmnCommon::TurnStartEvent{}, this->_pstate);
			} else if (op == "tie") {
				if (!this->_gameEnded) {
					this->_gameEnded = true;
					this->_events.emplace_back(PkmnCommon::GameEndEvent{
						false, false, false, false
					}, this->_pstate);
				}
			} else
				throw std::invalid_argument("Unknown directive for line \"" + line + "\"");
		}
	done:
		continue;
	}
	this->_chunk.clear();
	if (start) {
		this->_events.emplace_back(PkmnCommon::GameStartEvent{}, this->_pstate);
		this->_events.emplace_back(PkmnCommon::TurnStartEvent{}, this->_pstate);
	}
	if (first && !this->_converter)
		throw std::runtime_error("No generation found in replay");
	while (this->_converter->convertEvent(this->_state, this->_usedEvents, this->_events, output));
}
