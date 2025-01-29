/**
 * @file demo.c
 * 
 * @copyright Copyright (C) 2025 - All Rights Reserved 
 *  You may use, distribute and modify this code under the 
 *  terms of the GPL license.
 */
#include <SDL.h>
#include <SDL2_framerate.h>
#include <SDL_image.h>
#include <SDL_joystick.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dinorunner.h"

#define LOG(format, ...) fprintf(stderr, "[%24s(%3d)] " format "\n", __FUNCTION__, __LINE__, __VA_ARGS__)
#define AUDIO_FILE_COUNT 3u

typedef struct dinorunner_s dinorunner_s;
static const uint32_t kPadding                 = 20;
static const char* kWindowName                 = "dinorunner demo";
static const struct dimension_s kGameDimension = {.width = 600, .height = 150};
static const char* highscore_filename          = "dinorunner_highscore.txt";
static const char* sprite_filename             = "demo/assets/100-offline-sprite.png";
static const char* die_sound_file              = "demo/assets/die.wav";
static const char* jump_sound_file             = "demo/assets/jump.wav";
static const char* point_sound_file            = "demo/assets/point.wav";
static const SDL_Rect game_rect = {.x = kPadding, .y = kPadding, .w = kGameDimension.width, .h = kGameDimension.height};
static const SDL_Rect kCactusLargeSprite1    = {.x = 332, .y = 2, .w = 25, .h = 50};
static const SDL_Rect kCactusLargeSprite2    = {.x = 357, .y = 2, .w = 50, .h = 50};
static const SDL_Rect kCactusLargeSprite3    = {.x = 407, .y = 2, .w = 75, .h = 50};
static const SDL_Rect kCactusSmallSprite1    = {.x = 228, .y = 2, .w = 17, .h = 35};
static const SDL_Rect kCactusSmallSprite2    = {.x = 245, .y = 2, .w = 34, .h = 35};
static const SDL_Rect kCactusSmallSprite3    = {.x = 279, .y = 2, .w = 51, .h = 35};
static const SDL_Rect kPterodactylSprite1    = {.x = 134, .y = 2, .w = 46, .h = 40};
static const SDL_Rect kPterodactylSprite2    = {.x = 180, .y = 2, .w = 46, .h = 40};
static const SDL_Rect kCloudSprite           = {.x = 86, .y = 2, .w = 46, .h = 14};
static const SDL_Rect kHorizonSprite1        = {.x = 2, .y = 54, .w = 600, .h = 12};
static const SDL_Rect kHorizonSprite2        = {.x = 602, .y = 54, .w = 600, .h = 12};
static const SDL_Rect kMoonSprite            = {.x = 484, .y = 2, .w = 20, .h = 40};
static const SDL_Rect kRestartSprite         = {.x = 2, .y = 2, .w = 36, .h = 32};
static const SDL_Rect kTextSprite            = {.x = 655, .y = 2, .w = 191, .h = 11};
static const SDL_Rect kTrexSpriteStanding1   = {.x = 848, .y = 2, .w = 44, .h = 47};
static const SDL_Rect kTrexSpriteStanding2   = {.x = 892, .y = 2, .w = 44, .h = 47};
static const SDL_Rect kTrexSpriteRunning1    = {.x = 936, .y = 2, .w = 44, .h = 47};
static const SDL_Rect kTrexSpriteRunning2    = {.x = 980, .y = 2, .w = 44, .h = 47};
static const SDL_Rect kTrexSpriteJumping     = {.x = 848, .y = 2, .w = 44, .h = 47};
static const SDL_Rect kTrexSpriteCrashed     = {.x = 1068, .y = 2, .w = 44, .h = 47};
static const SDL_Rect kTrexSpriteDucking1    = {.x = 1112, .y = 2, .w = 59, .h = 47};
static const SDL_Rect kTrexSpriteDucking2    = {.x = 1171, .y = 2, .w = 59, .h = 47};
static const SDL_Rect kStarSprite            = {.x = 645, .y = 2, .w = 8, .h = 9};
static const SDL_Rect kHiSprite              = {.x = 754, .y = 2, .w = 20, .h = 13};
static const SDL_Rect k0Sprite               = {.x = 654, .y = 2, .w = 10, .h = 13};
static const SDL_Rect KGameoverRestartSprite = {.x = 2, .y = 2, .w = 36, .h = 32};
static const SDL_Rect KGameoverTextSprite    = {.x = 654, .y = 15, .w = 191, .h = 11};
static const uint32_t kWindowHeight          = kGameDimension.height + kPadding * 2;
static const uint32_t kWindowWidth           = kGameDimension.width + kPadding * 2;
static const uint32_t kFrameRate             = 60u;
static const uint8_t kBackgroundColor        = 0xF2;
static const uint16_t kRumbleFrequency       = 0xFFFF;
static const unsigned char kNightModePhases[DINORUNNER_CONFIG_NIGHTMODE_MOONPHASES] = {140, 120, 100, 60, 40, 20, 0};

typedef struct audio_s {
  uint8_t* pos;
  uint32_t length;
  SDL_AudioSpec spec;
  int audio_device_id;
} audio_s;

typedef struct hypervisor_s {
  dinorunner_s dinorunner;
  FPSmanager fps_manager;
  audio_s sound_effect[AUDIO_FILE_COUNT];
  FILE* highscore_store;
  SDL_Window* g_window;
  SDL_Surface* g_screen_surface;
  SDL_Renderer* g_renderer;
  SDL_Texture* g_sprite;
  SDL_Texture* g_background;
  SDL_Surface* g_image;
  SDL_Joystick* joystick;
} hypervisor_s;

static uint8_t system_preinit(hypervisor_s* hypervisor);
static uint8_t system_init(hypervisor_s* hypervisor);
static uint8_t system_run(hypervisor_s* hypervisor);
static uint8_t system_exit(hypervisor_s* hypervisor);

int main(int argc, char** argv) {
  hypervisor_s hypervisor;
  uint8_t status = system_preinit(&hypervisor);
  status &= system_init(&hypervisor);
  status &= system_run(&hypervisor);
  status &= system_exit(&hypervisor);
  return !status;
}

static uint8_t system_preinit(hypervisor_s* hypervisor) {
  hypervisor->highscore_store  = NULL;
  hypervisor->g_window         = NULL;
  hypervisor->g_screen_surface = NULL;
  hypervisor->g_renderer       = NULL;
  hypervisor->g_sprite         = NULL;
  hypervisor->g_image          = NULL;
  hypervisor->g_background     = NULL;
  hypervisor->joystick         = NULL;
  return 1u;
}

static uint8_t render_background(hypervisor_s* hypervisor) {
  hypervisor->g_background = SDL_CreateTexture(hypervisor->g_renderer, SDL_PIXELFORMAT_ARGB8888,
                                               SDL_TEXTUREACCESS_TARGET, kWindowWidth, kWindowHeight);
  SDL_SetTextureBlendMode(hypervisor->g_background, SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(hypervisor->g_renderer, hypervisor->g_background);
  int status;
  status = SDL_RenderClear(hypervisor->g_renderer);
  status = SDL_SetRenderDrawColor(hypervisor->g_renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
  // status = SDL_SetRenderDrawColor(hypervisor->g_renderer, kBackgroundColor, kBackgroundColor, kBackgroundColor,
  //                                 SDL_ALPHA_OPAQUE);
  SDL_assert(status == 0);
  // SDL_RenderFillRect(hypervisor->g_renderer, &game_rect);
  SDL_RenderFillRect(hypervisor->g_renderer, NULL);
  SDL_RenderPresent(hypervisor->g_renderer);
  return 1u;
}

/**
 * @brief Load given sprite image to the GPU
 * 
 * @warning The image used in the official Chrome's T-Rex is 
 * a Grayscale PNG image, in which each pixel is comprised of 
 * 2 bytes, the alpha and the grey scale value. 
 * This routine tries to detect the number of bytes per 
 * pixel. In case of two, it converts the image into its RGB
 * representation as SDL doesn't support grayscale images.
 * Otherwise the image is loaded as normal.
 */
static uint8_t load_sprite(hypervisor_s* hypervisor) {
  const int img_flags = IMG_INIT_PNG;
  int status          = IMG_Init(img_flags) & img_flags;
  SDL_assert(status == img_flags);
  SDL_Surface* image = IMG_Load(sprite_filename);
  SDL_assert(image != NULL);
  if (image->format->BytesPerPixel == 2u) {
    LOG("%s", "Detected grayscale sprite image. Converting to RGB ...");
    SDL_Texture* my_texture = SDL_CreateTexture(hypervisor->g_renderer, SDL_PIXELFORMAT_ARGB8888,
                                                SDL_TEXTUREACCESS_STREAMING, image->w, image->h);
    unsigned char* pixel;
    int pitch = 0;
    SDL_LockTexture(my_texture, NULL, (void**)&pixel, &pitch);
    SDL_LockSurface(image);
    unsigned char* sprite_pixels = image->pixels;
    for (int i = 0; i < image->h; ++i) {
      for (int j = 0, k = 0, l = 0; l < image->w; j += image->format->BytesPerPixel, k += 4, l++) {
        int source_pos        = (i * image->pitch) + j;
        int target_pos        = (i * pitch) + k;
        pixel[target_pos + 0] = sprite_pixels[source_pos + 0];
        pixel[target_pos + 1] = sprite_pixels[source_pos + 0];
        pixel[target_pos + 2] = sprite_pixels[source_pos + 0];
        pixel[target_pos + 3] = sprite_pixels[source_pos + 1];
      }
    }
    SDL_UnlockTexture(my_texture);
    SDL_UnlockSurface(image);
    hypervisor->g_sprite = my_texture;
  } else if (image->format->BytesPerPixel == 4u) {
    LOG("%s", "Detected RGB image. No conversion required.");
    hypervisor->g_sprite = IMG_LoadTexture(hypervisor->g_renderer, sprite_filename);
  } else {
    LOG("%s", "Could not process sprite image!");
    return 0;
  }
  SDL_FreeSurface(image);
  SDL_assert(hypervisor->g_sprite);
  SDL_SetTextureBlendMode(hypervisor->g_sprite, SDL_BLENDMODE_BLEND);
  return 1u;
}

static uint8_t load_sounds(hypervisor_s* hypervisor) {
  const char* audio_file_path[AUDIO_FILE_COUNT] = {jump_sound_file, point_sound_file, die_sound_file};
  for (unsigned i = 0; i < AUDIO_FILE_COUNT; ++i) {
    uint32_t wav_length    = 0;
    uint8_t* wav_buffer    = NULL;
    audio_s* current_audio = &hypervisor->sound_effect[i];
    SDL_AudioSpec* result  = SDL_LoadWAV(audio_file_path[i], &current_audio->spec, &wav_buffer, &wav_length);
    if (result == NULL) {
      LOG("%s %s", "Error loading sound", audio_file_path[i]);
      return 0u;
    }
    current_audio->spec.callback = NULL;
    current_audio->spec.userdata = NULL;
    current_audio->spec.channels = 2;
    current_audio->pos           = wav_buffer;
    current_audio->length        = wav_length;
    current_audio->audio_device_id =
        SDL_OpenAudioDevice(NULL, 0, &current_audio->spec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (current_audio->audio_device_id <= 0) {
      LOG("%s: %s", "Could not open audio device", SDL_GetError());
      return 0u;
    }
  }
  return 1u;
}

static uint8_t load_scorefile(hypervisor_s* hypervisor) {
  hypervisor->highscore_store = fopen(highscore_filename, "a+");
  if (hypervisor->highscore_store == NULL) {
    LOG("Could not open: %s", highscore_filename);
    return 0u;
  }
  fclose(hypervisor->highscore_store);
  hypervisor->highscore_store = fopen(highscore_filename, "r+");
  if (hypervisor->highscore_store == NULL) {
    LOG("Could not open: %s", highscore_filename);
    return 0u;
  }
  rewind(hypervisor->highscore_store);
  return 1u;
}

static void unload_joystick(hypervisor_s* hypervisor) {
  LOG("%s", "Unloading joysticks");
  if (hypervisor->joystick != NULL) {
    SDL_Joystick* joystick = hypervisor->joystick;
    if (SDL_JoystickGetAttached(joystick)) {
      const char* joystick_name = SDL_JoystickName(joystick);
      LOG("Closing joystick: %s", joystick_name);
      SDL_JoystickClose(joystick);
    }
  }
}

static uint8_t load_joystick(hypervisor_s* hypervisor) {
  LOG("%s", "Loading joystick");
  unload_joystick(hypervisor);
  int numb_joystick = SDL_NumJoysticks();
  LOG("Found %d joystick(s)", numb_joystick);
  if (numb_joystick == 0) {
    LOG("%s", "No joysticks found");
    return 0u;
  }
  if (numb_joystick < 0) {
    LOG("Error while getting joysticks: %s", SDL_GetError());
    return 0u;
  }
  for (int i = 0; i < numb_joystick; ++i) {
    SDL_Joystick* joystick = SDL_JoystickOpen(i);
    if (joystick == NULL) {
      LOG("%s", "Could not load joystick for vibration");
      continue;
    }
    const char* name = SDL_JoystickName(joystick);
    if (name == NULL) {
      name = "Unknown";
    }
    int rumble_status    = SDL_JoystickHasRumble(joystick);
    hypervisor->joystick = joystick;
    int event_state      = SDL_JoystickEventState(SDL_ENABLE);
    LOG("Found controller: %s. Rumble detected: %i. Enable_event: %i", name, rumble_status, event_state);
    return 1u;
  }
  hypervisor->joystick = NULL;
  return 0u;
}

static uint8_t system_init(hypervisor_s* hypervisor) {
  struct version_s version           = {0, 0, 0};
  const unsigned char kVersionResult = dinorunner_getversion(&version);
  if (!kVersionResult) {
    return 0u;
  }
  LOG("Compiled with libdinorunner v%hu.%hu.%hu", version.major, version.minor, version.patch);
  SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
  SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");
  SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5, "1");
  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    LOG("SDL could not initialize! SDL_Error: %s", SDL_GetError());
    return 0u;
  }
  LOG("Size of dinorunner structure: %u", (unsigned)sizeof(hypervisor->dinorunner));
  hypervisor->g_window = SDL_CreateWindow(kWindowName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, kWindowWidth,
                                          kWindowHeight, SDL_WINDOW_SHOWN);
  if (hypervisor->g_window == NULL) {
    LOG("SDL Window could not be created: %s", SDL_GetError());
    return 0u;
  }
  hypervisor->g_renderer = SDL_CreateRenderer(hypervisor->g_window, -1, SDL_RENDERER_PRESENTVSYNC);
  if (!hypervisor->g_renderer) {
    LOG("SDL Renderer could not be created: %s", SDL_GetError());
    return 0u;
  }
  render_background(hypervisor);
  load_sprite(hypervisor);
  SDL_SetRenderTarget(hypervisor->g_renderer, NULL);
  int status = SDL_RenderCopy(hypervisor->g_renderer, hypervisor->g_background, NULL, NULL);
  SDL_assert(status == 0);
  SDL_RenderPresent(hypervisor->g_renderer);
  SDL_UpdateWindowSurface(hypervisor->g_window);
  SDL_assert(status == 0);
  status = load_sounds(hypervisor);
  SDL_assert(status == 1);
  load_scorefile(hypervisor);
  SDL_initFramerate(&hypervisor->fps_manager);
  SDL_setFramerate(&hypervisor->fps_manager, kFrameRate);
  status = dinorunner_init(&hypervisor->dinorunner, &kGameDimension, hypervisor);
  return status;
}

static uint8_t system_run(hypervisor_s* hypervisor) {
  uint8_t quit = 0u;
  SDL_Event event;
  unsigned char game_result    = 0u;
  const int16_t kAxisThreshold = 0xFFF;
  while (!quit) {
    while (SDL_PollEvent(&event) != 0) {
      switch (event.type) {
        case SDL_QUIT:
          quit = 1u;
          break;
        case SDL_JOYDEVICEADDED:
        case SDL_JOYDEVICEREMOVED:
          load_joystick(hypervisor);
          break;
      }
    }
    SDL_Joystick* joystick           = hypervisor->joystick;
    uint8_t button_state             = SDL_JoystickGetButton(hypervisor->joystick, 2);
    int16_t axis_state               = SDL_JoystickGetAxis(hypervisor->joystick, 1);
    uint8_t hat_state                = SDL_JoystickGetHat(joystick, 0);
    const uint8_t* current_key_state = SDL_GetKeyboardState(NULL);
    uint8_t up_pressed =
        (current_key_state[SDL_SCANCODE_UP] || current_key_state[SDL_SCANCODE_SPACE] || (hat_state == SDL_HAT_UP)) ||
        (button_state == 1u) || (axis_state < -kAxisThreshold);
    uint8_t down_pressed = current_key_state[SDL_SCANCODE_DOWN] || event.type == SDL_JOYBUTTONDOWN ||
                           (hat_state == SDL_HAT_DOWN) || (axis_state > kAxisThreshold);
    if (up_pressed) {
      dinorunner_onkeyup(&hypervisor->dinorunner);
    } else if (down_pressed) {
      dinorunner_onkeydown(&hypervisor->dinorunner);
    } else if (current_key_state[SDL_SCANCODE_ESCAPE]) {
      quit = 1u;
    } else {
      dinorunner_onkeynone(&hypervisor->dinorunner);
    }
    game_result = dinorunner_update(&hypervisor->dinorunner);
    if (game_result != 1u) {
      quit = 1u;
    }
    SDL_framerateDelay(&hypervisor->fps_manager);
  }
  return 1u;
}

static uint8_t system_exit(hypervisor_s* hypervisor) {
  LOG("%s", "Exiting hypervisor");
  if (hypervisor->g_window) {
    SDL_DestroyWindow(hypervisor->g_window);
  }
  if (hypervisor->g_renderer) {
    SDL_DestroyRenderer(hypervisor->g_renderer);
  }
  IMG_Quit();
  unload_joystick(hypervisor);
  for (unsigned i = 0; i < sizeof(hypervisor->sound_effect) / sizeof(*hypervisor->sound_effect); ++i) {
    audio_s* sound_effect = &hypervisor->sound_effect[i];
    SDL_CloseAudioDevice(sound_effect->audio_device_id);
    if (sound_effect->pos != NULL) {
      SDL_FreeWAV(sound_effect->pos);
    }
  }
  SDL_Quit();
  if (hypervisor->highscore_store) {
    fclose(hypervisor->highscore_store);
  }
  return 1u;
}

unsigned char dinorunner_writehighscore(unsigned long high_score, void* user_data) {
  LOG("Writing highscore: %lu", high_score);
  if (user_data == NULL) {
    return 0u;
  }
  const hypervisor_s* hypervisor = (const hypervisor_s*)user_data;
  rewind(hypervisor->highscore_store);
  int seek_result = fseek(hypervisor->highscore_store, 0, SEEK_SET);
  if (seek_result < 0) {
    LOG("%s", "Could not seek");
    return 0u;
  }
  int print_result = fprintf(hypervisor->highscore_store, "%05lu", high_score);
  if (print_result <= 0) {
    LOG("Could not write: %lu. %s", high_score, strerror(errno));
    return 0u;
  }
  if (fflush(hypervisor->highscore_store) != 0) {
    LOG("Could not flush data to file: %s", strerror(errno));
    return 0u;
  }
  LOG("Highscore: %lu successfully written!", high_score);
  return 1u;
}

unsigned char dinorunner_readhighscore(unsigned long* high_score, void* user_data) {
  LOG("%s", "Reading Highscore");
  if (user_data == NULL) {
    return 0u;
  }
  const hypervisor_s* hypervisor = (const hypervisor_s*)user_data;
  rewind(hypervisor->highscore_store);
  int seek_result = fseek(hypervisor->highscore_store, 0, SEEK_SET);
  if (seek_result < 0) {
    LOG("%s", "Could not seek");
    return 0u;
  }
  unsigned long stored_highscore = 0;
  int scan_result                = fscanf(hypervisor->highscore_store, "%05lu", &stored_highscore);
  if (scan_result <= 0) {
    LOG("Could not read highscore: %s", strerror(errno));
    return 0u;
  }
  *high_score = stored_highscore;
  LOG("High score: %lu successfully read", stored_highscore);
  return 1u;
}

unsigned long dinorunner_gettimestamp(void* user_data) {
  (void)user_data;
  return (unsigned long)SDL_GetTicks();
}

unsigned char dinorunner_playsound(enum dinorunner_sound_e sound, void* user_data) {
  hypervisor_s* hypervisor = (hypervisor_s*)user_data;
  if (hypervisor == NULL) {
    return 0u;
  }
  int sound_index = sound - DINORUNNER_SOUND_BUTTON_PRESS;
  if (sound_index < 0 || sound_index >= AUDIO_FILE_COUNT) {
    LOG("%s: %d", "Invalid sound play request", (int)sound);
    return 0u;
  }
  audio_s* audio = &hypervisor->sound_effect[sound_index];
  SDL_QueueAudio(audio->audio_device_id, audio->pos, audio->length);
  SDL_PauseAudioDevice(audio->audio_device_id, 0);
  return 1u;
}

unsigned char dinorunner_vibrate(unsigned duration, void* user_data) {
  hypervisor_s* hypervisor = (hypervisor_s*)user_data;
  if (hypervisor == NULL) {
    return 0u;
  }
  if (hypervisor->joystick == NULL) {
    return 0u;
  }
  if (duration == 0) {
    return 1u;
  }
  int result = SDL_JoystickRumble(hypervisor->joystick, kRumbleFrequency, kRumbleFrequency, duration);
  return result != -1;
}

unsigned char dinorunner_clearcanvas(void* user_data) {
  hypervisor_s* hypervisor = (hypervisor_s*)user_data;
  if (hypervisor == NULL) {
    return 0u;
  }
  SDL_RenderPresent(hypervisor->g_renderer);
  int clear_result = SDL_RenderClear(hypervisor->g_renderer);
  if (clear_result != 0) {
    LOG("Could not clear canvas: %s", SDL_GetError());
    return 0u;
  }
  return 1u;
}

unsigned char dinorunner_draw(enum dinorunner_sprite_e sprite, const struct pos_s* pos, unsigned char opacity,
                              void* user_data) {
  if (user_data == NULL) {
    return 0u;
  }
  hypervisor_s* hypervisor = (hypervisor_s*)user_data;
  SDL_Rect destination_rect;
  destination_rect.x          = pos->x + kPadding;
  destination_rect.y          = pos->y + kPadding;
  const SDL_Rect* sprite_rect = NULL;
  SDL_Rect temp_target;
  switch (sprite) {
    case DINORUNNER_SPRITE_TREX_STANDING1:
      destination_rect.w = kTrexSpriteStanding1.w;
      destination_rect.h = kTrexSpriteStanding1.h;
      sprite_rect        = &kTrexSpriteStanding1;
      break;
    case DINORUNNER_SPRITE_TREX_STANDING2:
      destination_rect.w = kTrexSpriteStanding2.w;
      destination_rect.h = kTrexSpriteStanding2.h;
      sprite_rect        = &kTrexSpriteStanding2;
      break;
    case DINORUNNER_SPRITE_TREX_RUNNING1:
      destination_rect.w = kTrexSpriteRunning1.w;
      destination_rect.h = kTrexSpriteRunning1.h;
      sprite_rect        = &kTrexSpriteRunning1;
      break;
    case DINORUNNER_SPRITE_TREX_RUNNING2:
      destination_rect.w = kTrexSpriteRunning2.w;
      destination_rect.h = kTrexSpriteRunning2.h;
      sprite_rect        = &kTrexSpriteRunning2;
      break;
    case DINORUNNER_SPRITE_TREX_DUCKING1:
      destination_rect.w = kTrexSpriteDucking1.w;
      destination_rect.h = kTrexSpriteDucking1.h;
      sprite_rect        = &kTrexSpriteDucking1;
      break;
    case DINORUNNER_SPRITE_TREX_DUCKING2:
      destination_rect.w = kTrexSpriteDucking2.w;
      destination_rect.h = kTrexSpriteDucking2.h;
      sprite_rect        = &kTrexSpriteDucking2;
      break;
    case DINORUNNER_SPRITE_TREX_JUMPING:
      destination_rect.w = kTrexSpriteJumping.w;
      destination_rect.h = kTrexSpriteJumping.h;
      sprite_rect        = &kTrexSpriteJumping;
      break;
    case DINORUNNER_SPRITE_TREX_CRASHED:
      destination_rect.w = kTrexSpriteCrashed.w;
      destination_rect.h = kTrexSpriteCrashed.h;
      sprite_rect        = &kTrexSpriteCrashed;
      break;
    case DINORUNNER_SPRITE_HORIZON1:
      destination_rect.w = kHorizonSprite1.w;
      destination_rect.h = kHorizonSprite1.h;
      destination_rect.y -= kPadding;
      sprite_rect = &kHorizonSprite1;
      break;
    case DINORUNNER_SPRITE_HORIZON2:
      destination_rect.w = kHorizonSprite2.w;
      destination_rect.h = kHorizonSprite2.h;
      destination_rect.y -= kPadding;
      sprite_rect = &kHorizonSprite2;
      break;
    case DINORUNNER_SPRITE_CLOUD:
      destination_rect.w = kCloudSprite.w;
      destination_rect.h = kCloudSprite.h;
      destination_rect.y -= kPadding;
      sprite_rect = &kCloudSprite;
      break;
    case DINORUNNER_SPRITE_HI:
      destination_rect.w = kHiSprite.w;
      destination_rect.h = kHiSprite.h;
      sprite_rect        = &kHiSprite;
      break;
    case DINORUNNER_SPRITE_0:
    case DINORUNNER_SPRITE_1:
    case DINORUNNER_SPRITE_2:
    case DINORUNNER_SPRITE_3:
    case DINORUNNER_SPRITE_4:
    case DINORUNNER_SPRITE_5:
    case DINORUNNER_SPRITE_6:
    case DINORUNNER_SPRITE_7:
    case DINORUNNER_SPRITE_8:
    case DINORUNNER_SPRITE_9:
      destination_rect.w = k0Sprite.w;
      destination_rect.h = k0Sprite.h;
      temp_target.w      = k0Sprite.w;
      temp_target.h      = k0Sprite.h;
      temp_target.x      = k0Sprite.x + k0Sprite.w * (sprite - DINORUNNER_SPRITE_0);
      temp_target.y      = k0Sprite.y;
      sprite_rect        = &temp_target;
      break;
    case DINORUNNER_SPRITE_CACTUSSMALL1:
      destination_rect.w = kCactusSmallSprite1.w;
      destination_rect.h = kCactusSmallSprite1.h;
      sprite_rect        = &kCactusSmallSprite1;
      break;
    case DINORUNNER_SPRITE_CACTUSSMALL2:
      destination_rect.w = kCactusSmallSprite2.w;
      destination_rect.h = kCactusSmallSprite2.h;
      sprite_rect        = &kCactusSmallSprite2;
      break;
    case DINORUNNER_SPRITE_CACTUSSMALL3:
      destination_rect.w = kCactusSmallSprite3.w;
      destination_rect.h = kCactusSmallSprite3.h;
      sprite_rect        = &kCactusSmallSprite3;
      break;
    case DINORUNNER_SPRITE_CACTUSLARGE1:
      destination_rect.w = kCactusLargeSprite1.w;
      destination_rect.h = kCactusLargeSprite1.h;
      sprite_rect        = &kCactusLargeSprite1;
      break;
    case DINORUNNER_SPRITE_CACTUSLARGE2:
      destination_rect.w = kCactusLargeSprite2.w;
      destination_rect.h = kCactusLargeSprite2.h;
      sprite_rect        = &kCactusLargeSprite2;
      break;
    case DINORUNNER_SPRITE_CACTUSLARGE3:
      destination_rect.w = kCactusLargeSprite3.w;
      destination_rect.h = kCactusLargeSprite3.h;
      sprite_rect        = &kCactusLargeSprite3;
      break;
    case DINORUNNER_SPRITE_PTERODACTYL1:
      destination_rect.w = kPterodactylSprite1.w;
      destination_rect.h = kPterodactylSprite1.h;
      sprite_rect        = &kPterodactylSprite1;
      break;
    case DINORUNNER_SPRITE_PTERODACTYL2:
      destination_rect.w = kPterodactylSprite2.w;
      destination_rect.h = kPterodactylSprite2.h;
      sprite_rect        = &kPterodactylSprite2;
      break;
    case DINORUNNER_SPRITE_GAMEOVER:
      destination_rect.w = KGameoverTextSprite.w;
      destination_rect.h = KGameoverTextSprite.h;
      sprite_rect        = &KGameoverTextSprite;
      break;
    case DINORUNNER_SPRITE_RESTART:
      destination_rect.w = KGameoverRestartSprite.w;
      destination_rect.h = KGameoverRestartSprite.h;
      sprite_rect        = &KGameoverRestartSprite;
      break;
    case DINORUNNER_SPRITE_STAR1:
    case DINORUNNER_SPRITE_STAR2:
    case DINORUNNER_SPRITE_STAR3:
      destination_rect.w = kStarSprite.w;
      destination_rect.h = kStarSprite.h;
      temp_target.w      = kStarSprite.w;
      temp_target.h      = kStarSprite.h;
      temp_target.x      = kStarSprite.x;
      temp_target.y      = kStarSprite.y + ((sprite - DINORUNNER_SPRITE_STAR1) * temp_target.h);
      sprite_rect        = &temp_target;
      break;
    case DINORUNNER_SPRITE_MOON1:
    case DINORUNNER_SPRITE_MOON2:
    case DINORUNNER_SPRITE_MOON3:
    case DINORUNNER_SPRITE_MOON4:
    case DINORUNNER_SPRITE_MOON5:
    case DINORUNNER_SPRITE_MOON6:
    case DINORUNNER_SPRITE_MOON7: {
      unsigned moon_phase = (sprite - DINORUNNER_SPRITE_MOON1);
      destination_rect.w  = (sprite == DINORUNNER_SPRITE_MOON4) ? kMoonSprite.w * 2 : kMoonSprite.w;
      destination_rect.h  = kMoonSprite.h;
      temp_target.w       = destination_rect.w;
      temp_target.h       = destination_rect.h;
      temp_target.x       = kMoonSprite.x + kNightModePhases[moon_phase];
      temp_target.y       = kMoonSprite.y;
      sprite_rect         = &temp_target;
    } break;
    default:
      LOG("Request to draw invalid sprite: %d at position: (%d, %d)", (int)sprite, destination_rect.x,
          destination_rect.y);
      return 0u;
  }
  SDL_SetTextureAlphaMod(hypervisor->g_sprite, opacity);
  SDL_RenderCopy(hypervisor->g_renderer, hypervisor->g_sprite, sprite_rect, &destination_rect);
  return 1u;
}

unsigned char dinorunner_log(void* user_data, const char* format, ...) {
  (void)user_data;
  char buffer[256];
  va_list args;
  va_start(args, format);
  int ret = vsprintf(buffer, format, args);
  fprintf(stderr, "%s", buffer);
  va_end(args);
  return ret != 0;
}
