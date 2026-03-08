cmake_minimum_required(VERSION 3.10)

project(dinorunner-cpp
    VERSION 0.0.1
    DESCRIPTION "Running example of the dinorunner library in CXX"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(DINORUNNER_LIB_PATH ${PROJECT_SOURCE_DIR}/../../dinorunner)

if (MSVC)
    #warning level 4
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()


find_package(dinorunner PATHS ${DINORUNNER_LIB_PATH} REQUIRED)
add_subdirectory(${DINORUNNER_LIB_PATH} ${DINORUNNER_LIB_PATH}/lib EXCLUDE_FROM_ALL)

add_executable(dinorunner_cpp_example example/main.cpp)
target_include_directories(dinorunner_cpp_example PRIVATE "${PROJECT_SOURCE_DIR}/include" "${DINORUNNER_LIB_PATH}/include")
target_link_libraries(dinorunner_cpp_example PRIVATE dinorunner_static)
target_link_directories(dinorunner_cpp_example PRIVATE "${DINORUNNER_LIB_PATH}/lib")