# dinorunner for Python

Python bindings of [libdinorunner](https://github.com/AKJ7/dinorunner), Chrome's Dino game as a
platform independent C library.

The engine does not draw, play sounds or keep time by itself: it asks your application to do so
through a `Platform`. Subclass it for your frontend (pygame, a terminal, a test…), create a
`Dinorunner` and call `update()` once per frame.

## Installation

From a checkout of the repository; the C sources are compiled during installation, which needs a
C compiler and the Python development headers (e.g. `sudo apt install build-essential python3-dev`):

```shell
pip install ./binding/python
```

Python 3.10 or newer is required.

## Usage

```python
import time
import dinorunner


class Console(dinorunner.Platform):
    def timestamp(self):
        return time.monotonic_ns() // 1_000_000

    def clear_canvas(self):
        pass

    def draw(self, sprite, pos, opacity):
        print(f"{sprite.name} at ({pos.x}, {pos.y})")


game = dinorunner.Dinorunner(dinorunner.Dimension(600, 150), Console())
game.key_up()  # starts the game
while True:
    game.update()
    time.sleep(1 / 60)
```

`examples/headless.py` runs a few frames and prints every request of the engine.

## API

```python
def version() -> Version: ...              # Version(major, minor, patch), str() gives "v2.0.0"
def seed(random_seed: int) -> None: ...    # 0 <= random_seed < 65536, shared by all games

class Platform(abc.ABC):
    # Required
    def timestamp(self) -> int: ...        # milliseconds, must not go backwards
    def clear_canvas(self) -> bool | None: ...
    def draw(self, sprite: Sprite, pos: Pos, opacity: int) -> bool | None: ...  # opacity 0..255
    # Optional, they do nothing by default
    def play_sound(self, sound: Sound) -> bool | None: ...
    def vibrate(self, duration_ms: int) -> bool | None: ...
    def read_high_score(self) -> int | None: ...   # None: no high score yet
    def write_high_score(self, high_score: int) -> bool | None: ...
    def log(self, message: str) -> None: ...

class Dinorunner:
    def __init__(self, dimension: Dimension | tuple[int, int], platform: Platform): ...
    def update(self) -> None: ...          # one frame
    def key_up(self) -> None: ...          # jump; starts and restarts the game
    def key_down(self) -> None: ...        # duck; drops faster while jumping
    def key_none(self) -> None: ...        # keys released; ends jumps early
    alive: bool                            # read-only: the intro is over, the game runs
    inverted: bool                         # read-only: night mode
    platform: Platform                     # read-only
```

`Sprite` and `Sound` are `IntEnum`s, `Pos`, `Dimension` and `Version` are `NamedTuple`s.

### Callbacks and errors

- Platform methods may return `None` or a truthy value on success. Returning a falsy value other
  than `None` reports a failure to the engine, which ignores most of them.
- An exception raised by a platform method is raised again from the `Dinorunner` call that drove
  the engine (`update()`, a `key_*()` method or the constructor), with its original traceback. The
  engine skips the remaining callbacks of that call; the game can continue afterwards.
- Platform methods must not call `update()` or the `key_*()` methods, which raises `RuntimeError`.
  Reading `alive` and `inverted` is allowed, e.g. to darken the background in night mode.
- `DinorunnerError` (a `RuntimeError`) is raised when the engine itself reports a failure.

## Development

```shell
python3 -m venv .venv && . .venv/bin/activate
pip install ./binding/python
python -m unittest discover -s binding/python/tests
```

The build copies the C sources from `dinorunner/` of the repository into `build/`: the package
can only be built from a repository checkout, not from a standalone source distribution.
