//
// Created by PinkySmile on 30/08/2019.
//

#include <TGUI/TGUI.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>
#include <fstream>
#include <memory>
#include "gui.hpp"
#include "Utils.hpp"
#include "GameEngine/Team.hpp"
#include "Emulator/EmulatorGameHandle.hpp"
#include "Emulator/BgbHandler.hpp"
#include "../AIs/AI.hpp"
#include "../AIs/AIFactory.hpp"

using namespace PokemonGen1;

std::string lastIp;
std::string lastPort;

std::string strToUpper(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

std::string intToHex(unsigned char i)
{
	std::stringstream stream;
	stream << std::setfill ('0') << std::setw(2) << std::hex << std::uppercase << static_cast<int>(i);
	return stream.str();
}

void makeMainMenuGUI(
	sf::RenderWindow &window,
	tgui::Gui &gui,
	std::unique_ptr<EmulatorGameHandle> &emulator,
	BattleHandler &game,
	BattleResources &resources,
	std::pair<unsigned char, unsigned char> &aisSelected,
	bool &side,
	bool &ready
);
void populatePokemonPanel(
	sf::RenderWindow &window,
	tgui::Gui &gui,
	std::unique_ptr<EmulatorGameHandle> &emulator,
	BattleHandler &game,
	BattleResources &resources,
	tgui::Panel::Ptr panel,
	unsigned index,
	std::vector<Pokemon> &team,
	std::pair<unsigned char, unsigned char> &aisSelected,
	bool &side,
	bool &ready
);

void moveMovePanels(const std::vector<std::pair<unsigned, tgui::ScrollablePanel::Ptr>> &panels)
{
	for (unsigned i = 0; i < panels.size(); i++)
		panels[i].second->setPosition(i % 3 * 250, 10 + i / 3 * 160);
}

void applyMoveFilters(const std::set<AvailableMove> &set, unsigned sorting, std::string query, const std::string &type, bool onlyPkmnLearnSet, std::vector<std::pair<unsigned, tgui::ScrollablePanel::Ptr>> &panels)
{
	std::vector<std::function<bool(const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2)>> sortingAlgos = {
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = availableMoves[p1.first];
			auto &base2 = availableMoves[p2.first];

			if (base1.getName().substr(0, strlen("Move ")) == base2.getName().substr(0, strlen("Move ")))
				return p1.first < p2.first;
			return base2.getName().substr(0, strlen("Move ")) == "Move " || (
				base1.getName().substr(0, strlen("Move ")) != "Move " &&
				base1.getName() < base2.getName()
			);
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = availableMoves[p1.first];
			auto &base2 = availableMoves[p2.first];

			if (base1.getName().substr(0, strlen("Move ")) == base2.getName().substr(0, strlen("Move ")))
				return p1.first < p2.first;
			return base2.getName().substr(0, strlen("Move ")) == "Move " || (
				base1.getName().substr(0, strlen("Move ")) != "Move " &&
				base1.getName() > base2.getName()
			);
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = availableMoves[p1.first];
			auto &base2 = availableMoves[p2.first];

			if (base1.getName().substr(0, strlen("Move ")) == base2.getName().substr(0, strlen("Move ")))
				return p1.first < p2.first;
			return base2.getName().substr(0, strlen("Move ")) == "Move " || (
				base1.getName().substr(0, strlen("Move ")) != "Move " &&
				base1.getType() < base2.getType()
			);
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			return p1.first < p2.first;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			return p1.first > p2.first;
		}
	};

	for (auto &panel : panels)
		panel.second->setPosition(-300, -300);
	if (!query.empty()) {
		query = Utils::toLower(query);
		panels.erase(std::remove_if(
			panels.begin(),
			panels.end(),
			[&query](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1) {
				return Utils::toLower(availableMoves[p1.first].getName()).find(query) == std::string::npos;
			}
		), panels.end());
	}
	if (!type.empty())
		panels.erase(std::remove_if(
			panels.begin(),
			panels.end(),
			[&type](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1) {
				return typeToString(availableMoves[p1.first].getType()) != type;
			}
		), panels.end());
	if (onlyPkmnLearnSet)
		panels.erase(std::remove_if(
			panels.begin(),
			panels.end(),
			[&set](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1) {
				return p1.first != 0 && !set.contains(static_cast<AvailableMove>(p1.first));
			}
		), panels.end());
	std::sort(panels.begin(), panels.end(), sortingAlgos[sorting]);
}

void openChangeMoveBox(tgui::Gui &gui, BattleResources &resources, Pokemon &pkmn, unsigned moveIndex, tgui::Button::Ptr moveButton)
{
	auto bigPan = tgui::Panel::create({"100%", "100%"});
	auto panel = tgui::ScrollablePanel::create({"&.w - 20", "&.h - 50"});
	auto basePanel = tgui::ScrollablePanel::create({250, 160});
	auto panels = std::make_shared<std::vector<std::pair<unsigned, tgui::ScrollablePanel::Ptr>>>(availableMoves.size());
	auto displayedPanels = std::make_shared<std::vector<std::pair<unsigned, tgui::ScrollablePanel::Ptr>>>(availableMoves.size());
	auto filter = tgui::EditBox::create();
	auto typeFilter = tgui::ComboBox::create();
	auto sorting = tgui::ComboBox::create();
	auto onlyLearnable = tgui::CheckBox::create();
	auto close = tgui::Button::create();

	filter->setSize("&.w * 40 / 100 - 60", 20);
	typeFilter->setSize("&.w * 20 / 100 - 20", 20);
	sorting->setSize("&.w * 20 / 100 - 10", 20);
	close->setSize(20, 20);
	onlyLearnable->setSize(20, 20);

	close->setText("X");
	onlyLearnable->setText("Only learnable moves");
	onlyLearnable->setChecked(true);

	filter->setDefaultText("Search");
	sorting->addItem("Sort A -> Z");
	sorting->addItem("Sort Z -> A");
	sorting->addItem("Sort by type");
	sorting->addItem("Sort by ascending ID");
	sorting->addItem("Sort by descending ID");
	sorting->setSelectedItemByIndex(0);

	typeFilter->addItem("--Filter by type--", "");
	typeFilter->addItem("Normal", "Normal");
	typeFilter->addItem("Fighting", "Fighting");
	typeFilter->addItem("Fly", "Fly");
	typeFilter->addItem("Poison", "Poison");
	typeFilter->addItem("Ground", "Ground");
	typeFilter->addItem("Rock", "Rock");
	typeFilter->addItem("Bug", "Bug");
	typeFilter->addItem("Ghost", "Ghost");
	typeFilter->addItem("Fire", "Fire");
	typeFilter->addItem("Water", "Water");
	typeFilter->addItem("Grass", "Grass");
	typeFilter->addItem("Electric", "Electric");
	typeFilter->addItem("Psy", "Psy");
	typeFilter->addItem("Ice", "Ice");
	typeFilter->addItem("Dragon", "Dragon");
	typeFilter->addItem("???", "Unknown");
	typeFilter->setSelectedItemByIndex(0);

	auto refresh = [displayedPanels, panels, &pkmn](
		std::weak_ptr<tgui::EditBox> filter,
		std::weak_ptr<tgui::ComboBox> sorting,
		std::weak_ptr<tgui::ComboBox> typeFilter,
		std::weak_ptr<tgui::CheckBox> onlyLearnable
	){
		for (auto &panel : *panels)
			panel.second->setPosition(-200, -200);
		*displayedPanels = *panels;
		applyMoveFilters(
			pkmn.getLearnableMoveSet(),
			sorting.lock()->getSelectedItemIndex(),
			filter.lock()->getText().toStdString(),
			typeFilter.lock()->getSelectedItemId().toStdString(),
			onlyLearnable.lock()->isChecked(),
			*displayedPanels
		);
		moveMovePanels(*displayedPanels);
	};

	filter->onTextChange.connect(refresh, std::weak_ptr(filter), std::weak_ptr(sorting), std::weak_ptr(typeFilter), std::weak_ptr(onlyLearnable));
	sorting->onItemSelect.connect(refresh, std::weak_ptr(filter), std::weak_ptr(sorting), std::weak_ptr(typeFilter), std::weak_ptr(onlyLearnable));
	typeFilter->onItemSelect.connect(refresh, std::weak_ptr(filter), std::weak_ptr(sorting), std::weak_ptr(typeFilter), std::weak_ptr(onlyLearnable));
	onlyLearnable->onChange.connect(refresh, std::weak_ptr(filter), std::weak_ptr(sorting), std::weak_ptr(typeFilter), std::weak_ptr(onlyLearnable));

	filter->setPosition(10, 10);
	close->setPosition("&.w - 30", 10);
	onlyLearnable->setPosition("&.w * 80 / 100 - 50", 10);
	sorting->setPosition("&.w * 60 / 100 - 50", 10);
	typeFilter->setPosition("&.w * 40 / 100 - 40", 10);
	panel->setPosition(10, 40);

	bigPan->add(onlyLearnable);
	bigPan->add(typeFilter);
	bigPan->add(sorting);
	bigPan->add(filter);
	bigPan->add(close);

	basePanel->loadWidgetsFromFile("assets/movePanel.gui");

	close->onClick.connect([&gui](std::weak_ptr<tgui::Panel> bigPan){
		gui.remove(bigPan.lock());
	}, std::weak_ptr(bigPan));
	for (unsigned i = 0; i < panels->size(); i++) {
		auto &pan = (panels->operator[](i) = {i, tgui::ScrollablePanel::copy(basePanel)}).second;
		auto &move = availableMoves[i];
		auto type = tgui::Picture::create(resources.types[typeToString(move.getType())]);
		auto category = tgui::Picture::create(resources.categories[move.getCategory()]);
		auto pps = pan->get<tgui::EditBox>("PPs");
		auto pow = pan->get<tgui::EditBox>("Power");
		auto acc = pan->get<tgui::EditBox>("Accuracy");
		auto name = pan->get<tgui::Button>("Name");
		auto effects = pan->get<tgui::TextArea>("AdditionalEffects");

		name->setText(strToUpper(move.getName()));
		name->onClick.connect([&move, &gui, moveIndex, &pkmn](std::weak_ptr<tgui::Button> moveButton, std::weak_ptr<tgui::Panel> bigPan){
			pkmn.setMove(moveIndex, move);
			gui.remove(bigPan.lock());
			moveButton.lock()->setText(move.getName());
		}, std::weak_ptr(moveButton), std::weak_ptr(bigPan));
		pps->setText(std::to_string(move.getMaxPP()));
		pow->setText(move.getPower() ? std::to_string(move.getPower()) : "-");
		acc->setText(move.getAccuracy() > 100 ? "-" : std::to_string(move.getAccuracy()));
		if (move.getDescription().empty())
			effects->setText("No additional effect");
		else
			effects->setText(move.getDescription());

		type->setPosition(190, 2);
		category->setPosition(206, 29);
		pan->add(type);
		pan->add(category);

		panel->add(pan);
	}
	*displayedPanels = *panels;
	applyMoveFilters(pkmn.getLearnableMoveSet(), 0, "", "", true, *displayedPanels);
	moveMovePanels(*displayedPanels);
	bigPan->add(panel);
	gui.add(bigPan);
}

void movePkmnsPanels(const std::vector<std::pair<unsigned, tgui::ScrollablePanel::Ptr>> &panels)
{
	for (unsigned i = 0; i < panels.size(); i++)
		panels[i].second->setPosition(28 + i % 3 * 248, 10 + i / 3 * 190);
}

void applyPkmnsFilters(unsigned sorting, std::string query, const std::string &type, std::vector<std::pair<unsigned, tgui::ScrollablePanel::Ptr>> &panels)
{
	std::vector<std::function<bool(const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2)>> sortingAlgos = {
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			if (base1.name == base2.name)
				return p1.first < p2.first;
			return base2.name == "MISSINGNO." || (
				base1.name != "MISSINGNO." &&
				base1.name < base2.name
			);
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			if (base1.name == base2.name)
				return p1.first < p2.first;
			return base2.name == "MISSINGNO." || (
				base1.name != "MISSINGNO." &&
				base1.name > base2.name
			);
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			return base1.dexId < base2.dexId;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			return base1.dexId > base2.dexId;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			return p1.first < p2.first;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			return p1.first > p2.first;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			return base1.HP < base2.HP;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			return base1.HP > base2.HP;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			return base1.ATK < base2.ATK;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			return base1.ATK > base2.ATK;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			return base1.DEF < base2.DEF;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			return base1.DEF > base2.DEF;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			return base1.SPD < base2.SPD;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			return base1.SPD > base2.SPD;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			return base1.SPE < base2.SPE;
		},
		[](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1, const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p2){
			auto &base1 = pokemonList.at(p1.first);
			auto &base2 = pokemonList.at(p2.first);

			return base1.SPE > base2.SPE;
		}
	};

	if (!query.empty()) {
		query = Utils::toLower(query);

		panels.erase(std::remove_if(
			panels.begin(),
			panels.end(),
			[&query](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1) {
				return Utils::toLower(pokemonList.at(p1.first).name).find(query) == std::string::npos;
			}
		), panels.end());
	}
	if (!type.empty())
		panels.erase(std::remove_if(
			panels.begin(),
			panels.end(),
			[&type](const std::pair<unsigned, tgui::ScrollablePanel::Ptr> &p1) {
				auto &base = pokemonList.at(p1.first);

				return typeToString(base.typeA) != type && typeToString(base.typeB) != type;
			}
		), panels.end());
	std::sort(panels.begin(), panels.end(), sortingAlgos[sorting]);
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
	auto &state = game.getBattleState();
	auto bigPan = tgui::Panel::create({"100%", "100%"});
	auto panel = tgui::ScrollablePanel::create({"&.w - 20", "&.h - 50"});
	auto basePanel = tgui::ScrollablePanel::create({220, 170});
	auto panels = std::make_shared<std::vector<std::pair<unsigned, tgui::ScrollablePanel::Ptr>>>(pokemonList.size());
	auto displayedPanels = std::make_shared<std::vector<std::pair<unsigned, tgui::ScrollablePanel::Ptr>>>(pokemonList.size());
	auto filter = tgui::EditBox::create();
	auto sorting = tgui::ComboBox::create();
	auto typeFilter = tgui::ComboBox::create();

	filter->setSize("&.w * 50 / 100 - 30", 20);
	typeFilter->setSize("&.w * 20 / 100 - 20", 20);
	sorting->setSize("&.w * 30 / 100 - 10", 20);

	filter->setDefaultText("Search");
	sorting->addItem("Sort A -> Z");
	sorting->addItem("Sort Z -> A");
	sorting->addItem("Sort by ascending pokedex ID");
	sorting->addItem("Sort by descending pokedex ID");
	sorting->addItem("Sort by ascending ID");
	sorting->addItem("Sort by descending ID");
	sorting->addItem("Sort by ascending max HP");
	sorting->addItem("Sort by descending max HP");
	sorting->addItem("Sort by ascending ATK");
	sorting->addItem("Sort by descending ATK");
	sorting->addItem("Sort by ascending DEF");
	sorting->addItem("Sort by descending DEF");
	sorting->addItem("Sort by ascending SPD");
	sorting->addItem("Sort by descending SPD");
	sorting->addItem("Sort by ascending SPE");
	sorting->addItem("Sort by descending SPE");
	sorting->setSelectedItemByIndex(0);

	typeFilter->addItem("--Filter by type--", "");
	typeFilter->addItem("Normal", "Normal");
	typeFilter->addItem("Fighting", "Fighting");
	typeFilter->addItem("Fly", "Fly");
	typeFilter->addItem("Poison", "Poison");
	typeFilter->addItem("Ground", "Ground");
	typeFilter->addItem("Rock", "Rock");
	typeFilter->addItem("Bug", "Bug");
	typeFilter->addItem("Ghost", "Ghost");
	typeFilter->addItem("Fire", "Fire");
	typeFilter->addItem("Water", "Water");
	typeFilter->addItem("Grass", "Grass");
	typeFilter->addItem("Electric", "Electric");
	typeFilter->addItem("Psy", "Psy");
	typeFilter->addItem("Ice", "Ice");
	typeFilter->addItem("Dragon", "Dragon");
	typeFilter->addItem("???", "???");
	typeFilter->setSelectedItemByIndex(0);

	auto refresh = [displayedPanels, panels](
		std::weak_ptr<tgui::EditBox> filter,
		std::weak_ptr<tgui::ComboBox> sorting,
		std::weak_ptr<tgui::ComboBox> typeFilter
	){
		for (auto &panel : *panels)
			panel.second->setPosition(-200, -200);
		*displayedPanels = *panels;
		applyPkmnsFilters(sorting.lock()->getSelectedItemIndex(), filter.lock()->getText().toStdString(), typeFilter.lock()->getSelectedItemId().toStdString(), *displayedPanels);
		movePkmnsPanels(*displayedPanels);
	};

	filter->onTextChange.connect(refresh, std::weak_ptr(filter), std::weak_ptr(sorting), std::weak_ptr(typeFilter));
	sorting->onItemSelect.connect(refresh, std::weak_ptr(filter), std::weak_ptr(sorting), std::weak_ptr(typeFilter));
	typeFilter->onItemSelect.connect(refresh, std::weak_ptr(filter), std::weak_ptr(sorting), std::weak_ptr(typeFilter));

	filter->setPosition(10, 10);
	sorting->setPosition("&.w * 70 / 100", 10);
	typeFilter->setPosition("&.w * 50 / 100", 10);
	panel->setPosition(10, 40);

	bigPan->add(typeFilter);
	bigPan->add(sorting);
	bigPan->add(filter);

	basePanel->loadWidgetsFromFile("assets/pkmnPreview.gui");

	for (const auto &[i, base] : pokemonList) {
		auto pan = (panels->operator[](i) = {i, tgui::ScrollablePanel::copy(basePanel)}).second;
		auto type1 = tgui::Picture::create(resources.types[typeToString(base.typeA)]);
		auto type2 = tgui::Picture::create(resources.types[typeToString(base.typeB)]);
		auto sprite = pan->get<tgui::BitmapButton>("Species");
		auto hp = pan->get<tgui::TextArea>("HP");
		auto atk = pan->get<tgui::TextArea>("ATK");
		auto def = pan->get<tgui::TextArea>("DEF");
		auto spd = pan->get<tgui::TextArea>("SPD");
		auto spe = pan->get<tgui::TextArea>("SPE");
		auto name = pan->get<tgui::TextArea>("SpeciesName");
		auto &stats = base.statsAtLevel[pkmn.getLevel()];

		name->setText(strToUpper(base.name));
		hp->setText(std::to_string(stats.HP));
		atk->setText(std::to_string(stats.ATK));
		def->setText(std::to_string(stats.DEF));
		spd->setText(std::to_string(stats.SPD));
		spe->setText(std::to_string(stats.SPE));
		sprite->setImage(resources.pokemonsFront[base.id]);

		sprite->onClick.connect([&emulator, &state, &aisSelected, &side, &window, &resources, &gui, index, &pkmn, &game, &base, &ready](std::weak_ptr<tgui::Panel> pkmnPan, std::weak_ptr<tgui::Panel> bigPan){
			auto &s = (side ? state.op : state.me);

			s.team.at(index) = Pokemon(state.rng, state.battleLogger, pkmn.getNickname(), pkmn.getLevel(), base, pkmn.getMoveSet());
			gui.remove(bigPan.lock());
			populatePokemonPanel(window, gui, emulator, game, resources, pkmnPan.lock(), index, s.team, aisSelected, side, ready);
		}, std::weak_ptr(pkmnPan), std::weak_ptr(bigPan));

		type1->setPosition(115, 152);
		type2->setPosition(170, 152);
		type2->setVisible(base.typeA != base.typeB);
		pan->add(type1);
		pan->add(type2);

		panel->add(pan);
	}
	*displayedPanels = *panels;
	applyPkmnsFilters(0, "", "", *displayedPanels);
	movePkmnsPanels(*displayedPanels);
	bigPan->add(panel);
	gui.add(bigPan);
}

void populatePokemonPanel(
	sf::RenderWindow &window,
	tgui::Gui &gui,
	std::unique_ptr<EmulatorGameHandle> &emulator,
	BattleHandler &game,
	BattleResources &resources,
	tgui::Panel::Ptr panel,
	unsigned index,
	std::vector<Pokemon> &team,
	std::pair<unsigned char, unsigned char> &aisSelected,
	bool &side,
	bool &ready
)
{
	panel->loadWidgetsFromFile("assets/pkmnPanel.gui");

	auto &pkmn = team[index];
	auto type1 = tgui::Picture::create(resources.types[typeToString(pkmn.getTypes().first)]);
	auto type2 = tgui::Picture::create(resources.types[typeToString(pkmn.getTypes().second)]);
	auto sprite = panel->get<tgui::BitmapButton>("Species");
	auto hp = panel->get<tgui::TextArea>("HP");
	auto atk = panel->get<tgui::TextArea>("ATK");
	auto def = panel->get<tgui::TextArea>("DEF");
	auto spd = panel->get<tgui::TextArea>("SPD");
	auto spe = panel->get<tgui::TextArea>("SPE");
	std::vector<tgui::Button::Ptr> moves = {
		panel->get<tgui::Button>("Move1"),
		panel->get<tgui::Button>("Move2"),
		panel->get<tgui::Button>("Move3"),
		panel->get<tgui::Button>("Move4")
	};
	auto name = panel->get<tgui::TextArea>("SpeciesName");
	auto nick = panel->get<tgui::EditBox>("Nickname");
	auto level = panel->get<tgui::EditBox>("Level");
	auto remove = panel->get<tgui::Button>("Remove");
	auto &moveSet = pkmn.getMoveSet();

	for (size_t i = 0; i < moveSet.size(); i++) {
		auto &move = moves[i];

		move->setText(moveSet[i].getName());
		move->onClick.connect([i, &gui, &pkmn, &resources](std::weak_ptr<tgui::Button> move){
			openChangeMoveBox(gui,  resources, pkmn, i, move.lock());
		}, std::weak_ptr(move));
	}
	level->setText(std::to_string(pkmn.getLevel()));
	name->setText(strToUpper(pokemonList.at(pkmn.getID()).name));
	nick->setText(pkmn.getNickname());
	hp->setText(std::to_string(pkmn.getBaseStats().HP));
	atk->setText(std::to_string(pkmn.getBaseStats().ATK));
	def->setText(std::to_string(pkmn.getBaseStats().DEF));
	spd->setText(std::to_string(pkmn.getBaseStats().SPD));
	spe->setText(std::to_string(pkmn.getBaseStats().SPE));
	sprite->setImage(resources.pokemonsFront[pkmn.getID()]);

	nick->onTextChange.connect([&pkmn](tgui::String str){
		pkmn.setNickname(str.toStdString());
	});
	level->onTextChange.connect([&pkmn](
		std::weak_ptr<tgui::EditBox> level,
		std::weak_ptr<tgui::TextArea> hp,
		std::weak_ptr<tgui::TextArea> atk,
		std::weak_ptr<tgui::TextArea> def,
		std::weak_ptr<tgui::TextArea> spd,
		std::weak_ptr<tgui::TextArea> spe
	){
		if (level.lock()->getText().empty())
			return;

		auto newLevel = std::stoul(level.lock()->getText().toStdString());

		if (newLevel > 255) {
			newLevel = 255;
			level.lock()->setText("255");
		}

		pkmn.setLevel(newLevel);
		hp.lock()->setText(std::to_string(pkmn.getBaseStats().HP));
		atk.lock()->setText(std::to_string(pkmn.getBaseStats().ATK));
		def.lock()->setText(std::to_string(pkmn.getBaseStats().DEF));
		spd.lock()->setText(std::to_string(pkmn.getBaseStats().SPD));
		spe.lock()->setText(std::to_string(pkmn.getBaseStats().SPE));
	}, std::weak_ptr(level), std::weak_ptr(hp), std::weak_ptr(atk), std::weak_ptr(def), std::weak_ptr(spd), std::weak_ptr(spe));
	remove->onClick.connect([&emulator, &team, &aisSelected, &side, index, &window, &gui, &game, &resources, &ready]{
		team.erase(team.begin() + index);
		makeMainMenuGUI(window, gui, emulator, game, resources, aisSelected, side, ready);
	});
	sprite->onClick.connect([&emulator, &aisSelected, &side, &gui, &game, &resources, index, &pkmn, &window, &ready](std::weak_ptr<tgui::Panel> panel){
		openChangePkmnBox(gui, game, emulator, resources, index, pkmn, window, panel.lock(), aisSelected, side, ready);
	}, std::weak_ptr(panel));

	type1->setPosition(5, 100);
	type1->setSize(44, 16);
	type2->setPosition(52, 100);
	type2->setSize(44, 16);
	type2->setVisible(pkmn.getTypes().first != pkmn.getTypes().second);
	panel->add(type1);
	panel->add(type2);
}

void makeMainMenuGUI(
	sf::RenderWindow &window,
	tgui::Gui &gui,
	std::unique_ptr<EmulatorGameHandle> &emulator,
	BattleHandler &game,
	BattleResources &resources,
	std::pair<unsigned char, unsigned char> &aisSelected,
	bool &side,
	bool &ready
)
{
	gui.loadWidgetsFromFile("assets/mainMenu.gui");

	auto &state = game.getBattleState();
	PlayerState &player = side ? state.op : state.me;
	unsigned char &aiSelected = side ? aisSelected.second : aisSelected.first;
	auto ai = gui.get<tgui::ComboBox>("AI");
	auto switchSide = gui.get<tgui::Button>("SwitchSide");
	auto connect = gui.get<tgui::Button>("Connect");
	auto ip = gui.get<tgui::EditBox>("IP");
	auto port = gui.get<tgui::EditBox>("Port");
	auto error = gui.get<tgui::TextArea>("Error");
	auto team = gui.get<tgui::Panel>("Team");
	auto readyButton = gui.get<tgui::Button>("Ready");
	auto load = gui.get<tgui::Button>("Load");
	auto save = gui.get<tgui::Button>("Save");
	auto loadState = gui.get<tgui::Button>("LoadState");
	auto loadReplay = gui.get<tgui::Button>("LoadReplay");
	auto teamPanel = gui.get<tgui::Panel>("Team");
	std::array<tgui::Panel::Ptr, 6> panels{
		teamPanel->get<tgui::Panel>("Pkmn1"),
		teamPanel->get<tgui::Panel>("Pkmn2"),
		teamPanel->get<tgui::Panel>("Pkmn3"),
		teamPanel->get<tgui::Panel>("Pkmn4"),
		teamPanel->get<tgui::Panel>("Pkmn5"),
		teamPanel->get<tgui::Panel>("Pkmn6")
	};
	auto name = teamPanel->get<tgui::EditBox>("Name");

	switchSide->setText(side ? "Opponent side" : "Player side");
	if (emulator) {
		switchSide->setEnabled(false);
		port->setEnabled(false);
		ip->setEnabled(false);
	}
	ai->removeAllItems();
	for (auto &s : AIFactory::getList())
		ai->addItem(s);
	ai->setSelectedItemByIndex(aiSelected);
	for (auto &pkmn : player.team)
		pkmn.reset();
	ai->onItemSelect.connect([&aiSelected](unsigned char nb) {
		aiSelected = nb;
	});
	switchSide->onClick.connect([&window, &gui, &emulator, &game, &resources, &aisSelected, &side, &ready]{
		side = !side;
		makeMainMenuGUI(window, gui, emulator, game, resources, aisSelected, side, ready);
	});
	save->onClick.connect([&player]{
		std::string path = Utils::saveFileDialog("Save team", ".", {{".+[.]pkmns", "Pokemon team file"}});

		if (path.empty())
			return;

		std::ofstream stream{path, std::ofstream::binary};

		if (stream.fail()) {
			Utils::dispMsg("Error", "Cannot open file " + path + "\n" + strerror(errno), MB_ICONERROR);
			return;
		}

		auto data = saveTrainer({player.name, player.team});

		stream.write(reinterpret_cast<const char *>(data.data()), data.size());
	});
	load->onClick.connect([&aisSelected, &side, &window, &resources, &gui, &game, &state, &player, &emulator, &ready]{
		std::string path = Utils::openFileDialog("Open team file", ".", {{".+[.]pkmns", "Pokemon team file"}});

		if (path.empty())
			return;

		std::ifstream stream{path, std::ifstream::binary};

		if (stream.fail()) {
			Utils::dispMsg("Error", "Cannot open file " + path + "\n" + strerror(errno), MB_ICONERROR);
			return;
		}
		stream.seekg(0, std::ifstream::end);

		auto length = stream.tellg();
		std::vector<unsigned char> buffer;

		buffer.resize(length);
		stream.seekg(0, std::ifstream::beg);
		stream.read(reinterpret_cast<char *>(buffer.data()), length);
		try {
			auto t = loadTrainer(buffer, state.rng, state.battleLogger);

			player.name = t.first;
			player.team = t.second;
			makeMainMenuGUI(window, gui, emulator, game, resources, aisSelected, side, ready);
		} catch (std::exception &e) {
			Utils::dispMsg(Utils::getLastExceptionName(), "Cannot load save file \"" + path + "\"\n" + e.what(), MB_ICONERROR);
		}
	});
	loadReplay->onClick.connect([&game]{
		std::string path = Utils::openFileDialog("Open team file", ".", {{".+[.]replay", "Pokemon replay file"}});

		if (path.empty())
			return;

		try {
			game.loadReplay(path);
		} catch (std::exception &e) {
			Utils::dispMsg("Replay loading error", "Failed to load " + path + ": " + e.what(), MB_ICONERROR);
		}
	});
	ip->setText(lastIp);
	ip->onTextChange.connect([](tgui::String str){
		lastIp = str.toStdString();
	});
	port->setText(lastPort);
	port->onTextChange.connect([](tgui::String str){
		lastPort = str.toStdString();
	});
	readyButton->setText(emulator && emulator->isReady() ? "You are ready" : "You are not ready");
	readyButton->onClick.connect([&emulator, panels, name, &ready](std::weak_ptr<tgui::Button> readyButton){
		if (!emulator) {
			ready = true;
			return;
		}
		if (emulator->getStage() >= EmulatorGameHandle::EXCHANGE_POKEMONS)
			return;

		emulator->setReady(!emulator->isReady());
		readyButton.lock()->setText(emulator->isReady() ? "You are ready" : "You are not ready");
		name->setEnabled(!emulator->isReady());
		for (auto &panel : panels)
			for (auto &widget : panel->getWidgets())
				widget->setEnabled(!emulator->isReady());
	}, readyButton);
	name->setText(player.name);
	connect->setText(!emulator ? "Connect" : "Disconnect");
	connect->onClick.connect([&state, &emulator, error, port, ip, switchSide, &window, &gui, &game, &resources, &aisSelected, &side, &ready](std::weak_ptr<tgui::Button> connect){
		error->setText("");
		if (emulator) {
			emulator.reset();
			connect.lock()->setText("Connect");
			port->setEnabled(true);
			ip->setEnabled(true);
			switchSide->setEnabled(true);
		} else {
			try {
				auto p = std::stoul(port->getText().toStdString());
				auto ips = ip->getText().toStdString();

				if (p > 65535)
					throw std::out_of_range("");
				emulator = std::make_unique<EmulatorGameHandle>(
					[ips, p](const ByteHandle &byteHandle, const LoopHandle &loopHandler) {
						return new BGBHandler(byteHandle, byteHandle, loopHandler, ips, p, getenv("MAX_DEBUG"));
					},
					state,
					false,
					getenv("MIN_DEBUG") != nullptr
				);
				aisSelected.second = 0;
				side = false;
				makeMainMenuGUI(window, gui, emulator, game, resources, aisSelected, side, ready);
			} catch (std::invalid_argument &) {
				error->setText("The port is not a valid number");
				return;
			} catch (std::out_of_range &) {
				error->setText("The port is not in range 0-65535");
				return;
			} catch (std::exception &e) {
				error->setText(e.what());
				return;
			}
			connect.lock()->setText("Disconnect");
		}
	}, std::weak_ptr(connect));
	name->onTextChange.connect([&window, &player, &side](tgui::String str){
		player.name = str.toStdString();
		if (!side)
			window.setTitle(player.name + " - Preparing battle");
	});

	for (auto &pkmnPan : panels)
		pkmnPan->removeAllWidgets();
	for (unsigned i = 0; i < player.team.size(); i++)
		populatePokemonPanel(window, gui, emulator, game, resources, panels[i], i, player.team, aisSelected, side, ready);
	for (unsigned i = player.team.size(); i < 6; i++) {
		tgui::Button::Ptr but = tgui::Button::create("+");

		but->setPosition(10, 10);
		but->setSize({"&.w - 20", "&.h - 20"});
		but->onClick.connect([&player, &emulator, &state, &window, &gui, &game, &resources, &aisSelected, &side, &ready]{
			player.team.emplace_back(
				state.rng, state.battleLogger, "", 100,
				pokemonList.at(Rhydon),
				std::vector<Move>{
					availableMoves[Tackle],
					availableMoves[Tail_Whip]
				}
			);
			makeMainMenuGUI(window, gui, emulator, game, resources, aisSelected, side, ready);
		});
		panels[i]->add(but);
	}
}

void mainMenu(sf::RenderWindow &window, std::unique_ptr<EmulatorGameHandle> &emulator, BattleHandler &game, BattleResources &resources, std::pair<unsigned char, unsigned char> &ai, bool &ready)
{
	bool side;
	tgui::Gui gui{window};
	auto &state = game.getBattleState();

	if (emulator)
		emulator->setReady(false);

	ready = false;
	window.setSize({800, 640});
	makeMainMenuGUI(window, gui, emulator, game, resources, ai, side, ready);

	window.setTitle(state.me.name + " - Preparing battle");
	while (window.isOpen() && (!emulator || emulator->getStage() != EmulatorGameHandle::BATTLE) && (!ready || emulator) && !game.playingReplay()) {
		while (auto event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>())
				window.close();
			gui.handleEvent(*event);
		}

		window.clear();
		gui.draw();
		window.display();

		auto status = gui.get<tgui::TextArea>("Status");
		auto progress = gui.get<tgui::ProgressBar>("Progress");

		if (!status || !progress)
			return;

		if (emulator) {
			progress->setValue(emulator->getBattleSendingProgress().first);
			progress->setMaximum(emulator->getBattleSendingProgress().second);
			switch (emulator->getStage()) {
			case EmulatorGameHandle::PKMN_CENTER:
				status->setText("Opponent not ready");
				break;
			case EmulatorGameHandle::PINGING_OPPONENT:
				status->setText("Waiting for opponent to save the game");
				break;
			case EmulatorGameHandle::ROOM_CHOOSE:
				status->setText("Choosing colosseum");
				break;
			case EmulatorGameHandle::PING_POKEMON_EXCHANGE:
				status->setText(emulator->isReady() ? "Waiting for opponent to start the game" : "Waiting for you to be ready");
				break;
			case EmulatorGameHandle::EXCHANGE_POKEMONS:
				status->setText("Exchanging battle data");
				break;
			case EmulatorGameHandle::BATTLE:
				status->setText("In battle");
				break;
			}
		} else
			status->setText("Disconnected");
	}
}

void loadResources(BattleResources &resources)
{
	resources.start.openFromFile("assets/sounds/battle_intro.wav");
	resources.loop.openFromFile("assets/sounds/battle_loop.wav");
	resources.levelSprite.loadFromFile("assets/level_icon.png");

	resources.categories[0].loadFromFile("assets/move_categories/physical.png");
	resources.categories[1].loadFromFile("assets/move_categories/special.png");
	resources.categories[2].loadFromFile("assets/move_categories/status.png");

	for (int i = 0; i < 256; i++)
		if (!resources.pokemonsBack[i].loadFromFile("assets/back_sprites/" + std::to_string(i) + "_back.png"))
			resources.pokemonsBack[i].loadFromFile("assets/back_sprites/missingno_back.png");

	for (int i = 0; i < 256; i++)
		if (!resources.pokemonsFront[i].loadFromFile("assets/front_sprites/" + std::to_string(i) + "_front.png"))
			resources.pokemonsFront[i].loadFromFile("assets/front_sprites/missingno_front.png");

	resources.balls[0].loadFromFile("assets/pokeballs/pkmnOK.png");
	resources.balls[1].loadFromFile("assets/pokeballs/pkmnNO.png");
	resources.balls[2].loadFromFile("assets/pokeballs/pkmnFNT.png");
	resources.balls[3].loadFromFile("assets/pokeballs/pkmnSTATUS.png");

	resources.font.openFromFile("assets/font.ttf");
	resources.hitSounds[0].loadFromFile("assets/sounds/not_effective_hit_sound.wav");
	resources.hitSounds[1].loadFromFile("assets/sounds/hit_sound.wav");
	resources.hitSounds[2].loadFromFile("assets/sounds/very_effective_hit_sound.wav");

	for (int i = 0; i <= TYPE_DRAGON; i++)
		try {
			resources.types.at(typeToString(static_cast<Type>(i)));
		} catch (std::out_of_range &) {
			resources.types[typeToString(static_cast<Type>(i))].loadFromFile("assets/types/type_" + Utils::toLower(typeToString(static_cast<Type>(i))) + ".png");
		}

	resources.trainer[0][0].loadFromFile("assets/back_sprites/trainer_shadow_back.png");
	resources.trainer[0][1].loadFromFile("assets/front_sprites/trainer_shadow_front.png");
	resources.trainer[1][0].loadFromFile("assets/back_sprites/trainer_back.png");
	resources.trainer[1][1].loadFromFile("assets/front_sprites/trainer_front.png");

	resources.trainerLand.loadFromFile("assets/sounds/trainer_land.wav");
	resources.hpOverlay.loadFromFile("assets/hp_overlay.png");
	resources.choicesHUD.loadFromFile("assets/choices.png");
	resources.attackHUD.loadFromFile("assets/attacks_overlay.png");
	resources.waitingHUD.loadFromFile("assets/wait_overlay.png");

	resources.arrows[0].loadFromFile("assets/arrow.png");
	resources.arrows[1].loadFromFile("assets/selectArrow.png");

	resources.boxes[0].loadFromFile("assets/text_box.png");
	resources.boxes[1].loadFromFile("assets/VS_box.png");
	resources.boxes[2].loadFromFile("assets/pkmns_border.png");
	resources.boxes[3].loadFromFile("assets/pkmns_border_player_side.png");

	for (int i = 0; i < 256; i++)
		resources.battleCries[i].loadFromFile("assets/cries/" + std::to_string(i) + "_cry.wav");
}

static std::string splitText(std::string str)
{
	size_t lineSize = 0;
	std::string result;
	std::string token;

	result.reserve(str.size());
	for (size_t pos = str.find(' '); !str.empty(); pos = str.find(' ')) {
		token = str.substr(0, pos);

		if (!result.empty()) {
			if (lineSize + token.size() + 1 > 18) {
				result += "\n";
				lineSize = 0;
			} else {
				result += " ";
				lineSize++;
			}
		}
		result += token;
		lineSize += token.size();
		if (pos == std::string::npos)
			break;
		str.erase(0, pos + 1);
	}

	return result;
}

void gui(const std::string &trainerName)
{
	std::vector<std::string> battleLog;
	std::unique_ptr<EmulatorGameHandle> emulator;
	std::pair<unsigned char, unsigned char> ais = {0, 0};
	BattleHandler battleHandler{false, getenv("MIN_DEBUG") != nullptr};
	auto &state = battleHandler.getBattleState();
	sf::RenderWindow window{sf::VideoMode{{800, 640}}, trainerName};
	BattleResources resources;
	bool ready;

	state.rng.makeRandomList(9);
	state.battleLogger = [&battleLog](const std::string &msg){
		std::cout << "[BATTLE]: " << msg << "!" << std::endl;
		battleLog.push_back(splitText(msg));
	};
	loadResources(resources);
	state.me.team.resize(6, {
		state.rng, state.battleLogger, "", 100,
		pokemonList.at(Rhydon),
		std::vector<Move>{
			availableMoves[Tackle],
			availableMoves[Tail_Whip]
		}
	});
	state.op.team.resize(6, {
		state.rng, state.battleLogger, "", 100,
		pokemonList.at(Rhydon),
		std::vector<Move>{
			availableMoves[Tackle],
			availableMoves[Tail_Whip]
		}
	});

	window.setFramerateLimit(60);
	while (window.isOpen()) {
		if (
			!battleHandler.playingReplay() &&
			(!emulator || emulator->getStage() != EmulatorGameHandle::BATTLE) &&
			(!ready || emulator)
		)
			mainMenu(window, emulator, battleHandler, resources, ais, ready);
		else {
			if (emulator && emulator->getStage() != EmulatorGameHandle::BATTLE) {
				state.me.nextAction = Run;
				state.op.nextAction = Run;
				battleHandler.tick();
			}
			battle(window, battleHandler, resources, battleLog, ais, !emulator);
			ready = false;
		}
	}
}

int main(int argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "-h") == 0) {
		std::cerr << "Usage: " << argv[0] << " [<mode>, <trainerName>]" << std::endl;
		return EXIT_FAILURE;
	}
	gui(argc > 1 ? argv[1] : "PokeAI");
	return EXIT_SUCCESS;
}