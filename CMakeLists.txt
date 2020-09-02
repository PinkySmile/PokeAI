cmake_minimum_required(VERSION 3.14)
project(PokeAI)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE MATCHES Debug)
	add_compile_options(-W -Wall -Wextra)
endif (CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE MATCHES Debug)

add_executable(
	PokeAI
	src/main.cpp
	src/Networking/Socket.cpp
	src/Networking/Socket.hpp
	src/Networking/BgbHandler.cpp
	src/Networking/BgbHandler.hpp
	src/GameEngine/GameHandle.cpp
	src/GameEngine/GameHandle.hpp
	src/Networking/EmulatorHandle.hpp
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
	src/commandLine.cpp
	src/commandLine.hpp
	src/gui/gui.cpp
	src/gui/gui.hpp
	src/gui/Utils.cpp
	src/gui/Utils.hpp
	src/gui/battle.cpp
	src/GameEngine/StatsChange.cpp)

if (WIN32)
	target_link_libraries(
		PokeAI
		ws2_32
		tgui
		sfml-graphics
		sfml-system
		sfml-window
		sfml-audio
	)
else (WIN32)
	target_link_libraries(
		PokeAI
		pthread
		tgui
		sfml-graphics
		sfml-system
		sfml-window
		sfml-audio
	)
endif (WIN32)
