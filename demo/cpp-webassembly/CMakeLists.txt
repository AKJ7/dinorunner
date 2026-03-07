cmake_minimum_required(VERSION 3.10)

project(dinorunner-web 
    VERSION 0.0.1
    DESCRIPTION "Running example of the dinorunner library in CXX"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)


if (MSVC)
    #warning level 4
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(dinorunner_web_assembly example/main.cpp)
target_include_directories(dinorunner_web_assembly PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/../../dinorunner/include")
target_link_libraries(dinorunner_web_assembly PRIVATE "dinorunner")
target_link_directories(dinorunner_web_assembly PRIVATE "${PROJECT_SOURCE_DIR}/../../dinorunner/lib/")