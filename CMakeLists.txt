cmake_minimum_required(VERSION 3.14)
project(PokeAI)

set(CMAKE_CXX_STANDARD 17)

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
)

target_link_libraries(
	PokeAI
	ws2_32
)