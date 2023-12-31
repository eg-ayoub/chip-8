cmake_minimum_required(VERSION 3.20)
project(chip-8)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(SDL_SHARED TRUE CACHE BOOL "Build a SDL shared library (if available)")
set(SDL_STATIC TRUE CACHE BOOL "Build a SDL static library (if available)")

FetchContent_Declare(
	SDL
	GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG main  # Replace this with a particular git tag or git hash
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)

FetchContent_Declare(
	SDL_mixer
	GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
	GIT_TAG main  # Replace this with a particular git tag or git hash
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)

FetchContent_Declare(
	cxxopts
	GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
	GIT_TAG v3.1.1
)

FetchContent_Declare(
	spdlog
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
	GIT_TAG v1.12.0
)

FetchContent_MakeAvailable(cxxopts spdlog SDL SDL_mixer)

add_executable(
	chip-8

	src/application.hpp
	src/application.cpp

	src/memory/memory.hpp
	src/memory/memory.cpp

	src/font/font.hpp
	src/font/font.cpp

	src/timer/timer.hpp

	src/reg/reg.hpp

	src/stack/stack.hpp
	src/stack/stack.cpp

	src/display/display.hpp
	src/display/display.cpp

	src/keypad/keypad.hpp
	src/keypad/keypad.cpp
	
	src/beep/beep.hpp
	src/beep/beep.cpp

	src/main.cpp
)

target_include_directories(chip-8 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(chip-8 PRIVATE cxxopts)
target_link_libraries(chip-8 PRIVATE spdlog::spdlog)
target_link_libraries(chip-8 PRIVATE SDL3::SDL3)
target_link_libraries(chip-8 PRIVATE SDL3_mixer::SDL3_mixer)
