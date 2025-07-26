cmake_minimum_required(VERSION 3.10)

project(dinorunner-sdl
    VERSION 1.3.2
    DESCRIPTION "Running examples of the Dinorunner libray"
    LANGUAGES C CXX)

option(DINORUNNER_SDL_EXAMPLE_VENDORED "Use vendored libraries" OFF)

message(STATUS "Loading dinorunner SDL demo")

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(DINORUNNER_LIB_PATH ${PROJECT_SOURCE_DIR}/../../dinorunner)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(FindPkgConfig)


if (DINORUNNER_SDL_EXAMPLE_VENDORED)
    include(dependencies)
    include_directories(${DINORUNNER_SDL_EXAMPLE_INCLUDE_DIRS})
else()
    find_package(SDL2 REQUIRED)
    find_package(SDL2_IMAGE REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
    PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
    PKG_SEARCH_MODULE(SDL2_gfx REQUIRED SDL2_gfx)
    set(DINORUNNER_SDL_EXAMPLE_LIBARIES ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${SDL2_gfx_LIBRARIES})
endif()


add_executable(dinorunner_sdl demo.c)

find_package(dinorunner PATHS ${DINORUNNER_LIB_PATH} REQUIRED)
target_include_directories(dinorunner_sdl PRIVATE "${DINORUNNER_LIB_PATH}/include")
add_subdirectory(${DINORUNNER_LIB_PATH} ${DINORUNNER_LIB_PATH}/lib EXCLUDE_FROM_ALL)
target_link_directories(dinorunner_sdl PRIVATE ${DINORUNNER_LIB_PATH}/lib)
target_link_libraries(dinorunner_sdl PRIVATE ${DINORUNNER_SDL_EXAMPLE_LIBARIES} dinorunner_static)
