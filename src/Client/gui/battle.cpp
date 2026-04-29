//
// Created by PinkySmile on 17/08/2020.
//

#include <SFML/Graphics.hpp>
#include <ctime>
#include <cstdio>
#include <filesystem>
#include "gui.hpp"
#include "../AIs/AI.hpp"
#include "../AIs/AIFactory.hpp"
#include "Renderers/IRenderer.hpp"

using namespace PokemonGen1;

namespace
{
	enum BattleMenu {
		MENU_ROOT,
		MENU_MOVES,
		MENU_SWITCH
	};

	bool playerActionNeeded(const BattleHandler &game, const PkmnRenderer::IRenderer &renderer, const AI *ai, const BattleState &state)
	{
		return !ai && !game.playingReplay() && !game.isFinished() && renderer.hasAnimationEnded() && state.me.nextAction == EmptyAction;
	}

	void drawActionMenu(sf::RenderTarget &target, sf::Font &font, BattleMenu menu, unsigned char selectedMenu, const PlayerState &me)
	{
		static const char *const rootItems[4] = {"FIGHT", "PKMN", "ITEM", "RUN"};
		sf::RectangleShape background{{158, 46}};
		sf::Text text{font};

		background.setFillColor({255, 255, 255, 255});
		background.setOutlineColor({39, 39, 39, 255});
		background.setOutlineThickness(1);
		background.setPosition({1, 97});
		target.draw(background);

		text.setCharacterSize(8);
		text.setFillColor({39, 39, 39, 255});
		text.setLineSpacing(1);

		if (menu == MENU_ROOT) {
			for (int i = 0; i < 4; i++) {
				text.setString(std::string(i == selectedMenu ? ">" : " ") + rootItems[i]);
				text.setPosition({8.f + 76 * (i % 2), 104.f + 16 * (i / 2)});
				target.draw(text);
			}
		} else if (menu == MENU_MOVES) {
			auto &moveSet = me.team[me.pokemonOnField].getMoveSet();

			for (size_t i = 0; i < moveSet.size(); i++) {
				auto &move = moveSet[i];
				std::string label = (i == selectedMenu ? ">" : " ") + strToUpper(move.getName());

				text.setString(label);
				text.setPosition({4, 100.f + i * 10});
				target.draw(text);
			}
			if (selectedMenu < moveSet.size()) {
				auto &move = moveSet[selectedMenu];

				text.setString(std::to_string(move.getPP()) + "/" + std::to_string(move.getMaxPP()));
				text.setPosition({100, 120});
				target.draw(text);
			}
		} else if (menu == MENU_SWITCH) {
			for (size_t i = 0; i < me.team.size(); i++) {
				auto &pkmn = me.team[i];
				std::string label = (i == selectedMenu ? ">" : " ") + pkmn.getName(false);

				text.setString(label);
				text.setPosition({4, 98.f + i * 8});
				target.draw(text);
			}
		}
	}

	BattleAction firstUsableMove(const Pokemon &pkmn)
	{
		auto &moveSet = pkmn.getMoveSet();

		for (size_t i = 0; i < moveSet.size(); i++)
			if (moveSet[i].getPP() != 0 && i + 1 != pkmn.getMoveDisabled())
				return static_cast<BattleAction>(Attack1 + i);
		return StruggleMove;
	}

	void handleMenuNavigation(const sf::Event::KeyPressed &key, BattleMenu &menu, unsigned char &selectedMenu, BattleState &state)
	{
		auto &me = state.me;
		auto &activePkmn = me.team[me.pokemonOnField];

		if (key.code == sf::Keyboard::Key::Up || key.code == sf::Keyboard::Key::Down) {
			bool down = key.code == sf::Keyboard::Key::Down;

			if (menu == MENU_ROOT) {
				selectedMenu = (selectedMenu + 2) % 4;
			} else if (menu == MENU_MOVES) {
				auto &moveSet = activePkmn.getMoveSet();

				selectedMenu = (selectedMenu + (down ? 1 : 3)) % 4;
				if (selectedMenu >= moveSet.size() || moveSet[selectedMenu].getID() == 0)
					selectedMenu = 0;
			} else if (menu == MENU_SWITCH) {
				unsigned size = me.team.size();

				selectedMenu = (selectedMenu + (down ? 1 : size - 1)) % size;
			}
		} else if (key.code == sf::Keyboard::Key::Right || key.code == sf::Keyboard::Key::Left) {
			if (menu == MENU_ROOT)
				selectedMenu = (selectedMenu + 1) % 2 + (selectedMenu / 2) * 2;
		} else if (key.code == sf::Keyboard::Key::W || key.code == sf::Keyboard::Key::Enter) {
			if (menu == MENU_ROOT) {
				if (selectedMenu == 0) {
					auto action = firstUsableMove(activePkmn);

					if (action == StruggleMove)
						me.nextAction = StruggleMove;
					else {
						menu = MENU_MOVES;
						selectedMenu = 0;
					}
				} else if (selectedMenu == 1) {
					menu = MENU_SWITCH;
					selectedMenu = 0;
				} else if (selectedMenu == 3) {
					me.nextAction = Run;
				}
			} else if (menu == MENU_MOVES) {
				auto &moveSet = activePkmn.getMoveSet();

				if (selectedMenu < moveSet.size() && moveSet[selectedMenu].getPP() != 0)
					me.nextAction = static_cast<BattleAction>(Attack1 + selectedMenu);
			} else if (menu == MENU_SWITCH) {
				if (selectedMenu < me.team.size() && selectedMenu != me.pokemonOnField)
					me.nextAction = static_cast<BattleAction>(Switch1 + selectedMenu);
			}
		} else if (key.code == sf::Keyboard::Key::X) {
			if (menu == MENU_MOVES || menu == MENU_SWITCH) {
				menu = MENU_ROOT;
				selectedMenu = 0;
			}
		}
	}

	void saveBattleReplay(BattleHandler &game)
	{
		char buf[4096];
		time_t timer;
		char timebuffer[40];
		struct tm *tm_info;

		time(&timer);
		tm_info = localtime(&timer);
		strftime(timebuffer, 40, "%Y-%m-%d_%H_%M_%S", tm_info);
		snprintf(buf, sizeof(buf), "replays/%s.replay", timebuffer);
		std::filesystem::create_directory("replays");
		game.saveReplay(buf);
	}
}

bool battle(sf::RenderWindow &window, BattleHandler &game, PkmnRenderer::IRenderer &renderer, std::pair<unsigned char, unsigned char> aiNb, bool updateManually)
{
	auto ai1 = std::unique_ptr<AI>(AIFactory::create(aiNb.first));
	auto ai2 = std::unique_ptr<AI>(AIFactory::create(aiNb.second));
	auto &state = game.getBattleState();
	const std::string trainerName = state.me.name;
	const sf::Vector2u rendererSize = renderer.getSize();
	sf::View view{{rendererSize.x / 2.f, rendererSize.y / 2.f}, sf::Vector2f(rendererSize)};
	sf::Font menuFont;
	BattleMenu menu = MENU_ROOT;
	unsigned char selectedMenu = 0;
	bool escaped = false;

	(void)menuFont.openFromFile("assets/font.ttf");
	renderer.reset();
	renderer.state = PkmnRenderer::fromGen1(state);
	window.setSize({rendererSize.x * 4, rendererSize.y * 4});
	window.setView(view);
	window.setTitle(trainerName + " - Challenging " + state.op.name);

	if (updateManually)
		state.onBattleStart();

	while (window.isOpen() && (!game.isFinished() || !renderer.hasAnimationEnded())) {
		while (auto event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			} else if (auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
				if (keyEvent->code == sf::Keyboard::Key::Escape) {
					escaped = true;
					break;
				} else if (playerActionNeeded(game, renderer, ai1.get(), state)) {
					handleMenuNavigation(*keyEvent, menu, selectedMenu, state);
				}
			}
			renderer.consumeEvent(*event);
		}
		if (escaped)
			break;

		if (renderer.hasAnimationEnded() && !game.isFinished()) {
			if (ai1 && state.me.nextAction == EmptyAction)
				state.me.nextAction = ai1->getNextMove(state, false);
			if (ai2 && state.op.nextAction == EmptyAction)
				state.op.nextAction = ai2->getNextMove(state, true);
			if (updateManually && state.me.nextAction != EmptyAction && state.op.nextAction != EmptyAction) {
				state.onTurnStart();
				menu = MENU_ROOT;
				selectedMenu = 0;
			}
		}

		renderer.update();
		window.clear();
		renderer.render(window);
		if (playerActionNeeded(game, renderer, ai1.get(), state))
			drawActionMenu(window, menuFont, menu, selectedMenu, state.me);
		window.display();
	}

	if (!escaped && !game.playingReplay())
		saveBattleReplay(game);
	state.onBattleEnd();
	renderer.clear();

	view.setSize(static_cast<sf::Vector2f>(window.getSize()));
	view.setCenter({window.getSize().x / 2.f, window.getSize().y / 2.f});
	window.setSize({800, 640});
	window.setView(view);
	game.reset();
	game.stopReplay();
	return escaped;
}
