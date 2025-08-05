cmake_minimum_required(VERSION 3.14)
project(PokeAI)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE MATCHES Debug)
	add_compile_options(-W -Wall -Wextra)
endif (CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE MATCHES Debug)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio Network)
find_package(TGUI 1 REQUIRED)

include_directories(include)

add_library(
	PokemonGen1Core STATIC
	src/GameEngine/BattleHandler.cpp
	src/GameEngine/BattleHandler.hpp
	src/GameEngine/Pokemon.cpp
	src/GameEngine/Pokemon.hpp
	src/GameEngine/PokemonTypes.hpp
	src/GameEngine/Move.hpp
	src/GameEngine/Move.cpp
	src/GameEngine/Move.hpp
	src/GameEngine/StatusChange.hpp
	src/GameEngine/StatsChange.hpp
	src/GameEngine/PokemonRandomGenerator.cpp
	src/GameEngine/PokemonRandomGenerator.hpp
	src/GameEngine/StatusChange.cpp
	src/GameEngine/PokemonTypes.cpp
	src/GameEngine/EmulatorGameHandle.cpp
	src/GameEngine/EmulatorGameHandle.hpp
	src/GameEngine/State.hpp
	src/GameEngine/State.cpp
	src/GameEngine/Team.cpp
	src/GameEngine/Team.hpp
)

add_executable(
	PokeAI
	src/main.cpp
	src/Networking/Socket.cpp
	src/Networking/Socket.hpp
	src/Networking/BgbHandler.cpp
	src/Networking/BgbHandler.hpp
	src/Networking/EmulatorHandle.hpp
	src/commandLine.cpp
	src/commandLine.hpp
	src/gui/gui.cpp
	src/gui/gui.hpp
	src/gui/Utils.cpp
	src/gui/Utils.hpp
	src/gui/battle.cpp
	src/GameEngine/StatsChange.cpp
	src/AIs/AIHeuristic.cpp
	src/AIs/AIHeuristic.hpp
	src/AIs/AI.hpp
	src/AIs/AIFactory.hpp
	src/AIs/AIFactory.cpp
	src/AIs/RandomAI.cpp
	src/AIs/RandomAI.hpp
)

if (WIN32)
	target_link_libraries(
		PokeAI
		PokemonGen1Core
		ws2_32
		TGUI::TGUI
		SFML::Graphics
		SFML::Audio
	)
else (WIN32)
	target_link_libraries(
		PokeAI
		PokemonGen1Core
		pthread
		TGUI::TGUI
		SFML::Graphics
		SFML::Audio
	)
endif (WIN32)
