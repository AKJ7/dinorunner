/**
 * @file dinorunner.hpp
 * @author AKJ7 (akj123429@gmail.com)
 * @brief Abstraction layer over libdinorunner's C-API for ease of use in cpp context
 * 
 * @copyright Copyright (C) 2026 - All Rights Reserved 
 *  You may use, distribute and modify this code under the 
 *  terms of the GPL license.
 */

#pragma once

#include <cstdarg>
#include <cstdlib>
#include <optional>
#include <ostream>
extern "C" {
#include "dinorunner.h"
}

namespace dinorunner {
using Dimension = struct dimension_s;
using Version   = struct version_s;
using Sprite    = enum dinorunner_sprite_e;
using Sound     = enum dinorunner_sound_e;
using Pos       = struct pos_s;

/**
 * @brief Get the Version of the unlerlying libdinorunner library
 * 
 * @return std::optional<Version> Failure on error, value on success
 */
static inline std::optional<Version> GetVersion() {
  Version version{};
  if ([[maybe_unused]] auto result = dinorunner_getversion(&version)) {
    return version;
  }
  return {};
}

/**
 * @brief Perform seed to randomize pseudo-randomness
 * 
 * @param random_seed Random seed
 */
inline static void Seed(unsigned short random_seed) {
  dinorunner_seed(random_seed);
}

/**
 * @brief Ugly trick to avoid templating the complete Dinorunner class
 * as the compiler doesn't know the user defined implementations for 
 * the reinterpret_casting below in the `extern C` functions. Yes, loss 
 * of performance occures, but it is merely the price we are ready to pay 
 * for accessability ...
 */
struct DinorunnerImpl {
  virtual bool ReadHighScore(unsigned long& high_score)                   = 0;
  virtual bool WriteHighScore(unsigned long high_score)                   = 0;
  virtual unsigned long GetTimestamp()                                    = 0;
  virtual bool PlaySound(Sound sound)                                     = 0;
  virtual bool Vibrate(unsigned duration)                                 = 0;
  virtual bool ClearCanvas()                                              = 0;
  virtual bool Draw(Sprite sprite, const Pos& pos, unsigned char opacity) = 0;
  virtual bool Log(const char* message)                                   = 0;
};

class Dinorunner : private dinorunner_s {
 public:
  Dinorunner(const Dimension& game_dimension, DinorunnerImpl& data) : data_{data}, game_dimension_{game_dimension} {}

  /**
   * @brief Initialize engine
   * 
   * @return true on success, false otherwise
   */
  inline bool Init() {
    auto result = dinorunner_init(this, &game_dimension_, this);
    return result == 1u;
  }

  /**
   * @brief Perform a single run. Basically run one frame pass
   * 
   * @return true on success, false otherwise
   */
  inline bool Update() {
    const auto result = dinorunner_update(this);
    return result == 1u;
  }

  /**
   * @brief Get the nightmode state of the engine.
   * 
   * @return std::optional<bool> Empty optional on error, otherwise value as nightmode status
   */
  inline std::optional<bool> IsInverted() const {
    unsigned char is_inverted = false;
    if ([[maybe_unused]] auto status = dinorunner_isinverted(this, &is_inverted)) {
      return is_inverted == 1u;
    }
    return {};
  }

  /**
   * @brief Get the running state of the engine
   * 
   * @return std::optional<bool> Empty optional on error, otherwise value as alive state
   */
  std::optional<bool> IsAlive() const {
    unsigned char is_alive;
    if ([[maybe_unused]] auto status = dinorunner_isalive(this, &is_alive)) {
      return is_alive == 1u;
    }
    return {};
  }

  /**
   * @brief Trigger an UP pressed event
   */
  inline void KeyUp() noexcept { dinorunner_onkeydown(this); }

  /**
   * @brief Trigger a DOWN pressed event
   */
  inline void KeyDown() noexcept { dinorunner_onkeydown(this); }

  /**
   * @brief Trigger a `no-key` pressed event
   * Useful for variadic jump heights
   */
  inline void KeyNone() noexcept { dinorunner_onkeynone(this); }

  /**
   * @brief Called by C-API to request a write of the high-score
   * 
   * @param high_score value to write
   * @return true on success, false otherwise
   */
  inline bool WriteHighScore(unsigned long high_score) { return data_.WriteHighScore(high_score); }

  /**
   * @brief Called by the C-API to request a read of the high-score
   * 
   * @param high_score value to read into
   * @return true on success, false otherwise
   */
  inline bool ReadHighScore(unsigned long& high_score) { return data_.ReadHighScore(high_score); }

  /**
   * @brief Called by the C-APi to get the engine's timestamp
   * 
   * @return unsigned long timestamp
   */
  inline unsigned long GetTimestamp() { return data_.GetTimestamp(); }

  /**
   * @brief Called by the C-API to request a sound play
   * 
   * @param sound sound to play
   * @return true on success, false otherwise
   */
  inline bool PlaySound(Sound sound) { return data_.PlaySound(sound); }

  /**
   * @brief Called by the C-API to request vibration
   * 
   * @param duration duration of the vibration in milliseconds
   * @return true on success, false otherwise
   */
  inline bool Vibrate(unsigned duration) { return data_.Vibrate(duration); }

  /**
   * @brief Called by the C-API to request a flush of the canvas
   * 
   * @return true on success, false otherwise
   */
  inline bool ClearCanvas() { return data_.ClearCanvas(); }

  /**
   * @brief Called by the C-API to request a draw of an object on the canvas
   * 
   * @param sprite Sprite to draw 
   * @param pos Position at which to draw
   * @param opacity Opacity of the sprite
   * @return true on success, false otherwise
   */
  inline bool Draw(Sprite sprite, const Pos& pos, unsigned char opacity) {
    std::clog << "Drawing sprite: " << (unsigned)sprite << ", at: (" << pos.x << ", " << pos.y << ")\n";
    return data_.Draw(sprite, pos, opacity);
  }

  /**
   * @brief Called by the C-API to request logging notification
   * 
   * @param message message to log
   * @return true on success, false otherwise
   */
  bool Log(const char* message) { return data_.Log(message); }

 private:
  DinorunnerImpl& data_;
  const Dimension& game_dimension_;
};

extern "C" {
unsigned long dinorunner_gettimestamp(void* user_data) {
  if (user_data == nullptr) {
    std::abort();
  }
  const auto result = reinterpret_cast<Dinorunner*>(user_data)->GetTimestamp();
  return result;
}

unsigned char dinorunner_writehighscore(unsigned long high_score, void* user_data) {
  if (user_data == nullptr) {
    std::abort();
  }
  auto result = reinterpret_cast<Dinorunner*>(user_data)->WriteHighScore(high_score);
  return result == true;
}

unsigned char dinorunner_readhighscore(unsigned long* high_score, void* user_data) {
  if (high_score == nullptr) {
    std::abort();
  }
  const auto result = reinterpret_cast<Dinorunner*>(user_data)->ReadHighScore(*high_score);
  return result == true;
}

unsigned char dinorunner_playsound(enum dinorunner_sound_e sound, void* user_data) {
  if (user_data == nullptr) {
    std::abort();
  }
  const auto result = reinterpret_cast<Dinorunner*>(user_data)->PlaySound(sound);
  return result == true;
}

unsigned char dinorunner_vibrate(unsigned duration, void* user_data) {
  if (user_data == nullptr) {
    std::abort();
  }
  const auto result = reinterpret_cast<Dinorunner*>(user_data)->Vibrate(duration);
  return result == true;
}

unsigned char dinorunner_clearcanvas(void* user_data) {
  if (user_data == nullptr) {
    std::abort();
  }
  const auto result = reinterpret_cast<Dinorunner*>(user_data)->ClearCanvas();
  return result == true;
}

unsigned char dinorunner_draw(enum dinorunner_sprite_e sprite, const struct pos_s* pos, unsigned char opacity,
                              void* user_data) {
  if (user_data == nullptr || pos == nullptr) {
    std::abort();
  }
  const auto result = reinterpret_cast<Dinorunner*>(user_data)->Draw(sprite, *pos, opacity);
  return result == true;
}

unsigned char dinorunner_log(void* user_data, const char* format, ...) {
  if (user_data == nullptr) {
    std::abort();
  }
  char buffer[256];
  std::va_list args;
  va_start(args, format);
  std::vsprintf(buffer, format, args);
  va_end(args);
  const auto result = reinterpret_cast<Dinorunner*>(user_data)->Log(buffer);
  return result == true;
}
}
}  // namespace dinorunner

std::ostream& operator<<(std::ostream& o, const dinorunner::Version& version) {
  return o << "v" << static_cast<unsigned>(version.major) << "." << static_cast<unsigned>(version.minor) << "."
           << static_cast<unsigned>(version.patch);
}
