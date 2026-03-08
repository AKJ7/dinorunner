# Dinorunner CPP binding  

C++ binding of the dinorunner API. 


## Usage  
This binding encapsulates the C-APIs into a namespace `dinorunner` for C++ usage.  
The parametrisable functions are virtual methods inside `dinorunner::Dinorunner` and should 
be implemented.

## Install 


## APIs
The following routines are available
```cpp
namespace dinorunner {
static inline std::optional<Version> GetVersion();
static inline void Seed(unsigned short random_seed);
class Dinorunner {
 public:
  Dinorunner(const Dimension& game_dimension);
  inline bool Init();
  inline bool Update();
  inline std::optional<bool> IsInverted();
  inline std::optional<bool> IsAlive();
  inline void KeyUp() noexcept;
  inline void KeyDown() noexcept;
  inline void KeyNone() noexcept;
  virtual bool ReadHighScore(unsigned long& high_score);
  virtual bool WriteHighScore(unsigned long high_score);
  virtual unsigned long GetTimestamp();
  virtual bool PlaySound(Sound sound);
  virtual bool Vibrate(unsigned duration);
  virtual bool ClearCanvas();
  virtual bool Draw(Sprite sprite, const Pos& pos, unsigned char opacity);
  virtual bool Log(const char* message);
}
```

## Example
Check out the demo for a proper example.  
```cpp
#include "dinorunner.hpp"

static constexpr dinorunner::Dimension kGameDimension{600, 150};

class MyDinorunner final : private virtual dinorunner::Dinorunner {
public:
  MyDinorunner(): Dinorunner(kGameDimension) {}
  bool ReadHighScore(unsigned long& high_score) override { /* TODO */ }
  bool WriteHighScore(unsigned long high_score) override { /* TODO */ }
  unsigned long GetTimestamp() override { /* TODO */ }
  bool PlaySound([[maybe_unused]] dinorunner::Sound sound) override { /* TODO */ }
  bool Vibrate(unsigned duration) override { /* TODO */ }
  bool ClearCanvas() override { /* TODO */ }
  bool Draw(dinorunner::Sprite sprite, const dinorunner::Pos& pos, unsigned char opacity) override{ /* TODO */ }
  bool Log(const char* message) override { /* TODO */ }
};
```