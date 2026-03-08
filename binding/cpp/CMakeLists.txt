cmake_minimum_required(VERSION 3.11)

project(dinorunner-cpp
    VERSION 0.0.1
    DESCRIPTION "Running example of the dinorunner library in CXX"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif()
set(DINORUNNER_LIB_PATH ${PROJECT_SOURCE_DIR}/../../dinorunner)
option(DINORUNNER_BUILD_EXAMPLE OFF)

if (MSVC)
    #warning level 4
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(${DINORUNNER_LIB_PATH} ${DINORUNNER_LIB_PATH}/dinorunner)

add_library(dinorunner_static_cpp INTERFACE)
add_library(dinorunner::dinorunner_static_cpp ALIAS dinorunner_static_cpp)
target_include_directories(dinorunner_static_cpp INTERFACE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(dinorunner_static_cpp INTERFACE dinorunner::dinorunner_static)

add_library(dinorunner_shared_cpp INTERFACE)
add_library(dinorunner::dinorunner_shared_cpp ALIAS dinorunner_shared_cpp)
target_include_directories(dinorunner_shared_cpp INTERFACE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(dinorunner_shared_cpp INTERFACE dinorunner::dinorunner_shared)

if (${DINORUNNER_BUILD_EXAMPLE})
    add_executable(dinorunner_cpp_example example/main.cpp)
    target_link_libraries(dinorunner_cpp_example PRIVATE dinorunner::dinorunner_static_cpp)
endif()
