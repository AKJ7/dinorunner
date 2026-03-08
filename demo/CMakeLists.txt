cmake_minimum_required(VERSION 3.14)

project(dinorunner-demos DESCRIPTION "Running examples of the dinorunner library" LANGUAGES C CXX)

set(DINORUNNER_SDL_EXAMPLE_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/sdl")

add_subdirectory(${DINORUNNER_SDL_EXAMPLE_SRC_DIR})
