/**
 * @file platform.c
 *
 * @copyright Copyright (C) 2025 - All Rights Reserved
 *  You may use, distribute and modify this code under the
 *  terms of the GPL license.
 */

/* Freestanding header, available without a C library */
#include <stdarg.h>
#include "platform.h"

#define DINORUNNER_LOG_BUFFER_SIZE 256u

unsigned char dinorunner_platform_init(struct dinorunner_platform_s* platform,
                                       const struct dinorunner_callbacks_s* callbacks, void* user_data) {
  if (callbacks == 0 || callbacks->gettimestamp == 0 || callbacks->clearcanvas == 0 || callbacks->draw == 0) {
    return 0u;
  }
  platform->callbacks = *callbacks;
  platform->user_data = user_data;
  return 1u;
}

struct writer_s {
  char* buffer;
  unsigned size;
  unsigned length;
};

static void writer_put(struct writer_s* writer, char character) {
  if (writer->length + 1u < writer->size) {
    writer->buffer[writer->length++] = character;
  }
}

static void writer_put_unsigned(struct writer_s* writer, unsigned long value, unsigned base) {
  char digits[sizeof(unsigned long) * 8u];
  unsigned count = 0u;
  do {
    unsigned digit  = (unsigned)(value % base);
    digits[count++] = (char)(digit < 10u ? '0' + digit : 'a' + digit - 10u);
    value /= base;
  } while (value != 0u);
  while (count != 0u) {
    writer_put(writer, digits[--count]);
  }
}

/* The engine has no C library: a small vsnprintf for the conversions it uses */
static void format_message(char* buffer, unsigned size, const char* format, va_list args) {
  struct writer_s writer = {buffer, size, 0u};
  for (; *format != '\0'; ++format) {
    int is_long = 0;
    if (*format != '%') {
      writer_put(&writer, *format);
      continue;
    }
    ++format;
    if (*format == 'l') {
      is_long = 1;
      ++format;
    }
    switch (*format) {
      case 'd':
      case 'i': {
        long value = is_long ? va_arg(args, long) : va_arg(args, int);
        if (value < 0) {
          writer_put(&writer, '-');
          writer_put_unsigned(&writer, 0ul - (unsigned long)value, 10u);
        } else {
          writer_put_unsigned(&writer, (unsigned long)value, 10u);
        }
      } break;
      case 'u':
        writer_put_unsigned(&writer, is_long ? va_arg(args, unsigned long) : va_arg(args, unsigned), 10u);
        break;
      case 'x':
        writer_put_unsigned(&writer, is_long ? va_arg(args, unsigned long) : va_arg(args, unsigned), 16u);
        break;
      case 'c':
        writer_put(&writer, (char)va_arg(args, int));
        break;
      case 's': {
        const char* string = va_arg(args, const char*);
        for (string = string ? string : "(null)"; *string != '\0'; ++string) {
          writer_put(&writer, *string);
        }
      } break;
      case '%':
        writer_put(&writer, '%');
        break;
      case '\0':
        /* Trailing '%': step back so that the loop ends on the terminator */
        --format;
        break;
      default:
        writer_put(&writer, '%');
        writer_put(&writer, *format);
        break;
    }
  }
  buffer[writer.length] = '\0';
}

unsigned char dinorunner_platform_log(const struct dinorunner_platform_s* platform, const char* format, ...) {
  char buffer[DINORUNNER_LOG_BUFFER_SIZE];
  va_list args;
  if (platform->callbacks.log == 0 || format == 0) {
    return 0u;
  }
  va_start(args, format);
  format_message(buffer, DINORUNNER_LOG_BUFFER_SIZE, format, args);
  va_end(args);
  return platform->callbacks.log(buffer, platform->user_data);
}
