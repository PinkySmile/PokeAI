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
std::map<std::string, PkmnCommon::PokemonSpecies> pokemonNames{
	{ "Rhydon",          PkmnCommon::Rhydon },
	{ "Kangaskhan",      PkmnCommon::Kangaskhan },
	{ "Nidoran-M",       PkmnCommon::Nidoran_M },
	{ "Nidoran_M",       PkmnCommon::Nidoran_M },
	{ "Clefairy",        PkmnCommon::Clefairy },
	{ "Spearow",         PkmnCommon::Spearow },
	{ "Voltorb",         PkmnCommon::Voltorb },
	{ "Nidoking",        PkmnCommon::Nidoking },
	{ "Slowbro",         PkmnCommon::Slowbro },
	{ "Ivysaur",         PkmnCommon::Ivysaur },
	{ "Exeggutor",       PkmnCommon::Exeggutor },
	{ "Lickitung",       PkmnCommon::Lickitung },
	{ "Exeggcute",       PkmnCommon::Exeggcute },
	{ "Grimer",          PkmnCommon::Grimer },
	{ "Gengar",          PkmnCommon::Gengar },
	{ "Nidoran-F",       PkmnCommon::Nidoran_F },
	{ "Nidoran_F",       PkmnCommon::Nidoran_F },
	{ "Nidoqueen",       PkmnCommon::Nidoqueen },
	{ "Cubone",          PkmnCommon::Cubone },
	{ "Rhyhorn",         PkmnCommon::Rhyhorn },
	{ "Lapras",          PkmnCommon::Lapras },
	{ "Arcanine",        PkmnCommon::Arcanine },
	{ "Mew",             PkmnCommon::Mew },
	{ "Gyarados",        PkmnCommon::Gyarados },
	{ "Shellder",        PkmnCommon::Shellder },
	{ "Tentacool",       PkmnCommon::Tentacool },
	{ "Gastly",          PkmnCommon::Gastly },
	{ "Scyther",         PkmnCommon::Scyther },
	{ "Staryu",          PkmnCommon::Staryu },
	{ "Blastoise",       PkmnCommon::Blastoise },
	{ "Pinsir",          PkmnCommon::Pinsir },
	{ "Tangela",         PkmnCommon::Tangela },
	{ "Growlithe",       PkmnCommon::Growlithe },
	{ "Onix",            PkmnCommon::Onix },
	{ "Fearow",          PkmnCommon::Fearow },
	{ "Pidgey",          PkmnCommon::Pidgey },
	{ "Slowpoke",        PkmnCommon::Slowpoke },
	{ "Kadabra",         PkmnCommon::Kadabra },
	{ "Graveler",        PkmnCommon::Graveler },
	{ "Chansey",         PkmnCommon::Chansey },
	{ "Machoke",         PkmnCommon::Machoke },
	{ "Mr. Mime",        PkmnCommon::Mr_Mime },
	{ "Mr_Mime",         PkmnCommon::Mr_Mime },
	{ "Hitmonlee",       PkmnCommon::Hitmonlee },
	{ "Hitmonchan",      PkmnCommon::Hitmonchan },
	{ "Arbok",           PkmnCommon::Arbok },
	{ "Parasect",        PkmnCommon::Parasect },
	{ "Psyduck",         PkmnCommon::Psyduck },
	{ "Drowzee",         PkmnCommon::Drowzee },
	{ "Golem",           PkmnCommon::Golem },
	{ "Magmar",          PkmnCommon::Magmar },
	{ "Electabuzz",      PkmnCommon::Electabuzz },
	{ "Magneton",        PkmnCommon::Magneton },
	{ "Koffing",         PkmnCommon::Koffing },
	{ "Mankey",          PkmnCommon::Mankey },
	{ "Seel",            PkmnCommon::Seel },
	{ "Diglett",         PkmnCommon::Diglett },
	{ "Tauros",          PkmnCommon::Tauros },
	{ "Farfetch'd",      PkmnCommon::Farfetchd },
	{ "Farfetch\u2019d", PkmnCommon::Farfetchd },
	{ "Farfetchd",       PkmnCommon::Farfetchd },
	{ "Venonat",         PkmnCommon::Venonat },
	{ "Dragonite",       PkmnCommon::Dragonite },
	{ "Doduo",           PkmnCommon::Doduo },
	{ "Poliwag",         PkmnCommon::Poliwag },
	{ "Jynx",            PkmnCommon::Jynx },
	{ "Moltres",         PkmnCommon::Moltres },
	{ "Articuno",        PkmnCommon::Articuno },
	{ "Zapdos",          PkmnCommon::Zapdos },
	{ "Ditto",           PkmnCommon::Ditto },
	{ "Meowth",          PkmnCommon::Meowth },
	{ "Krabby",          PkmnCommon::Krabby },
	{ "Vulpix",          PkmnCommon::Vulpix },
	{ "Ninetales",       PkmnCommon::Ninetales },
	{ "Pikachu",         PkmnCommon::Pikachu },
	{ "Raichu",          PkmnCommon::Raichu },
	{ "Dratini",         PkmnCommon::Dratini },
	{ "Dragonair",       PkmnCommon::Dragonair },
	{ "Kabuto",          PkmnCommon::Kabuto },
	{ "Kabutops",        PkmnCommon::Kabutops },
	{ "Horsea",          PkmnCommon::Horsea },
	{ "Seadra",          PkmnCommon::Seadra },
	{ "Sandshrew",       PkmnCommon::Sandshrew },
	{ "Sandslash",       PkmnCommon::Sandslash },
	{ "Omanyte",         PkmnCommon::Omanyte },
	{ "Omastar",         PkmnCommon::Omastar },
	{ "Jigglypuff",      PkmnCommon::Jigglypuff },
	{ "Wigglytuff",      PkmnCommon::Wigglytuff },
	{ "Eevee",           PkmnCommon::Eevee },
	{ "Flareon",         PkmnCommon::Flareon },
	{ "Jolteon",         PkmnCommon::Jolteon },
	{ "Vaporeon",        PkmnCommon::Vaporeon },
	{ "Machop",          PkmnCommon::Machop },
	{ "Zubat",           PkmnCommon::Zubat },
	{ "Ekans",           PkmnCommon::Ekans },
	{ "Paras",           PkmnCommon::Paras },
	{ "Poliwhirl",       PkmnCommon::Poliwhirl },
	{ "Poliwrath",       PkmnCommon::Poliwrath },
	{ "Weedle",          PkmnCommon::Weedle },
	{ "Kakuna",          PkmnCommon::Kakuna },
	{ "Beedrill",        PkmnCommon::Beedrill },
	{ "Dodrio",          PkmnCommon::Dodrio },
	{ "Primeape",        PkmnCommon::Primeape },
	{ "Dugtrio",         PkmnCommon::Dugtrio },
	{ "Venomoth",        PkmnCommon::Venomoth },
	{ "Dewgong",         PkmnCommon::Dewgong },
	{ "Caterpie",        PkmnCommon::Caterpie },
	{ "Metapod",         PkmnCommon::Metapod },
	{ "Butterfree",      PkmnCommon::Butterfree },
	{ "Machamp",         PkmnCommon::Machamp },
	{ "Golduck",         PkmnCommon::Golduck },
	{ "Hypno",           PkmnCommon::Hypno },
	{ "Golbat",          PkmnCommon::Golbat },
	{ "Mewtwo",          PkmnCommon::Mewtwo },
	{ "Snorlax",         PkmnCommon::Snorlax },
	{ "Magikarp",        PkmnCommon::Magikarp },
	{ "Muk",             PkmnCommon::Muk },
	{ "Kingler",         PkmnCommon::Kingler },
	{ "Cloyster",        PkmnCommon::Cloyster },
	{ "Electrode",       PkmnCommon::Electrode },
	{ "Clefable",        PkmnCommon::Clefable },
	{ "Weezing",         PkmnCommon::Weezing },
	{ "Persian",         PkmnCommon::Persian },
	{ "Marowak",         PkmnCommon::Marowak },
	{ "Haunter",         PkmnCommon::Haunter },
	{ "Abra",            PkmnCommon::Abra },
	{ "Alakazam",        PkmnCommon::Alakazam },
	{ "Pidgeotto",       PkmnCommon::Pidgeotto },
	{ "Pidgeot",         PkmnCommon::Pidgeot },
	{ "Starmie",         PkmnCommon::Starmie },
	{ "Bulbasaur",       PkmnCommon::Bulbasaur },
	{ "Venusaur",        PkmnCommon::Venusaur },
	{ "Tentacruel",      PkmnCommon::Tentacruel },
	{ "Goldeen",         PkmnCommon::Goldeen },
	{ "Seaking",         PkmnCommon::Seaking },
	{ "Ponyta",          PkmnCommon::Ponyta },
	{ "Rapidash",        PkmnCommon::Rapidash },
	{ "Rattata",         PkmnCommon::Rattata },
	{ "Raticate",        PkmnCommon::Raticate },
	{ "Nidorino",        PkmnCommon::Nidorino },
	{ "Nidorina",        PkmnCommon::Nidorina },
	{ "Geodude",         PkmnCommon::Geodude },
	{ "Porygon",         PkmnCommon::Porygon },
	{ "Aerodactyl",      PkmnCommon::Aerodactyl },
	{ "Magnemite",       PkmnCommon::Magnemite },
	{ "Charmander",      PkmnCommon::Charmander },
	{ "Squirtle",        PkmnCommon::Squirtle },
	{ "Charmeleon",      PkmnCommon::Charmeleon },
	{ "Wartortle",       PkmnCommon::Wartortle },
	{ "Charizard",       PkmnCommon::Charizard },
	{ "Oddish",          PkmnCommon::Oddish },
	{ "Gloom",           PkmnCommon::Gloom },
	{ "Vileplume",       PkmnCommon::Vileplume },
	{ "Bellsprout",      PkmnCommon::Bellsprout },
	{ "Weepinbell",      PkmnCommon::Weepinbell },
	{ "Victreebel",      PkmnCommon::Victreebel },
	{ "MissingNo.",      PkmnCommon::Missingno },
};
std::map<std::string, PkmnCommon::MoveIndex> movesNames{
	{ "Pound",         PkmnCommon::Pound },
	{ "Karate Chop",   PkmnCommon::Karate_Chop },
	{ "Doubleslap",    PkmnCommon::Doubleslap },
	{ "Comet Punch",   PkmnCommon::Comet_Punch },
	{ "Mega Punch",    PkmnCommon::Mega_Punch },
	{ "Pay Day",       PkmnCommon::Pay_Day },
	{ "Fire Punch",    PkmnCommon::Fire_Punch },
	{ "Ice Punch",     PkmnCommon::Ice_Punch },
	{ "Thunder Punch", PkmnCommon::Thunder_Punch },
	{ "Scratch",       PkmnCommon::Scratch },
	{ "Vise Grip",     PkmnCommon::Vise_Grip },
	{ "Guillotine",    PkmnCommon::Guillotine },
	{ "Razor Wind",    PkmnCommon::Razor_Wind },
	{ "Swords Dance",  PkmnCommon::Swords_Dance },
	{ "Cut",           PkmnCommon::Cut },
	{ "Gust",          PkmnCommon::Gust },
	{ "Wing Attack",   PkmnCommon::Wing_Attack },
	{ "Whirlwind",     PkmnCommon::Whirlwind },
	{ "Fly",           PkmnCommon::Fly },
	{ "Bind",          PkmnCommon::Bind },
	{ "Slam",          PkmnCommon::Slam },
	{ "Vine Whip",     PkmnCommon::Vine_Whip },
	{ "Stomp",         PkmnCommon::Stomp },
	{ "Double Kick",   PkmnCommon::Double_Kick },
	{ "Mega Kick",     PkmnCommon::Mega_Kick },
	{ "Jump Kick",     PkmnCommon::Jump_Kick },
	{ "Rolling Kick",  PkmnCommon::Rolling_Kick },
	{ "Sand Attack",   PkmnCommon::Sand_Attack },
	{ "Headbutt",      PkmnCommon::Headbutt },
	{ "Horn Attack",   PkmnCommon::Horn_Attack },
	{ "Fury Attack",   PkmnCommon::Fury_Attack },
	{ "Horn Drill",    PkmnCommon::Horn_Drill },
	{ "Tackle",        PkmnCommon::Tackle },
	{ "Body Slam",     PkmnCommon::Body_Slam },
	{ "Wrap",          PkmnCommon::Wrap },
	{ "Take Down",     PkmnCommon::Take_Down },
	{ "Thrash",        PkmnCommon::Thrash },
	{ "Double-Edge",   PkmnCommon::Double_Edge },
	{ "Double Edge",   PkmnCommon::Double_Edge },
	{ "Tail Whip",     PkmnCommon::Tail_Whip },
	{ "Poison Sting",  PkmnCommon::Poison_Sting },
	{ "Twineedle",     PkmnCommon::Twineedle },
	{ "Pin Missile",   PkmnCommon::Pin_Missile },
	{ "Leer",          PkmnCommon::Leer },
	{ "Bite",          PkmnCommon::Bite },
	{ "Growl",         PkmnCommon::Growl },
	{ "Roar",          PkmnCommon::Roar },
	{ "Sing",          PkmnCommon::Sing },
	{ "Supersonic",    PkmnCommon::Supersonic },
	{ "Sonic Boom",    PkmnCommon::Sonic_Boom },
	{ "Disable",       PkmnCommon::Disable },
	{ "Acid",          PkmnCommon::Acid },
	{ "Ember",         PkmnCommon::Ember },
	{ "Flamethrower",  PkmnCommon::Flamethrower },
	{ "Mist",          PkmnCommon::Mist },
	{ "Water Gun",     PkmnCommon::Water_Gun },
	{ "Hydro Pump",    PkmnCommon::Hydro_Pump },
	{ "Surf",          PkmnCommon::Surf },
	{ "Ice Beam",      PkmnCommon::Ice_Beam },
	{ "Blizzard",      PkmnCommon::Blizzard },
	{ "Psybeam",       PkmnCommon::Psybeam },
	{ "Bubblebeam",    PkmnCommon::Bubblebeam },
	{ "Bubble Beam",   PkmnCommon::Bubblebeam },
	{ "Aurora Beam",   PkmnCommon::Aurora_Beam },
	{ "Hyper Beam",    PkmnCommon::Hyper_Beam },
	{ "Peck",          PkmnCommon::Peck },
	{ "Drill Peck",    PkmnCommon::Drill_Peck },
	{ "Submission",    PkmnCommon::Submission },
	{ "Low Kick",      PkmnCommon::Low_Kick },
	{ "Counter",       PkmnCommon::Counter },
	{ "Seismic Toss",  PkmnCommon::Seismic_Toss },
	{ "Strength",      PkmnCommon::Strength },
	{ "Absorb",        PkmnCommon::Absorb },
	{ "Mega Drain",    PkmnCommon::Mega_Drain },
	{ "Leech Seed",    PkmnCommon::Leech_Seed },
	{ "Growth",        PkmnCommon::Growth },
	{ "Razor Leaf",    PkmnCommon::Razor_Leaf },
	{ "Solarbeam",     PkmnCommon::Solarbeam },
	{ "Solar Beam",    PkmnCommon::Solarbeam },
	{ "Poisonpowder",  PkmnCommon::Poisonpowder },
	{ "Stun Spore",    PkmnCommon::Stun_Spore },
	{ "Sleep Powder",  PkmnCommon::Sleep_Powder },
	{ "Petal Dance",   PkmnCommon::Petal_Dance },
	{ "String Shot",   PkmnCommon::String_Shot },
	{ "Dragon Rage",   PkmnCommon::Dragon_Rage },
	{ "Fire Spin",     PkmnCommon::Fire_Spin },
	{ "Thundershock",  PkmnCommon::Thundershock },
	{ "Thunderbolt",   PkmnCommon::Thunderbolt },
	{ "Thunder Wave",  PkmnCommon::Thunder_Wave },
	{ "Thunder",       PkmnCommon::Thunder },
	{ "Rock Throw",    PkmnCommon::Rock_Throw },
	{ "Earthquake",    PkmnCommon::Earthquake },
	{ "Fissure",       PkmnCommon::Fissure },
	{ "Dig",           PkmnCommon::Dig },
	{ "Toxic",         PkmnCommon::Toxic },
	{ "Confusion",     PkmnCommon::Confusion },
	{ "Psychic",       PkmnCommon::Psychic_M },
	{ "Hypnosis",      PkmnCommon::Hypnosis },
	{ "Meditate",      PkmnCommon::Meditate },
	{ "Agility",       PkmnCommon::Agility },
	{ "Quick Attack",  PkmnCommon::Quick_Attack },
	{ "Rage",          PkmnCommon::Rage },
	{ "Teleport",      PkmnCommon::Teleport },
	{ "Night Shade",   PkmnCommon::Night_Shade },
	{ "Mimic",         PkmnCommon::Mimic },
	{ "Screech",       PkmnCommon::Screech },
	{ "Double Team",   PkmnCommon::Double_Team },
	{ "Recover",       PkmnCommon::Recover },
	{ "Harden",        PkmnCommon::Harden },
	{ "Minimize",      PkmnCommon::Minimize },
	{ "Smokescreen",   PkmnCommon::Smokescreen },
	{ "Confuse Ray",   PkmnCommon::Confuse_Ray },
	{ "Withdraw",      PkmnCommon::Withdraw },
	{ "Defense Curl",  PkmnCommon::Defense_Curl },
	{ "Barrier",       PkmnCommon::Barrier },
	{ "Light Screen",  PkmnCommon::Light_Screen },
	{ "Haze",          PkmnCommon::Haze },
	{ "Reflect",       PkmnCommon::Reflect },
	{ "Focus Energy",  PkmnCommon::Focus_Energy },
	{ "Bide",          PkmnCommon::Bide },
	{ "Metronome",     PkmnCommon::Metronome },
	{ "Mirror Move",   PkmnCommon::Mirror_Move },
	{ "Self-Destruct", PkmnCommon::Self_Destruct },
	{ "Selfdestruct",  PkmnCommon::Self_Destruct },
	{ "Egg Bomb",      PkmnCommon::Egg_Bomb },
	{ "Lick",          PkmnCommon::Lick },
	{ "Smog",          PkmnCommon::Smog },
	{ "Sludge",        PkmnCommon::Sludge },
	{ "Bone Club",     PkmnCommon::Bone_Club },
	{ "Fire Blast",    PkmnCommon::Fire_Blast },
	{ "Waterfall",     PkmnCommon::Waterfall },
	{ "Clamp",         PkmnCommon::Clamp },
	{ "Swift",         PkmnCommon::Swift },
	{ "Skull Bash",    PkmnCommon::Skull_Bash },
	{ "Spike Cannon",  PkmnCommon::Spike_Cannon },
	{ "Constrict",     PkmnCommon::Constrict },
	{ "Amnesia",       PkmnCommon::Amnesia },
	{ "Kinesis",       PkmnCommon::Kinesis },
	{ "Soft-Boiled",   PkmnCommon::Softboiled },
	{ "Softboiled",    PkmnCommon::Softboiled },
	{ "Hi Jump Kick",  PkmnCommon::Hi_Jump_Kick },
	{ "High Jump Kick",PkmnCommon::Hi_Jump_Kick },
	{ "Glare",         PkmnCommon::Glare },
	{ "Dream Eater",   PkmnCommon::Dream_Eater },
	{ "Poison Gas",    PkmnCommon::Poison_Gas },
	{ "Barrage",       PkmnCommon::Barrage },
	{ "Leech Life",    PkmnCommon::Leech_Life },
	{ "Lovely Kiss",   PkmnCommon::Lovely_Kiss },
	{ "Sky Attack",    PkmnCommon::Sky_Attack },
	{ "Transform",     PkmnCommon::Transform },
	{ "Bubble",        PkmnCommon::Bubble },
	{ "Dizzy Punch",   PkmnCommon::Dizzy_Punch },
	{ "Spore",         PkmnCommon::Spore },
	{ "Flash",         PkmnCommon::Flash },
	{ "Psywave",       PkmnCommon::Psywave },
	{ "Splash",        PkmnCommon::Splash },
	{ "Acid Armor",    PkmnCommon::Acid_Armor },
	{ "Crabhammer",    PkmnCommon::Crabhammer },
	{ "Explosion",     PkmnCommon::Explosion },
	{ "Fury Swipes",   PkmnCommon::Fury_Swipes },
	{ "Bonemerang",    PkmnCommon::Bonemerang },
	{ "Rest",          PkmnCommon::Rest },
	{ "Rock Slide",    PkmnCommon::Rock_Slide },
	{ "Hyper Fang",    PkmnCommon::Hyper_Fang },
	{ "Sharpen",       PkmnCommon::Sharpen },
	{ "Conversion",    PkmnCommon::Conversion },
	{ "Tri Attack",    PkmnCommon::Tri_Attack },
	{ "Super Fang",    PkmnCommon::Super_Fang },
	{ "Slash",         PkmnCommon::Slash },
	{ "Substitute",    PkmnCommon::Substitute },
	{ "Struggle",      PkmnCommon::Struggle },
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
					pstate.team[i].id = i < size ? PkmnCommon::Rhydon : PkmnCommon::Empty;
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
				// "[from] WrapName" (no "move:" prefix) means the trapping move continues.
				// "[from] move: X" means used via Metronome/Sleep Talk/Mirror Move — not a continuation.
				bool isContinuation = vals.size() >= 5 &&
					vals.at(4).starts_with("[from]") &&
					!vals.at(4).starts_with("[from] move:");

				if (!movesNames.contains(move))
					throw std::invalid_argument("Unknown move \"" + move + "\"");

				// Reset move context for each new move
				this->_moveCtx = {};
				this->_moveCtx.lastMoveId     = movesNames[move];
				this->_moveCtx.attackerIsP1   = isP1;
				this->_moveCtx.isContinuation = isContinuation;

				this->_events.emplace_back(
					PkmnCommon::MoveEvent{movesNames[move], isP1, false, isContinuation},
					this->_pstate
				);
			} else if (op == "-status") {
				auto target = split(vals.at(1), ':');
				auto &type  = vals.at(2);
				bool p1     = target[0].starts_with("p1");
				bool silent = vals.size() > 3 && (vals.at(3) == "[silent]" || vals.at(3) == "[from] move: Rest");

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
			} else if (op == "-miss" || op == "-fail" || op == "-immune") {
				this->_events.emplace_back(
					PkmnCommon::MoveMissEvent{this->_moveCtx.lastMoveId, this->_moveCtx.attackerIsP1},
					this->_pstate
				);
			} else if (op == "-nothing") {
				this->_events.emplace_back(PkmnCommon::TextEvent{"No effect!"}, this->_pstate);
			} else if (op == "-boost") {
				auto target = split(vals.at(1), ':');
				auto &stat  = vals.at(2);
				int amount = std::stoi(vals.at(3));
				bool p1 = target[0].starts_with("p1");
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
				} else if (cond == "typechange" && vals.size() >= 4) {
					this->_events.emplace_back(
						PkmnCommon::TextEvent{"Converted type to " + vals.at(3) + "!"},
						this->_pstate
					);
				}
				// Reflect, Light Screen: no intermediary event
			} else if (op == "-end") {
				auto target = split(vals.at(1), ':');
				auto &cond  = vals.at(2);
				bool p1     = target[0].starts_with("p1");

				if (cond == "Substitute") {
					auto &s      = p1 ? this->_pstate.first : this->_pstate.second;
					auto &pstate = p1 ? this->_state.p1 : this->_state.p2;
					std::string name = pstate.team[s.onField].name;

					this->_events.emplace_back(PkmnCommon::TextEvent{"The SUBSTITUTE took damage for " + name + "!"}, this->_pstate);
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
				} else if (cond == "Substitute" && vals.size() > 3 && vals.at(3) == "[damage]") {
					bool p1 = target[0].starts_with("p1");
					auto &pstate = p1 ? this->_state.p1 : this->_state.p2;
					auto &s      = p1 ? this->_pstate.first : this->_pstate.second;
					std::string name = pstate.team[s.onField].name;

					this->_events.emplace_back(
						PkmnCommon::TextEvent{"The SUBSTITUTE took damage for " + name + "!"},
						this->_pstate
					);
				}
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
