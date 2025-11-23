//
// Created by PinkySmile on 20/11/2025.
//

#include <SFML/Graphics.hpp>
#include <cstring>
#include <fstream>
#include "Gen1Renderer.hpp"
#include "nlohmann/json.hpp"

int main()
{
	Gen1Renderer renderer;
	auto size = renderer.getSize();
	sf::RenderWindow win{sf::VideoMode{{size.x * 4, size.y * 4}}, "test"};
	sf::View view;

	memset((void*)&renderer.state, 0, sizeof(renderer.state));

	renderer.state.p1.spriteId = PokemonGen1::Pikachu;
	strcpy(renderer.state.p1.team[0].name, "PIKACHU");
	renderer.state.p1.team[0].id = PokemonGen1::Pikachu;
	renderer.state.p1.team[0].hp = 50;
	renderer.state.p1.team[0].maxHp = 75;
	renderer.state.p1.team[0].level = 75;
	renderer.state.p1.active = 0;

	renderer.state.p2.spriteId = PokemonGen1::Beedrill;
	strcpy(renderer.state.p2.team[0].name, "BEEDRILL");
	renderer.state.p2.team[0].id = PokemonGen1::Beedrill;
	renderer.state.p2.team[0].hp = 50;
	renderer.state.p2.team[0].maxHp = 75;
	renderer.state.p2.team[0].level = 100;
	renderer.state.p2.active = 0;

	view.setCenter({size.x / 2.f, size.y / 2.f});
	view.setSize(sf::Vector2f(size));
	win.setFramerateLimit(60);
	win.setView(view);
	renderer.reset();

	std::ifstream stream{"assets/gen1/moves/list.json"};
	nlohmann::json json;
	stream >> json;
	for (auto &id : json) {
		renderer.consumeEvent(Pokemon::Event(Pokemon::TextEvent{"PIKACHU used " + PokemonGen1::availableMoves[id].getName() + "!"}));
		renderer.consumeEvent(Pokemon::Event(Pokemon::MoveEvent{id, true}));
		renderer.consumeEvent(Pokemon::Event(Pokemon::TextEvent{"BEEDRILL used " + PokemonGen1::availableMoves[id].getName() + "!"}));
		renderer.consumeEvent(Pokemon::Event(Pokemon::MoveEvent{id, false}));
	}

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