/**
 * @file horizon.c
 * 
 * @copyright Copyright (C) 2025 - All Rights Reserved 
 *  You may use, distribute and modify this code under the 
 *  terms of the GPL license.
 */

#include "dinorunner.h"

static void add_cloud(struct horizon_s* horizon, const struct dimension_s* dimension) {
  for (unsigned i = 0; i < DINORUNNER_CONFIG_CLOUD_MAX_COUNT; ++i) {
    if (!horizon->clouds[i].is_alive) {
      horizon->clouds[i].is_alive = 1;
      dinorunner_cloud_init(&horizon->clouds[i], dimension->width);
      return;
    }
  }
}

static inline unsigned char dinorunner_obstacle_isvisible(const struct obstacle_s* obstacle) {
  return obstacle->x_pos + obstacle->width > 0;
}

static unsigned get_active_obstacle_history(const struct horizon_s* horizon) {
  unsigned size = 0;
  for (unsigned i = 0u; i < DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT; ++i) {
    size += (horizon->obstacle_history[i] != OBSTACLE_TYPE_NONE);
  }
  return size;
}

static unsigned get_active_obstacles(const struct horizon_s* horizon) {
  unsigned size = 0;
  for (unsigned i = 0u; i < DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT; ++i) {
    size += (horizon->obstacles[i].is_alive);
  }
  return size;
}

static unsigned char duplicate_obstacle_check(const struct horizon_s* horizon,
                                              enum obstacle_type_e next_obstacle_type) {
  unsigned duplicate_count                  = 0;
  const unsigned number_of_obstacle_history = get_active_obstacle_history(horizon);
  for (unsigned i = 0u; i < number_of_obstacle_history; ++i) {
    duplicate_count = (horizon->obstacle_history[i] == next_obstacle_type);
  }
  return duplicate_count >= DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_DUPLICATION;
}

unsigned char dinorunner_horizon_init(struct horizon_s* horizon, const struct dimension_s* dimension,
                                      float gap_coefficient) {
  horizon->cloud_speed     = DINORUNNER_CONFIG_CLOUD_SPEED;
  horizon->first           = 1u;
  horizon->dimension       = *dimension;
  horizon->gap_coefficient = gap_coefficient;
  dinorunner_horizonline_init(&horizon->horizon_line, dimension);
  dinorunner_nightmode_init(&horizon->nightmode, horizon->dimension.width);
  for (unsigned i = 0; i < DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT; ++i) {
    horizon->obstacle_history[i]   = OBSTACLE_TYPE_NONE;
    horizon->obstacles[i].is_alive = 0u;
  }
  for (unsigned i = 0; i < DINORUNNER_CONFIG_CLOUD_MAX_COUNT; ++i) {
    horizon->clouds[i].is_alive = 0;
  }
  if (horizon->first) {
    add_cloud(horizon, dimension);
    horizon->first = 0;
  }
  return 1u;
}

unsigned char update_clouds(struct horizon_s* horizon, float delta_time, float current_speed, void* user_data) {
  float cloud_speed     = (horizon->cloud_speed / 1000.0f) * delta_time * current_speed;
  int last_pos          = 0;
  float last_gap        = 0;
  unsigned clouds_count = 0;
  for (unsigned counter = 0u; counter < DINORUNNER_CONFIG_CLOUD_MAX_COUNT; ++counter) {
    struct cloud_s* cloud = &horizon->clouds[counter];
    if (cloud->is_alive && !cloud->remove) {
      dinorunner_cloud_update(cloud, cloud_speed, user_data);
      if (last_pos < cloud->x) {
        last_pos = cloud->x;
        last_gap = cloud->cloud_gap;
      }
      ++clouds_count;
    }
  }
  if ((clouds_count < DINORUNNER_CONFIG_CLOUD_MAX_COUNT) && ((horizon->dimension.width - last_pos) > last_gap) &&
      (DINORUNNER_CONFIG_CLOUD_FREQUENCY > dinorunner_srand())) {
    add_cloud(horizon, &horizon->dimension);
    ++clouds_count;
  }
  for (unsigned i = 0u; i < DINORUNNER_CONFIG_CLOUD_MAX_COUNT; ++i) {
    struct cloud_s* cloud = &horizon->clouds[i];
    if (cloud->is_alive) {
      if (cloud->remove) {
        cloud->is_alive = 0u;
      }
    }
  }
  if (clouds_count == 0) {
    add_cloud(horizon, &horizon->dimension);
  }
  return 1u;
}

static void add_new_obstacle(struct horizon_s* horizon, float current_speed, void* user_data) {
  int obstacle_index                 = dinorunner_getrandomnumb(0, 2);
  enum obstacle_type_e obstacle_type = dinorunner_obstacle_fromIndex(obstacle_index);
  if (duplicate_obstacle_check(horizon, obstacle_type) ||
      (current_speed < dinorunner_obstacle_getMinSpeed(obstacle_type))) {
    add_new_obstacle(horizon, current_speed, user_data);
  } else {
    for (unsigned i = 0u; i < DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT; ++i) {
      struct obstacle_s* obstacle = &horizon->obstacles[i];
      if (!obstacle->is_alive) {
        dinorunner_obstacle_init(obstacle, obstacle_type, &horizon->dimension, horizon->gap_coefficient, current_speed,
                                 0, user_data);
        obstacle->is_alive = 1u;
        return;
      }
    }
  }
}

unsigned char update_obstacles(struct horizon_s* horizon, float delta_time, float current_speed, void* user_data) {
  for (unsigned i = 0u; i < DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT; ++i) {
    struct obstacle_s* obstacle = &horizon->obstacles[i];
    if (obstacle->is_alive) {
      dinorunner_obstacle_update(obstacle, delta_time, current_speed, user_data);
    }
  }
  unsigned active_obstacle_count = get_active_obstacles(horizon);
  if (active_obstacle_count > 0u) {
    int x_pos           = 0;
    unsigned last_index = 0;
    for (unsigned i = 0u; i < DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT; ++i) {
      const struct obstacle_s* obstacle = &horizon->obstacles[i];
      if (obstacle->is_alive) {
        if (obstacle->x_pos > x_pos) {
          x_pos      = obstacle->x_pos;
          last_index = i;
        }
      }
    }
    struct obstacle_s* last_obstacle = &horizon->obstacles[last_index];
    if (!last_obstacle->following_obstacle_created && dinorunner_obstacle_isvisible(last_obstacle) &&
        (last_obstacle->x_pos + last_obstacle->width + last_obstacle->gap) < horizon->dimension.width) {
      add_new_obstacle(horizon, current_speed, user_data);
      last_obstacle->following_obstacle_created = 1;
    }
  } else {
    add_new_obstacle(horizon, current_speed, user_data);
  }
  return 1u;
}

unsigned char dinorunner_horizon_update(struct horizon_s* horizon, float delta_time, float current_speed,
                                        unsigned char update_obstacles_request, unsigned char show_night_mode,
                                        void* user_data) {
  horizon->running_time += delta_time;
  dinorunner_horizonline_update(&horizon->horizon_line, delta_time, current_speed, user_data);
  dinorunner_nightmode_update(&horizon->nightmode, show_night_mode, horizon->dimension.width, user_data);
  update_clouds(horizon, delta_time, current_speed, user_data);
  if (update_obstacles_request) {
    update_obstacles(horizon, delta_time, current_speed, user_data);
  }
  return 1u;
}

void dinorunner_horizon_reset(struct horizon_s* horizon, void* user_data) {
  for (unsigned i = 0u; i < DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT; ++i) {
    horizon->obstacles[i].is_alive = 0;
  }
  dinorunner_horizonline_reset(&horizon->horizon_line);
  dinorunner_nightmode_reset(&horizon->nightmode, horizon->dimension.width, user_data);
}
