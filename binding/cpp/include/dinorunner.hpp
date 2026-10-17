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

#include <exception>
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
static inline void Seed(unsigned short random_seed) {
  dinorunner_seed(random_seed);
}

class Dinorunner : private dinorunner_s {
 public:
  explicit Dinorunner(const Dimension& game_dimension) : game_dimension_{game_dimension} {}
  virtual ~Dinorunner() = default;

  // The C engine keeps a pointer to this object to call it back: it must not be copied or moved
  Dinorunner(const Dinorunner&)            = delete;
  Dinorunner& operator=(const Dinorunner&) = delete;
  Dinorunner(Dinorunner&&)                 = delete;
  Dinorunner& operator=(Dinorunner&&)      = delete;

  /**
   * @brief Get the dimension of the game canvas
   */
  inline const Dimension& GameDimension() const noexcept { return game_dimension_; }

  /**
   * @brief Initialize engine
   * 
   * @return true on success, false otherwise
   */
  inline bool Init() {
    static constexpr dinorunner_callbacks_s kCallbacks{
        &OnGetTimestamp, &OnClearCanvas,    &OnDraw,           &OnPlaySound,
        &OnVibrate,      &OnReadHighScore, &OnWriteHighScore, &OnLog,
    };
    auto result = dinorunner_init(this, &game_dimension_, &kCallbacks, this);
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
    unsigned char is_inverted;
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
  inline std::optional<bool> IsAlive() const {
    unsigned char is_alive;
    if ([[maybe_unused]] auto status = dinorunner_isalive(this, &is_alive)) {
      return is_alive == 1u;
    }
    return {};
  }

  /**
   * @brief Trigger an UP pressed event
   */
  inline void KeyUp() noexcept { dinorunner_onkeyup(this); }

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
  * The following callbacks are called by the C engine. Exceptions thrown by an
  * implementation are caught at the C boundary and reported as failure, except in
  * GetTimestamp, where they terminate the program: the game cannot run without a clock.
  */

  /**
  * @brief Called by the C-API to request a read of the high-score
  *
  * @param high_score value to read into
  * @return true on success, false otherwise
  */
  virtual bool ReadHighScore(unsigned long& high_score) = 0;

  /**
  * @brief Called by C-API to request a write of the high-score
  *
  * @param high_score value to write
  * @return true on success, false otherwise
  */
  virtual bool WriteHighScore(unsigned long high_score) = 0;

  /**
  * @brief Called by the C-APi to get the engine's timestamp
  *
  * @return unsigned long timestamp
  */
  virtual unsigned long GetTimestamp() = 0;

  /**
  * @brief Called by the C-API to request a sound play
  *
  * @param sound sound to play
  * @return true on success, false otherwise
  */
  virtual bool PlaySound(Sound sound) = 0;

  /**
  * @brief Called by the C-API to request vibration
  *
  * @param duration duration of the vibration in milliseconds
  * @return true on success, false otherwise
  */
  virtual bool Vibrate(unsigned duration) = 0;

  /**
  * @brief Called by the C-API to request a flush/rendering of the canvas
  *
  * @return true on success, false otherwise
  */
  virtual bool ClearCanvas() = 0;

  /**
  * @brief Called by the C-API to request a draw of an object on the canvas
  *
  * @param sprite Sprite to draw
  * @param pos Position at which to draw
  * @param opacity Opacity of the sprite
  * @return true on success, false otherwise
  */
  virtual bool Draw(Sprite sprite, const Pos& pos, unsigned char opacity) = 0;

  /**
  * @brief Called by the C-API to request logging notification
  *
  * @param message message to log
  * @return true on success, false otherwise
  */
  virtual bool Log(const char* message) = 0;

 private:
  static Dinorunner& Self(void* user_data) noexcept {
    if (user_data == nullptr) {
      std::terminate();
    }
    return *static_cast<Dinorunner*>(user_data);
  }

  // Keeps exceptions from unwinding through the C engine
  template <typename Callback>
  static unsigned char Forward(Callback&& callback) noexcept {
    try {
      return callback() ? 1u : 0u;
    } catch (...) {
      return 0u;
    }
  }

  static unsigned long OnGetTimestamp(void* user_data) noexcept {
    // noexcept: an exception terminates the program, there is no meaningful fallback time
    return Self(user_data).GetTimestamp();
  }

  static unsigned char OnClearCanvas(void* user_data) noexcept {
    return Forward([&] { return Self(user_data).ClearCanvas(); });
  }

  static unsigned char OnDraw(Sprite sprite, const Pos* pos, unsigned char opacity, void* user_data) noexcept {
    if (pos == nullptr) {
      return 0u;
    }
    return Forward([&] { return Self(user_data).Draw(sprite, *pos, opacity); });
  }

  static unsigned char OnPlaySound(Sound sound, void* user_data) noexcept {
    return Forward([&] { return Self(user_data).PlaySound(sound); });
  }

  static unsigned char OnVibrate(unsigned duration, void* user_data) noexcept {
    return Forward([&] { return Self(user_data).Vibrate(duration); });
  }

  static unsigned char OnReadHighScore(unsigned long* high_score, void* user_data) noexcept {
    if (high_score == nullptr) {
      return 0u;
    }
    return Forward([&] { return Self(user_data).ReadHighScore(*high_score); });
  }

  static unsigned char OnWriteHighScore(unsigned long high_score, void* user_data) noexcept {
    return Forward([&] { return Self(user_data).WriteHighScore(high_score); });
  }

  static unsigned char OnLog(const char* message, void* user_data) noexcept {
    if (message == nullptr) {
      return 0u;
    }
    return Forward([&] { return Self(user_data).Log(message); });
  }

  Dimension game_dimension_;
};
}  // namespace dinorunner

inline std::ostream& operator<<(std::ostream& o, const dinorunner::Version& version) {
  return o << "v" << static_cast<unsigned>(version.major) << "." << static_cast<unsigned>(version.minor) << "."
           << static_cast<unsigned>(version.patch);
}
