include(FetchContent)

set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG 6510d6ccbf446e058ae8ed95233f59057312df02
    GIT_PROGRESS TRUE
)
FetchContent_Declare(
    SDL2_IMAGE
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image
    GIT_TAG 770174c726a7dcf9bc506eaae6f0ea3e32172bbc
    GIT_PROGRESS TRUE
)
FetchContent_Declare(
    SDL_GFX
    GIT_REPOSITORY https://github.com/keera-studios/SDL2_gfx
    GIT_TAG master
    GIT_PROGRESS TRUE
)

SET(DINORUNNER_SDL_EXAMPLE_DEPENDENCIES "SDL2;SDL2_IMAGE;SDL_GFX")
SET(DINORUNNER_SDL_EXAMPLE_LIBARIES "SDL2::SDL2;SDL2_image::SDL2_image;sdl2_gfx")

FetchContent_MakeAvailable("${DINORUNNER_SDL_EXAMPLE_DEPENDENCIES}")

# SDL silently drops every backend whose development headers are missing, leaving a build
# that can neither open a window nor play sound. Point that out at configure time.
# SDL writes SDL_config.h itself only at generate time; the intermediate file exists after configure.
set(DINORUNNER_SDL_CONFIG_HEADER "${sdl2_BINARY_DIR}/SDL_config.h.intermediate")
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND EXISTS "${DINORUNNER_SDL_CONFIG_HEADER}")
    file(STRINGS "${DINORUNNER_SDL_CONFIG_HEADER}" DINORUNNER_SDL_AUDIO_BACKENDS
         REGEX "#define SDL_AUDIO_DRIVER_(ALSA|PULSEAUDIO|PIPEWIRE|JACK) ")
    file(STRINGS "${DINORUNNER_SDL_CONFIG_HEADER}" DINORUNNER_SDL_VIDEO_BACKENDS
         REGEX "#define SDL_VIDEO_DRIVER_(X11|WAYLAND) ")
    set(DINORUNNER_SDL_MISSING_BACKENDS "")
    if (NOT DINORUNNER_SDL_AUDIO_BACKENDS)
        list(APPEND DINORUNNER_SDL_MISSING_BACKENDS "audio (ALSA/PulseAudio/PipeWire)")
    endif()
    if (NOT DINORUNNER_SDL_VIDEO_BACKENDS)
        list(APPEND DINORUNNER_SDL_MISSING_BACKENDS "video (X11/Wayland)")
    endif()
    if (DINORUNNER_SDL_MISSING_BACKENDS)
        list(JOIN DINORUNNER_SDL_MISSING_BACKENDS " and " DINORUNNER_SDL_MISSING_BACKENDS)
        message(WARNING
            "The vendored SDL2 was configured without ${DINORUNNER_SDL_MISSING_BACKENDS} support. "
            "Install the development packages, e.g. on Debian/Ubuntu:\n"
            "  sudo apt install libasound2-dev libpulse-dev libx11-dev libxext-dev libwayland-dev "
            "libxkbcommon-dev libegl-dev\n"
            "then delete ${sdl2_BINARY_DIR} and reconfigure.")
    endif()
endif()

# SDL2_gfx has no CMake support: build the framerate module ourselves.
# Linking SDL2::SDL2 provides the SDL include paths (sdl*_SOURCE_DIR is only known after MakeAvailable).
add_library(sdl2_gfx STATIC "${sdl_gfx_SOURCE_DIR}/SDL2_framerate.c")
target_include_directories(sdl2_gfx PUBLIC "${sdl_gfx_SOURCE_DIR}")
target_link_libraries(sdl2_gfx PUBLIC SDL2::SDL2)
