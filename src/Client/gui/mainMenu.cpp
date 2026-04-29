//
// Created by PinkySmile on 22/04/2026.
//

#include <TGUI/TGUI.hpp>
#include <SFML/Audio.hpp>
#include <array>
#include <filesystem>
#include <fstream>
#include "mainMenu.hpp"
#include "pickers.hpp"
#include "Utils.hpp"
#include "GameEngine/Gen1/Team.hpp"
#include "Emulator/BgbHandler.hpp"
#include "../AIs/AIFactory.hpp"
#include "Renderers/Gen1Renderer.hpp"

using namespace PokemonGen1;

std::string lastIp;
std::string lastPort;
unsigned char lastRendererIndex = 0;

namespace
{
	struct RendererPreset {
		const char *label;
		const char *variant;
		bool hasColors;
	};

	constexpr std::array<RendererPreset, 6> rendererPresets{{
		{"Yellow",          "",   true},
		{"Yellow (B&W)",    "",   false},
		{"Red/Blue",        "r",  true},
		{"Red/Blue (B&W)",  "r",  false},
		{"Green",           "rg", true},
		{"Green (B&W)",     "rg", false},
	}};

	std::unique_ptr<PkmnRenderer::IRenderer> makeRenderer(unsigned char index)
	{
		if (index >= rendererPresets.size())
			index = 0;
		const auto &p = rendererPresets[index];
		return std::make_unique<PkmnRenderer::Gen1Renderer>(p.variant, p.hasColors);
	}
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
	std::array<tgui::Button::Ptr, 4> moves{
		panel->get<tgui::Button>("Move1"),
		panel->get<tgui::Button>("Move2"),
		panel->get<tgui::Button>("Move3"),
		panel->get<tgui::Button>("Move4"),
	};
	auto name = panel->get<tgui::TextArea>("SpeciesName");
	auto nick = panel->get<tgui::EditBox>("Nickname");
	auto level = panel->get<tgui::EditBox>("Level");
	auto remove = panel->get<tgui::Button>("Remove");
	auto &moveSet = pkmn.getMoveSet();

	for (size_t i = 0; i < moveSet.size(); i++) {
		auto &move = moves[i];

		move->setText(moveSet[i].getName());
		move->onClick.connect([i, &gui, &pkmn, &resources](std::weak_ptr<tgui::Button> btn){
			openChangeMoveBox(gui, resources, pkmn, i, btn.lock());
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
	sprite->setImage(resources.renderer->getPkmnFace(PkmnRenderer::gen1SpeciesToCommon(pkmn.getID())));

	nick->onTextChange.connect([&pkmn](tgui::String str){
		pkmn.setNickname(str.toStdString());
	});
	level->onTextChange.connect([&pkmn, hp, atk, def, spd, spe](tgui::String str){
		if (str.empty())
			return;
		auto newLevel = std::stoul(str.toStdString());

		if (newLevel > 255)
			newLevel = 255;
		pkmn.setLevel(newLevel);
		hp->setText(std::to_string(pkmn.getBaseStats().HP));
		atk->setText(std::to_string(pkmn.getBaseStats().ATK));
		def->setText(std::to_string(pkmn.getBaseStats().DEF));
		spd->setText(std::to_string(pkmn.getBaseStats().SPD));
		spe->setText(std::to_string(pkmn.getBaseStats().SPE));
	});
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
	auto rendererCombo = gui.get<tgui::ComboBox>("Renderer");
	auto switchSide = gui.get<tgui::Button>("SwitchSide");
	auto connect = gui.get<tgui::Button>("Connect");
	auto ip = gui.get<tgui::EditBox>("IP");
	auto port = gui.get<tgui::EditBox>("Port");
	auto error = gui.get<tgui::TextArea>("Error");
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
		teamPanel->get<tgui::Panel>("Pkmn6"),
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
	ai->onItemSelect.connect([&aiSelected](unsigned char nb){
		aiSelected = nb;
	});
	if (rendererCombo) {
		rendererCombo->removeAllItems();
		for (auto &p : rendererPresets)
			rendererCombo->addItem(p.label);
		if (lastRendererIndex >= rendererPresets.size())
			lastRendererIndex = 0;
		rendererCombo->setSelectedItemByIndex(lastRendererIndex);
		rendererCombo->onItemSelect.connect([&window, &gui, &emulator, &game, &resources, &aisSelected, &side, &ready](unsigned char nb){
			if (nb >= rendererPresets.size())
				return;
			lastRendererIndex = nb;
			resources.renderer = makeRenderer(nb);
			invalidatePkmnPickerCache();
			makeMainMenuGUI(window, gui, emulator, game, resources, aisSelected, side, ready);
		});
	}
	switchSide->onClick.connect([&window, &gui, &emulator, &game, &resources, &aisSelected, &side, &ready]{
		side = !side;
		makeMainMenuGUI(window, gui, emulator, game, resources, aisSelected, side, ready);
	});
	save->onClick.connect([&player, &state]{
		std::string path = Utils::saveFileDialog("Save team", ".", {{".+[.]pkmns", "Pokemon team file"},{".+[.]scenario", "Pokemon scenario file"}});

		if (path.empty())
			return;

		std::ofstream stream{path, std::ofstream::binary};

		if (stream.fail()) {
			Utils::dispMsg("Error", "Cannot open file " + path + "\n" + strerror(errno), MB_ICONERROR);
			return;
		}

		if (path.ends_with(".scenario")) {
			auto data = saveTrainer({state.me.name, state.me.team});

			stream.write(reinterpret_cast<const char *>(data.data()), data.size());
			data = saveTrainer({state.op.name, state.op.team});
			stream.write(reinterpret_cast<const char *>(data.data()), data.size());
		} else {
			auto data = saveTrainer({player.name, player.team});

			stream.write(reinterpret_cast<const char *>(data.data()), data.size());
		}
	});
	load->onClick.connect([&aisSelected, &side, &window, &resources, &gui, &game, &state, &player, &emulator, &ready]{
		std::string path = Utils::openFileDialog("Open team file", ".", {{".+[.]pkmns", "Pokemon team file"}, {".+[.]scenario", "Pokemon scenario file"}});

		if (path.empty())
			return;

		std::ifstream stream{path, std::ifstream::binary};

		if (stream.fail()) {
			Utils::dispMsg("Error", "Cannot open file " + path + "\n" + strerror(errno), MB_ICONERROR);
			return;
		}

		auto length = std::filesystem::file_size(path);
		std::vector<unsigned char> buffer;

		if (path.ends_with(".scenario")) {
			Trainer me;
			Trainer op;

			buffer.resize(length / 2);
			try {
				stream.read(reinterpret_cast<char *>(buffer.data()), length / 2);
				me = loadTrainer(buffer, state, false);
				stream.read(reinterpret_cast<char *>(buffer.data()), length / 2);
				op = loadTrainer(buffer, state, true);
			} catch (std::exception &e) {
				Utils::dispMsg(Utils::getLastExceptionName(), "Cannot load load file \"" + path + "\"\n" + e.what(), MB_ICONERROR);
			}

			state.me.name = me.first;
			state.me.team = me.second;
			state.op.name = op.first;
			state.op.team = op.second;
			makeMainMenuGUI(window, gui, emulator, game, resources, aisSelected, side, ready);
		} else {
			buffer.resize(length);
			stream.read(reinterpret_cast<char *>(buffer.data()), length);
			try {
				auto t = loadTrainer(buffer, state, side);

				player.name = t.first;
				player.team = t.second;
				makeMainMenuGUI(window, gui, emulator, game, resources, aisSelected, side, ready);
			} catch (std::exception &e) {
				Utils::dispMsg(Utils::getLastExceptionName(), "Cannot load load file \"" + path + "\"\n" + e.what(), MB_ICONERROR);
			}
		}
	});
	loadReplay->onClick.connect([&game]{
		std::string path = Utils::openFileDialog("Open replay file", ".", {{".+[.]replay", "Pokemon replay file"}});

		if (path.empty())
			return;

		try {
			game.loadReplay(path);
		} catch (std::exception &e) {
			Utils::dispMsg("Replay loading error", "Failed to load " + path + ": " + e.what(), MB_ICONERROR);
		}
	});
	loadState->onClick.connect([&game]{
		std::string path = Utils::openFileDialog("Open state file", ".", {{".+[.]json", "Pokemon state file"}});

		if (path.empty())
			return;

		try {
			game.loadState(path);
		} catch (std::exception &e) {
			Utils::dispMsg("State loading error", "Failed to load " + path + ": " + e.what(), MB_ICONERROR);
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
			return;
		}
		try {
			auto p = std::stoul(port->getText().toStdString());
			auto ips = ip->getText().toStdString();

			if (p > 65535)
				throw std::out_of_range("");
			emulator = std::make_unique<EmulatorGameHandle>(
				[ips, p](const ByteHandle &byteHandle, const LoopHandle &loopHandler){
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
				state, "", 100,
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
	bool side = false;
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

		if (!emulator) {
			status->setText("Disconnected");
			continue;
		}
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
	}
}
