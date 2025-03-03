cmake_minimum_required(VERSION 3.10)

cmake_policy(PUSH)

project(dinorunner 
    VERSION 1.3.1
    DESCRIPTION "Chrome's Dino game in C"
    LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
SET(CMAKE_C_STANDARD_LIBRARIES "")
SET(CMAKE_C_STANDARD_LIBRARIES_INIT "")
SET(CMAKE_CXX_STANDARD_LIBRARIES "")
SET(CMAKE_CXX_STANDARD_LIBRARIES_INIT "")
SET(CMAKE_BUILD_TYPE "Release")
set(CMAKE_C_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_C_IMPLICIT_LINK_DIRECTORIES "")
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/dinorunner
    CACHE PATH "Location of header files" )
set(SYSCONFIG_INSTALL_DIR ${CMAKE_INSTALL_SYSCONFDIR}/dinorunner
    CACHE PATH "Location of configuration files" )


configure_file(${PROJECT_SOURCE_DIR}/src/version.c.in ${PROJECT_SOURCE_DIR}/src/version.c)
file(GLOB DINORUNNER_CORE_FILES ${PROJECT_SOURCE_DIR}/src/*[.h.c])

include(GenerateExportHeader)

add_library(dinorunner_shared SHARED ${DINORUNNER_CORE_FILES})
add_library(dinorunner_static STATIC ${DINORUNNER_CORE_FILES})
target_include_directories(dinorunner_shared PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_include_directories(dinorunner_static PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_link_options(dinorunner_shared PUBLIC "-nostdlib")
target_link_options(dinorunner_static PUBLIC "-nostdlib")

set_target_properties(dinorunner_shared
    PROPERTIES OUTPUT_NAME dinorunner
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
set_target_properties(dinorunner_static
    PROPERTIES OUTPUT_NAME dinorunner
)

target_compile_options(dinorunner_shared PRIVATE -Wall -Wextra -Wpedantic -fstack-usage)

generate_export_header(dinorunner_shared)
include(GNUInstallDirs)
include(GenerateExportHeader)

install(TARGETS dinorunner_shared 
    EXPORT dinorunnerTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin 
    INCLUDES DESTINATION include
)

install(TARGETS dinorunner_static
    EXPORT dinorunnerTargets
    RUNTIME DESTINATION bin 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

install(FILES include/dinorunner.h
    DESTINATION include
    COMPONENT Devel)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/dinorunner/dinorunnerConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

export(EXPORT dinorunnerTargets 
    FILE "${CMAKE_CURRENT_BINARY_DIR}/dinorunner/dinorunnerTargets.cmake"
    NAMESPACE dinorunner::)

configure_file(cmake/dinorunnerConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/dinorunner/dinorunnerConfig.cmake" COPYONLY)
configure_package_config_file(cmake/dinorunnerConfig.cmake.in "${PROJECT_SOURCE_DIR}/cmake/dinorunnerConfig.cmake" 
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dinorunner 
    PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)

set(ConfigPackageLocation lib/cmake/dinorunner)
install(EXPORT dinorunnerTargets
    FILE dinorunnerTargets.cmake
    NAMESPACE dinorunner::
    DESTINATION ${ConfigPackageLocation})
install(FILES cmake/dinorunnerConfig.cmake 
    "${CMAKE_CURRENT_BINARY_DIR}/dinorunner/dinorunnerConfigVersion.cmake"
    DESTINATION ${ConfigPackageLocation}
    COMPONENT Devel)


cmake_policy(POP)
