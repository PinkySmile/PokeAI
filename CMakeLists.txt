cmake_minimum_required(VERSION 3.14)
project(PokeAI C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE MATCHES Debug)
	add_compile_options(-W -Wall -Wextra)
endif (CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE MATCHES Debug)

if (ASAN)
	add_compile_options(-fsanitize=address)
	add_link_options(-fsanitize=address)
endif ()

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
	src/GameEngine/Gen1/BattleHandler.cpp
	src/GameEngine/Gen1/BattleHandler.hpp
	src/GameEngine/Gen1/Pokemon.cpp
	src/GameEngine/Gen1/Pokemon.hpp
	src/GameEngine/Gen1/Type.hpp
	src/GameEngine/Gen1/Move.cpp
	src/GameEngine/Gen1/Move.hpp
	src/GameEngine/Gen1/StatusChange.hpp
	src/GameEngine/Gen1/StatsChange.cpp
	src/GameEngine/Gen1/StatsChange.hpp
	src/GameEngine/Gen1/RandomGenerator.cpp
	src/GameEngine/Gen1/RandomGenerator.hpp
	src/GameEngine/Gen1/StatusChange.cpp
	src/GameEngine/Gen1/Type.cpp
	src/GameEngine/Gen1/State.hpp
	src/GameEngine/Gen1/State.cpp
	src/GameEngine/Gen1/Team.cpp
	src/GameEngine/Gen1/Team.hpp
	src/GameEngine/Gen1/Utils.cpp
	src/GameEngine/Gen1/Utils.hpp
	src/GameEngine/Gen1/Damage.hpp
	src/GameEngine/Event.hpp
	src/GameEngine/Gen1/Common.hpp
)
target_link_libraries(
	PokemonGen1Core
	SFML::Graphics
	SFML::Audio
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
	src/Client/gui/IRenderer.hpp
	src/Client/gui/IRenderer.cpp
	src/Client/gui/Gen1Renderer.cpp
	src/Client/gui/Gen1Renderer.hpp
)
target_link_libraries(
	PokeAI_gui
	PokemonGen1Client
	TGUI::TGUI
)

add_executable(
	test_renderer
	src/Client/gui/test.cpp
	src/Client/gui/Utils.cpp
	src/Client/gui/Utils.hpp
	src/Client/gui/IRenderer.hpp
	src/Client/gui/IRenderer.cpp
	src/Client/gui/Gen1Renderer.cpp
	src/Client/gui/Gen1Renderer.hpp
)
target_link_libraries(
	test_renderer
	PokemonGen1Client
	TGUI::TGUI
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
	src/GameEngine/Gen1/BattleHandler.cpp
	src/GameEngine/Gen1/BattleHandler.hpp
	src/GameEngine/Gen1/Pokemon.cpp
	src/GameEngine/Gen1/Pokemon.hpp
	src/GameEngine/Gen1/Type.hpp
	src/GameEngine/Gen1/Move.cpp
	src/GameEngine/Gen1/Move.hpp
	src/GameEngine/Gen1/StatusChange.hpp
	src/GameEngine/Gen1/StatsChange.cpp
	src/GameEngine/Gen1/StatsChange.hpp
	src/GameEngine/Gen1/RandomGenerator.cpp
	src/GameEngine/Gen1/RandomGenerator.hpp
	src/GameEngine/Gen1/StatusChange.cpp
	src/GameEngine/Gen1/Type.cpp
	src/GameEngine/Gen1/State.hpp
	src/GameEngine/Gen1/State.cpp
	src/GameEngine/Gen1/Team.cpp
	src/GameEngine/Gen1/Team.hpp
	src/GameEngine/Gen1/Utils.cpp
	src/GameEngine/Gen1/Utils.hpp
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
