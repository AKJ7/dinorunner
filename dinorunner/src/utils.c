#include "dinorunner.h"

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
    return 0.0;
  if (fractional_bits > 0) {
    val.i &= ~((1U << fractional_bits) - 1);
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
  // https://en.wikipedia.org/wiki/Lehmer_random_number_generator
  unsigned bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
  return lfsr  = (lfsr >> 1) | (bit << 15);
}

float dinorunner_srand(void) {
  unsigned rand = dinorunner_rand();
  return (float)rand / 0xFFFF;
}

unsigned dinorunner_getrandomnumb(unsigned min_value, unsigned max_value) {
  unsigned value = dinorunner_rand();
  value %= (max_value + 1);
  if (value >= max_value) {
    value = max_value;
  } else if (value <= min_value) {
    value = min_value;
  }
  return value;
}
