cmake_minimum_required(VERSION 3.14)
project(PokeAI)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE MATCHES Debug)
	add_compile_options(-W -Wall -Wextra)
endif (CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE MATCHES Debug)

add_executable(
	PokeAI
	src/main.cpp
	src/Socket.cpp
	src/Socket.hpp
	src/BgbHandler.cpp
	src/BgbHandler.hpp
	src/GameHandle.cpp
	src/GameHandle.hpp
	src/EmulatorHandle.hpp
	src/Pokemon.cpp
	src/Pokemon.hpp
	src/PokemonTypes.hpp
	src/Move.hpp
	src/Move.cpp
	src/Move.hpp
	src/StatusChange.hpp
	src/StatsChange.hpp
	src/PokemonRandomGenerator.cpp
	src/PokemonRandomGenerator.hpp
	src/StatusChange.cpp
	src/PokemonTypes.cpp
	src/commandLine.cpp
	src/commandLine.hpp
	src/gui.cpp src/gui.hpp)

if (WIN32)
	target_link_libraries(
		PokeAI
		ws2_32
		tgui
		sfml-graphics
		sfml-system
		sfml-window
	)
else (WIN32)
	target_link_libraries(
		PokeAI
		pthread
		tgui
		sfml-graphics
		sfml-system
		sfml-window
	)
endif (WIN32)
