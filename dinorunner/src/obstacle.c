#include "dinorunner.h"

static const struct obstacle_config_s kCactusSmallConfig = {
    .type_config        = OBSTACLE_TYPE_CACTUS_SMALL,
    .width              = 17,
    .height             = 35,
    .y_pos              = 105,
    .min_gap            = 120,
    .min_speed          = 0,
    .frame_rate         = DINORUNNER_CONFIG_CORE_FPS,
    .num_frames         = 1,
    .collision_box[0]   = {0, 7, 5, 27},
    .collision_box[1]   = {4, 6, 6, 34},
    .collision_box[2]   = {10, 4, 7, 14},
    .collision_box_size = 3,
};
static const struct obstacle_config_s kCactusLargeConfig = {
    .type_config        = OBSTACLE_TYPE_CACTUS_LARGE,
    .width              = 25,
    .height             = 50,
    .y_pos              = 90,
    .multiple_speed     = 7,
    .min_gap            = 120,
    .min_speed          = 0,
    .frame_rate         = DINORUNNER_CONFIG_CORE_FPS,
    .num_frames         = 1,
    .collision_box[0]   = {0, 12, 7, 38},
    .collision_box[1]   = {8, 0, 7, 49},
    .collision_box[2]   = {13, 10, 10, 38},
    .collision_box_size = 3,
};
static const struct obstacle_config_s kPterodactylConfig = {
    .type_config        = OBSTACLE_TYPE_PTERODACTYL,
    .width              = 46,
    .height             = 40,
    .y_pos              = 50 + 10,
    .multiple_speed     = 999,
    .min_gap            = 150,
    .min_speed          = 7.0f,
    .speed_offset       = 0.8f,
    .frame_rate         = 1000.0f / 6,
    .num_frames         = 2,
    .collision_box[0]   = {15, 15, 16, 5},
    .collision_box[1]   = {18, 21, 24, 6},
    .collision_box[2]   = {2, 14, 4, 3},
    .collision_box[3]   = {6, 10, 4, 7},
    .collision_box[4]   = {10, 8, 6, 9},
    .collision_box_size = 5,
};

enum obstacle_type_e dinorunner_obstacle_fromIndex(int obstacle_index) {
  enum obstacle_type_e obstacle_types[3] = {OBSTACLE_TYPE_CACTUS_SMALL, OBSTACLE_TYPE_CACTUS_LARGE,
                                            OBSTACLE_TYPE_PTERODACTYL};
  const unsigned kObstacleSize           = sizeof(obstacle_types) / sizeof(enum obstacle_type_e);
  unsigned normalized_index              = obstacle_index % kObstacleSize;
  return obstacle_types[normalized_index];
}

static inline unsigned char obstacle_isvisible(const struct obstacle_s* obstacle) {
  return obstacle->x_pos + obstacle->width > 0;
}

static inline int obstacle_getgap(const struct obstacle_s* obstacle, float gap_coefficient, float speed) {
  float min_gap = dinorunner_roundf(obstacle->width * speed + obstacle->config.min_gap * gap_coefficient);
  float max_gap = dinorunner_roundf(min_gap * DINORUNNER_CONFIG_MAX_GAP_COEFFICIENT);
  return dinorunner_getrandomnumb((int)min_gap, (int)max_gap);
}

static void dinorunner_obstacle_setconfig(struct obstacle_s* obstacle, enum obstacle_type_e obstacle_type) {
  const struct obstacle_config_s* config = 0;
  switch (obstacle_type) {
    case OBSTACLE_TYPE_CACTUS_SMALL:
      config = &kCactusSmallConfig;
      break;
    case OBSTACLE_TYPE_CACTUS_LARGE:
      config = &kCactusLargeConfig;
      break;
    case OBSTACLE_TYPE_PTERODACTYL:
      config = &kPterodactylConfig;
      break;
    default:
      return;
      break;
  }
  obstacle->config.type_config        = config->type_config;
  obstacle->config.width              = config->width;
  obstacle->config.height             = config->height;
  obstacle->config.y_pos              = config->y_pos;
  obstacle->config.multiple_speed     = config->multiple_speed;
  obstacle->config.min_gap            = config->min_gap;
  obstacle->config.min_speed          = config->min_speed;
  obstacle->config.num_frames         = config->num_frames;
  obstacle->config.frame_rate         = config->frame_rate;
  obstacle->config.speed_offset       = config->speed_offset;
  obstacle->config.collision_box_size = config->collision_box_size;
  for (unsigned counter = 0; counter < config->collision_box_size; ++counter) {
    struct collision_box_s* dest      = &obstacle->config.collision_box[counter];
    const struct collision_box_s* src = &config->collision_box[counter];
    dest->x                           = src->x;
    dest->y                           = src->y;
    dest->width                       = src->width;
    dest->height                      = src->height;
  }
}

float dinorunner_obstacle_getMinSpeed(enum obstacle_type_e obstacle_type) {
  const struct obstacle_config_s* config = 0;
  switch (obstacle_type) {
    case OBSTACLE_TYPE_CACTUS_SMALL:
      config = &kCactusSmallConfig;
      break;
    case OBSTACLE_TYPE_CACTUS_LARGE:
      config = &kCactusLargeConfig;
      break;
    case OBSTACLE_TYPE_PTERODACTYL:
      config = &kPterodactylConfig;
      break;
    default:
      return 0.0f;
      break;
  }
  return config->min_speed;
}

static void obstacle_draw(const struct obstacle_s* obstacle, void* user_data) {
  enum dinorunner_sprite_e sprite;
  switch (obstacle->config.type_config) {
    case OBSTACLE_TYPE_CACTUS_SMALL:
      sprite = DINORUNNER_SPRITE_CACTUSSMALL1;
      break;
    case OBSTACLE_TYPE_CACTUS_LARGE:
      sprite = DINORUNNER_SPRITE_CACTUSLARGE1;
      break;
    case OBSTACLE_TYPE_PTERODACTYL:
      sprite = obstacle->current_frame == 0 ? DINORUNNER_SPRITE_PTERODACTYL2 : DINORUNNER_SPRITE_PTERODACTYL1;
      break;
    default:
      dinorunner_log("Invalid obstacle type: %d\n", obstacle->config.type_config);
      return;
  }
  sprite                 = sprite + (obstacle->size - 1);
  const struct pos_s pos = {.x = obstacle->x_pos, .y = obstacle->y_pos};
  dinorunner_draw(sprite, &pos, user_data);
}

void dinorunner_obstacle_init(struct obstacle_s* obstacle, enum obstacle_type_e obstacle_type,
                              const struct dimension_s* dimension, float gap_coefficient, float speed, int opt_x_offset,
                              void* user_data) {
  TREX_UNUSED(gap_coefficient);
  obstacle->y_pos         = 0;
  obstacle->width         = 0;
  obstacle->gap           = 0;
  obstacle->current_frame = 0;
  obstacle->timer         = 0;
  obstacle->remove        = 0u;
  dinorunner_obstacle_setconfig(obstacle, obstacle_type);
  if (obstacle->config.type_config != OBSTACLE_TYPE_PTERODACTYL) {
    obstacle->size = dinorunner_getrandomnumb(1, DINORUNNER_CONFIG_MAX_OBSTACLE_LENGTH);
  } else {
    obstacle->size = 1;
  }
  obstacle->dimension.width            = dimension->width;
  obstacle->dimension.height           = dimension->height;
  obstacle->width                      = obstacle->config.width;
  obstacle->remove                     = 0u;
  obstacle->y_pos                      = 0u;
  obstacle->x_pos                      = obstacle->dimension.width + opt_x_offset;
  obstacle->gap                        = 0u;
  obstacle->timer                      = 0u;
  obstacle->following_obstacle_created = 0u;
  // obstacle->gap_coefficient            = gap_coefficient;
  obstacle->gap_coefficient = DINORUNNER_CONFIG_MAX_GAP_COEFFICIENT;
  if (obstacle->size > 1 && (obstacle->config.multiple_speed > speed)) {
    obstacle->size = 1;
  }
  obstacle->config.width = obstacle->config.width * obstacle->size;
  obstacle->y_pos        = obstacle->config.y_pos;
  if (obstacle->config.type_config == OBSTACLE_TYPE_PTERODACTYL) {
    obstacle->y_pos += dinorunner_getrandomnumb(0, 2) * 25;
  }
  obstacle_draw(obstacle, user_data);
  if (obstacle->size > 1) {
    obstacle->config.collision_box[1].width =
        obstacle->width - obstacle->config.collision_box[0].width - obstacle->config.collision_box[2].width;
    obstacle->config.collision_box[2].x = obstacle->width - obstacle->config.collision_box[2].width;
  }
  if (obstacle->config.speed_offset) {
    obstacle->speed_offset = dinorunner_srand() > 0.5f ? obstacle->config.speed_offset : -obstacle->config.speed_offset;
  }
  obstacle->gap = obstacle_getgap(obstacle, obstacle->gap_coefficient, speed);
}

void dinorunner_obstacle_update(struct obstacle_s* obstacle, float delta_time, float speed, void* user_data) {
  if (!obstacle->remove) {
    if (obstacle->config.speed_offset) {
      speed += obstacle->speed_offset;
    }
    obstacle->x_pos -= dinorunner_floorf((speed * DINORUNNER_CONFIG_CORE_FPS / 1000.0f) * delta_time);
    if (obstacle->config.num_frames) {
      obstacle->timer += delta_time;
      if ((obstacle->config.num_frames > 1) && (obstacle->timer >= obstacle->config.frame_rate)) {
        obstacle->current_frame =
            (obstacle->current_frame == obstacle->config.num_frames - 1) ? 0 : obstacle->current_frame + 1;
        obstacle->timer = 0;
      }
    }
    obstacle_draw(obstacle, user_data);
    if (!obstacle_isvisible(obstacle)) {
      obstacle->remove = 1u;
    }
  }
}
