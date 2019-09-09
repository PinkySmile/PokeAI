//
// Created by Gegel85 on 30/08/2019.
//

#include <TGUI/TGUI.hpp>
#include <SFML/Audio.hpp>
#include <utility>
#include "gui.hpp"
#include "GameHandle.hpp"
#include "BgbHandler.hpp"

struct BattleRessources {
	sf::Music				loop;
	sf::Music				start;
	sf::Font				font;
	sf::Texture				categories[3];
	std::map<std::string, sf::Texture>	types;
	sf::Texture				balls[4];
	sf::Texture				pokemonsBack[256];
	sf::Texture				pokemonsFront[256];
	sf::Texture				trainer[2][2];
	sf::SoundBuffer				hitSounds[3];
	sf::SoundBuffer				trainerLand;
	sf::SoundBuffer				battleCries[256];
	sf::Texture				boxes[3];
	sf::Texture				arrow;
};

std::string intToHex(unsigned char i)
{
	std::stringstream stream;
	stream << std::setfill ('0') << std::setw(2) << std::hex << std::uppercase << static_cast<int>(i);
	return stream.str();
}

tgui::Button::Ptr makeButton(const std::string &content, tgui::Layout x, tgui::Layout y, const std::function<void (tgui::Button::Ptr button)> &handler, int width = 0, int height = 0)
{
	tgui::Button::Ptr button = tgui::Button::create(content);

	button->setPosition(std::move(x), std::move(y));
	if (handler)
		button->connect("pressed", handler, button);
	if (height && width)
		button->setSize(width, height);
	return button;
}

tgui::TextBox::Ptr makeTextBox(tgui::Layout x, tgui::Layout y, tgui::Layout width, tgui::Layout height, const std::string &placeholder = "")
{
	tgui::TextBox::Ptr box = tgui::TextBox::create();

	box->setPosition(std::move(x), std::move(y));
	box->setSize({width, height});
	box->setText(placeholder);
	box->setReadOnly(true);
	box->setEnabled(false);
	return box;
}

tgui::TextBox::Ptr makeTypeBox(tgui::Layout x, tgui::Layout y, tgui::Layout width, tgui::Layout height, const std::string &placeholder = "")
{
	tgui::TextBox::Ptr box = tgui::TextBox::create();

	box->setPosition(std::move(x), std::move(y));
	box->setSize({std::move(width), std::move(height)});
	box->setText(placeholder);
	box->setReadOnly(false);
	return box;
}

tgui::Picture::Ptr makePicture(const tgui::Texture &texture, tgui::Layout x, tgui::Layout y, tgui::Layout width, tgui::Layout height)
{
	tgui::Picture::Ptr pic = tgui::Picture::create(texture);

	pic->setPosition(std::move(x), std::move(y));
	pic->setSize({std::move(width), std::move(height)});
	pic->ignoreMouseEvents();
	return pic;
}

std::string toLower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
	return str;
}

void updatePokemonTeam(tgui::Gui &gui, PokemonGen1::GameHandle &game, BattleRessources &ressources)
{
	for (unsigned i = 0; i < game.getPokemonTeam().size(); i++) {
		auto &pkmn = game.getPokemonTeam()[i];

		gui.get("pkmnFrontButton" + std::to_string(i))->setVisible(true);
		gui.remove(gui.get("pkmnFront" + std::to_string(i)));
		gui.add(makePicture(ressources.pokemonsFront[pkmn.getID()], 300 + (i % 3) * 150, 50 + (i / 3) * 300, 96, 96), "pkmnFront" + std::to_string(i));
		gui.remove(gui.get("type1" + std::to_string(i)));
		gui.add(makePicture(ressources.types[typeToString(pkmn.getTypes().first)], 300 + (i % 3) * 150, 150 + (i / 3) * 300, 48, 16), "type1" + std::to_string(i));
		if (pkmn.getTypes().first != pkmn.getTypes().second) {
			gui.remove(gui.get("type2" + std::to_string(i)));
			gui.add(makePicture(ressources.types[typeToString(pkmn.getTypes().second)], 348 + (i % 3) * 150, 150 + (i / 3) * 300, 48, 16), "type2" + std::to_string(i));
		} else
			gui.get("type2" + std::to_string(i))->setVisible(false);
		gui.get("name" + std::to_string(i))->setVisible(true);
		gui.get<tgui::TextBox>("name" + std::to_string(i))->setText(pkmn.getSpeciesName());
		gui.get("nickname" + std::to_string(i))->setVisible(true);
		gui.get<tgui::TextBox>("nickname" + std::to_string(i))->setText(pkmn.getNickname());
		for (int j = 0; j < 4; j++) {
			gui.get("move" + std::to_string(j) + std::to_string(i))->setVisible(true);
			gui.get<tgui::Button>("move" + std::to_string(j) + std::to_string(i))->setText(game.getPokemonTeam()[i].getMoveSet()[j].getName());
		}
	}

	for (unsigned i = game.getPokemonTeam().size(); i < 6; i++) {
		gui.get("pkmnFrontButton" + std::to_string(i))->setVisible(false);
		gui.get("pkmnFront" + std::to_string(i))->setVisible(false);
		gui.get("type1" + std::to_string(i))->setVisible(false);
		gui.get("type2" + std::to_string(i))->setVisible(false);
		gui.get("name" + std::to_string(i))->setVisible(false);
		gui.get("nickname" + std::to_string(i))->setVisible(false);
		for (unsigned j = 0; j < 4; j++)
			gui.get("move" + std::to_string(j) + std::to_string(i))->setVisible(false);
	}
}

void makeMainMenuGUI(tgui::Gui &gui, tgui::Gui &selectPkmnMenu, tgui::Gui &selectMovePanel, PokemonGen1::GameHandle &game, unsigned &id, unsigned &menu, BattleRessources &ressources)
{
	gui.removeAllWidgets();
	selectMovePanel.removeAllWidgets();
	selectPkmnMenu.removeAllWidgets();
	gui.add(makeTypeBox(10, 10, 200, 25, "address"), "ip");
	gui.add(makeTypeBox(10, 40, 200, 25, "port"), "port");
	gui.add(makeButton(game.isConnected() ? "Disconnect" : "Connect", 10, 70, [&game, &gui](tgui::Button::Ptr button){
		if (game.isConnected()) {
			gui.get<tgui::TextBox>("ip")->setEnabled(true);
			gui.get<tgui::TextBox>("port")->setEnabled(true);
			try {
				game.disconnect();
				gui.get<tgui::TextBox>("errorBox")->setText("Disconnected");
			} catch (std::exception &e) {
				gui.get<tgui::TextBox>("errorBox")->setText(e.what());
			}
			button->setText("Connect");
			return;
		}
		try {
			unsigned long port = std::stoul(gui.get<tgui::TextBox>("port")->getText().toAnsiString());

			if (port > 65535)
				throw std::out_of_range("out_of_range");

			game.connect(gui.get<tgui::TextBox>("ip")->getText(), port);
			button->setText("Disconnect");
			gui.get<tgui::TextBox>("ip")->setEnabled(false);
			gui.get<tgui::TextBox>("port")->setEnabled(false);
			gui.get<tgui::TextBox>("errorBox")->setText("Connected to " + gui.get<tgui::TextBox>("ip")->getText());
		} catch (std::out_of_range &) {
			gui.get<tgui::TextBox>("errorBox")->setText("Error: invalid port number");
		} catch (std::invalid_argument &) {
			gui.get<tgui::TextBox>("errorBox")->setText("Error: invalid port number");
		} catch (std::exception &e) {
			gui.get<tgui::TextBox>("errorBox")->setText(e.what());
		}
	}), "connect");
	gui.add(makeTextBox(10, 100, 200, 100), "errorBox");
	gui.add(makeTextBox(10, 580, 200, 50, "Disconnected"), "status");

	tgui::Slider::Ptr slider = tgui::Slider::create();
	slider->setPosition(300, 10);
	slider->setSize(200, 18);
	slider->setValue(6);
	slider->setMaximum(6);
	slider->setMinimum(1);
	slider->connect("ValueChanged", [&game, &gui, &ressources](tgui::Slider::Ptr slider) {
		slider->setValue(static_cast<int>(slider->getValue()));
		if (slider->getValue() != game.getPokemonTeam().size()) {
			game.setTeamSize(slider->getValue());
			updatePokemonTeam(gui, game, ressources);
		}
	}, slider);
	gui.add(slider);

	for (int i = 0; i < 6; i++) {
		gui.add(makeButton("", 300 + (i % 3) * 150, 50 + (i / 3) * 300, [&menu, &gui, &id](tgui::Button::Ptr button){
			id = std::stol(gui.getWidgetName(button).substr(15));
			menu = 1;
		}, 96, 96), "pkmnFrontButton" + std::to_string(i));
		gui.add(makePicture(ressources.pokemonsFront[1], 300 + (i % 3) * 150, 50 + (i / 3) * 300, 96, 96), "pkmnFront" + std::to_string(i));
		gui.add(makePicture(ressources.types["ground"], 300 + (i % 3) * 150, 150 + (i / 3) * 300, 48, 16), "type1" + std::to_string(i));
		gui.add(makePicture(ressources.types["rock"], 348 + (i % 3) * 150, 150 + (i / 3) * 300, 48, 16), "type2" + std::to_string(i));
		gui.add(makeTextBox(300 + (i % 3) * 150, 170 + (i / 3) * 300, 96, 20, "RHYDON"), "name" + std::to_string(i));
		gui.add(makeTypeBox(300 + (i % 3) * 150, 190 + (i / 3) * 300, 96, 20, ""), "nickname" + std::to_string(i));
		for (int j = 0; j < 4; j++)
			gui.add(makeButton("--", 300 + (i % 3) * 150, 210 + (i / 3) * 300 + 20 * j, [&id, &gui, &menu](tgui::Button::Ptr button) {
				id = std::stol(gui.getWidgetName(button).substr(4));
				menu = 2;
			}, 96, 20), "move" + std::to_string(j) + std::to_string(i));
		gui.get("nickname" + std::to_string(i))->connect("TextChanged", [&gui, &game](tgui::TextBox::Ptr box){
			if (box->getText().getSize() > 10) {
				box->setText(box->getText().substring(0, 10));
				return;
			}

			unsigned id = std::stol(gui.getWidgetName(box).substr(8));

			game.changePokemon(
				id,
				box->getText(),
				game.getPokemonTeam()[id].getLevel(),
				PokemonGen1::pokemonList.at(game.getPokemonTeam()[id].getID()),
				std::vector<PokemonGen1::Move>(game.getPokemonTeam()[id].getMoveSet())
			);
		}, gui.get<tgui::TextBox>("nickname" + std::to_string(i)));
	}

	gui.add(makeButton("You are not ready", 10, 550, [&game, &gui, slider](tgui::Button::Ptr){
		slider->setEnabled(game.isReady());
		for (int i = 0; i < 6; i++) {
			gui.get("pkmnFrontButton" + std::to_string(i))->setEnabled(game.isReady());
			for (int j = 0; j < 4; j++)
				gui.get("move" + std::to_string(j) + std::to_string(i))->setEnabled(game.isReady());
		}
		game.setReady(!game.isReady());
	}), "ready");


	//Pokemon select
	tgui::Scrollbar::Ptr scrollbar = tgui::Scrollbar::create();
	scrollbar->setPosition(780, 10);
	scrollbar->setSize(18, 620);
	scrollbar->setMaximum(6384);
	scrollbar->setViewportSize(640);
	scrollbar->connect("ValueChanged", [&selectPkmnMenu](tgui::Scrollbar::Ptr scrollbar){
		for (int i = 0; i < 256; i++) {
			selectPkmnMenu.get("pkmnSelect" + std::to_string(i))->setPosition(10 + (i % 6) * 128, 10 + (i / 6) * 148 - scrollbar->getValue());
			selectPkmnMenu.get("pkmnFrontSelect" + std::to_string(i))->setPosition(11 + (i % 6) * 128, 11 + (i / 6) * 148 - scrollbar->getValue());
			selectPkmnMenu.get("pkmnNameSelect" + std::to_string(i))->setPosition(11 + (i % 6) * 128, 138 + (i / 6) * 148 - scrollbar->getValue());
		}
	}, scrollbar);
	selectPkmnMenu.add(scrollbar, "scrollbar");

	for (int i = 0; i < 256; i++) {
		selectPkmnMenu.add(makeButton("", 10 + (i % 6) * 128, 10 + (i / 6) * 148, [&menu, i, &gui, &ressources, &game, &id](tgui::Button::Ptr){
			game.changePokemon(
				id,
				game.getPokemonTeam()[id].getNickname(),
				game.getPokemonTeam()[id].getLevel(),
				PokemonGen1::pokemonList.at(i),
				std::vector<PokemonGen1::Move>(game.getPokemonTeam()[id].getMoveSet())
			);
			updatePokemonTeam(gui, game, ressources);
			menu = 0;
		}, 128, 128), "pkmnSelect" + std::to_string(i));
		selectPkmnMenu.add(makePicture(ressources.pokemonsFront[i], 11 + (i % 6) * 128, 11 + (i / 6) * 148, 126, 126), "pkmnFrontSelect" + std::to_string(i));
		selectPkmnMenu.add(makeTextBox(10 + (i % 6) * 128, 138 + (i / 6) * 148, 128, 20, intToHex(i) +  " " + PokemonGen1::pokemonList[i].name), "pkmnNameSelect" + std::to_string(i));
	}


	//Move select
	tgui::Scrollbar::Ptr scrollbar2 = tgui::Scrollbar::create();
	scrollbar2->setPosition(780, 10);
	scrollbar2->setSize(18, 620);
	scrollbar2->setMaximum(12748);
	scrollbar2->setViewportSize(640);
	scrollbar2->connect("ValueChanged", [&selectMovePanel](tgui::Scrollbar::Ptr scrollbar){
		for (int i = 0; i < 256; i++) {
			selectMovePanel.get("moveSelect" + std::to_string(i))->setPosition(10 + (i % 3) * 256, 10 + (i / 3) * 148 - scrollbar->getValue());
			selectMovePanel.get("moveSelectButton" + std::to_string(i))->setPosition(10 + (i % 3) * 256, 10 + (i / 3) * 148 - scrollbar->getValue());
			selectMovePanel.get("moveSelectCategoryPicture" + std::to_string(i))->setPosition(11 + (i % 3) * 256, 26 + (i / 3) * 148 - scrollbar->getValue());
			selectMovePanel.get("moveSelectTypePicture" + std::to_string(i))->setPosition(217 + (i % 3) * 256, 11 + (i / 3) * 148 - scrollbar->getValue());
		}
	}, scrollbar2);
	selectMovePanel.add(scrollbar2, "scrollbar2");

	for (int i = 0; i < 256; i++) {
		const auto &move = PokemonGen1::availableMoves.at(i);
		std::stringstream stream;

		stream << std::endl;
		switch (move.getCategory()) {
		case PokemonGen1::PHYSICAL:
			stream << "        Physical" << std::endl;
			break;
		case PokemonGen1::SPECIAL:
			stream << "        Special" << std::endl;
			break;
		case PokemonGen1::STATUS:
			stream << "        Status" << std::endl;
			break;
		}
		stream << "Power:      " << (move.getPower() ? std::to_string(move.getPower()) : "--") << std::endl;
		stream << "Accuracy: " << (move.getAccuracy() <= 100 ? std::to_string(move.getAccuracy()) : "--") << std::endl;
		stream << static_cast<int>(move.getPP()) << "/" << static_cast<int>(move.getMaxPP()) << " PP" << std::endl;

		selectMovePanel.add(makeTextBox(10 + (i % 3) * 256, 10 + (i / 3) * 148, 256, 128, stream.str()), "moveSelect" + std::to_string(i));
		selectMovePanel.add(makeButton(move.getName(), 10 + (i % 3) * 256, 10 + (i / 3) * 148, [&menu, i, &gui, &ressources, &game, &id](tgui::Button::Ptr) {
			std::vector<PokemonGen1::Move> moves = game.getPokemonTeam()[id % 10].getMoveSet();

			moves[id / 10] = PokemonGen1::availableMoves.at(i);

			game.changePokemon(
				id % 10,
				game.getPokemonTeam()[id % 10].getNickname(),
				game.getPokemonTeam()[id % 10].getLevel(),
				PokemonGen1::pokemonList.at(game.getPokemonTeam()[id % 10].getID()),
				moves
			);
			updatePokemonTeam(gui, game, ressources);
			menu = 0;
		}, 100, 16), "moveSelectButton" + std::to_string(i));
		selectMovePanel.add(makePicture(ressources.types[typeToString(move.getType())], 217 + (i % 3) * 256, 11 + (i / 3) * 148, 48, 16), "moveSelectTypePicture" + std::to_string(i));
		selectMovePanel.add(makePicture(ressources.categories[move.getCategory()], 11 + (i % 3) * 256, 26 + (i / 3) * 148, 32, 14), "moveSelectCategoryPicture" + std::to_string(i));
	}
	updatePokemonTeam(gui, game, ressources);
}

void mainMenu(sf::RenderWindow &window, PokemonGen1::GameHandle &game, const std::string &trainerName, BattleRessources &ressources)
{
	unsigned id = 0;
	unsigned menu = 0;
	tgui::Gui gui{window};
	tgui::Gui panel = tgui::Gui(window);
	tgui::Gui panel2 = tgui::Gui(window);

	window.setSize({800, 640});
	makeMainMenuGUI(gui, panel, panel2, game, id, menu, ressources);
	window.setTitle(trainerName + " - Main menu");
	while (window.isOpen() && game.getStage() != PokemonGen1::BATTLE) {
		sf::Event event;

		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			switch (menu) {
			case 0:
				gui.handleEvent(event);
				break;
			case 1:
				panel.handleEvent(event);
				break;
			default:
				panel2.handleEvent(event);
			}
		}

		gui.get<tgui::Button>("ready")->setText(game.isReady() ? "You are ready for battle" : "You are not ready");
		if (game.isConnected())
			switch (game.getStage()) {
			case PokemonGen1::PKMN_CENTER:
				gui.get<tgui::TextBox>("status")->setText("Opponent not ready");
				break;
			case PokemonGen1::PINGING_OPPONENT:
				gui.get<tgui::TextBox>("status")->setText("Waiting for opponent to save the game");
                                break;
			case PokemonGen1::ROOM_CHOOSE:
				gui.get<tgui::TextBox>("status")->setText("Choosing colosseum");
				break;
			case PokemonGen1::PING_POKEMON_EXCHANGE:
				gui.get<tgui::TextBox>("status")->setText(game.isReady() ? "Waiting for opponent to start the game" : "Waiting for you to be ready");
				break;
			case PokemonGen1::EXCHANGE_POKEMONS:
				gui.get<tgui::TextBox>("status")->setText("Exchanging battle data");
				break;
			case PokemonGen1::BATTLE:
				gui.get<tgui::TextBox>("status")->setText("In battle");
				break;
			}
		else
			gui.get<tgui::TextBox>("status")->setText("Disconnected");
		window.clear();
		switch (menu) {
		case 0:
			gui.draw();
			break;
		case 1:
			panel.draw();
			break;
		default:
			panel2.draw();
		}
		window.display();
	}
}

void drawSprite(sf::RenderWindow &window, sf::Sprite &sprite, sf::Texture &texture, int x, int y, int width = 0, int height = 0)
{
	sprite.setTexture(texture, true);
	sprite.setPosition(x, y);

	sprite.setScale(
		!width ?  1 : static_cast<float>(width) /  texture.getSize().x,
		!height ? 1 : static_cast<float>(height) / texture.getSize().y
	);
	window.draw(sprite);
}

void playSound(sf::Sound &sound, sf::SoundBuffer &buffer)
{
	sound.setBuffer(buffer);
	sound.play();
}

void drawText(sf::RenderWindow &window, sf::Text &text, const std::string &str, int x, int y)
{
	text.setString(str);
	text.setPosition(x, y);
	window.draw(text);
}

void executeBattleStartAnimation(sf::RenderWindow &window, PokemonGen1::GameHandle &game, const std::string &trainerName, BattleRessources &ressources, std::vector<std::string> &log, const PokemonGen1::BattleState &state)
{
	float		last = 0;
	float		seconds;
	sf::View	view{{320, 288}, {640, 576}};
	sf::Clock	clock;
	sf::Text	text;
	sf::Sprite	sprite;
	sf::Sound	sound;
	sf::Event	event;

	window.setView(view);
	text.setFont(ressources.font);
	text.setCharacterSize(32);
	text.setFillColor({39, 39, 39, 255});
	text.setLineSpacing(2);
	window.setTitle(trainerName + " - Challenging " + game.getBattleState().opponentName);
	ressources.start.play();
	ressources.loop.setLoop(true);
	while (window.isOpen() && clock.getElapsedTime().asSeconds() < 20) {
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear({255, 255, 255, 255});

		seconds = clock.getElapsedTime().asSeconds();

		drawSprite(window, sprite, ressources.boxes[0], 0, 384);
		if (seconds < 4.5) {
			window.clear({255, 255, 255, 255});
			drawSprite(window, sprite, ressources.boxes[1], 96, 144);
			drawText(window, text, trainerName, 144, 192);
			for (unsigned i = 0; i < state.team.size(); i++)
				drawSprite(window, sprite, ressources.balls[
					state.team[i].hasStatus(static_cast<PokemonGen1::StatusChange>(0xFF)) ? 3 : (state.team[i].getHealth() ? 0 : 2)
				], 320 + 32 * i, 224);

			drawText(window, text, state.opponentName, 144, 320);
			for (unsigned i = 0; i < state.opponentTeam.size(); i++)
				drawSprite(window, sprite, ressources.balls[
					state.opponentTeam[i].hasStatus(static_cast<PokemonGen1::StatusChange>(0xFF)) ? 3 : (state.opponentTeam[i].getHealth() ? 0 : 2)
				], 320 + 32 * i, 352);

		} else if (seconds < 6) {
			drawSprite(window, sprite, ressources.trainer[0][1], -224 + 632 * (seconds - 4.5) / 1.5, 0);
			drawSprite(window, sprite, ressources.trainer[0][0], 640 * (6 - seconds) / 1.5, 160);

		} else if (seconds < 7) {
			if (last <= 6)
				playSound(sound, ressources.trainerLand);
			drawSprite(window, sprite, ressources.trainer[1][1], 408, 0);
			drawSprite(window, sprite, ressources.trainer[1][0], 8, 160);

		} else if (seconds < 10) {
			drawSprite(window, sprite, ressources.trainer[1][1], 408, 0);
			drawSprite(window, sprite, ressources.trainer[1][0], 8, 160);

			drawSprite(window, sprite, ressources.boxes[2], 32, 64);
			for (int i = 6; i-- > 0; )
				drawSprite(window, sprite, ressources.balls[
					static_cast<unsigned>(i) >= state.opponentTeam.size() ? 1 : (state.opponentTeam[i].hasStatus(static_cast<PokemonGen1::StatusChange>(0xFF)) ? 3 : (state.opponentTeam[i].getHealth() ? 0 : 2))
					], 64 + 32 * (6 - i), 64);

			drawSprite(window, sprite, ressources.boxes[2], 608, 320, -ressources.boxes[2].getSize().x);
			for (unsigned i = 0; i < 6; i++)
				drawSprite(window, sprite, ressources.balls[
					i >= state.team.size() ? 1 : (state.team[i].hasStatus(static_cast<PokemonGen1::StatusChange>(0xFF)) ? 3 : (state.team[i].getHealth() ? 0 : 2))
					], 352 + 32 * i, 318);

			drawText(window, text, log[0].substr(0, (seconds - 7) * 15), 32, 440);

		} else if (seconds < 10.25) {
			if (last <= 10)
				log.erase(log.begin());

			drawSprite(window, sprite, ressources.trainer[1][1], 408 + 896 * (seconds - 10), 0);
			drawSprite(window, sprite, ressources.trainer[1][0], 8, 160);

		} else if (seconds < 10.5) {
			drawSprite(window, sprite, ressources.trainer[1][0], 8, 160);

		} else if (seconds < 13) {
			drawSprite(window, sprite, ressources.trainer[1][0], 8, 160);
			drawText(window, text, log[0].substr(0, (seconds - 10.5) * 15), 32, 440);

		} else if (seconds < 13.2) {
			auto &texture = ressources.pokemonsFront[state.opponentTeam[state.opponentPokemonOnField].getID()];

			drawSprite(window, sprite, texture,
				408 + (texture.getSize().x - texture.getSize().x * 5 * (seconds - 13)) / 2,
				0 + texture.getSize().y - texture.getSize().y * 5 * (seconds - 13),
				texture.getSize().x * 5 * (seconds - 13),
				texture.getSize().y * 5 * (seconds - 13)
			);
			drawSprite(window, sprite, ressources.trainer[1][0], 8, 160);
			drawText(window, text, log[0], 32, 440);

		} else if (seconds < 14) {
			drawSprite(window, sprite, ressources.pokemonsFront[state.opponentTeam[state.opponentPokemonOnField].getID()], 408, 0);
			drawSprite(window, sprite, ressources.trainer[1][0], 8, 160);
			drawText(window, text, log[0], 32, 440);
		} else if (seconds < 14.2) {
			drawSprite(window, sprite, ressources.pokemonsFront[state.opponentTeam[state.opponentPokemonOnField].getID()], 408, 0);
			drawSprite(window, sprite, ressources.trainer[1][0], 8 - 256 * (seconds - 14) * 5, 160);
			drawText(window, text, log[0], 32, 440);
			//displayOpponentStats(window, text, sprite, ressources, state.opponentTeam[state.opponentPokemonOnField]);
		} else if (seconds < 14.5) {
			if (last <= 14.2)
				log.erase(log.begin());
			drawSprite(window, sprite, ressources.pokemonsFront[state.opponentTeam[state.opponentPokemonOnField].getID()], 408, 0);
			drawText(window, text, log[0].substr(0, (seconds - 14.2) * 15), 32, 440);
		} else {
			drawSprite(window, sprite, ressources.pokemonsFront[state.opponentTeam[state.opponentPokemonOnField].getID()], 408, 0);
			drawText(window, text, log[0].substr(0, (seconds - 14.2) * 15), 32, 440);
		}

		last = seconds;
		if (seconds + 1. / 60 >= ressources.start.getDuration().asSeconds() && ressources.loop.getStatus() != sf::Music::Playing)
			ressources.loop.play();

		window.display();
	}
}

void battle(sf::RenderWindow &window, PokemonGen1::GameHandle &game, const std::string &trainerName, BattleRessources &ressources, std::vector<std::string> &log, PokemonGen1::BattleAction &nextAction)
{
	sf::RectangleShape rect;
	sf::Sprite	sprite;
	sf::Sound	sound;
	sf::Text	text;
	sf::Clock	clock;
	unsigned char	selectedMenu = 0;
	sf::View	view{{320, 288}, {640, 576}};
	PokemonGen1::BattleState state;

	log.emplace_back("Ash\nwants to fight!");
	log.emplace_back("Ash sent\nout RHYDON!");
	log.emplace_back("RHYDON,go!");
	game.setTeamSize(5);
	state.opponentName = "Ash";
	state.opponentPokemonOnField = 0;
	for (const auto &o : game.getPokemonTeam())
		state.team.push_back(o);
	state.opponentTeam.push_back(state.team[0]);
	window.setSize({640, 576});
	window.setView(view);
	text.setFont(ressources.font);
	text.setCharacterSize(32);
	text.setFillColor({39, 39, 39, 255});
	text.setLineSpacing(2);
	window.setTitle(trainerName + " - Challenging " + game.getBattleState().opponentName);
	ressources.start.play();
	ressources.loop.setLoop(true);
	executeBattleStartAnimation(window, game, trainerName, ressources, log, state);
	clock.restart();
	while (window.isOpen()){//} && game.getStage() == PokemonGen1::BATTLE) {
		sf::Event event;
		//const auto &state = game.getBattleState();

		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear({255, 255, 255, 255});


		window.display();
	}
	ressources.start.stop();
	ressources.loop.stop();

	view.setSize(window.getSize().x, window.getSize().y);
	view.setCenter(window.getSize().x / 2., window.getSize().y / 2.);
	window.setView(view);
}

void loadRessources(BattleRessources &ressources)
{
	ressources.start.openFromFile("assets/sounds/battle_intro.wav");
	ressources.loop.openFromFile("assets/sounds/battle_loop.wav");

	ressources.categories[0].loadFromFile("assets/move_categories/physical.png");
	ressources.categories[1].loadFromFile("assets/move_categories/special.png");
	ressources.categories[2].loadFromFile("assets/move_categories/status.png");

	for (int i = 0; i < 256; i++)
		if (!ressources.pokemonsBack[i].loadFromFile("assets/back_sprites/" + std::to_string(i) + "_back.png"))
			ressources.pokemonsBack[i].loadFromFile("assets/back_sprites/missingno_back.png");

	for (int i = 0; i < 256; i++)
		if (!ressources.pokemonsFront[i].loadFromFile("assets/front_sprites/" + std::to_string(i) + "_front.png"))
			ressources.pokemonsFront[i].loadFromFile("assets/front_sprites/missingno_front.png");

	ressources.balls[0].loadFromFile("assets/pokeballs/pkmnOK.png");
	ressources.balls[1].loadFromFile("assets/pokeballs/pkmnNO.png");
	ressources.balls[2].loadFromFile("assets/pokeballs/pkmnFNT.png");
	ressources.balls[3].loadFromFile("assets/pokeballs/pkmnSTATUS.png");

	ressources.font.loadFromFile("assets/font.ttf");
	ressources.hitSounds[0].loadFromFile("assets/sounds/not_effective_hit_sound.wav");
	ressources.hitSounds[1].loadFromFile("assets/sounds/hit_sound.wav");
	ressources.hitSounds[2].loadFromFile("assets/sounds/very_effective_hit_sound.wav");

	for (int i = 0; i <= TYPE_DRAGON; i++)
		try {
			ressources.types.at(typeToString(static_cast<PokemonTypes>(i)));
		} catch (std::out_of_range &) {
			ressources.types[typeToString(static_cast<PokemonTypes>(i))].loadFromFile("assets/types/type_" + toLower(typeToString(static_cast<PokemonTypes>(i))) + ".png");
		}

	ressources.trainer[0][0].loadFromFile("assets/back_sprites/trainer_shadow_back.png");
	ressources.trainer[0][1].loadFromFile("assets/front_sprites/trainer_shadow_front.png");
	ressources.trainer[1][0].loadFromFile("assets/back_sprites/trainer_back.png");
	ressources.trainer[1][1].loadFromFile("assets/front_sprites/trainer_front.png");

	ressources.trainerLand.loadFromFile("assets/sounds/trainer_land.wav");
	ressources.arrow.loadFromFile("assets/arrow.png");

	ressources.boxes[0].loadFromFile("assets/text_box.png");
	ressources.boxes[1].loadFromFile("assets/VS_box.png");
	ressources.boxes[2].loadFromFile("assets/pkmns_border.png");

	for (int i = 0; i < 256; i++)
		ressources.battleCries[i].loadFromFile("assets/cries/" + std::to_string(i) + "_cry.wav");
}

void gui(const std::string &trainerName)
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
	BattleRessources ressources;

	loadRessources(ressources);
	handler.setTeamSize(6);
	window.setFramerateLimit(60);
	handler.setReady(false);
	while (window.isOpen()) {
		//mainMenu(window, handler, trainerName, ressources);
		battle(window, handler, trainerName, ressources, battleLog, nextAction);
	}
}