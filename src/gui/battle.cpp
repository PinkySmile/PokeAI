//
// Created by Gegel85 on 17/08/2020.
//

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "gui.hpp"
#include "../AIs/AI.hpp"
#include "../AIs/AIHeuristic.hpp"

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

void displayOpponentStats(sf::RenderWindow &window, sf::Text &text, sf::RectangleShape rect, sf::Sprite &sprite, BattleResources &resources, const PokemonGen1::Pokemon &pkmn)
{
	float percent = static_cast<float>(pkmn.getHealth()) / pkmn.getMaxHealth();

	drawSprite(window, sprite, resources.boxes[2], 32, 64);
	drawSprite(window, sprite, resources.pokemonsFront[pkmn.getID()], 408, 0);
	rect.setFillColor(
		percent >= 0.5 ? sf::Color{0, 255, 0, 255} : (
			percent >= 0.1 ? sf::Color{255, 255, 0, 255} : sf::Color{255, 0, 0, 255})
	);
	rect.setSize({192 * percent, 16});
	rect.setPosition(124, 68);
	window.draw(rect);
	drawSprite(window, sprite, resources.hpOverlay, 64, 64);
	if (pkmn.hasStatus(PokemonGen1::STATUS_BURNED)) {
		drawText(window, text, "BRN", 124, 32);
	} else if (pkmn.hasStatus(PokemonGen1::STATUS_POISONED) || pkmn.hasStatus(PokemonGen1::STATUS_BADLY_POISONED)) {
		drawText(window, text, "PSN", 124, 32);
	} else if (pkmn.hasStatus(PokemonGen1::STATUS_FROZEN)) {
		drawText(window, text, "FRZ", 124, 32);
	} else if (pkmn.hasStatus(PokemonGen1::STATUS_ASLEEP)) {
		drawText(window, text, "SLP", 124, 32);
	} else {
		drawSprite(window, sprite, resources.levelSprite, 124, 32);
		drawText(window, text, std::to_string(pkmn.getLevel()), 156, 30);
	}
	drawText(window, text, pkmn.getNickname(), 32, -2);
}

void displayMyStats(sf::RenderWindow &window, sf::Text &text, sf::RectangleShape rect, sf::Sprite &sprite, BattleResources &resources, const PokemonGen1::Pokemon &pkmn)
{
	float percent = static_cast<float>(pkmn.getHealth()) / pkmn.getMaxHealth();

	drawSprite(window, sprite, resources.boxes[3], 288, 288);
	drawSprite(window, sprite, resources.pokemonsBack[pkmn.getID()], 8, 160);
	rect.setFillColor(
		percent >= 0.5 ? sf::Color{0, 255, 0, 255} : (
			percent >= 0.1 ? sf::Color{255, 255, 0, 255} : sf::Color{255, 0, 0, 255})
	);
	rect.setSize({192 * percent, 16});
	rect.setPosition(380, 292);
	window.draw(rect);
	drawSprite(window, sprite, resources.hpOverlay, 320, 288);
	if (pkmn.hasStatus(PokemonGen1::STATUS_BURNED)) {
		drawText(window, text, "BRN", 412, 256);
	} else if (pkmn.hasStatus(PokemonGen1::STATUS_POISONED) || pkmn.hasStatus(PokemonGen1::STATUS_BADLY_POISONED)) {
		drawText(window, text, "PSN", 412, 256);
	} else if (pkmn.hasStatus(PokemonGen1::STATUS_FROZEN)) {
		drawText(window, text, "FRZ", 412, 256);
	} else if (pkmn.hasStatus(PokemonGen1::STATUS_ASLEEP)) {
		drawText(window, text, "SLP", 412, 256);
	} else {
		drawSprite(window, sprite, resources.levelSprite, 412, 256);
		drawText(window, text, std::to_string(pkmn.getLevel()), 444, 254);
	}
	drawText(window, text, pkmn.getName(), 320, 222);

	drawText(window, text, std::to_string(pkmn.getHealth()), 578 - std::to_string(pkmn.getHealth()).size() * 32, 318);
	drawText(window, text, "/", 450, 318);
	drawText(window, text, std::to_string(pkmn.getMaxHealth()), 450 - std::to_string(pkmn.getMaxHealth()).size() * 32, 318);
}

void executeBattleStartAnimation(sf::RenderWindow &window, PokemonGen1::GameHandle &game, const std::string &trainerName, BattleResources &resources, std::vector<std::string> &log, const PokemonGen1::BattleState &state)
{
	sf::RectangleShape rect;
	float		last = 0;
	float		seconds;
	sf::View	view{{320, 288}, {640, 576}};
	sf::Clock	clock;
	sf::Text	text;
	sf::Sprite	sprite;
	sf::Sound	sound;
	sf::Event	event;

	window.setView(view);
	text.setFont(resources.font);
	text.setCharacterSize(32);
	text.setFillColor({39, 39, 39, 255});
	text.setLineSpacing(2);
	window.setTitle(trainerName + " - Challenging " + game.getBattleState().opponentName);
	resources.start.play();
	resources.loop.setLoop(true);
	while (window.isOpen() && clock.getElapsedTime().asSeconds() < 20) {
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear({255, 255, 255, 255});

		seconds = clock.getElapsedTime().asSeconds();

		drawSprite(window, sprite, resources.boxes[0], 0, 384);
		if (seconds < 4.5) {
			window.clear({255, 255, 255, 255});
			drawSprite(window, sprite, resources.boxes[1], 96, 144);
			drawText(window, text, trainerName, 144, 190);
			for (unsigned i = 0; i < state.team.size(); i++)
				drawSprite(window, sprite, resources.balls[
					state.team[i].hasStatus(static_cast<PokemonGen1::StatusChange>(0xFF)) ? 3 : (state.team[i].getHealth() ? 0 : 2)
				], 288 + 32 * i, 232);

			drawText(window, text, state.opponentName, 144, 318);
			for (unsigned i = 0; i < state.opponentTeam.size(); i++)
				drawSprite(window, sprite, resources.balls[
					state.opponentTeam[i].hasStatus(static_cast<PokemonGen1::StatusChange>(0xFF)) ? 3 : (state.opponentTeam[i].getHealth() ? 0 : 2)
				], 288 + 32 * i, 352);

		} else if (seconds < 6) {
			drawSprite(window, sprite, resources.trainer[0][1], -224 + 632 * (seconds - 4.5) / 1.5, 0);
			drawSprite(window, sprite, resources.trainer[0][0], 640 * (6 - seconds) / 1.5, 160);

		} else if (seconds < 7) {
			if (last <= 6)
				playSound(sound, resources.trainerLand);
			drawSprite(window, sprite, resources.trainer[1][1], 408, 0);
			drawSprite(window, sprite, resources.trainer[1][0], 8, 160);

		} else if (seconds < 10) {
			drawSprite(window, sprite, resources.trainer[1][1], 408, 0);
			drawSprite(window, sprite, resources.trainer[1][0], 8, 160);

			drawSprite(window, sprite, resources.boxes[2], 32, 64);
			for (int i = 6; i-- > 0; )
				drawSprite(window, sprite, resources.balls[
					static_cast<unsigned>(i) >= state.opponentTeam.size() ? 1 : (state.opponentTeam[i].hasStatus(static_cast<PokemonGen1::StatusChange>(0xFF)) ? 3 : (state.opponentTeam[i].getHealth() ? 0 : 2))
				], 64 + 32 * (6 - i), 64);

			drawSprite(window, sprite, resources.boxes[2], 608, 320, -resources.boxes[2].getSize().x);
			for (unsigned i = 0; i < 6; i++)
				drawSprite(window, sprite, resources.balls[
					i >= state.team.size() ? 1 : (state.team[i].hasStatus(static_cast<PokemonGen1::StatusChange>(0xFF)) ? 3 : (state.team[i].getHealth() ? 0 : 2))
				], 352 + 32 * i, 318);

			drawText(window, text, log[0].substr(0, (seconds - 7) * 15), 32, 440);

		} else if (seconds < 10.25) {
			if (last <= 10)
				log.erase(log.begin());

			drawSprite(window, sprite, resources.trainer[1][1], 408 + 896 * (seconds - 10), 0);
			drawSprite(window, sprite, resources.trainer[1][0], 8, 160);

		} else if (seconds < 10.5) {
			drawSprite(window, sprite, resources.trainer[1][0], 8, 160);

		} else if (seconds < 13) {
			drawSprite(window, sprite, resources.trainer[1][0], 8, 160);
			drawText(window, text, log[0].substr(0, (seconds - 10.5) * 15), 32, 440);

		} else if (seconds < 13.2) {
			auto &texture = resources.pokemonsFront[state.opponentTeam[state.opponentPokemonOnField].getID()];

			drawSprite(window, sprite, texture,
				   408 + (texture.getSize().x - texture.getSize().x * 5 * (seconds - 13)) / 2,
				   0 + texture.getSize().y - texture.getSize().y * 5 * (seconds - 13),
				   texture.getSize().x * 5 * (seconds - 13),
				   texture.getSize().y * 5 * (seconds - 13)
			);
			drawSprite(window, sprite, resources.trainer[1][0], 8, 160);
			drawText(window, text, log[0], 32, 440);

		} else if (seconds < 14) {
			drawSprite(window, sprite, resources.pokemonsFront[state.opponentTeam[state.opponentPokemonOnField].getID()], 408, 0);
			drawSprite(window, sprite, resources.trainer[1][0], 8, 160);
			drawText(window, text, log[0], 32, 440);

		} else if (seconds < 14.2) {
			drawSprite(window, sprite, resources.trainer[1][0], 8 - 256 * (seconds - 14) * 5, 160);
			drawText(window, text, log[0], 32, 440);
			displayOpponentStats(window, text, rect, sprite, resources, state.opponentTeam[state.opponentPokemonOnField]);

		} else if (seconds < 14.5) {
			if (last <= 14.2)
				log.erase(log.begin());
			drawText(window, text, log[0].substr(0, (seconds - 14.2) * 15), 32, 440);
			displayOpponentStats(window, text, rect, sprite, resources, state.opponentTeam[state.opponentPokemonOnField]);

		} else if (seconds < 15.5) {
			drawText(window, text, log[0].substr(0, (seconds - 14.2) * 15), 32, 440);
			displayOpponentStats(window, text, rect, sprite, resources, state.opponentTeam[state.opponentPokemonOnField]);

		} else {
			drawText(window, text, log[0], 32, 440);
			displayOpponentStats(window, text, rect, sprite, resources, state.opponentTeam[state.opponentPokemonOnField]);
			displayMyStats(window, text, rect, sprite, resources, state.team[state.pokemonOnField]);

		}

		last = seconds;
		if (seconds + 1. / 60 >= resources.start.getDuration().asSeconds() && resources.loop.getStatus() != sf::Music::Playing)
			resources.loop.play();

		window.display();
	}
	log.erase(log.begin());
}

//TODO: Rewrite all of this mess
void battle(sf::RenderWindow &window, PokemonGen1::GameHandle &game, BattleResources &resources, std::vector<std::string> &log, PokemonGen1::BattleAction &nextAction, unsigned char aiNb)
{
	std::unique_ptr<PokemonGen1::AI> ai{
		aiNb == 1 ? new PokemonGen1::AIHeuristic(game) : nullptr
	};
	int		menu = 0;
	sf::RectangleShape rect;
	sf::Sprite	sprite;
	sf::Sound	sound;
	sf::Text	text;
	sf::Clock	clock;
	unsigned char	selectedMenu = 0;
	sf::View	view{{320, 288}, {640, 576}};
	const auto &state = game.getBattleState();
	std::string trainerName = game.getTrainerName();

	window.setSize({640, 576});
	window.setView(view);
	text.setFont(resources.font);
	text.setCharacterSize(32);
	text.setFillColor({39, 39, 39, 255});
	text.setLineSpacing(2);
	window.setTitle(trainerName + " - Challenging " + state.opponentName);
	executeBattleStartAnimation(window, game, trainerName, resources, log, state);
	clock.restart();
	while (window.isOpen() && game.getStage() == PokemonGen1::BATTLE) {
		sf::Event event;

		if (!log.empty())
			menu = 4;
		else if (game.getBattleState().nextAction)
			menu = 3;
		else if (menu == 3)
			menu = 0;

		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::KeyPressed && !ai) {
				if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down) {
					if (menu == 0) {
						selectedMenu = (selectedMenu + 2) % 4;
					} else if (menu == 1) {
						selectedMenu += (event.key.code == sf::Keyboard::Down) * 2 - 1;
						selectedMenu %= 4;
						if (state.team[state.pokemonOnField].getMoveSet()[selectedMenu].getID() == 0)
							selectedMenu = 0;
					} else if (menu == 2) {
						selectedMenu += (event.key.code == sf::Keyboard::Down) * 2 - 1;
						selectedMenu %= state.team.size();
					}
				} else if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::Left) {
					if (menu == 0) {
						selectedMenu = (selectedMenu + 1) % 2 + (selectedMenu / 2) * 2;
					}
				} else if (event.key.code == sf::Keyboard::W) {
					if (menu == 0) {
						if (selectedMenu == 0) {
							for (int i = 0; i < 4; i++)
								if (state.team[state.pokemonOnField].getMoveSet()[i].getPP() != 0)
									menu = 1;
							if (menu == 0)
								nextAction = PokemonGen1::StruggleMove;
						} else if (selectedMenu == 1) {
							menu = 2;
							selectedMenu = 0;
						} else if (selectedMenu == 2) {
							menu = 4;
							clock.restart();
							log.emplace_back("Cannot use items\nin trainer battles");
						} else {
							menu = 3;
							nextAction = PokemonGen1::Run;
						}
					} else if (menu == 1) {
						if (state.team[state.pokemonOnField].getMoveSet()[selectedMenu].getPP() != 0) {
							menu = 3;
							nextAction = static_cast<PokemonGen1::BattleAction>(PokemonGen1::Attack1 + selectedMenu);
						} else {
							menu = 5;
							log.emplace_back("No PP left !");
						}
					} else if (menu == 2) {
						menu = 3;
						nextAction = static_cast<PokemonGen1::BattleAction>(PokemonGen1::Switch1 + selectedMenu);
					}
				} else if (event.key.code == sf::Keyboard::X) {
					if (menu == 1 || menu == 2) {
						menu = 0;
						selectedMenu = 0;
					}
				}
			}
		}

		window.clear({255, 255, 255, 255});

		drawSprite(window, sprite, resources.boxes[0], 0, 384);
		displayOpponentStats(window, text, rect, sprite, resources, state.opponentTeam[state.opponentPokemonOnField]);
		displayMyStats(window, text, rect, sprite, resources, state.team[state.pokemonOnField]);

		if (menu == 0) {
			drawSprite(window, sprite, resources.choicesHUD, 256, 384);
			drawSprite(window, sprite, resources.arrows[1], 288 + 192 * (selectedMenu % 2), 448 + 64 * (selectedMenu / 2));
			if (ai)
				nextAction = ai->getNextMove();
		} else if (menu == 1) {
			auto move = state.team[state.pokemonOnField].getMoveSet()[selectedMenu];

			drawSprite(window, sprite, resources.attackHUD, 0, 256);
			for (int i = 0; i < 4; i++)
				drawText(window, text, strToUpper(state.team[state.pokemonOnField].getMoveSet()[i].getName()), 192, 416 + 32 * i);
			drawText(window, text, strToUpper(typeToString(move.getType())), 64, 320);
			drawText(window, text, std::to_string(move.getPP()), 160 + (move.getPP() < 10) * 32, 352);
			drawText(window, text, "/" + std::to_string(move.getMaxPP()), 224, 352);
			drawText(window, text, std::to_string(move.getMaxPP()), 256 + (move.getMaxPP() < 10) * 32, 352);
			drawSprite(window, sprite, resources.arrows[1], 160, 416 + 32 * selectedMenu);
		} else if (menu == 2) {
			window.clear({255, 255, 255, 255});
			drawSprite(window, sprite, resources.boxes[0], 0, 384);
			for (size_t i = 0; i < state.team.size(); i++) {
				auto &pkmn = state.team[i];
				float percent = static_cast<float>(pkmn.getHealth()) / pkmn.getMaxHealth();

				rect.setFillColor(
					percent >= 0.5 ? sf::Color{0, 255, 0, 255} : (
						percent >= 0.1 ? sf::Color{255, 255, 0, 255} : sf::Color{255, 0, 0, 255})
				);
				rect.setSize({192 * percent, 16});
				rect.setPosition(188, i * 64 + 36);
				window.draw(rect);
				drawSprite(window, sprite, resources.hpOverlay, 128, 32 + i * 64);
				if (pkmn.getHealth() == 0) {
					drawText(window, text, "FNT", 416, i * 64);
				} else if (pkmn.hasStatus(PokemonGen1::STATUS_BURNED)) {
					drawText(window, text, "BRN", 416, i * 64);
				} else if (pkmn.hasStatus(PokemonGen1::STATUS_POISONED) || pkmn.hasStatus(PokemonGen1::STATUS_BADLY_POISONED)) {
					drawText(window, text, "PSN", 416, i * 64);
				} else if (pkmn.hasStatus(PokemonGen1::STATUS_FROZEN)) {
					drawText(window, text, "FRZ", 416, i * 64);
				} else if (pkmn.hasStatus(PokemonGen1::STATUS_ASLEEP)) {
					drawText(window, text, "SLP", 416, i * 64);
				} else {
					drawSprite(window, sprite, resources.levelSprite, 416, i * 64);
					drawText(window, text, std::to_string(pkmn.getLevel()), 448, i * 64);
				}
				drawText(window, text, pkmn.getName(), 96, i * 64);

				drawText(window, text, std::to_string(pkmn.getHealth()), 512 - std::to_string(pkmn.getHealth()).size() * 32, i * 64 + 32);
				drawText(window, text, "/", 512, 64 * 2 + 32);
				drawText(window, text, std::to_string(pkmn.getMaxHealth()), 640 - std::to_string(pkmn.getMaxHealth()).size() * 32, i * 64 + 32);
			}
			drawSprite(window, sprite, resources.arrows[1], 0, 64 * selectedMenu + 32);
		} else if (menu == 3) {
			if (log.empty()) {
				drawSprite(window, sprite, resources.waitingHUD, 192, 320);
				drawText(window, text, "Waiting", 224, 352);
			} else
				menu = 4 + (state.team[state.pokemonOnField].getHealth() == 0) * 2;
		} else if (menu >= 4) {
			drawText(window, text, log[0].substr(0, clock.getElapsedTime().asSeconds() * 15), 32, 440);
			if (clock.getElapsedTime().asSeconds() > 4) {
				log.erase(log.begin());
				if (log.empty())
					menu -= 4;
				clock.restart();
			}
		}
		window.display();
	}
	resources.start.stop();
	resources.loop.stop();

	view.setSize(window.getSize().x, window.getSize().y);
	view.setCenter(window.getSize().x / 2., window.getSize().y / 2.);
	window.setSize({800, 640});
	window.setView(view);
	log.clear();
}

