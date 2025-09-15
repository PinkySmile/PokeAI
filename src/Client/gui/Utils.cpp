//
// Created by PinkySmile on 07/04/2020.
//

#ifdef __GNUG__
#include <cxxabi.h>
#endif
#include <regex>
#include <codecvt>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <numeric>
#include <fstream>
#include "Utils.hpp"

namespace Utils
{
	static const std::map<std::string, std::string> _icons{
		{"folder", "assets/icons/folder.png"     },
		{".rar",   "assets/icons/archive.png"    },
		{".zip",   "assets/icons/archive.png"    },
		{".7z",    "assets/icons/archive.png"    },
		{".tgz",   "assets/icons/archive.png"    },
		{".gz",    "assets/icons/archive.png"    },
		{".xz",    "assets/icons/archive.png"    },
		{".htm",   "assets/icons/webFile.png"    },
		{".html",  "assets/icons/webFile.png"    },
		{".css",   "assets/icons/webFile.png"    },
		{".mimp",  "assets/icons/mimpFile.png"   },
		{".iso",   "assets/icons/discFile.png"   },
		{".txt",   "assets/icons/textFile.png"   },
		{".doc",   "assets/icons/textFile.png"   },
		{".mid",   "assets/icons/midiFile.png"   },
		{".midi",  "assets/icons/midiFile.png"   },
		{".class", "assets/icons/javaFile.png"   },
		{".jar",   "assets/icons/javaFile.png"   },
		{".ttf",   "assets/icons/fontsFile.png"  },
		{".otf",   "assets/icons/fontsFile.png"  },
		{".wav",   "assets/icons/musicFile.png"  },
		{".ogg",   "assets/icons/musicFile.png"  },
		{".mp3",   "assets/icons/musicFile.png"  },
		{".png",   "assets/icons/imageFile.png"  },
		{".jpg",   "assets/icons/imageFile.png"  },
		{".jpeg",  "assets/icons/imageFile.png"  },
		{".bmp",   "assets/icons/imageFile.png"  },
		{".gif",   "assets/icons/imageFile.png"  },
		{".exe",   "assets/icons/binaryFile.png" },
		{".o",     "assets/icons/binaryFile.png" },
		{".out",   "assets/icons/binaryFile.png" },
		{".a",     "assets/icons/binaryFile.png" },
		{".gch",   "assets/icons/binaryFile.png" },
		{".dll",   "assets/icons/binaryFile.png" },
		{".so",    "assets/icons/binaryFile.png" },
		{".xml",   "assets/icons/configFile.png" },
		{".ini",   "assets/icons/configFile.png" },
		{".json",  "assets/icons/configFile.png" },
		{".md",    "assets/icons/configFile.png" },
		{".c",     "assets/icons/sourceFile.png" },
		{".h",     "assets/icons/sourceFile.png" },
		{".hpp",   "assets/icons/sourceFile.png" },
		{".cpp",   "assets/icons/sourceFile.png" },
		{".lua",   "assets/icons/sourceFile.png" },
		{".java",  "assets/icons/sourceFile.png" },
		{".py",    "assets/icons/sourceFile.png" },
		{".asm",   "assets/icons/sourceFile.png" },
		{".php",   "assets/icons/sourceFile.png" },
		{".js",    "assets/icons/sourceFile.png" },
		{".sh",    "assets/icons/shellScript.png"},
		{".run",   "assets/icons/shellScript.png"},
		{".bat",   "assets/icons/shellScript.png"},
		{".pkmns", "assets/icons/pkmnFile.png"   },
		{"",       "assets/icons/unknownFile.png"},
	};

	int	dispMsg(const std::string &title, const std::string &content, int variate)
	{
		auto button = tgui::Button::create("OK");
		auto text = tgui::TextArea::create();
		tgui::Gui gui;
		auto font = tgui::Font::getGlobalFont();
		const auto startWidth = button->getSize().x + 102;
		unsigned width = startWidth;
		unsigned height = button->getSize().y + 60;
		float currentWidth = startWidth;
		auto size = text->getTextSize();

		std::cerr << title << std::endl << content << std::endl;
		for (char c : content) {
			currentWidth += font.getGlyph(c, size, false).advance;
			width = std::max(static_cast<unsigned>(currentWidth), width);
			if (c == '\n' || c == '\r')
				currentWidth = startWidth;
			if (c == '\n' || c == '\v')
				height += size;
			if (currentWidth >= 700) {
				currentWidth = startWidth;
				height += size;
			}
		}

		sf::RenderWindow win{
			sf::VideoMode{{std::min(700U, width), std::min(220U, height)}},
			title, sf::Style::Titlebar | sf::Style::Close
		};
		auto pic = tgui::Picture::create("assets/icons/error.png");

		gui.setTarget(win);
		gui.add(button, "ok");
		gui.add(text);

		button->setPosition("&.w - w - 10", "&.h - h - 10");
		button->onPress.connect([&win]{
			win.close();
		});

		text->setText(content);
		text->setPosition(52, 10);
		text->setSize("ok.x - 62", "ok.y - 20");
		text->setReadOnly();
		text->getRenderer()->setBorderColor("transparent");
		text->getRenderer()->setBackgroundColor("transparent");

		pic->setPosition(10, 10);
		pic->setSize(32, 32);

		if (variate & MB_ICONERROR)
			gui.add(pic);

		while (win.isOpen()) {
			while (auto event = win.pollEvent()) {
				if (event->is<sf::Event::Closed>())
					win.close();
				gui.handleEvent(*event);
			}

			win.clear({230, 230, 230, 255});
			gui.draw();
			win.display();
		}

		return 0;
	}

	static void _makeFolders(
		std::filesystem::path &currentPath,
		const tgui::ScrollablePanel::Ptr &panel,
		const tgui::EditBox::Ptr &file,
		const std::function<void()> &open,
		const std::regex &pattern = std::regex(".*", std::regex_constants::icase)
	) {
		auto pos = 10;
		std::vector<std::filesystem::directory_entry> paths = {
			std::filesystem::directory_entry("."),
			std::filesystem::directory_entry("..")
		};

		for (auto &entry : std::filesystem::directory_iterator(currentPath))
			paths.push_back(entry);
		std::sort(paths.begin(), paths.end(), [](const std::filesystem::directory_entry &a, const std::filesystem::directory_entry &b){
			if (a.is_directory() != b.is_directory())
				return a.is_directory();

			std::string ap = a.path().string();
			std::string bp = b.path().string();

			std::transform(ap.begin(), ap.end(), ap.begin(), [](char c){ return std::tolower(c); });
			std::transform(bp.begin(), bp.end(), bp.begin(), [](char c){ return std::tolower(c); });
			return ap < bp;
		});

		panel->removeAllWidgets();
		for (auto &entry : paths) {
			std::string pic;
			const auto &filePath = entry.path();
			auto fileStr = pathToString(filePath.filename());

			if (entry.is_directory())
				pic = _icons.at("folder");
			else if (!std::regex_search(pathToString(filePath), pattern))
				continue;
			else
				try {
					pic = _icons.at(pathToString(filePath.extension()));
				} catch (std::out_of_range &) {
					pic = _icons.at("");
				}

			auto button = tgui::Button::create("");
			auto picture = tgui::Picture::create(tgui::Texture{pic});
			auto label = tgui::Label::create(fileStr);
			auto renderer = button->getRenderer();

			button->setPosition(10, pos);
			button->setSize({"&.w - 40", 20});
			button->onClick.connect([button, file, fileStr, &open]{
				if (file->getText() == fileStr)
					open();
				else
					file->setText(fileStr);
			});
			renderer->setBackgroundColor("transparent");
			renderer->setBackgroundColorFocused("blue");
			renderer->setBackgroundColorHover("#00BFFF");
			renderer->setBorderColor("transparent");
			renderer->setBorderColorHover("#00BFFF");
			label->setPosition(40, pos + 2);
			label->ignoreMouseEvents();
			picture->setPosition(12, pos + 2);
			picture->ignoreMouseEvents();
			panel->add(button);
			panel->add(label);
			panel->add(picture);
			pos += 30;
		}

		auto label = tgui::Label::create();

		label->setPosition(40, pos - 11);
		label->setSize(10, 9);
		panel->add(label);

		panel->setVerticalScrollAmount(30);
		panel->setVerticalScrollbarValue(0);
	}

	std::string openFileDialog(const std::string &title, const std::string &basePath, const std::vector<std::pair<std::string, std::string>> &patterns, bool overWriteWarning, bool mustExist)
	{
		sf::RenderWindow window{sf::VideoMode{{500, 300}}, title, sf::Style::Titlebar};
		tgui::Gui gui{window};
		std::string result;
		std::string startText;
		std::filesystem::path currentPath = basePath;

		currentPath = std::filesystem::absolute(currentPath);
		while (!std::filesystem::is_directory(currentPath)) {
			if (!startText.empty())
				startText += std::filesystem::path::preferred_separator;
			startText += pathToString(currentPath.filename());
			currentPath = currentPath.parent_path();
		}

		gui.loadWidgetsFromFile("assets/open_file_dialog.gui");

		auto path = gui.get<tgui::TextArea>("Path");
		auto file = gui.get<tgui::EditBox>("file");
		auto box = gui.get<tgui::ComboBox>("Patterns");
		auto panel = gui.get<tgui::ScrollablePanel>("Folders");
		std::function<void()> open = [&gui, &result, &window, path, box, file, &currentPath, panel, &open, mustExist, overWriteWarning]{
			if (file->getText().empty())
				return;

			std::string ext = box->getSelectedItemId().toStdString();

			if (std::filesystem::path(file->getText().toStdString()).is_relative())
				result = path->getText().toStdString() + std::filesystem::path::preferred_separator + file->getText().toStdString();
			else
				result = file->getText().toStdString();

			if (std::filesystem::is_directory(result)) {
				result = cleanPath(result);
				currentPath = result + static_cast<char>(std::filesystem::path::preferred_separator);
				path->setText(pathToString(currentPath));
				file->setText("");
				_makeFolders(currentPath, panel, file, open, std::regex(box->getSelectedItemId().toStdString(), std::regex_constants::icase));
				return;
			}

			if (mustExist && !std::filesystem::exists(result))
				return;

			if (overWriteWarning && std::filesystem::exists(result)) {
				auto pan = tgui::Panel::create({"100%", "100%"});
				pan->getRenderer()->setBackgroundColor({0, 0, 0, 175});
				gui.add(pan);

				auto win = tgui::ChildWindow::create();
				win->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
				gui.add(win);

				win->setFocused(true);

				const bool tabUsageEnabled = gui.isTabKeyUsageEnabled();
				auto closeWindow = [&gui, win, pan, tabUsageEnabled]{
					gui.remove(win);
					gui.remove(pan);
					gui.setTabKeyUsageEnabled(tabUsageEnabled);
				};

				pan->onClick.connect(closeWindow);
				win->onClose.connect(closeWindow);
				win->onEscapeKeyPress.connect(closeWindow);
				win->loadWidgetsFromFile("assets/overwrite_warning.gui");

				auto label = win->get<tgui::Label>("Label");

				label->setText(result + label->getText());
				win->setSize(label->getSize().x + 20, 100);
				win->get<tgui::Button>("Yes")->onClick.connect([&window]{
					window.close();
				});
				win->get<tgui::Button>("No")->onClick.connect([win]{
					win->close();
				});
				return;
			}

			window.close();
		};

		gui.get<tgui::Button>("Cancel")->onClick.connect([&result, &window]{
			result = "";
			window.close();
		});
		gui.get<tgui::Button>("Open")->onClick.connect(open);

		if (overWriteWarning)
			gui.get<tgui::Button>("Open")->setText("Save");

		for (auto &pair : patterns)
			box->addItem(pair.second, pair.first);
		box->addItem("All files", ".*");
		box->setSelectedItemByIndex(0);
		box->onItemSelect.connect([&currentPath, &panel, &file, &box, &open]{
			_makeFolders(currentPath, panel, file, open, std::regex(box->getSelectedItemId().toStdString(), std::regex_constants::icase));
		});

		path->setText(pathToString(currentPath));
		file->setText(startText);
		_makeFolders(currentPath, panel, file, open, std::regex(box->getSelectedItemId().toStdString(), std::regex_constants::icase));

		while (window.isOpen()) {
			while (auto event = window.pollEvent()) {
				if (event->is<sf::Event::Closed>()) {
					result = "";
					window.close();
				} else if (auto resizeEvent = event->getIf<sf::Event::Resized>()) {
					window.setView(sf::View{sf::FloatRect({0, 0}, static_cast<sf::Vector2f>(resizeEvent->size))});
					gui.setAbsoluteView({{0, 0}, static_cast<sf::Vector2f>(resizeEvent->size)});
				}
				gui.handleEvent(*event);
			}
			window.clear({200, 200, 200});
			gui.draw();
			window.display();
		}
		return result;
	}

	std::string saveFileDialog(const std::string &title, const std::string &basePath, const std::vector<std::pair<std::string, std::string>> &patterns)
	{
		return openFileDialog(title, basePath, patterns, true, false);
	}

	tgui::ChildWindow::Ptr openWindowWithFocus(tgui::Gui &gui, tgui::Layout width, tgui::Layout height)
	{
		auto panel = tgui::Panel::create({"100%", "100%"});

		panel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
		gui.add(panel);

		auto window = tgui::ChildWindow::create();
		window->setSize(width, height);
		window->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
		gui.add(window);

		window->setFocused(true);

		const bool tabUsageEnabled = gui.isTabKeyUsageEnabled();
		auto closeWindow = [&gui, window, panel, tabUsageEnabled]{
			gui.remove(window);
			gui.remove(panel);
			gui.setTabKeyUsageEnabled(tabUsageEnabled);
		};

		panel->onClick.connect(closeWindow);
		window->onClose.connect(closeWindow);
		window->onEscapeKeyPress.connect(closeWindow);
		return window;
	}
}
