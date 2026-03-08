#include <chrono>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <type_traits>
#include "dinorunner.hpp"

static constexpr dinorunner::Dimension kGameDimension{600, 150};

class MyDinorunner final : private virtual dinorunner::Dinorunner {
 public:
  MyDinorunner(const dinorunner::Dimension& game_dimension)
      : Dinorunner(game_dimension), nightmode_interpolation_{}, high_score_{} {
    const auto seed = GetSeed<unsigned short>();
    dinorunner::Seed(seed);
  }

  template <typename T>
  T GetSeed() {
    std::mt19937 gen(random_device_());
    std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    return dist(gen);
  }

  bool Start() { return Init(); }

  bool Run() {
    bool result = true;
    // Run for 3 frame.
    // In real applications, keep running until the rendering window is closed
    for (unsigned frame_count = 0; frame_count < 3u; ++frame_count) {
      result &= Update();
    }
    return true;
  }

 private:
  bool ReadHighScore(unsigned long& high_score) override {
    high_score = high_score_;
    return true;
  }

  bool WriteHighScore(unsigned long high_score) override {
    high_score_ = high_score;
    return true;
  }

  unsigned long GetTimestamp() override {
    const auto now      = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    const auto millis   = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return static_cast<unsigned long>(millis);
  }

  bool PlaySound([[maybe_unused]] dinorunner::Sound sound) override {
    std::clog << "Playing sound: " << static_cast<int>(sound) << '\n';
    return true;
  }

  bool Vibrate([[maybe_unused]] unsigned duration) override {
    std::clog << "Vibrating for: " << static_cast<unsigned>(duration) << '\n';
    return true;
  }

  bool ClearCanvas() override {
    std::clog << "Clearing canvas\n";
    return true;
  }

  bool Draw(dinorunner::Sprite sprite, const dinorunner::Pos& pos, unsigned char opacity) override {
    std::clog << "Drawing: " << (int)sprite << " at (" << pos.x << ", " << pos.y
              << "), opacity: " << static_cast<unsigned>(opacity) << "\n";
    return true;
  }

  bool Log(const char* message) override {
    std::clog << "[libdinorunner]: " << message << '\n';
    return true;
  }

 private:
  std::int16_t nightmode_interpolation_;
  unsigned long high_score_;
  std::random_device random_device_;
};

auto main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) -> int {
  const auto version = dinorunner::GetVersion();
  if (!version) {
    std::cerr << "Could not read libdinorunner version\n";
    return EXIT_FAILURE;
  }
  std::clog << "Compiled with libdinorunner: " << version.value() << '\n';
  auto hypervisor = MyDinorunner(kGameDimension);
  auto result     = hypervisor.Start();
  if (!result) {
    std::clog << "Could not initialize hypervisor\n";
    return EXIT_FAILURE;
  }
  result = hypervisor.Run();
  if (!result) {
    std::clog << "Could not successfully run library\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}