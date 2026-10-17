cmake_minimum_required(VERSION 3.11)

project(dinorunner
    VERSION 2.0.0
    DESCRIPTION "Chrome's Dino game in C"
    LANGUAGES C)

set(CMAKE_C_STANDARD 99)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# Default to dinorunner/{lib,bin}, unless a parent project or the command line chose otherwise
if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
endif()
if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
endif()
if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_file(${PROJECT_SOURCE_DIR}/src/version.c.in ${PROJECT_BINARY_DIR}/src/version.c @ONLY)

set(DINORUNNER_CORE_FILES
    ${PROJECT_SOURCE_DIR}/src/clouds.c
    ${PROJECT_SOURCE_DIR}/src/dinorunner.c
    ${PROJECT_SOURCE_DIR}/src/distance_meter.c
    ${PROJECT_SOURCE_DIR}/src/game_over_panel.c
    ${PROJECT_SOURCE_DIR}/src/horizon.c
    ${PROJECT_SOURCE_DIR}/src/horizon_line.c
    ${PROJECT_SOURCE_DIR}/src/nightmode.c
    ${PROJECT_SOURCE_DIR}/src/obstacle.c
    ${PROJECT_SOURCE_DIR}/src/platform.c
    ${PROJECT_SOURCE_DIR}/src/trex.c
    ${PROJECT_SOURCE_DIR}/src/utils.c
    ${PROJECT_BINARY_DIR}/src/version.c
)

add_library(dinorunner_shared SHARED ${DINORUNNER_CORE_FILES})
add_library(dinorunner_static STATIC ${DINORUNNER_CORE_FILES})
add_library(dinorunner::dinorunner_shared ALIAS dinorunner_shared)
add_library(dinorunner::dinorunner_static ALIAS dinorunner_static)

foreach(target dinorunner_shared dinorunner_static)
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        # No stack protector: it would require __stack_chk_fail from the C library
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -fno-stack-protector)
    endif()
    set_target_properties(${target} PROPERTIES OUTPUT_NAME dinorunner)
endforeach()

# The library is freestanding: it needs no C library
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_link_options(dinorunner_shared PRIVATE "-nostdlib")
endif()
set_target_properties(dinorunner_shared PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

set(DINORUNNER_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/dinorunner)

install(TARGETS dinorunner_shared dinorunner_static
    EXPORT dinorunnerTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES include/dinorunner.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel)

configure_package_config_file(cmake/dinorunnerConfig.cmake.in
    "${PROJECT_BINARY_DIR}/dinorunnerConfig.cmake"
    INSTALL_DESTINATION ${DINORUNNER_CONFIG_INSTALL_DIR})
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/dinorunnerConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

# Build tree package: find_package(dinorunner PATHS <build dir>)
export(EXPORT dinorunnerTargets
    FILE "${PROJECT_BINARY_DIR}/dinorunnerTargets.cmake"
    NAMESPACE dinorunner::)

install(EXPORT dinorunnerTargets
    FILE dinorunnerTargets.cmake
    NAMESPACE dinorunner::
    DESTINATION ${DINORUNNER_CONFIG_INSTALL_DIR})
install(FILES
    "${PROJECT_BINARY_DIR}/dinorunnerConfig.cmake"
    "${PROJECT_BINARY_DIR}/dinorunnerConfigVersion.cmake"
    DESTINATION ${DINORUNNER_CONFIG_INSTALL_DIR}
    COMPONENT Devel)
