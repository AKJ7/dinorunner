/**
 * @file version.c
 * 
 * @copyright Copyright (C) 2025 - All Rights Reserved 
 *  You may use, distribute and modify this code under the 
 *  terms of the GPL license.
 */

#include "dinorunner.h"

static const unsigned char kVersionMajor = 1;
static const unsigned char kVersionMinor = 1;
static const unsigned char kVersionPatch = 0;

unsigned char dinorunner_getversion(struct version_s* version) {
  version->major = kVersionMajor;
  version->minor = kVersionMinor;
  version->patch = kVersionPatch;
  return 1u;
}
