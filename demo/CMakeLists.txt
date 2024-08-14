cmake_minimum_required(VERSION 3.10)

project(dinorunner-demos
    VERSION 1.0.1
    DESCRIPTION "Running examples of the Dinorunner libray"
    LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/../dinorunner/include)

include(FindPkgConfig)
message(STATUS "Loading 'demo' subdirectory. ${PROJECT_SOURCE_DIR}")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(SDL2_IMAGE REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})

find_package(dinorunner 
    PATHS ${PROJECT_SOURCE_DIR}/../dinorunner)

PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
PKG_SEARCH_MODULE(SDL2_gfx REQUIRED SDL2_gfx)

add_executable(dinorunner_sdl sdl/demo.c)

target_link_directories(dinorunner_sdl PRIVATE ${PROJECT_SOURCE_DIR}/../dinorunner/lib)
target_link_libraries(dinorunner_sdl dinorunner ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${SDL2_gfx_LIBRARIES})