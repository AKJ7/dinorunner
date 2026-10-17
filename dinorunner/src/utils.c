/**
 * @file utils.c
 * 
 * @copyright Copyright (C) 2025 - All Rights Reserved 
 *  You may use, distribute and modify this code under the 
 *  terms of the GPL license.
 */

#include "dinorunner.h"
#include "platform.h"

static unsigned short lfsr = 0xACE1u;

void dinorunner_memcpy(void* dest, const void* src, unsigned n) {
  char* csrc  = (char*)src;
  char* cdest = (char*)dest;
  for (unsigned i = 0; i < n; i++) {
    cdest[i] = csrc[i];
  }
}

float dinorunner_ceilf(float f) {
  unsigned input;
  dinorunner_memcpy(&input, &f, 4);
  int exponent = ((input >> 23) & 255) - 127;
  if (exponent < 0)
    return (f > 0);
  // small numbers get rounded to 0 or 1, depending on their sign
  int fractional_bits = 23 - exponent;
  if (fractional_bits <= 0)
    return f;
  // numbers without fractional bits are mapped to themselves
  unsigned integral_mask = 0xffffffff << fractional_bits;
  unsigned output        = input & integral_mask;
  // round the number down by masking out the fractional bits
  dinorunner_memcpy(&f, &output, 4);
  if (f > 0 && output != input)
    ++f;
  // positive numbers need to be rounded up, not down
  return f;
}

float dinorunner_floorf(float x) {
  union {
    float f;
    unsigned i;
  } val;
  val.f               = x;
  int exponent        = (val.i >> 23) & 0xff;  // extract the exponent field;
  int fractional_bits = 127 + 23 - exponent;
  if (fractional_bits > 23)  // abs(x) < 1.0
    return x < 0.0f ? -1.0f : 0.0f;
  if (fractional_bits > 0) {
    unsigned fractional_mask = (1U << fractional_bits) - 1;
    if (val.i & fractional_mask) {
      val.i &= ~fractional_mask;
      // truncation rounds towards zero, negative numbers need to be rounded down
      if (x < 0.0f)
        val.f -= 1.0f;
    }
  }
  return val.f;
}

float dinorunner_pow(float x, int y) {
  float temp;
  if (y == 0) {
    return 1;
  }
  temp = dinorunner_pow(x, y / 2);
  if ((y % 2) == 0) {
    return temp * temp;
  } else {
    if (y > 0) {
      return x * temp * temp;
    } else {
      return (temp * temp) / x;
    }
  }
}

float dinorunner_roundf(float x) {
  return (x >= 0.0f) ? dinorunner_floorf(x + 0.5f) : dinorunner_ceilf(x - 0.5f);
}

// 16 bits rand
unsigned dinorunner_rand(void) {
  unsigned bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
  return lfsr  = (lfsr >> 1) | (bit << 15);
}

float dinorunner_srand(void) {
  unsigned rand = dinorunner_rand();
  return (float)rand / (unsigned short)(-1);
}

unsigned dinorunner_getrandomnumb(unsigned min_value, unsigned max_value) {
  float random_value = dinorunner_srand();
  if (min_value > max_value) {
    min_value = max_value;
  }
  unsigned interval = max_value - min_value;
  unsigned offset   = dinorunner_floorf(random_value * (interval + 1u));
  // random_value can be exactly 1.0f
  return min_value + (offset > interval ? interval : offset);
}

void dinorunner_seed(unsigned short random_seed) {
  // The LFSR never leaves the all-zero state
  lfsr = random_seed ? random_seed : 0xACE1u;
}