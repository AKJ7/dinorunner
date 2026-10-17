/**
 * @file shim.c
 * @brief C side of the Rust binding: the layout of the engine state, opaque to Rust
 *
 * @copyright Copyright (C) 2026 - All Rights Reserved
 *  You may use, distribute and modify this code under the
 *  terms of the GPL license.
 */

#include <stddef.h>
#include "dinorunner.h"

struct dinorunner_rs_alignment {
  char offset;
  struct dinorunner_s state;
};

/* The engine state is opaque to Rust, which allocates it with this size and alignment */
size_t dinorunner_rs_state_size(void) {
  return sizeof(struct dinorunner_s);
}

size_t dinorunner_rs_state_align(void) {
  return offsetof(struct dinorunner_rs_alignment, state);
}

/* Used by the tests to keep the Rust declarations in sync with dinorunner.h */
size_t dinorunner_rs_pos_size(void) {
  return sizeof(struct pos_s);
}

size_t dinorunner_rs_dimension_size(void) {
  return sizeof(struct dimension_s);
}

size_t dinorunner_rs_version_size(void) {
  return sizeof(struct version_s);
}

size_t dinorunner_rs_callbacks_size(void) {
  return sizeof(struct dinorunner_callbacks_s);
}

int dinorunner_rs_sprite_count(void) {
  return DINORUNNER_SPRITE_GAMEOVER + 1;
}

int dinorunner_rs_sound_count(void) {
  return DINORUNNER_SOUND_HIT + 1;
}
