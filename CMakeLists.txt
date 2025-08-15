cmake_minimum_required(VERSION 3.14)
project(PokeAI C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE MATCHES Debug)
	add_compile_options(-W -Wall -Wextra)
endif (CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE MATCHES Debug)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio Network)
find_package(TGUI 1 REQUIRED)

include_directories(include src)

if (SHARED_LIBRARY)
	set(LIBRARY_TYPE SHARED)
else()
	set(LIBRARY_TYPE STATIC)
endif()

add_library(
	PokemonGen1Core ${LIBRARY_TYPE}
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
	PokemonGen1Emulator ${LIBRARY_TYPE}
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
	PokemonGen1Client ${LIBRARY_TYPE}
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
)
target_link_libraries(
	PokeAI_cli
	PokemonGen1Client
)


# =========================== Bindings ===========================
add_library(
	PokemonGen1CBinding STATIC
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
	src/Emulator/EmulatorGameHandle.cpp
	src/Emulator/EmulatorGameHandle.hpp
	src/Emulator/EmulatorHandle.hpp
	src/Emulator/Socket.cpp
	src/Emulator/Socket.hpp
	src/Emulator/BgbHandler.cpp
	src/Emulator/BgbHandler.hpp
	src/CBinding/GameEngine/Pokemon.cpp
	src/CBinding/GameEngine/Pokemon.h
	src/CBinding/GameEngine/BattleHandler.cpp
	src/CBinding/GameEngine/BattleHandler.h
	src/CBinding/Emulator/BgbHandler.cpp
	src/CBinding/Emulator/BgbHandler.h
	src/CBinding/GameEngine/State.h
	src/CBinding/GameEngine/RandomGenerator.cpp
	src/CBinding/GameEngine/RandomGenerator.h
	src/CBinding/GameEngine/c_cpp.h
	src/CBinding/GameEngine/Type.h
	src/CBinding/GameEngine/Type.cpp
	src/CBinding/GameEngine/Move.cpp
	src/CBinding/GameEngine/Move.h
	src/CBinding/GameEngine/StatusChange.h
	src/CBinding/GameEngine/StatusChange.cpp
	src/CBinding/GameEngine/StatsChange.cpp
	src/CBinding/GameEngine/StatsChange.h
)
target_link_libraries(
	PokemonGen1CBinding
	PokemonGen1Core
	PokemonGen1Emulator
)
target_compile_definitions(PokemonGen1CBinding PRIVATE BUILD_LIB)
target_include_directories(PokemonGen1CBinding PUBLIC src/CBinding)

add_executable(
	PokeAIC_cli
	src/CBinding/commandLine.c
)
target_link_libraries(
	PokeAIC_cli
	PokemonGen1CBinding
)