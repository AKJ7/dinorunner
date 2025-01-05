/**
 * @file distance_meter.c
 * 
 * @copyright Copyright (C) 2025 - All Rights Reserved 
 *  You may use, distribute and modify this code under the 
 *  terms of the GPL license.
 */

#include "dinorunner.h"

unsigned dinorunner_distancemeter_getactualdistance(unsigned distance) {
  return distance ? dinorunner_roundf(distance * DINORUNNER_CONFIG_DISTANCEMETER_COEFFICIENT) : 0;
}

static unsigned long distancemeter_readhighscore(void* user_data) {
  unsigned long high_score  = 0;
  unsigned char read_result = dinorunner_readhighscore(&high_score, user_data);
  if (read_result != 0) {
    return high_score;
  }
  return 0u;
}

void dinorunner_distancemeter_writehighscore(struct distance_meter_s* distance_meter, unsigned distance,
                                             void* user_data) {
  unsigned char read_result = dinorunner_writehighscore(distance, user_data);
  if (read_result != 0) {
    if (distance_meter->high_score < distance) {
      distance_meter->high_score = distance;
    }
  }
}

static unsigned long_to_digit(unsigned long number, char* data, unsigned max_size) {
  unsigned digit_parsed;
  unsigned long rest = number;
  for (digit_parsed = 0; digit_parsed < max_size; ++digit_parsed) {
    unsigned divisor = dinorunner_pow(10, (max_size - digit_parsed - 1));
    int current      = rest / divisor;
    if ((rest != 0)) {
      rest    = rest - current * divisor;
      current = (int)current;
    } else {
      current = 0;
    }
    current            = current + '0';
    data[digit_parsed] = current;
  }
  data[+digit_parsed] = 0;
  return digit_parsed;
}

static void dinorunner_distancemeter_calcxpos(struct distance_meter_s* distance_meter, unsigned canvas_width) {
  distance_meter->x = (int)canvas_width - (int)((distance_meter->width * distance_meter->max_score_units + 1)) * 2;
}

unsigned char dinorunner_distancemeter_draw(const struct distance_meter_s* distance_meter, unsigned digit_pos,
                                            unsigned value, unsigned short opt_high, void* user_data) {
  TREX_UNUSED(digit_pos);
  char buffer[24];
  char* ptr = buffer;
  if (opt_high) {
    struct pos_s pos = {.x = distance_meter->x - distance_meter->width * distance_meter->max_score_units,
                        .y = distance_meter->y};
    dinorunner_draw(DINORUNNER_SPRITE_HI, &pos, user_data);
    unsigned parsed = long_to_digit(distance_meter->high_score, ptr, distance_meter->max_score_units);
    for (unsigned counter = 0; counter < parsed; ++counter) {
      struct pos_s pos = {.x = distance_meter->x - distance_meter->width * 2 + distance_meter->width * counter,
                          .y = distance_meter->y};
      dinorunner_draw(DINORUNNER_SPRITE_0 + (int)(ptr[counter] - '0'), &pos, user_data);
    }
  } else {
    unsigned parsed = long_to_digit(value, ptr, distance_meter->max_score_units);
    for (unsigned counter = 0; counter < parsed; ++counter) {
      struct pos_s pos = {.x = distance_meter->x + distance_meter->width * 2 + distance_meter->width * counter +
                               distance_meter->width * 2,
                          .y = distance_meter->y};
      dinorunner_draw(DINORUNNER_SPRITE_0 + (int)(ptr[counter] - '0'), &pos, user_data);
    }
  }
  return 1u;
}

void dinorunner_distancemeter_drawhighscore(const struct distance_meter_s* distance_meter, void* user_data) {
  dinorunner_distancemeter_draw(distance_meter, 0, distance_meter->high_score, 1, user_data);
}

unsigned char dinorunner_distancemeter_init(struct distance_meter_s* distance_meter,
                                            const struct dimension_s* sprite_info, unsigned int canvas_width,
                                            void* user_data) {
  distance_meter->max_score_units  = DINORUNNER_CONFIG_DISTANCEMETER_MAXSCOREUNIT;
  distance_meter->achievement      = 0u;
  distance_meter->flash_timer      = 0;
  distance_meter->flash_iterations = 0;
  distance_meter->invert_timer     = 0u;
  distance_meter->x                = 0;
  distance_meter->y                = 5;
  distance_meter->width            = sprite_info->width;
  dinorunner_distancemeter_calcxpos(distance_meter, canvas_width);
  unsigned long high_score   = distancemeter_readhighscore(user_data);
  distance_meter->high_score = high_score;
  dinorunner_distancemeter_draw(distance_meter, 0, 0u, 1u, user_data);
  return 1u;
}

unsigned char dinorunner_distancemeter_update(struct distance_meter_s* distance_meter, float delta_time,
                                              unsigned long distance, void* user_data) {
  unsigned char play_sound = 0u;
  unsigned char paint      = 1u;
  distance                 = dinorunner_distancemeter_getactualdistance(distance);
  if (!distance_meter->achievement) {
    if (distance > distance_meter->high_score &&
        distance_meter->max_score_units == DINORUNNER_CONFIG_DISTANCEMETER_MAXSCOREUNIT) {
      // TODO:
      // distance_meter->max_score_units++;
    } else {
      distance_meter->current_distance = 0;
    }
    if (distance > 0) {
      if (distance % DINORUNNER_CONFIG_DISTANCEMETER_ACHIEVEMENT_DISTANCE == 0) {
        distance_meter->achievement = 1u;
        distance_meter->flash_timer = 0u;
        play_sound                  = 1u;
      }
    }
  } else {
    if (distance_meter->flash_iterations <= DINORUNNER_CONFIG_DISTANCEMETER_FLASHITERATION) {
      distance_meter->flash_timer += delta_time;
      if (distance_meter->flash_timer < DINORUNNER_CONFIG_DISTANCEMETER_FLASHDURATION) {
        paint = 0u;
      } else if (distance_meter->flash_timer > DINORUNNER_CONFIG_DISTANCEMETER_FLASHDURATION * 2) {
        distance_meter->flash_timer = 0u;
        distance_meter->flash_iterations++;
      }
    } else {
      distance_meter->achievement      = 0u;
      distance_meter->flash_iterations = 0u;
      distance_meter->flash_timer      = 0u;
    }
  }
  if (paint) {
    dinorunner_distancemeter_draw(distance_meter, 0, distance, 0, user_data);
  }
  dinorunner_distancemeter_drawhighscore(distance_meter, user_data);
  return play_sound;
}

void dinorunner_distancemeter_reset(struct distance_meter_s* distance_meter, unsigned long score, void* user_data) {
  dinorunner_distancemeter_update(distance_meter, 0, score, user_data);
  distance_meter->achievement = 0u;
}
