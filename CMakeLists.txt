cmake_minimum_required(VERSION 3.14)
project(PokeAI)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE MATCHES Debug)
	add_compile_options(-W -Wall -Wextra)
endif (CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE MATCHES Debug)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio Network)
find_package(TGUI 1 REQUIRED)

include_directories(include src)

add_library(
	PokemonGen1Core STATIC
	src/GameEngine/BattleHandler.cpp
	src/GameEngine/BattleHandler.hpp
	src/GameEngine/Pokemon.cpp
	src/GameEngine/Pokemon.hpp
	src/GameEngine/Type.hpp
	src/GameEngine/Move.cpp
	src/GameEngine/Move.hpp
	src/GameEngine/StatusChange.hpp
	src/GameEngine/StatsChange.cpp
	src/GameEngine/StatsChange.hpp
	src/GameEngine/RandomGenerator.cpp
	src/GameEngine/RandomGenerator.hpp
	src/GameEngine/StatusChange.cpp
	src/GameEngine/Type.cpp
	src/GameEngine/State.hpp
	src/GameEngine/State.cpp
	src/GameEngine/Team.cpp
	src/GameEngine/Team.hpp
	src/GameEngine/Utils.cpp
	src/GameEngine/Utils.hpp
)


add_library(
	PokemonGen1Emulator STATIC
	src/Emulator/EmulatorGameHandle.cpp
	src/Emulator/EmulatorGameHandle.hpp
	src/Emulator/EmulatorHandle.hpp
	src/Emulator/Socket.cpp
	src/Emulator/Socket.hpp
	src/Emulator/BgbHandler.cpp
	src/Emulator/BgbHandler.hpp
)
if (WIN32)
	target_link_libraries(
		PokemonGen1Emulator
		PokemonGen1Core
		ws2_32
	)
else (WIN32)
	target_link_libraries(
		PokemonGen1Emulator
		PokemonGen1Core
		pthread
	)
endif (WIN32)


add_library(
	PokemonGen1Client STATIC
	src/Client/AIs/AIHeuristic.cpp
	src/Client/AIs/AIHeuristic.hpp
	src/Client/AIs/AI.hpp
	src/Client/AIs/AIFactory.hpp
	src/Client/AIs/AIFactory.cpp
	src/Client/AIs/RandomAI.cpp
	src/Client/AIs/RandomAI.hpp
)
if (WIN32)
	target_link_libraries(
		PokemonGen1Client
		PokemonGen1Core
		PokemonGen1Emulator
	)
else (WIN32)
	target_link_libraries(
		PokemonGen1Client
		PokemonGen1Core
		PokemonGen1Emulator
	)
endif (WIN32)



add_executable(
	PokeAI_gui
	src/Client/gui/gui.cpp
	src/Client/gui/gui.hpp
	src/Client/gui/Utils.cpp
	src/Client/gui/Utils.hpp
	src/Client/gui/battle.cpp
)
target_link_libraries(
	PokeAI_gui
	PokemonGen1Client
	TGUI::TGUI
	SFML::Graphics
	SFML::Audio
)

add_executable(
	PokeAI_cli
	src/Client/commandLine.cpp
	src/Client/commandLine.hpp
)
target_link_libraries(
	PokeAI_cli
	PokemonGen1Client
)
