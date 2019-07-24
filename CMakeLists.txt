cmake_minimum_required(VERSION 3.14)
project(PokeAI)

set(CMAKE_CXX_STANDARD 17)


if (CMAKE_COMPILER_IS_GNUCC)
	add_compile_options(-W -Wall -Wextra)
endif (CMAKE_COMPILER_IS_GNUCC)

add_executable(
	PokeAI
	src/main.cpp
	src/Socket.cpp
	src/Socket.hpp
	src/BgbHandler.cpp
	src/BgbHandler.hpp
	src/PkmnGen1Handle.cpp
	src/PkmnGen1Handle.hpp
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
	src/StatusChange.cpp src/PokemonTypes.cpp)

if (WIN32)
target_link_libraries(
	PokeAI
	ws2_32
)
else (WIN32)
target_link_libraries(
	PokeAI
	pthread
)
endif (WIN32)
