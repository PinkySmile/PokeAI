//
// Created by PinkySmile on 22/02/2026.
//

#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include "Gen1Renderer.hpp"
#include "IRenderer.hpp"
#include "ShowdownReader.hpp"

std::vector<std::string> readReplay(const std::filesystem::path &path, std::optional<std::string> &renderer)
{
	std::string firstLine = R"(<script type="text/plain" class="battle-log-data">)";
	std::string line;
	std::ifstream stream{path};
	std::vector<std::string> lines;
	bool found = false;

	while (std::getline(stream, line)) {
		if (found) {
			if (line.ends_with("</script>")) {
				lines.push_back(line.substr(0, line.size() - 9));
				break;
			}
			if (line.starts_with("|gen|") && !renderer)
				renderer = line.substr(5);
			lines.push_back(line);
		} else if (line.starts_with(firstLine)) {
			found = true;
			lines.push_back(line.substr(firstLine.size()));
		} else if (line.starts_with("|")) {
			found = true;
			lines.push_back(line);
		}
	}
	lines.emplace_back("|");
	return lines;
}

std::unique_ptr<PkmnRenderer::IRenderer> createRenderer(const std::string &id, const std::string &extraArgs)
{
	if (id == "1") {
		auto pos = extraArgs.find(',');
		std::string version = extraArgs.substr(0, pos);
		bool color = true;

		if (pos != std::string::npos) {
			auto t = extraArgs.substr(pos + 1);

			color = t.starts_with("t") || t.starts_with("y") || t == "1";
		}
		return std::make_unique<PkmnRenderer::Gen1Renderer>(version, color);
	} else
		throw std::invalid_argument("Unsupported renderer \"" + id + R"(": Supported ones are "1")");
}

int main(int argc, char *argv[])
{
	std::string extraArgs;
	std::string replay;
	std::optional<std::string> renderer;
	bool argsDisabled = false;

	for (int index = 1; index < argc && replay.empty(); index++) {
		if (!argsDisabled && argv[index][0] == '-') {
			if (strcmp(argv[index], "--") == 0) {
				argsDisabled = true;
				continue;
			} else if (strcmp(argv[index], "-r") == 0) {
				index++;
				if (index > argc) {
					puts("Expected argument for option -r");
					return EXIT_FAILURE;
				}
				renderer = argv[index];
				continue;
			} else if (strcmp(argv[index], "-a") == 0) {
				index++;
				if (index > argc) {
					puts("Expected argument for option -a");
					return EXIT_FAILURE;
				}
				extraArgs = argv[index];
				continue;
			} else {
				printf("Unknown option %s\n", argv[index]);
				return EXIT_FAILURE;
			}
		}
		replay = argv[index];
	}

	std::vector<std::string> lines = readReplay(replay, renderer);
	if (!renderer) {
		puts("Failed to detect renderer.");
		return EXIT_FAILURE;
	}
	std::unique_ptr<PkmnRenderer::IRenderer> rendererPtr = createRenderer(*renderer, extraArgs);
	auto &state = rendererPtr->state;
	memset(&state, 0, sizeof(state));
	ShowdownReader reader{state};
	std::vector<PkmnCommon::Event> events;

	for (const auto &line : lines) {
		reader.feed(events, line);
		for (auto &event : events)
			rendererPtr->consumeEvent(event);
	}

	auto size = rendererPtr->getSize();
	sf::RenderWindow win{sf::VideoMode{{size.x * 4, size.y * 4}}, std::string(state.p1.name) + " vs " + state.p2.name};
	sf::View view;

	view.setCenter({size.x / 2.f, size.y / 2.f});
	view.setSize(sf::Vector2f(size));
	win.setFramerateLimit(60);
	win.setView(view);
	renderer.reset();

	bool paused = false;
	bool ok = false;
	unsigned div = 1;
	unsigned mul = 1;

	while (win.isOpen()) {
		while (auto event = win.pollEvent()) {
			if (event->is<sf::Event::Closed>())
				win.close();
			if (auto key = event->getIf<sf::Event::KeyPressed>()) {
				if (key->code == sf::Keyboard::Key::Space)
					paused = !paused;
				if (key->code == sf::Keyboard::Key::Enter)
					ok = true;
				if (key->code == sf::Keyboard::Key::Right)
					rendererPtr->nextTurn();
				if (key->code == sf::Keyboard::Key::Left)
					rendererPtr->previousTurn();
				if (key->code == sf::Keyboard::Key::Up) {
					if (div == 1) {
						mul++;
						win.setFramerateLimit(60 * mul);
					} else {
						div--;
						win.setFramerateLimit(60 / div);
					}
				}
				if (key->code == sf::Keyboard::Key::Down) {
					if (mul == 1) {
						div++;
						win.setFramerateLimit(60 / div);
					} else {
						mul--;
						win.setFramerateLimit(60 * mul);
					}
				}
				if (key->code == sf::Keyboard::Key::Home)
					rendererPtr->goToTurn(0);
				if (key->code == sf::Keyboard::Key::End)
					rendererPtr->goToTurn(-1);
				if (key->code == sf::Keyboard::Key::Backspace)
					rendererPtr->swapSide = !rendererPtr->swapSide;
			}
			rendererPtr->consumeEvent(*event);
		}
		if (!paused || ok)
			rendererPtr->update();
		ok = false;
		rendererPtr->render(win);
		win.display();
	}
}
