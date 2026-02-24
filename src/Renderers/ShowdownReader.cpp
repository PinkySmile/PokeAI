//
// Created by PinkySmile on 22/02/2026.
//

#include "ShowdownReader.hpp"

#include <cstring>

#include "Gen1ResultBasedGenerator.hpp"

static const char *ignore[] = {
	"j",
	"t:",
	"gametype",
	"rated",
	"tier",
	"upkeep",
	"inactive"
};

std::map<std::string, unsigned> statusNames{
	{ "frz", PkmnCommon::SYSANIM_NOW_FROZEN },
	{ "slp", PkmnCommon::SYSANIM_NOW_ASLEEP },
	{ "brn", PkmnCommon::SYSANIM_NOW_BURNED },
	{ "psn", PkmnCommon::SYSANIM_NOW_POISONED },
	{ "par", PkmnCommon::SYSANIM_NOW_PARALYZED },
	{ "tox", PkmnCommon::SYSANIM_NOW_PARALYZED },
	{ "cfz", PkmnCommon::SYSANIM_NOW_CONFUSED },
};
std::map<std::string, unsigned> pokemonNames{
	{ "Rhydon",      PokemonGen1::Rhydon },
	{ "Kangaskhan",  PokemonGen1::Kangaskhan },
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
	{ "Farfetchd",   PokemonGen1::Farfetchd },
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
	{ "Self Destruct", PokemonGen1::Self_Destruct },
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
	{ "Softboiled",    PokemonGen1::Softboiled },
	{ "Hi Jump Kick",  PokemonGen1::Hi_Jump_Kick },
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

static std::unique_ptr<IResultBasedGenerator> makeRenderer(const std::string &id)
{
	if (id == "1")
		return std::make_unique<Gen1ResultBasedGenerator>();
	throw std::invalid_argument("Invalid generation: \"" + id + "\"");
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
		std::vector<std::string> vals = split(line, '|');

		if (vals.front().empty())
			vals.erase(vals.begin());

		auto &op = vals.front();

		for (auto i : ignore)
			if (vals.front() == i)
				goto done;
		if (first) {
			if (op == "gen" && !this->_converter)
				this->_converter = makeRenderer(line.substr(5));
			else if (op == "player") {
				auto &p = vals.at(1);
				auto &name = vals.at(2);
				//auto &sprite = vals.at(3);
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
					pstate.team[0].id = i < size;
			} else if (op == "start") {
				start = true;
			} else if (op == "switch") {
				auto target = split(vals.at(1), ':');
				auto nameDual = split(vals.at(2), ',');
				auto health = split(vals.at(3), '/');
				health[0].pop_back();
				auto current = std::stoul(health[0]);
				auto maxHealth = std::stoul(health[1]);
				//auto &slot = target[0];
				auto species = target[1].substr(1);
				auto isP1 = target[0].starts_with("p1");
				auto &pstate = isP1 ? this->_state.p1 : this->_state.p2;
				auto &s = isP1 ? this->_pstate.first : this->_pstate.second;
				auto it = std::ranges::find(s.allocatedNames, species);
				unsigned index;

				if (it == s.allocatedNames.end()) {
					s.allocatedNames.emplace_back(species);
					index = s.allocatedNames.size() - 1;
					if (nameDual.size() >= 2 && nameDual[1][1] == 'L')
						pstate.team[index].level = std::stoul(nameDual[1].substr(1));
					else
						pstate.team[index].level = 100;
					pstate.team[index].name[32] = 0;
					strncpy(pstate.team[index].name, nameDual[0].c_str(), sizeof(pstate.team[index].name) - 1);
					pstate.team[index].maxHp = maxHealth;
					pstate.team[index].hp = current;
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
					} else  {
						if (s.hp != 0)
							this->_events.emplace_back(PkmnCommon::WithdrawEvent{false}, this->_pstate);
						this->_events.emplace_back(PkmnCommon::SwitchEvent{index, false}, this->_pstate);
					}
				} else {
					pstate.spriteId = pstate.team[index].id;
					pstate.active = index;
				}
				s.onField = index;
				s.hp = current;
			} else if (op == "move") {
				auto user = split(vals.at(1), ':');
				auto &move = vals.at(2);
				auto target = split(vals.at(3), ':');

				if (!movesNames.contains(move))
					throw std::invalid_argument("Unknown move \"" + move + "\"");
				this->_events.emplace_back(PkmnCommon::MoveEvent{movesNames[move], user[0].starts_with("p1"), false}, this->_pstate);
			} else if (op == "-status") {
				auto target = split(vals.at(1), ':');
				auto &type = vals.at(2);
				bool p1 = target[0].starts_with("p1");

				if (!statusNames.contains(type))
					throw std::invalid_argument("Unknown status \"" + type + "\"");
				(p1 ? this->_pstate.first : this->_pstate.second).status = statusNames[type];
				this->_events.emplace_back(PkmnCommon::AnimEvent{
					statusNames[type],
					false,
					p1,
					false
				}, this->_pstate);
			} else if (op == "-damage") {
				auto target = split(vals.at(1), ':');
				auto health = split(vals.at(2), '/');
				health[0].pop_back();
				unsigned current = std::stoul(health[0]);
				//auto maxHealth = std::stoul(health[1]);

				this->_events.emplace_back(PkmnCommon::HealthModEvent{
					current,
					target[0].starts_with("p1")
				}, this->_pstate);
			} else if (op == "turn") {
				if (!start)
					this->_events.emplace_back(PkmnCommon::TurnStartEvent{}, this->_pstate);
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
