#include "dinorunner.h"

extern const struct sprite_s dinorunner_trex_stand_sprite;

struct trex_animation_frame_s {
  const enum dinorunner_sprite_e sprite[2];
  const float ms_per_frames;
};

static const struct trex_animation_frame_s trex_waiting_frame = {
    .sprite        = {DINORUNNER_SPRITE_TREX_STANDING1, DINORUNNER_SPRITE_TREX_STANDING1},
    .ms_per_frames = 1000.0f / 3};
static const struct trex_animation_frame_s trex_running_frame = {
    .sprite        = {DINORUNNER_SPRITE_TREX_RUNNING1, DINORUNNER_SPRITE_TREX_RUNNING2},
    .ms_per_frames = 1000.0f / 12};
static const struct trex_animation_frame_s trex_crashed_frame = {
    .sprite        = {DINORUNNER_SPRITE_TREX_CRASHED, DINORUNNER_SPRITE_TREX_CRASHED},
    .ms_per_frames = 1000.0f / 60};
static const struct trex_animation_frame_s trex_jumping_frame = {
    .sprite        = {DINORUNNER_SPRITE_TREX_JUMPING, DINORUNNER_SPRITE_TREX_JUMPING},
    .ms_per_frames = 1000.0f / 60};
static const struct trex_animation_frame_s trex_ducking_frame = {
    .sprite        = {DINORUNNER_SPRITE_TREX_DUCKING1, DINORUNNER_SPRITE_TREX_DUCKING2},
    .ms_per_frames = 1000.0f / 8};

unsigned char dinorunner_trex_update(struct trex_s* trex, unsigned long delta_time, enum trex_status_e opt_status,
                                     void* user_data);
unsigned char dinorunner_trex_draw(struct trex_s* trex, enum dinorunner_sprite_e sprite, void* user_data);

const struct trex_animation_frame_s* get_animation_frame(enum trex_status_e status) {
  switch (status) {
    case TREX_STATUS_CRASHED:
      return &trex_crashed_frame;
    case TREX_STATUS_DUCKING:
      return &trex_ducking_frame;
    case TREX_STATUS_JUMPING:
      return &trex_jumping_frame;
    case TREX_STATUS_RUNNING:
      return &trex_running_frame;
    case TREX_STATUS_WAITING:
      return &trex_waiting_frame;
    default:
      dinorunner_log("Invalid frame: %d received\n", (int)status);
      return 0;
  }
  return 0;
}

static inline void set_blink_delay(struct trex_s* trex) {
  float random_blink     = dinorunner_rand();
  float normalized_blink = random_blink / 0xFFFF;
  trex->blink_delay      = dinorunner_ceilf(normalized_blink * DINORUNNER_CONFIG_TREX_BLINK_TIMING);
}

static void trex_blink(struct trex_s* trex, float time, void* user_data) {
  float delta_time = time - trex->animation_start_time;
  if ((delta_time >= trex->blink_delay) && (trex->blink_count < DINORUNNER_CONFIG_MAX_BLINK_COUNT)) {
    dinorunner_trex_draw(trex, DINORUNNER_SPRITE_TREX_STANDING2, user_data);
    if (trex->current_frame == 1) {
      set_blink_delay(trex);
      trex->animation_start_time = time;
      trex->blink_count++;
    }
  } else {
    dinorunner_trex_draw(trex, DINORUNNER_SPRITE_TREX_STANDING1, user_data);
  }
}

void dinorunner_trex_setduck(struct trex_s* trex, unsigned char is_ducking, void* user_data) {
  if (is_ducking && trex->status != TREX_STATUS_DUCKING) {
    dinorunner_trex_update(trex, 0, TREX_STATUS_DUCKING, user_data);
    trex->ducking = 1u;
  } else if (trex->status == TREX_STATUS_DUCKING) {
    dinorunner_trex_update(trex, 0, TREX_STATUS_RUNNING, user_data);
    trex->ducking = 0u;
  }
}

unsigned char dinorunner_trex_update(struct trex_s* trex, unsigned long delta_time, enum trex_status_e opt_status,
                                     void* user_data) {
  trex->timer += delta_time;
  if (opt_status != TREX_STATUS_NONE) {
    trex->status                                         = opt_status;
    trex->current_frame                                  = 0;
    const struct trex_animation_frame_s* animation_frame = get_animation_frame(opt_status);
    trex->ms_per_frame                                   = animation_frame->ms_per_frames;
    trex->current_animation_frame[0]                     = animation_frame->sprite[0];
    trex->current_animation_frame[1]                     = animation_frame->sprite[1];
    if (opt_status == TREX_STATUS_WAITING) {
      trex->animation_start_time = dinorunner_gettimestamp(user_data);
      set_blink_delay(trex);
    }
  }
  if (trex->status == TREX_STATUS_WAITING) {
    trex_blink(trex, dinorunner_gettimestamp(user_data), user_data);
  } else {
    dinorunner_trex_draw(trex, trex->current_animation_frame[trex->current_frame], user_data);
  }
  if (trex->timer >= trex->ms_per_frame) {
    trex->current_frame += 1;
    trex->current_frame %= 2;
    trex->timer = 0;
  }
  if (trex->speed_drop && (trex->y_pos <= trex->ground_y_pos)) {
    trex->speed_drop = 0u;
    dinorunner_trex_setduck(trex, 0, user_data);
  }
  return 1u;
}

void dinorunner_trex_reset(struct trex_s* trex, void* user_data) {
  trex->y_pos         = trex->ground_y_pos;
  trex->jump_velocity = 0u;
  trex->jumping       = 0u;
  trex->ducking       = 0u;
  trex->mid_air       = 0u;
  trex->speed_drop    = 0u;
  trex->jump_count    = 0u;
  dinorunner_trex_update(trex, 0, TREX_STATUS_RUNNING, user_data);
}

void dinorunner_trex_startjump(struct trex_s* trex, float speed, void* user_data) {
  if (!trex->jumping) {
    dinorunner_trex_update(trex, 0, TREX_STATUS_JUMPING, user_data);
    trex->jump_velocity      = DINORUNNER_CONFIG_TREX_INITIAL_JUMP_VELOCITY - (speed / 10.0f);
    trex->jumping            = 1u;
    trex->reached_min_height = 0u;
    trex->speed_drop         = 0u;
  }
}

void dinorunner_trex_endjump(struct trex_s* trex) {
  if (trex->reached_min_height && (trex->jump_velocity < DINORUNNER_CONFIG_TREX_DROP_VELOCIY)) {
    trex->jump_velocity = DINORUNNER_CONFIG_TREX_DROP_VELOCIY;
  }
}

static void trex_reset(struct trex_s* trex, void* user_data) {
  trex->y_pos         = trex->ground_y_pos;
  trex->jump_velocity = 0;
  trex->jumping       = 0u;
  trex->ducking       = 0u;
  dinorunner_trex_update(trex, 0, TREX_STATUS_RUNNING, user_data);
  trex->mid_air    = 0u;
  trex->speed_drop = 0u;
  trex->jump_count = 0u;
}

void dinorunner_trex_updatejump(struct trex_s* trex, float delta_time, void* user_data) {
  const struct trex_animation_frame_s* ms_per_frame = get_animation_frame(trex->status);
  float frames_elapsed_f                            = delta_time / dinorunner_ceilf(ms_per_frame->ms_per_frames);
  int frames_elapsed                                = dinorunner_ceilf(frames_elapsed_f);
  if (trex->speed_drop) {
    trex->y_pos +=
        dinorunner_roundf(trex->jump_velocity * DINORUNNER_CONFIG_TREX_SPEED_DROP_COEFFICIENT * frames_elapsed);
  } else {
    trex->y_pos += dinorunner_roundf(trex->jump_velocity * frames_elapsed);
  }
  trex->jump_velocity += DINORUNNER_CONFIG_TREX_GRAVITY * frames_elapsed;
  if (trex->y_pos < DINORUNNER_CONFIG_TREX_MIN_JUMP_HEIGHT || trex->speed_drop) {
    trex->reached_min_height = 1u;
  }
  if (trex->y_pos < DINORUNNER_CONFIG_TREX_MAX_JUMP_HEIGHT || trex->speed_drop) {
    dinorunner_trex_endjump(trex);
  }
  if (trex->y_pos > trex->ground_y_pos) {
    trex_reset(trex, user_data);
    trex->jump_count++;
  }
  dinorunner_trex_update(trex, delta_time, TREX_STATUS_NONE, user_data);
}

unsigned char dinorunner_trex_draw(struct trex_s* trex, enum dinorunner_sprite_e sprite, void* user_data) {
  struct pos_s pos     = {.x = trex->x_pos, .y = trex->y_pos};
  unsigned char result = dinorunner_draw(sprite, &pos, user_data);
  return result;
}

unsigned char dinorunner_trex_init(struct trex_s* trex, const struct pos_s* sprite_pos, unsigned container_width,
                                   unsigned container_height, void* user_data) {
  trex->x_pos                = 0;
  trex->ground_y_pos         = 0;
  trex->current_frame        = 0;
  trex->blink_delay          = 0;
  trex->blink_count          = 0;
  trex->animation_start_time = 0;
  trex->timer                = 0;
  trex->ms_per_frame         = 1000.0 / DINORUNNER_CONFIG_CORE_FPS;
  trex->status               = TREX_STATUS_WAITING;
  trex->jumping              = 0u;
  trex->ducking              = 0u;
  trex->reached_min_height   = 0;
  trex->speed_drop           = 0;
  trex->jump_count           = 0;
  trex->jump_spot_x          = 0;
  trex->sprite_pos.x         = sprite_pos->x;
  trex->sprite_pos.y         = sprite_pos->y;
  trex->ground_y_pos         = container_height - DINORUNNER_CONFIG_TREX_HEIGHT - DINORUNNER_CONFIG_TREX_BOTTOM_PAD;
  trex->y_pos                = trex->ground_y_pos;
  trex->min_jump_height      = trex->ground_y_pos - DINORUNNER_CONFIG_TREX_MIN_JUMP_HEIGHT;
  trex->jump_velocity        = -9;
  trex->playing_intro        = 0u;
  (void)container_width;
  dinorunner_trex_draw(trex, DINORUNNER_SPRITE_TREX_STANDING1, user_data);
  dinorunner_trex_update(trex, 0, TREX_STATUS_WAITING, user_data);
  return 1u;
}

void dinorunner_trex_setjumpvelocity(struct trex_s* trex, int setting) {
  trex->initial_jump_velocity = -setting;
  trex->drop_velocity         = -setting / 2.0f;
}

void dinorunner_trex_setspeeddrop(struct trex_s* trex) {
  trex->speed_drop    = 1u;
  trex->jump_velocity = -DINORUNNER_CONFIG_TREX_DROP_VELOCIY / 2.0f;
}