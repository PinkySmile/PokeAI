//
// Created by Gegel85 on 30/08/2019.
//

#include <TGUI/TGUI.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>
#include "gui.hpp"
#include "../Networking/BgbHandler.hpp"

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

std::string toLower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
	return str;
}

void makeMainMenuGUI(sf::RenderWindow &window, tgui::Gui &gui, PokemonGen1::GameHandle &game, BattleResources &resources, std::string &lastIp, std::string &lastPort);

void populatePokemonPanel(sf::RenderWindow &window, tgui::Gui &gui, PokemonGen1::GameHandle &game, BattleResources &resources, std::string &lastIp, std::string &lastPort, unsigned index, tgui::Panel::Ptr panel, PokemonGen1::Pokemon &pkmn)
{
	panel->loadWidgetsFromFile("assets/pkmnPanel.gui");

	auto type1 = tgui::Picture::create(resources.types[typeToString(pkmn.getTypes().first)]);
	auto type2 = tgui::Picture::create(resources.types[typeToString(pkmn.getTypes().second)]);
	auto sprite = panel->get<tgui::BitmapButton>("Species");
	auto hp = panel->get<tgui::TextBox>("HP");
	auto atk = panel->get<tgui::TextBox>("ATK");
	auto def = panel->get<tgui::TextBox>("DEF");
	auto spd = panel->get<tgui::TextBox>("SPD");
	auto spe = panel->get<tgui::TextBox>("SPE");
	std::vector<tgui::Button::Ptr> moves = {
		panel->get<tgui::Button>("Move1"),
		panel->get<tgui::Button>("Move2"),
		panel->get<tgui::Button>("Move3"),
		panel->get<tgui::Button>("Move4")
	};
	auto name = panel->get<tgui::TextBox>("SpeciesName");
	auto nick = panel->get<tgui::EditBox>("Nickname");
	auto level = panel->get<tgui::EditBox>("Level");
	auto remove = panel->get<tgui::Button>("Remove");
	auto &moveSet = pkmn.getMoveSet();

	for (size_t i = 0; i < moveSet.size(); i++) {
		moves[i]->setText(moveSet[i].getName());
	}
	level->setText(std::to_string(pkmn.getLevel()));
	name->setText(strToUpper(PokemonGen1::pokemonList[pkmn.getID()].name));
	nick->setText(pkmn.getNickname());
	hp->setText(std::to_string(pkmn.getBaseStats().HP));
	atk->setText(std::to_string(pkmn.getBaseStats().ATK));
	def->setText(std::to_string(pkmn.getBaseStats().DEF));
	spd->setText(std::to_string(pkmn.getBaseStats().SPD));
	spe->setText(std::to_string(pkmn.getBaseStats().SPE));
	sprite->setImage(resources.pokemonsFront[pkmn.getID()]);

	nick->onTextChange.connect([&pkmn, nick]{
		pkmn.setNickname(nick->getText());
	});
	level->onTextChange.connect([&pkmn, level, hp, atk, def, spd, spe]{
		if (level->getText().isEmpty())
			return;

		auto newLevel = std::stoul(level->getText().toAnsiString());

		if (newLevel > 255) {
			newLevel = 255;
			level->setText("255");
		}

		pkmn.setLevel(newLevel);
		hp->setText(std::to_string(pkmn.getBaseStats().HP));
		atk->setText(std::to_string(pkmn.getBaseStats().ATK));
		def->setText(std::to_string(pkmn.getBaseStats().DEF));
		spd->setText(std::to_string(pkmn.getBaseStats().SPD));
		spe->setText(std::to_string(pkmn.getBaseStats().SPE));
	});
	remove->onClick.connect([index, &window, &gui, &game, &resources, &lastIp, &lastPort]{
		game.deletePokemon(index);
		makeMainMenuGUI(window, gui, game, resources, lastIp, lastPort);
	});

	type1->setPosition(5, 100);
	type1->setSize(44, 16);
	type2->setPosition(52, 100);
	type2->setSize(44, 16);
	type2->setVisible(pkmn.getTypes().first != pkmn.getTypes().second);
	panel->add(type1);
	panel->add(type2);
}

void makeMainMenuGUI(sf::RenderWindow &window, tgui::Gui &gui, PokemonGen1::GameHandle &game, BattleResources &resources, std::string &lastIp, std::string &lastPort)
{
	gui.loadWidgetsFromFile("assets/mainMenu.gui");

	auto connect = gui.get<tgui::Button>("Connect");
	auto ip = gui.get<tgui::EditBox>("IP");
	auto port = gui.get<tgui::EditBox>("Port");
	auto error = gui.get<tgui::TextBox>("Error");
	auto team = gui.get<tgui::Panel>("Team");
	auto ready = gui.get<tgui::Button>("Ready");
	auto name = gui.get<tgui::EditBox>("Name");
	auto teamPanel = gui.get<tgui::Panel>("Team");
	std::vector<tgui::Panel::Ptr> panels{
		teamPanel->get<tgui::Panel>("Pkmn1"),
		teamPanel->get<tgui::Panel>("Pkmn2"),
		teamPanel->get<tgui::Panel>("Pkmn3"),
		teamPanel->get<tgui::Panel>("Pkmn4"),
		teamPanel->get<tgui::Panel>("Pkmn5"),
		teamPanel->get<tgui::Panel>("Pkmn6"),
	};

	ip->setText(lastIp);
	ip->onTextChange.connect([&lastIp, ip]{
		lastIp = ip->getText();
	});
	port->setText(lastPort);
	port->onTextChange.connect([&lastPort, port]{
		lastPort = port->getText();
	});
	ready->setText(game.isReady() ? "You are ready" : "You are not ready");
	ready->onClick.connect([&game, ready, panels, name] {
		if (game.getStage() >= PokemonGen1::EXCHANGE_POKEMONS)
			return;

		game.setReady(!game.isReady());
		ready->setText(game.isReady() ? "You are ready" : "You are not ready");
		name->setEnabled(!game.isReady());
		for (auto &panel : panels)
			for (auto &widget : panel->getWidgets())
				widget->setEnabled(!game.isReady());
	});
	name->setText(game.getTrainerName());
	connect->onClick.connect([&game, connect, error, port, ip]{
		error->setText("");
		if (game.isConnected()) {
			game.disconnect();
			connect->setText("Connect");
			port->setEnabled(true);
			ip->setEnabled(true);
		} else {
			try {
				auto p = std::stoul(port->getText().toAnsiString());

				if (p > 65535)
					throw std::out_of_range("");
				game.connect(ip->getText(), p);
				port->setEnabled(false);
				ip->setEnabled(false);
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
			connect->setText("Disconnect");
		}
	});
	name->onTextChange.connect([&window, &game, name]{
		game.setTrainerName(name->getText());
		window.setTitle(game.getTrainerName() + " - Preparing battle");
	});

	for (auto &pkmnPan : panels)
		pkmnPan->removeAllWidgets();
	for (unsigned i = 0; i < game.getPokemonTeam().size(); i++)
		populatePokemonPanel(window, gui, game, resources, lastIp, lastPort, i, panels[i], game.getPokemon(i));
	for (unsigned i = game.getPokemonTeam().size(); i < 6; i++) {
		tgui::Button::Ptr but = tgui::Button::create("+");

		but->setPosition(10, 10);
		but->setSize({"&.w - 20", "&.h - 20"});
		but->onClick.connect([&window, &gui, &game, &resources, &lastPort, &lastIp]{
			game.setTeamSize(game.getPokemonTeam().size() + 1);
			makeMainMenuGUI(window, gui, game, resources, lastIp, lastPort);
		});
		panels[i]->add(but);
	}
}

void mainMenu(sf::RenderWindow &window, PokemonGen1::GameHandle &game, BattleResources &resources)
{
	tgui::Gui gui{window};
	std::string lastIp;
	std::string lastPort;

	window.setSize({800, 640});
	makeMainMenuGUI(window, gui, game, resources, lastIp, lastPort);

	game.setReady(false);
	window.setTitle(game.getTrainerName() + " - Preparing battle");
	while (window.isOpen() && game.getStage() != PokemonGen1::BATTLE) {
		sf::Event event;

		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			gui.handleEvent(event);
		}

		window.clear();
		gui.draw();
		window.display();

		auto status = gui.get<tgui::TextBox>("Status");

		if (!status)
			return;

		if (game.isConnected())
			switch (game.getStage()) {
			case PokemonGen1::PKMN_CENTER:
				status->setText("Opponent not ready");
				break;
			case PokemonGen1::PINGING_OPPONENT:
				status->setText("Waiting for opponent to save the game");
				break;
			case PokemonGen1::ROOM_CHOOSE:
				status->setText("Choosing colosseum");
				break;
			case PokemonGen1::PING_POKEMON_EXCHANGE:
				status->setText(game.isReady() ? "Waiting for opponent to start the game" : "Waiting for you to be ready");
				break;
			case PokemonGen1::EXCHANGE_POKEMONS:
				status->setText("Exchanging battle data");
				break;
			case PokemonGen1::BATTLE:
				status->setText("In battle");
				break;
			}
		else
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

	resources.font.loadFromFile("assets/font.ttf");
	resources.hitSounds[0].loadFromFile("assets/sounds/not_effective_hit_sound.wav");
	resources.hitSounds[1].loadFromFile("assets/sounds/hit_sound.wav");
	resources.hitSounds[2].loadFromFile("assets/sounds/very_effective_hit_sound.wav");

	for (int i = 0; i <= TYPE_DRAGON; i++)
		try {
			resources.types.at(typeToString(static_cast<PokemonTypes>(i)));
		} catch (std::out_of_range &) {
			resources.types[typeToString(static_cast<PokemonTypes>(i))].loadFromFile("assets/types/type_" + toLower(typeToString(static_cast<PokemonTypes>(i))) + ".png");
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

void gui(const std::string &trainerName, bool ai)
{
	std::vector<std::string> battleLog;
	PokemonGen1::BattleAction nextAction = PokemonGen1::NoAction;
	PokemonGen1::GameHandle handler(
		[](const ByteHandle &byteHandle, const LoopHandle &loopHandler, const std::string &ip, unsigned short port)
		{
			return new BGBHandler(byteHandle, byteHandle, loopHandler, ip, port, getenv("MAX_DEBUG"));
		},
		[&nextAction](PokemonGen1::GameHandle &handle) {
			PokemonGen1::BattleAction action = nextAction;
			const PokemonGen1::BattleState &state = handle.getBattleState();

			if (!state.opponentTeam[state.opponentPokemonOnField].getHealth())
				return static_cast<PokemonGen1::BattleAction>(PokemonGen1::Switch1 + state.pokemonOnField);

			if (action)
				nextAction = PokemonGen1::NoAction;
			return action;
		},
		trainerName,
		[&battleLog](const std::string &msg){ battleLog.push_back(msg); },
		false,
		getenv("MIN_DEBUG")
	);
	sf::RenderWindow window{{800, 640}, trainerName};
	BattleResources resources;

	loadResources(resources);
	handler.setTeamSize(6);
	for (int i = 0; i < 6; i++)
		handler.changePokemon(
			i,
			handler.getPokemonTeam()[i].getNickname(),
			100,
			PokemonGen1::pokemonList.at(handler.getPokemonTeam()[i].getID()),
			std::vector<PokemonGen1::Move>(handler.getPokemonTeam()[i].getMoveSet())
		);
	window.setFramerateLimit(60);
	handler.setReady(false);
	while (window.isOpen()) {
		if (handler.getStage() != PokemonGen1::BATTLE)
			mainMenu(window, handler, resources);
		else
			battle(window, handler, resources, battleLog, nextAction);
	}
}