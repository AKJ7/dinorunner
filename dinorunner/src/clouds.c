/**
 * @file clouds.c
 * 
 * @copyright Copyright (C) 2025 - All Rights Reserved 
 *  You may use, distribute and modify this code under the 
 *  terms of the GPL license.
 */

#include "dinorunner.h"

static inline unsigned char dinorunner_cloud_isvisible(const struct cloud_s* cloud) {
  return cloud->x + DINORUNNER_CONFIG_CLOUD_WIDTH > 0;
}

void dinorunner_cloud_init(struct cloud_s* cloud, unsigned container_width) {
  cloud->container_width = container_width;
  cloud->x               = container_width;
  cloud->remove          = 0;
  cloud->is_alive        = 1u;
  cloud->cloud_gap       = dinorunner_getrandomnumb(DINORUNNER_CONFIG_CLOUD_MIN_GAP, DINORUNNER_CONFIG_CLOUD_MAX_GAP);
  cloud->y = dinorunner_getrandomnumb(DINORUNNER_CONFIG_CLOUD_MAX_SKY_LEVEL, DINORUNNER_CONFIG_CLOUD_MIN_SKY_LEVEL);
}

unsigned char dinorunner_cloud_update(struct cloud_s* cloud, float speed, void* user_data) {
  unsigned char res = 1;
  if (!cloud->remove) {
    cloud->x -= dinorunner_ceilf(speed);
    struct pos_s pos = {.x = cloud->x, .y = cloud->y};
    res              = dinorunner_draw(DINORUNNER_SPRITE_CLOUD, &pos, user_data);
    if (!dinorunner_cloud_isvisible(cloud)) {
      cloud->remove = 1;
    }
  }
  return res;
}