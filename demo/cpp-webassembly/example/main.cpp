#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <type_traits>
#include "dinorunner.hpp"

static constexpr dinorunner::Dimension kGameDimension{.width = 600, .height = 150};

class MyDinorunner final : public dinorunner::Dinorunner, virtual private dinorunner::DinorunnerImpl {
 public:
  MyDinorunner(const dinorunner::Dimension& game_dimension)
      : Dinorunner(game_dimension, *this), nightmode_interpolation_{0}, high_score_{0} {
    const auto seed = GetSeed<unsigned short>();
    dinorunner::Seed(seed);
  }

  template <typename T>
  T GetSeed() {
    std::mt19937 gen(random_device_());
    std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    return dist(gen);
  }

  bool ReadHighScore(unsigned long& high_score) override {
    high_score = high_score_;
    return true;
  }

  bool WriteHighScore(unsigned long high_score) override {
    high_score_ = high_score;
    return true;
  }

  unsigned long GetTimestamp() override { return 2; }

  bool PlaySound(dinorunner::Sound sound) override { return true; }

  bool Vibrate(unsigned duration) override {
    // noop
    return true;
  }

  bool ClearCanvas() override { return true; }

  bool Draw(dinorunner::Sprite sprite, const dinorunner::Pos& pos, unsigned char opacity) override { return true; }

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
  auto result     = hypervisor.Init();
  auto run_result = hypervisor.Update();
  return EXIT_SUCCESS;
}