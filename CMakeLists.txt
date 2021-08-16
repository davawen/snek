cmake_minimum_required(VERSION 3.12...3.21)

# Flags / Options
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

project(snek
		VERSION 1.0
		LANGUAGES CXX
		DESCRIPTION "A simple terminal based snake program.")

# Packages
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(CURSES_USE_NCURSES TRUE)
find_package(Curses REQUIRED)

add_executable(snek ./src/main.cpp)
# Headers
target_include_directories(snek PRIVATE ${CURSES_INCLUDE_DIR})

# Libraries
target_link_libraries(snek Threads::Threads)
target_link_libraries(snek ${CURSES_LIBRARIES})

# Compilation

target_compile_features(snek PRIVATE cxx_std_17)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_compile_options(snek PRIVATE /W4)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_compile_options(snek PRIVATE -Wall -Wextra -Wpedantic -Wno-unknown-pragmas)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	target_compile_options(snek PRIVATE)
endif()
