/**
 * @file nightmode.c
 * 
 * @copyright Copyright (C) 2025 - All Rights Reserved 
 *  You may use, distribute and modify this code under the 
 *  terms of the GPL license.
 */

#include "dinorunner.h"

static float dinorunner_nightmode_updateXPos(float current_pos, float speed, unsigned container_width) {
  if (current_pos < -DINORUNNER_CONFIG_NIGHTMODE_WIDTH) {
    current_pos = container_width;
  } else {
    current_pos -= speed;
  }
  return current_pos;
}

static unsigned char dinorunner_nightmode_draw(const struct nightmode_s* nightmode, void* user_data) {
  int opacity = nightmode->opacity * 0xFF;
  if (opacity < 0) {
    opacity = 0;
  } else if (opacity > 0xFF) {
    opacity = 0xFF;
  }
  if (nightmode->draw_stars) {
    for (int i = 0; i < DINORUNNER_CONFIG_NIGHTMODE_NUMBSTARS; ++i) {
      struct pos_s pos              = {.x = dinorunner_roundf(nightmode->stars[i].x), nightmode->stars[i].y};
      enum dinorunner_sprite_e star = DINORUNNER_SPRITE_STAR1 + i;
      dinorunner_draw(star, &pos, (unsigned char)opacity, user_data);
    }
  }
  const struct pos_s pos        = {.x = dinorunner_roundf(nightmode->x_pos), nightmode->y_pos};
  enum dinorunner_sprite_e moon = DINORUNNER_SPRITE_MOON1 + nightmode->current_phase;
  dinorunner_draw(moon, &pos, (unsigned char)opacity, user_data);
  return 1u;
}

static void dinorunner_nightmode_placestars(struct nightmode_s* nightmode, unsigned container_width) {
  const unsigned kSegmentSize = dinorunner_roundf((float)container_width / DINORUNNER_CONFIG_NIGHTMODE_NUMBSTARS);
  for (unsigned i = 0; i < DINORUNNER_CONFIG_NIGHTMODE_NUMBSTARS; ++i) {
    struct star_s* star = &nightmode->stars[i];
    star->x             = dinorunner_getrandomnumb(kSegmentSize * i, kSegmentSize * (i + 1));
    star->y             = dinorunner_getrandomnumb(0, DINORUNNER_CONFIG_NIGHTMODE_STARMAXY);
  }
}

void dinorunner_nightmode_init(struct nightmode_s* nightmode, unsigned container_width) {
  nightmode->x_pos         = container_width - 50;
  nightmode->y_pos         = 30;
  nightmode->current_phase = 0u;
  nightmode->opacity       = 0;
  nightmode->draw_stars    = 0;
  dinorunner_nightmode_placestars(nightmode, container_width);
}

unsigned char dinorunner_nightmode_update(struct nightmode_s* nightmode, unsigned char show_nightmode,
                                          unsigned container_width, void* user_data) {
  if (show_nightmode && nightmode->opacity == 0) {
    nightmode->current_phase++;
    if (nightmode->current_phase >= DINORUNNER_CONFIG_NIGHTMODE_MOONPHASES) {
      nightmode->current_phase = 0;
    }
  }
  if (show_nightmode) {
    if ((nightmode->opacity < 1.0f || nightmode->opacity == 0.0f)) {
      nightmode->opacity += DINORUNNER_CONFIG_NIGHTMODE_FADESPEED;
    }
  } else if (nightmode->opacity > 0.0f) {
    nightmode->opacity -= DINORUNNER_CONFIG_NIGHTMODE_FADESPEED;
  }
  if (nightmode->opacity > 0) {
    nightmode->x_pos =
        dinorunner_nightmode_updateXPos(nightmode->x_pos, DINORUNNER_CONFIG_NIGHTMODE_MOONSPEED, container_width);
    if (nightmode->draw_stars) {
      for (unsigned i = 0; i < DINORUNNER_CONFIG_NIGHTMODE_NUMBSTARS; ++i) {
        nightmode->stars[i].x = dinorunner_nightmode_updateXPos(
            nightmode->stars[i].x, (float)DINORUNNER_CONFIG_NIGHTMODE_STARSPEED, container_width);
      }
    }
    dinorunner_nightmode_draw(nightmode, user_data);
  } else {
    nightmode->opacity = 0;
    dinorunner_nightmode_placestars(nightmode, container_width);
  }
  nightmode->draw_stars = 1u;
  return 1u;
}

void dinorunner_nightmode_reset(struct nightmode_s* nightmode, unsigned container_width, void* user_data) {
  nightmode->current_phase = 0;
  nightmode->opacity       = 0;
  dinorunner_nightmode_update(nightmode, 0, container_width, user_data);
}
