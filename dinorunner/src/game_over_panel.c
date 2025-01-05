#include "dinorunner.h"

unsigned char dinorunner_gameoverpanel_init(struct gameoverpanel_s* gameoverpanel,
                                            const struct dimension_s* dimension) {
  gameoverpanel->canvas_dimension.width  = dimension->width;
  gameoverpanel->canvas_dimension.height = dimension->height;
  return 1u;
}

unsigned dinorunner_gameoverpanel_draw(struct gameoverpanel_s* gameoverpanel, void* user_data) {
  float center_x                   = gameoverpanel->canvas_dimension.width / 2.0f;
  int text_source_width            = DINORUNNER_CONFIG_GAMEOVERPANEL_TEXT_WIDTH;
  int restart_source_width         = DINORUNNER_CONFIG_GAMEOVERPANEL_RESTART_WIDTH;
  int text_target_x                = dinorunner_roundf(center_x - (text_source_width / 2));
  int text_target_y                = dinorunner_roundf((gameoverpanel->canvas_dimension.height - 25.0f) / 3.0f);
  int restart_target_x             = dinorunner_roundf(center_x - ((float)restart_source_width / 2));
  int restart_target_y             = gameoverpanel->canvas_dimension.height / 2;
  const struct pos_s gameover_dest = {.x = text_target_x, .y = text_target_y};
  unsigned char res                = dinorunner_draw(DINORUNNER_SPRITE_GAMEOVER, &gameover_dest, user_data);
  const struct pos_s restart_dest  = {.x = restart_target_x, .y = restart_target_y};
  res &= dinorunner_draw(DINORUNNER_SPRITE_RESTART, &restart_dest, user_data);
  return res;
}
