//
// Created by PinkySmile on 22/04/2026.
//

#include <algorithm>
#include <array>
#include <unordered_map>
#include "pickers.hpp"
#include "mainMenu.hpp"
#include "Utils.hpp"
#include "Renderers/Gen1Renderer.hpp"

using namespace PokemonGen1;

struct PickerEntry {
	unsigned id = 0;
	tgui::ScrollablePanel::Ptr widget;
};

struct PkmnPickerCache {
	tgui::Panel::Ptr overlay;
	tgui::ScrollablePanel::Ptr scroll;
	tgui::EditBox::Ptr filter;
	tgui::ComboBox::Ptr sort;
	tgui::ComboBox::Ptr typeFilter;
	std::vector<PickerEntry> entries;
	std::vector<PickerEntry> displayed;
	std::function<void(unsigned id)> onSelect;
	unsigned level = 100;
};

struct MovePickerCache {
	tgui::Panel::Ptr overlay;
	tgui::ScrollablePanel::Ptr scroll;
	tgui::EditBox::Ptr filter;
	tgui::ComboBox::Ptr sort;
	tgui::ComboBox::Ptr typeFilter;
	tgui::CheckBox::Ptr onlyLearnable;
	std::vector<PickerEntry> entries;
	std::vector<PickerEntry> displayed;
	std::function<void(unsigned id)> onSelect;
	const std::set<AvailableMove> *learnSet = nullptr;
};

static PkmnPickerCache g_pkmnPicker;
static MovePickerCache g_movePicker;

static const std::array<std::pair<const char *, const char *>, 16> typeFilterItems{{
	{"--Filter by type--", ""},
	{"Normal",   "Normal"},
	{"Fighting", "Fighting"},
	{"Fly",      "Fly"},
	{"Poison",   "Poison"},
	{"Ground",   "Ground"},
	{"Rock",     "Rock"},
	{"Bug",      "Bug"},
	{"Ghost",    "Ghost"},
	{"Fire",     "Fire"},
	{"Water",    "Water"},
	{"Grass",    "Grass"},
	{"Electric", "Electric"},
	{"Psy",      "Psy"},
	{"Ice",      "Ice"},
	{"Dragon",   "Dragon"},
}};

static void populateTypeFilter(const tgui::ComboBox::Ptr &combo, const char *unknownId)
{
	for (auto &[label, id] : typeFilterItems)
		combo->addItem(label, id);
	combo->addItem("???", unknownId);
	combo->setSelectedItemByIndex(0);
}

static bool byName(const std::string &a, const std::string &b, const std::string &sentinel, bool descending)
{
	if (a == b)
		return false;
	if (a == sentinel)
		return false;
	if (b == sentinel)
		return true;
	return descending ? a > b : a < b;
}

using MoveComp = bool(*)(const PickerEntry &, const PickerEntry &);
static const std::array<MoveComp, 5> moveSortAlgos{
	[](const PickerEntry &a, const PickerEntry &b){
		if (availableMoves[a.id].getName() == availableMoves[b.id].getName())
			return a.id < b.id;
		return byName(availableMoves[a.id].getName(), availableMoves[b.id].getName(), "Move ", false);
	},
	[](const PickerEntry &a, const PickerEntry &b){
		if (availableMoves[a.id].getName() == availableMoves[b.id].getName())
			return a.id < b.id;
		return byName(availableMoves[a.id].getName(), availableMoves[b.id].getName(), "Move ", true);
	},
	[](const PickerEntry &a, const PickerEntry &b){
		if (availableMoves[a.id].getType() != availableMoves[b.id].getType())
			return availableMoves[a.id].getType() < availableMoves[b.id].getType();
		return a.id < b.id;
	},
	[](const PickerEntry &a, const PickerEntry &b){ return a.id < b.id; },
	[](const PickerEntry &a, const PickerEntry &b){ return a.id > b.id; },
};

using PkmnComp = bool(*)(const PickerEntry &, const PickerEntry &);
static const std::array<PkmnComp, 16> pkmnSortAlgos{
	[](const PickerEntry &a, const PickerEntry &b){
		auto &ba = pokemonList.at(a.id);
		auto &bb = pokemonList.at(b.id);
		if (ba.name == bb.name)
			return a.id < b.id;
		return byName(ba.name, bb.name, "MISSINGNO.", false);
	},
	[](const PickerEntry &a, const PickerEntry &b){
		auto &ba = pokemonList.at(a.id);
		auto &bb = pokemonList.at(b.id);
		if (ba.name == bb.name)
			return a.id < b.id;
		return byName(ba.name, bb.name, "MISSINGNO.", true);
	},
	[](const PickerEntry &a, const PickerEntry &b){ return pokemonList.at(a.id).dexId < pokemonList.at(b.id).dexId; },
	[](const PickerEntry &a, const PickerEntry &b){ return pokemonList.at(a.id).dexId > pokemonList.at(b.id).dexId; },
	[](const PickerEntry &a, const PickerEntry &b){ return a.id < b.id; },
	[](const PickerEntry &a, const PickerEntry &b){ return a.id > b.id; },
	[](const PickerEntry &a, const PickerEntry &b){ return pokemonList.at(a.id).HP  < pokemonList.at(b.id).HP;  },
	[](const PickerEntry &a, const PickerEntry &b){ return pokemonList.at(a.id).HP  > pokemonList.at(b.id).HP;  },
	[](const PickerEntry &a, const PickerEntry &b){ return pokemonList.at(a.id).ATK < pokemonList.at(b.id).ATK; },
	[](const PickerEntry &a, const PickerEntry &b){ return pokemonList.at(a.id).ATK > pokemonList.at(b.id).ATK; },
	[](const PickerEntry &a, const PickerEntry &b){ return pokemonList.at(a.id).DEF < pokemonList.at(b.id).DEF; },
	[](const PickerEntry &a, const PickerEntry &b){ return pokemonList.at(a.id).DEF > pokemonList.at(b.id).DEF; },
	[](const PickerEntry &a, const PickerEntry &b){ return pokemonList.at(a.id).SPD < pokemonList.at(b.id).SPD; },
	[](const PickerEntry &a, const PickerEntry &b){ return pokemonList.at(a.id).SPD > pokemonList.at(b.id).SPD; },
	[](const PickerEntry &a, const PickerEntry &b){ return pokemonList.at(a.id).SPE < pokemonList.at(b.id).SPE; },
	[](const PickerEntry &a, const PickerEntry &b){ return pokemonList.at(a.id).SPE > pokemonList.at(b.id).SPE; },
};

static const std::string &lowerMoveName(unsigned id)
{
	static std::unordered_map<unsigned, std::string> cache;
	auto it = cache.find(id);
	if (it == cache.end())
		it = cache.emplace(id, Utils::toLower(availableMoves[id].getName())).first;
	return it->second;
}

static const std::string &lowerPkmnName(unsigned id)
{
	static std::unordered_map<unsigned, std::string> cache;
	auto it = cache.find(id);
	if (it == cache.end())
		it = cache.emplace(id, Utils::toLower(pokemonList.at(id).name)).first;
	return it->second;
}

static void layoutMovePicker()
{
	for (size_t i = 0; i < g_movePicker.displayed.size(); i++)
		g_movePicker.displayed[i].widget->setPosition(i % 3 * 250, 10 + i / 3 * 160);
}

static void layoutPkmnPicker()
{
	for (size_t i = 0; i < g_pkmnPicker.displayed.size(); i++)
		g_pkmnPicker.displayed[i].widget->setPosition(28 + i % 3 * 248, 10 + i / 3 * 190);
}

static void refreshMovePicker()
{
	g_movePicker.displayed = g_movePicker.entries;

	auto query = g_movePicker.filter->getText().toStdString();
	auto type = g_movePicker.typeFilter->getSelectedItemId().toStdString();
	bool onlyLearn = g_movePicker.onlyLearnable->isChecked();
	auto sort = g_movePicker.sort->getSelectedItemIndex();

	for (auto &entry : g_movePicker.entries)
		entry.widget->setPosition(-300, -300);

	if (!query.empty()) {
		query = Utils::toLower(query);
		std::erase_if(g_movePicker.displayed, [&](const PickerEntry &p){
			return lowerMoveName(p.id).find(query) == std::string::npos;
		});
	}
	if (!type.empty())
		std::erase_if(g_movePicker.displayed, [&](const PickerEntry &p){
			return typeToString(availableMoves[p.id].getType()) != type;
		});
	if (onlyLearn && g_movePicker.learnSet)
		std::erase_if(g_movePicker.displayed, [&](const PickerEntry &p){
			return p.id != 0 && !g_movePicker.learnSet->contains(static_cast<AvailableMove>(p.id));
		});

	if (sort >= 0 && sort < static_cast<int>(moveSortAlgos.size()))
		std::sort(g_movePicker.displayed.begin(), g_movePicker.displayed.end(), moveSortAlgos[sort]);
	layoutMovePicker();
}

static void refreshPkmnPicker()
{
	g_pkmnPicker.displayed = g_pkmnPicker.entries;

	auto query = g_pkmnPicker.filter->getText().toStdString();
	auto type = g_pkmnPicker.typeFilter->getSelectedItemId().toStdString();
	auto sort = g_pkmnPicker.sort->getSelectedItemIndex();

	for (auto &entry : g_pkmnPicker.entries)
		entry.widget->setPosition(-300, -300);

	if (!query.empty()) {
		query = Utils::toLower(query);
		std::erase_if(g_pkmnPicker.displayed, [&](const PickerEntry &p){
			return lowerPkmnName(p.id).find(query) == std::string::npos;
		});
	}
	if (!type.empty())
		std::erase_if(g_pkmnPicker.displayed, [&](const PickerEntry &p){
			auto &base = pokemonList.at(p.id);
			return typeToString(base.typeA) != type && typeToString(base.typeB) != type;
		});

	if (sort >= 0 && sort < static_cast<int>(pkmnSortAlgos.size()))
		std::sort(g_pkmnPicker.displayed.begin(), g_pkmnPicker.displayed.end(), pkmnSortAlgos[sort]);
	layoutPkmnPicker();
}

static void buildMovePicker(tgui::Gui &gui, BattleResources &resources)
{
	g_movePicker.overlay = tgui::Panel::create({"100%", "100%"});
	g_movePicker.scroll = tgui::ScrollablePanel::create({"&.w - 20", "&.h - 50"});
	g_movePicker.filter = tgui::EditBox::create();
	g_movePicker.typeFilter = tgui::ComboBox::create();
	g_movePicker.sort = tgui::ComboBox::create();
	g_movePicker.onlyLearnable = tgui::CheckBox::create();

	auto close = tgui::Button::create("X");

	g_movePicker.filter->setSize("&.w * 40 / 100 - 60", 20);
	g_movePicker.typeFilter->setSize("&.w * 20 / 100 - 20", 20);
	g_movePicker.sort->setSize("&.w * 20 / 100 - 10", 20);
	g_movePicker.onlyLearnable->setSize(20, 20);
	close->setSize(20, 20);

	g_movePicker.onlyLearnable->setText("Only learnable moves");
	g_movePicker.onlyLearnable->setChecked(true);

	g_movePicker.filter->setDefaultText("Search");
	g_movePicker.sort->addItem("Sort A -> Z");
	g_movePicker.sort->addItem("Sort Z -> A");
	g_movePicker.sort->addItem("Sort by type");
	g_movePicker.sort->addItem("Sort by ascending ID");
	g_movePicker.sort->addItem("Sort by descending ID");
	g_movePicker.sort->setSelectedItemByIndex(0);
	populateTypeFilter(g_movePicker.typeFilter, "Unknown");

	g_movePicker.filter->onTextChange.connect(refreshMovePicker);
	g_movePicker.sort->onItemSelect.connect(refreshMovePicker);
	g_movePicker.typeFilter->onItemSelect.connect(refreshMovePicker);
	g_movePicker.onlyLearnable->onChange.connect(refreshMovePicker);
	close->onClick.connect([&gui]{ gui.remove(g_movePicker.overlay); });

	g_movePicker.filter->setPosition(10, 10);
	close->setPosition("&.w - 30", 10);
	g_movePicker.onlyLearnable->setPosition("&.w * 80 / 100 - 50", 10);
	g_movePicker.sort->setPosition("&.w * 60 / 100 - 50", 10);
	g_movePicker.typeFilter->setPosition("&.w * 40 / 100 - 40", 10);
	g_movePicker.scroll->setPosition(10, 40);

	g_movePicker.overlay->add(g_movePicker.onlyLearnable);
	g_movePicker.overlay->add(g_movePicker.typeFilter);
	g_movePicker.overlay->add(g_movePicker.sort);
	g_movePicker.overlay->add(g_movePicker.filter);
	g_movePicker.overlay->add(close);
	g_movePicker.overlay->add(g_movePicker.scroll);

	auto base = tgui::ScrollablePanel::create({250, 160});
	base->loadWidgetsFromFile("assets/movePanel.gui");

	g_movePicker.entries.reserve(availableMoves.size());
	for (unsigned i = 0; i < availableMoves.size(); i++) {
		auto &move = availableMoves[i];
		auto pan = tgui::ScrollablePanel::copy(base);
		auto typeIcon = tgui::Picture::create(resources.types[typeToString(move.getType())]);
		auto category = tgui::Picture::create(resources.categories[move.getCategory()]);
		auto name = pan->get<tgui::Button>("Name");
		auto effects = pan->get<tgui::TextArea>("AdditionalEffects");

		name->setText(strToUpper(move.getName()));
		name->onClick.connect([i]{ if (g_movePicker.onSelect) g_movePicker.onSelect(i); });
		pan->get<tgui::EditBox>("PPs")->setText(std::to_string(move.getMaxPP()));
		pan->get<tgui::EditBox>("Power")->setText(move.getPower() ? std::to_string(move.getPower()) : "-");
		pan->get<tgui::EditBox>("Accuracy")->setText(move.skipAccuracyCheck() ? "-" : std::to_string(move.getAccuracy() * 100 / 256) + "%");
		effects->setText(move.getDescription().empty() ? "No additional effect" : move.getDescription());

		typeIcon->setPosition(190, 2);
		category->setPosition(206, 29);
		pan->add(typeIcon);
		pan->add(category);

		g_movePicker.scroll->add(pan);
		g_movePicker.entries.push_back({i, pan});
	}
}

static void buildPkmnPicker(tgui::Gui &gui, BattleResources &resources)
{
	g_pkmnPicker.overlay = tgui::Panel::create({"100%", "100%"});
	g_pkmnPicker.scroll = tgui::ScrollablePanel::create({"&.w - 20", "&.h - 50"});
	g_pkmnPicker.filter = tgui::EditBox::create();
	g_pkmnPicker.sort = tgui::ComboBox::create();
	g_pkmnPicker.typeFilter = tgui::ComboBox::create();

	auto close = tgui::Button::create("X");

	g_pkmnPicker.filter->setSize("&.w * 50 / 100 - 30", 20);
	g_pkmnPicker.typeFilter->setSize("&.w * 20 / 100 - 20", 20);
	g_pkmnPicker.sort->setSize("&.w * 30 / 100 - 10", 20);
	close->setSize(20, 20);

	g_pkmnPicker.filter->setDefaultText("Search");
	for (auto label : {
		"Sort A -> Z",
		"Sort Z -> A",
		"Sort by ascending pokedex ID",
		"Sort by descending pokedex ID",
		"Sort by ascending ID",
		"Sort by descending ID",
		"Sort by ascending max HP",
		"Sort by descending max HP",
		"Sort by ascending ATK",
		"Sort by descending ATK",
		"Sort by ascending DEF",
		"Sort by descending DEF",
		"Sort by ascending SPD",
		"Sort by descending SPD",
		"Sort by ascending SPE",
		"Sort by descending SPE",
	})
		g_pkmnPicker.sort->addItem(label);
	g_pkmnPicker.sort->setSelectedItemByIndex(0);
	populateTypeFilter(g_pkmnPicker.typeFilter, "???");

	g_pkmnPicker.filter->onTextChange.connect(refreshPkmnPicker);
	g_pkmnPicker.sort->onItemSelect.connect(refreshPkmnPicker);
	g_pkmnPicker.typeFilter->onItemSelect.connect(refreshPkmnPicker);
	close->onClick.connect([&gui]{ gui.remove(g_pkmnPicker.overlay); });

	g_pkmnPicker.filter->setPosition(10, 10);
	close->setPosition("&.w - 30", 10);
	g_pkmnPicker.sort->setPosition("&.w * 70 / 100", 10);
	g_pkmnPicker.typeFilter->setPosition("&.w * 50 / 100", 10);
	g_pkmnPicker.scroll->setPosition(10, 40);

	g_pkmnPicker.overlay->add(g_pkmnPicker.typeFilter);
	g_pkmnPicker.overlay->add(g_pkmnPicker.sort);
	g_pkmnPicker.overlay->add(g_pkmnPicker.filter);
	g_pkmnPicker.overlay->add(close);
	g_pkmnPicker.overlay->add(g_pkmnPicker.scroll);

	auto base = tgui::ScrollablePanel::create({220, 170});
	base->loadWidgetsFromFile("assets/pkmnPreview.gui");

	g_pkmnPicker.entries.reserve(pokemonList.size());
	for (const auto &[id, data] : pokemonList) {
		auto pan = tgui::ScrollablePanel::copy(base);
		auto type1 = tgui::Picture::create(resources.types[typeToString(data.typeA)]);
		auto type2 = tgui::Picture::create(resources.types[typeToString(data.typeB)]);
		auto sprite = pan->get<tgui::BitmapButton>("Species");

		pan->get<tgui::TextArea>("SpeciesName")->setText(strToUpper(data.name));
		sprite->setImage(resources.renderer->getPkmnFace(PkmnRenderer::gen1SpeciesToCommon(data.id)));
		sprite->onClick.connect([pkId = id]{ if (g_pkmnPicker.onSelect) g_pkmnPicker.onSelect(pkId); });

		type1->setPosition(115, 152);
		type2->setPosition(170, 152);
		type2->setVisible(data.typeA != data.typeB);
		pan->add(type1);
		pan->add(type2);

		g_pkmnPicker.scroll->add(pan);
		g_pkmnPicker.entries.push_back({id, pan});
	}
}

static void updatePkmnPickerStats(unsigned level)
{
	if (g_pkmnPicker.level == level)
		return;
	g_pkmnPicker.level = level;
	for (auto &entry : g_pkmnPicker.entries) {
		auto &stats = pokemonList.at(entry.id).statsAtLevel[level];

		entry.widget->get<tgui::TextArea>("HP")->setText(std::to_string(stats.HP));
		entry.widget->get<tgui::TextArea>("ATK")->setText(std::to_string(stats.ATK));
		entry.widget->get<tgui::TextArea>("DEF")->setText(std::to_string(stats.DEF));
		entry.widget->get<tgui::TextArea>("SPD")->setText(std::to_string(stats.SPD));
		entry.widget->get<tgui::TextArea>("SPE")->setText(std::to_string(stats.SPE));
	}
}

void invalidatePkmnPickerCache()
{
	g_pkmnPicker = PkmnPickerCache{};
}

void openChangeMoveBox(tgui::Gui &gui, BattleResources &resources, Pokemon &pkmn, unsigned moveIndex, tgui::Button::Ptr moveButton)
{
	if (!g_movePicker.overlay)
		buildMovePicker(gui, resources);

	g_movePicker.learnSet = &pkmn.getLearnableMoveSet();
	g_movePicker.onSelect = [&gui, &pkmn, moveIndex, mbWeak = std::weak_ptr(moveButton)](unsigned moveId){
		auto &move = availableMoves[moveId];

		pkmn.setMove(moveIndex, move);
		if (auto mb = mbWeak.lock())
			mb->setText(move.getName());
		gui.remove(g_movePicker.overlay);
	};
	refreshMovePicker();
	gui.add(g_movePicker.overlay);
}

void openChangePkmnBox(
	tgui::Gui &gui,
	BattleHandler &game,
	std::unique_ptr<EmulatorGameHandle> &emulator,
	BattleResources &resources,
	unsigned index,
	Pokemon &pkmn,
	sf::RenderWindow &window,
	tgui::Panel::Ptr pkmnPan,
	std::pair<unsigned char, unsigned char> &aisSelected,
	bool &side,
	bool &ready
)
{
	if (!g_pkmnPicker.overlay)
		buildPkmnPicker(gui, resources);
	updatePkmnPickerStats(pkmn.getLevel());

	auto &state = game.getBattleState();

	g_pkmnPicker.onSelect = [
		&gui, &game, &emulator, &resources, &state, &aisSelected, &side, &ready,
		&pkmn, &window, index, panWeak = std::weak_ptr(pkmnPan)
	](unsigned id){
		auto &base = pokemonList.at(id);
		auto &s = side ? state.op : state.me;

		s.team.at(index) = Pokemon(state, pkmn.getNickname(), pkmn.getLevel(), base, pkmn.getMoveSet());
		resources.crySound.setBuffer(resources.renderer->getPkmnCry(PkmnRenderer::gen1SpeciesToCommon(base.id)));
		resources.crySound.play();
		gui.remove(g_pkmnPicker.overlay);
		if (auto pan = panWeak.lock())
			populatePokemonPanel(window, gui, emulator, game, resources, pan, index, s.team, aisSelected, side, ready);
	};
	refreshPkmnPicker();
	gui.add(g_pkmnPicker.overlay);
}
