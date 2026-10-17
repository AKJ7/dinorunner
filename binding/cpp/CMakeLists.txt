cmake_minimum_required(VERSION 3.11)

project(dinorunner-cpp
    VERSION 0.0.1
    DESCRIPTION "Running example of the dinorunner library in CXX"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
# Default to binding/cpp/{lib,bin}, unless a parent project or the command line chose otherwise
if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
endif()
if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
endif()
if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endif()
set(DINORUNNER_LIB_PATH ${PROJECT_SOURCE_DIR}/../../dinorunner)
option(DINORUNNER_BUILD_EXAMPLE "Build the C++ binding example" OFF)

# A parent project may already provide the C library
if (NOT TARGET dinorunner_static)
    add_subdirectory(${DINORUNNER_LIB_PATH} ${CMAKE_CURRENT_BINARY_DIR}/dinorunner)
endif()

if (MSVC)
    set(DINORUNNER_CPP_WARNINGS /W4)
else()
    set(DINORUNNER_CPP_WARNINGS -Wall -Wextra -Wpedantic)
endif()

add_library(dinorunner_static_cpp INTERFACE)
add_library(dinorunner::dinorunner_static_cpp ALIAS dinorunner_static_cpp)
target_include_directories(dinorunner_static_cpp INTERFACE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(dinorunner_static_cpp INTERFACE dinorunner::dinorunner_static)

add_library(dinorunner_shared_cpp INTERFACE)
add_library(dinorunner::dinorunner_shared_cpp ALIAS dinorunner_shared_cpp)
target_include_directories(dinorunner_shared_cpp INTERFACE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(dinorunner_shared_cpp INTERFACE dinorunner::dinorunner_shared)

if (DINORUNNER_BUILD_EXAMPLE)
    add_executable(dinorunner_cpp_example example/main.cpp)
    target_compile_options(dinorunner_cpp_example PRIVATE ${DINORUNNER_CPP_WARNINGS})
    target_link_libraries(dinorunner_cpp_example PRIVATE dinorunner::dinorunner_static_cpp)
endif()
