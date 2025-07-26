include(FetchContent)

set(FETCHCONTENT_QUIET FALSE)
SET(DINORUNNER_SDL_EXAMPLE_DEPENDENCIES "SDL2;SDL2_IMAGE;SDL_GFX")

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

FetchContent_MakeAvailable("${DINORUNNER_SDL_EXAMPLE_DEPENDENCIES}")

add_library(sdl2_gfx STATIC "${sdl_gfx_SOURCE_DIR}/SDL2_framerate.c")

SET(DINORUNNER_SDL_EXAMPLE_INCLUDE_DIRS "${sdl2_SOURCE_DIR}/include;${sdl2_image_SOURCE_DIR}/include;${sdl_gfx_SOURCE_DIR}")
SET(DINORUNNER_SDL_EXAMPLE_LIBARIES "SDL2;SDL2::image;sdl2_gfx")
# FetchContent_MakeAvailable(SDL2 SDL2_IMAGE SDL_GFX)
 

