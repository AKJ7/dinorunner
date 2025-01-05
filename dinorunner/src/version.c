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
