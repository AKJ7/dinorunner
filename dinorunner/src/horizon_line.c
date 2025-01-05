#include "dinorunner.h"

static enum dinorunner_sprite_e get_random_type(const struct horizonline_s* horizonline) {
  return dinorunner_srand() > horizonline->bump_threshold ? DINORUNNER_SPRITE_HORIZON2 : DINORUNNER_SPRITE_HORIZON1;
}

void dinorunner_horizonline_setsourcedimensions() {}

void dinorunner_horizonline_reset(struct horizonline_s* horizonline) {
  horizonline->x_pos[0] = 0;
  horizonline->x_pos[1] = horizonline->dimensions.width;
}

void dinorunner_horizonline_updatexpos(struct horizonline_s* horizon_line, int pos, int increment) {
  int line1 = pos;
  int line2 = (pos == 0);
  horizon_line->x_pos[line1] -= increment;
  horizon_line->x_pos[line2] = horizon_line->x_pos[line1] + (int)horizon_line->dimensions.width;
  if (horizon_line->x_pos[line1] <= -(int)horizon_line->dimensions.width) {
    horizon_line->x_pos[line1] += (int)horizon_line->dimensions.width * 2;
    horizon_line->x_pos[line2]             = horizon_line->x_pos[line1] - horizon_line->dimensions.width;
    enum dinorunner_sprite_e random_sprite = get_random_type(horizon_line);
    if (line1 == 0) {
      horizon_line->sprite[0] = random_sprite;
    } else {
      horizon_line->sprite[1] = random_sprite;
    }
  }
}

void dinorunner_horizonline_update(struct horizonline_s* horizonline, float delta_time, float speed, void* user_data) {
  int increment = dinorunner_floorf(speed * (DINORUNNER_CONFIG_CORE_FPS / 1000.0f) * delta_time);
  if (horizonline->x_pos[0] <= 0) {
    dinorunner_horizonline_updatexpos(horizonline, 0, increment);
  } else {
    dinorunner_horizonline_updatexpos(horizonline, 1, increment);
  }
  const struct pos_s pos1 = {.x = horizonline->x_pos[0], horizonline->y_pos};
  const struct pos_s pos2 = {.x = horizonline->x_pos[1], horizonline->y_pos};
  dinorunner_draw(horizonline->sprite[0], &pos1, user_data);
  dinorunner_draw(horizonline->sprite[1], &pos2, user_data);
}

void dinorunner_horizonline_init(struct horizonline_s* horizoneline, const struct dimension_s* src_dimension) {
  horizoneline->x_pos[0]          = 0;
  horizoneline->x_pos[1]          = src_dimension->width;
  horizoneline->y_pos             = src_dimension->height;
  horizoneline->bump_threshold    = 0.5f;
  horizoneline->dimensions.width  = src_dimension->width;
  horizoneline->dimensions.height = src_dimension->height;
  horizoneline->sprite[0]         = DINORUNNER_SPRITE_HORIZON1;
  horizoneline->sprite[1]         = DINORUNNER_SPRITE_HORIZON2;
}