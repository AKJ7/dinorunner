/**
 * @file dinorunner.c
 * 
 * @copyright Copyright (C) 2025 - All Rights Reserved 
 *  You may use, distribute and modify this code under the 
 *  terms of the GPL license.
 */

#include "dinorunner.h"

struct dimension_s text_dimension = {.width = 10, .height = 13};

static void dinorunner_invert(struct dinorunner_s* dinorunner, unsigned char reset) {
  if (reset) {
    dinorunner->invert_timer = 0u;
    dinorunner->inverted     = 0u;
  } else {
    dinorunner->inverted = dinorunner->invert_trigger;
  }
}

unsigned char dinorunner_init(struct dinorunner_s* dinorunner, const struct dimension_s* dimension, void* user_data) {
  unsigned char res            = 1u;
  dinorunner->dimension.width  = dimension->width;
  dinorunner->dimension.height = dimension->height;
  dinorunner->running_time     = 0;
  dinorunner->activated        = 0u;
  dinorunner->time             = 0;
  dinorunner->running_time     = 0u;
  dinorunner->ms_per_frame     = 1000.0f / DINORUNNER_CONFIG_CORE_FPS;
  dinorunner->current_speed    = DINORUNNER_CONFIG_SPEED;
  dinorunner->invert_timer     = 0u;
  dinorunner->play_count       = 0;
  dinorunner->playing          = 0u;
  dinorunner->distance_ran     = 0u;
  dinorunner->crashed          = 0u;
  dinorunner->invert_trigger   = 0u;
  dinorunner->playing_intro    = 0u;
  dinorunner->user_data        = user_data;
  dinorunner->inverted         = 0u;
  res &=
      dinorunner_horizon_init(&dinorunner->horizon, &dinorunner->dimension, -DINORUNNER_CONFIG_HORIZON_GAP_COEFFICIENT);
  dinorunner_distancemeter_init(&dinorunner->distance_meter, &text_dimension, dimension->width, dinorunner->user_data);
  res &= dinorunner_trex_init(&dinorunner->trex, dimension->width, dimension->height, dinorunner->user_data);
  res &= dinorunner_gameoverpanel_init(&dinorunner->gameoverpanel, dimension);
  return res;
}

unsigned char dinorunner_startgame(struct dinorunner_s* dinorunner) {
  dinorunner->running_time       = 0;
  dinorunner->playing_intro      = 0u;
  dinorunner->trex.playing_intro = 0u;
  dinorunner->play_count++;
  return 1u;
}

unsigned char dinorunner_restart(struct dinorunner_s* dinorunner) {
  dinorunner->play_count++;
  dinorunner->running_time  = 0u;
  dinorunner->playing       = 0u;
  dinorunner->distance_ran  = 0u;
  dinorunner->crashed       = 0u;
  dinorunner->current_speed = DINORUNNER_CONFIG_SPEED;
  // set_speed(DINORUNNER_CONFIG_SPEED);
  dinorunner->time = dinorunner_gettimestamp(dinorunner->user_data);
  dinorunner_canvas_clear(dinorunner->user_data);
  dinorunner_distancemeter_reset(&dinorunner->distance_meter, dinorunner->distance_meter.high_score,
                                 dinorunner->user_data);
  dinorunner_horizon_reset(&dinorunner->horizon, dinorunner->user_data);
  dinorunner_trex_reset(&dinorunner->trex, dinorunner->user_data);
  dinorunner_playsound(DINORUNNER_SOUND_BUTTON_PRESS, dinorunner->user_data);
  dinorunner_invert(dinorunner, 1u);
  dinorunner_update(dinorunner);
  return 1u;
}

unsigned char dinorunner_playintro(struct dinorunner_s* dinorunner) {
  if (!dinorunner->activated && !dinorunner->crashed) {
    dinorunner->playing_intro      = 1u;
    dinorunner->trex.playing_intro = 1u;
    dinorunner->playing            = 1u;
    dinorunner->activated          = 1u;
  } else if (dinorunner->crashed) {
    dinorunner_restart(dinorunner);
  }
  return 1u;
}

void dinorunner_stop(struct dinorunner_s* dinorunner) {
  dinorunner->playing = 0u;
  dinorunner->paused  = 1u;
  dinorunner->raq_id  = 0u;
}

static void dinorunner_gameover(struct dinorunner_s* dinorunner) {
  if (dinorunner->playing) {
    dinorunner_playsound(DINORUNNER_SOUND_HIT, dinorunner->user_data);
    dinorunner_vibrate(200, dinorunner->user_data);
  }
  dinorunner_stop(dinorunner);
  dinorunner->crashed                    = 1u;
  dinorunner->distance_meter.achievement = 0u;
  dinorunner_trex_update(&dinorunner->trex, 100, TREX_STATUS_CRASHED, dinorunner->user_data);
  dinorunner_gameoverpanel_draw(&dinorunner->gameoverpanel, dinorunner->user_data);
  unsigned last_score = dinorunner_distancemeter_getactualdistance(dinorunner->distance_ran);
  if (last_score > dinorunner->distance_meter.high_score) {
    dinorunner->distance_meter.high_score = last_score;
    dinorunner_distancemeter_writehighscore(&dinorunner->distance_meter, dinorunner->distance_meter.high_score,
                                            dinorunner->user_data);
  }
  dinorunner->time = dinorunner_gettimestamp(dinorunner->user_data);
}

static inline void dinorunner_createadjustedcollisionbox(const struct collision_box_s* box,
                                                         const struct collision_box_s* adjustment,
                                                         struct collision_box_s* destination) {
  destination->x      = box->x + adjustment->x;
  destination->y      = box->y + adjustment->y;
  destination->width  = box->width;
  destination->height = box->height;
}

static inline unsigned char dinorunner_boxcompare(const struct collision_box_s* trex_box,
                                                  const struct collision_box_s* obstacle_box) {
  unsigned char crashed = 0u;
  int trex_box_x        = trex_box->x;
  int trex_box_y        = trex_box->y;
  int obstacle_box_x    = obstacle_box->x;
  int obstacle_box_y    = obstacle_box->y;
  if ((trex_box_x < obstacle_box_x + obstacle_box->width) && (trex_box_x + trex_box->width > obstacle_box_x) &&
      (trex_box_y < obstacle_box_y + obstacle_box->height) && (trex_box->height + trex_box_y > obstacle_box_y)) {
    crashed = 1u;
  }
  return crashed;
}

static unsigned char dinorunner_checkforcollision(const struct obstacle_s* obstacle, struct trex_s* trex) {
  unsigned char crashed                       = 0u;
  const struct collision_box_s kTrexBox       = {trex->x_pos + 1, trex->y_pos + 1, 44 - 2, 47 - 2};
  const struct collision_box_s kObstacleBox   = {obstacle->x_pos + 1, obstacle->y_pos + 1,
                                                 obstacle->config.width * obstacle->size - 2,
                                                 obstacle->config.height - 2};
  const struct collision_box_s kDuckingBox[1] = {{1, 18, 55, 25}};
  const struct collision_box_s kRunningBox[6] = {{22, 0, 17, 16}, {1, 18, 30, 9}, {10, 35, 14, 8},
                                                 {1, 24, 29, 5},  {5, 30, 21, 4}, {9, 34, 15, 4}};
  if (dinorunner_boxcompare(&kTrexBox, &kObstacleBox)) {
    const struct collision_box_s* collision_boxes      = obstacle->config.collision_box;
    unsigned box_size                                  = obstacle->config.collision_box_size;
    const struct collision_box_s* trex_collision_boxes = trex->ducking ? kDuckingBox : kRunningBox;
    unsigned trex_box_size                             = trex->ducking ? 1 : 6;
    for (unsigned t = 0; t < trex_box_size; ++t) {
      for (unsigned i = 0; i < box_size; ++i) {
        struct collision_box_s adj_trex_box;
        dinorunner_createadjustedcollisionbox(&trex_collision_boxes[t], &kTrexBox, &adj_trex_box);
        struct collision_box_s adj_obstacle_box;
        dinorunner_createadjustedcollisionbox(&collision_boxes[i], &kObstacleBox, &adj_obstacle_box);
        crashed = dinorunner_boxcompare(&adj_trex_box, &adj_obstacle_box);
        if (crashed == 1) {
          return crashed;
        }
      }
    }
  }
  return crashed;
}

static int dinorunner_getfirstobstacle(const struct horizon_s* horizon, unsigned canvas_width) {
  int last_index = 0;
  int x_pos      = canvas_width;
  for (unsigned i = 0u; i < DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT; ++i) {
    const struct obstacle_s* obstacle = &horizon->obstacles[i];
    if (obstacle->is_alive) {
      if (obstacle->x_pos < x_pos) {
        x_pos      = obstacle->x_pos;
        last_index = i;
      }
    }
  }
  return last_index;
}

unsigned char dinorunner_update(struct dinorunner_s* dinorunner) {
  unsigned long now        = dinorunner_gettimestamp(dinorunner->user_data);
  unsigned long delta_time = now - dinorunner->time;
  dinorunner->time         = now;
  if (dinorunner->playing) {
    dinorunner_canvas_clear(dinorunner->user_data);
    if (dinorunner->trex.jumping) {
      dinorunner_trex_updatejump(&dinorunner->trex, delta_time, dinorunner->user_data);
    }
    dinorunner->running_time += delta_time;
    unsigned char has_obtacles = dinorunner->running_time > DINORUNNER_CONFIG_CORE_CLEARTIME;
    if (dinorunner->trex.jump_count == 1 && !dinorunner->trex.jumping && dinorunner->playing_intro) {
      dinorunner_startgame(dinorunner);
    }
    if (dinorunner->trex.jump_count == 1 && !dinorunner->playing_intro) {
      dinorunner_playintro(dinorunner);
    }
    if (dinorunner->playing_intro) {
      dinorunner_horizon_update(&dinorunner->horizon, 0, dinorunner->current_speed, has_obtacles, 0,
                                dinorunner->user_data);
    } else {
      delta_time = !dinorunner->activated ? 0 : delta_time;
      dinorunner_horizon_update(&dinorunner->horizon, delta_time, dinorunner->current_speed, has_obtacles,
                                dinorunner->inverted, dinorunner->user_data);
    }
    int index = dinorunner_getfirstobstacle(&dinorunner->horizon, dinorunner->dimension.width);
    unsigned char collision =
        has_obtacles && dinorunner_checkforcollision(&dinorunner->horizon.obstacles[index], &dinorunner->trex);
    if (!collision) {
      dinorunner->distance_ran += dinorunner->current_speed * delta_time / dinorunner->ms_per_frame;
      if (dinorunner->current_speed < DINORUNNER_CONFIG_CORE_MAXSPEED) {
        dinorunner->current_speed += DINORUNNER_CONFIG_CORE_ACCELERATION;
      }
    } else {
      dinorunner_gameover(dinorunner);
    }
    unsigned char play_achievement_sound = dinorunner_distancemeter_update(
        &dinorunner->distance_meter, delta_time, dinorunner_ceilf(dinorunner->distance_ran), dinorunner->user_data);
    if (play_achievement_sound) {
      dinorunner_playsound(DINORUNNER_SOUND_SCORE, dinorunner->user_data);
    }
    if (dinorunner->invert_timer > DINORUNNER_CONFIG_CORE_INVERTFADEDURATION) {
      dinorunner->invert_timer   = 0u;
      dinorunner->invert_trigger = 0u;
      dinorunner_invert(dinorunner, 0u);
    } else if (dinorunner->invert_timer) {
      dinorunner->invert_timer += delta_time;
    } else {
      unsigned long actual_distance = dinorunner_distancemeter_getactualdistance(dinorunner->distance_ran);
      if (actual_distance > 0) {
        dinorunner->invert_trigger = !(actual_distance % DINORUNNER_CONFIG_CORE_INVERTDISTANCE);
        if (dinorunner->invert_trigger && dinorunner->invert_timer == 0) {
          dinorunner->invert_timer += delta_time;
          dinorunner_invert(dinorunner, 0);
        }
      }
    }
  }
  if (dinorunner->crashed) {
    dinorunner_gameover(dinorunner);
  } else if ((dinorunner->playing)) {
    dinorunner_trex_update(&dinorunner->trex, delta_time, TREX_STATUS_NONE, dinorunner->user_data);
  } else if ((!dinorunner->activated) && (dinorunner->trex.blink_count < DINORUNNER_CONFIG_MAX_BLINK_COUNT)) {
    dinorunner_trex_update(&dinorunner->trex, delta_time, TREX_STATUS_NONE, dinorunner->user_data);
  } else {
    dinorunner_trex_update(&dinorunner->trex, delta_time, TREX_STATUS_WAITING, dinorunner->user_data);
  }

  return 1u;
}

void dinorunner_onkeyup(struct dinorunner_s* dinorunner) {
  if (!dinorunner->crashed) {
    if (!dinorunner->playing) {
      dinorunner->playing = 1u;
      dinorunner_update(dinorunner);
    }
    if (!dinorunner->trex.jumping) {
      dinorunner_trex_setduck(&dinorunner->trex, 0u, dinorunner->user_data);
      dinorunner_playsound(DINORUNNER_SOUND_BUTTON_PRESS, dinorunner->user_data);
      dinorunner_trex_startjump(&dinorunner->trex, dinorunner->current_speed, dinorunner->user_data);
    }
  } else {
    dinorunner_restart(dinorunner);
  }
}

void dinorunner_onkeydown(struct dinorunner_s* dinorunner) {
  if (dinorunner->playing && !dinorunner->crashed) {
    if (dinorunner->trex.jumping) {
      dinorunner_trex_setspeeddrop(&dinorunner->trex);
    } else if (!dinorunner->trex.jumping && !dinorunner->trex.ducking) {
      dinorunner_trex_setduck(&dinorunner->trex, 1u, dinorunner->user_data);
    }
  }
}

void dinorunner_onkeynone(struct dinorunner_s* dinorunner) {
  dinorunner_trex_setduck(&dinorunner->trex, 0u, dinorunner->user_data);
  dinorunner_trex_endjump(&dinorunner->trex);
}

unsigned char dinorunner_opacity(struct dinorunner_s* dinorunner, unsigned char* opacity) {
  *opacity = dinorunner->inverted ? (dinorunner->horizon.nightmode.opacity * 0xFF) : 0xFF;
  return 1u;
}

unsigned char dinorunner_isinverted(struct dinorunner_s* dinorunner, unsigned char* night_mode) {
  *night_mode = dinorunner->inverted;
  return 1u;
}