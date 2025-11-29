//
// Created by PinkySmile on 20/11/2025.
//

#include <SFML/Graphics.hpp>
#include <cstring>
#include <fstream>
#include "Gen1Renderer.hpp"
#include "GameEngine/Gen1/BattleHandler.hpp"
#include "nlohmann/json.hpp"
#include "GameEngine/Gen1/Team.hpp"

int main(int argc, char **argv)
{
	if (argc <= 1) {
		printf("Usage: %s <replay_path>\n", argv[0]);
		return 1;
	}

	PokemonGen1::BattleHandler handler{false, false};
	auto &state = handler.getBattleState();
	Gen1Renderer renderer;

	state.battleLogger = [&renderer](const PkmnCommon::Event &event){ renderer.consumeEvent(event); };
	handler.loadReplay(argv[1]);
	handler.start();

	auto size = renderer.getSize();
	sf::RenderWindow win{sf::VideoMode{{size.x * 4, size.y * 4}}, state.me.name + " vs " + state.op.name};
	sf::View view;

	renderer.state = fromGen1(state);
	view.setCenter({size.x / 2.f, size.y / 2.f});
	view.setSize(sf::Vector2f(size));
	win.setFramerateLimit(60);
	win.setView(view);
	renderer.reset();

	while (!handler.isFinished())
		handler.tick();
	while (win.isOpen()) {
		while (auto event = win.pollEvent()) {
			if (event->is<sf::Event::Closed>())
				win.close();
			renderer.consumeEvent(*event);
		}
		renderer.update();
		renderer.render(win);
		win.display();
	}
}