/**
 * @file platform.h
 * @brief Calls into the application's callbacks, see struct dinorunner_callbacks_s
 *
 * @copyright Copyright (C) 2026 - All Rights Reserved
 *  You may use, distribute and modify this code under the
 *  terms of the GPL license.
 */

#ifndef DINORUNNER_PLATFORM_H
#define DINORUNNER_PLATFORM_H

#include "dinorunner.h"

/**
 * @brief Store the callbacks and user data of a game
 *
 * @return unsigned char 1 on success, 0 when callbacks is NULL or a required callback is missing
 */
unsigned char dinorunner_platform_init(struct dinorunner_platform_s* platform,
                                       const struct dinorunner_callbacks_s* callbacks, void* user_data);

/**
 * @brief Format a message printf-like and pass it to the log callback, if any.
 * Supports %d %i %u %x %c %s %% with an optional 'l' length modifier. Longer messages
 * than 255 characters are truncated.
 */
unsigned char dinorunner_platform_log(const struct dinorunner_platform_s* platform, const char* format, ...);

static inline unsigned long dinorunner_platform_gettimestamp(const struct dinorunner_platform_s* platform) {
  return platform->callbacks.gettimestamp(platform->user_data);
}

static inline unsigned char dinorunner_platform_clearcanvas(const struct dinorunner_platform_s* platform) {
  return platform->callbacks.clearcanvas(platform->user_data);
}

static inline unsigned char dinorunner_platform_draw(enum dinorunner_sprite_e sprite, const struct pos_s* pos,
                                                     unsigned char opacity,
                                                     const struct dinorunner_platform_s* platform) {
  return platform->callbacks.draw(sprite, pos, opacity, platform->user_data);
}

static inline unsigned char dinorunner_platform_playsound(enum dinorunner_sound_e sound,
                                                          const struct dinorunner_platform_s* platform) {
  return platform->callbacks.playsound ? platform->callbacks.playsound(sound, platform->user_data) : 0u;
}

static inline unsigned char dinorunner_platform_vibrate(unsigned duration,
                                                        const struct dinorunner_platform_s* platform) {
  return platform->callbacks.vibrate ? platform->callbacks.vibrate(duration, platform->user_data) : 0u;
}

static inline unsigned char dinorunner_platform_readhighscore(unsigned long* high_score,
                                                              const struct dinorunner_platform_s* platform) {
  return platform->callbacks.readhighscore ? platform->callbacks.readhighscore(high_score, platform->user_data)
                                           : 0u;
}

static inline unsigned char dinorunner_platform_writehighscore(unsigned long high_score,
                                                               const struct dinorunner_platform_s* platform) {
  return platform->callbacks.writehighscore ? platform->callbacks.writehighscore(high_score, platform->user_data)
                                            : 0u;
}

#endif  // DINORUNNER_PLATFORM_H
