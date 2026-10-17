"""Python bindings of libdinorunner, a port of Chrome's Dino game in C.

The engine does not draw, play sounds or keep time by itself: it asks the
application to do so through a :class:`Platform`. Subclass it for your
frontend, create a :class:`Dinorunner` and call :meth:`Dinorunner.update`
once per frame::

    import time
    import dinorunner

    class Console(dinorunner.Platform):
        def timestamp(self):
            return time.monotonic_ns() // 1_000_000

        def clear_canvas(self):
            pass

        def draw(self, sprite, pos, opacity):
            print(f"{sprite.name} at {pos}")

    game = dinorunner.Dinorunner(dinorunner.Dimension(600, 150), Console())
    game.key_up()  # starts the game
    while True:
        game.update()
"""

from __future__ import annotations

import abc
import enum
from typing import NamedTuple, Optional, Union

from . import _dinorunner
from ._dinorunner import DinorunnerError

__all__ = [
    "Dimension",
    "Dinorunner",
    "DinorunnerError",
    "Platform",
    "Pos",
    "Sound",
    "Sprite",
    "Version",
    "seed",
    "version",
]


class Sprite(enum.IntEnum):
    """Sprites the engine asks the :class:`Platform` to draw."""

    CACTUS_LARGE_1 = 0
    CACTUS_LARGE_2 = 1
    CACTUS_LARGE_3 = 2
    CACTUS_SMALL_1 = 3
    CACTUS_SMALL_2 = 4
    CACTUS_SMALL_3 = 5
    PTERODACTYL_1 = 6
    PTERODACTYL_2 = 7
    PTERODACTYL_3 = 8
    CLOUD = 9
    HORIZON_1 = 10
    HORIZON_2 = 11
    MOON_1 = 12
    MOON_2 = 13
    MOON_3 = 14
    MOON_4 = 15
    MOON_5 = 16
    MOON_6 = 17
    MOON_7 = 18
    RESTART = 19
    TEXT = 20
    TREX_STANDING_1 = 21
    TREX_STANDING_2 = 22
    TREX_RUNNING_1 = 23
    TREX_RUNNING_2 = 24
    TREX_CRASHED = 25
    TREX_DUCKING_1 = 26
    TREX_DUCKING_2 = 27
    TREX_JUMPING = 28
    STAR_1 = 29
    STAR_2 = 30
    STAR_3 = 31
    DIGIT_0 = 32
    DIGIT_1 = 33
    DIGIT_2 = 34
    DIGIT_3 = 35
    DIGIT_4 = 36
    DIGIT_5 = 37
    DIGIT_6 = 38
    DIGIT_7 = 39
    DIGIT_8 = 40
    DIGIT_9 = 41
    #: The "HI" label in front of the high score
    HI = 42
    GAME_OVER = 43


class Sound(enum.IntEnum):
    """Sound effects the engine asks the :class:`Platform` to play."""

    #: Jump
    BUTTON_PRESS = 0
    #: Every 100 points
    SCORE = 1
    #: Collision with an obstacle
    HIT = 2


class Pos(NamedTuple):
    """Position on the canvas, in pixels from the top left corner."""

    x: int
    y: int


class Dimension(NamedTuple):
    """Size of the game canvas in pixels."""

    width: int
    height: int


class Version(NamedTuple):
    """Semantic version of libdinorunner."""

    major: int
    minor: int
    patch: int

    def __str__(self) -> str:
        return f"v{self.major}.{self.minor}.{self.patch}"


def version() -> Version:
    """Version of the compiled libdinorunner."""
    return Version(*_dinorunner.version())


def seed(random_seed: int) -> None:
    """Reseed the engine's pseudo-random generator, shared by all games of the process.

    ``random_seed`` must be in ``range(0, 65536)``.
    """
    _dinorunner.seed(random_seed)


class Platform(abc.ABC):
    """Services the engine needs from the application: time, drawing, sound and storage.

    :meth:`timestamp`, :meth:`clear_canvas` and :meth:`draw` must be implemented,
    the other methods do nothing by default.

    Methods that report success may return ``None`` or a truthy value for success,
    a falsy value other than ``None`` reports failure to the engine. An exception
    raised by a method is raised again from the :class:`Dinorunner` method that
    drove the engine; the engine skips the remaining callbacks of that call.
    """

    @abc.abstractmethod
    def timestamp(self) -> int:
        """Milliseconds elapsed since an arbitrary, fixed point in time. Must not go backwards."""

    @abc.abstractmethod
    def clear_canvas(self) -> Optional[bool]:
        """Clear the canvas before a new frame is drawn."""

    @abc.abstractmethod
    def draw(self, sprite: Sprite, pos: Pos, opacity: int) -> Optional[bool]:
        """Draw ``sprite`` with its top left corner at ``pos``.

        ``opacity`` goes from 0 (invisible) to 255 (opaque).
        """

    def play_sound(self, sound: Sound) -> Optional[bool]:
        """Play a sound effect."""
        return None

    def vibrate(self, duration_ms: int) -> Optional[bool]:
        """Vibrate the device, e.g. a gamepad, for ``duration_ms`` milliseconds."""
        return None

    def read_high_score(self) -> Optional[int]:
        """Stored high score, ``None`` when there is none yet."""
        return None

    def write_high_score(self, high_score: int) -> Optional[bool]:
        """Store a new high score."""
        return None

    def log(self, message: str) -> None:
        """Diagnostic message from the engine."""


class _Adapter:
    """Converts the engine's raw values for the platform."""

    __slots__ = ("_platform",)

    def __init__(self, platform: Platform) -> None:
        self._platform = platform

    def timestamp(self) -> int:
        return self._platform.timestamp()

    def clear_canvas(self) -> Optional[bool]:
        return self._platform.clear_canvas()

    def draw(self, sprite: int, x: int, y: int, opacity: int) -> Optional[bool]:
        return self._platform.draw(Sprite(sprite), Pos(x, y), opacity)

    def play_sound(self, sound: int) -> Optional[bool]:
        return self._platform.play_sound(Sound(sound))

    def vibrate(self, duration_ms: int) -> Optional[bool]:
        return self._platform.vibrate(duration_ms)

    def read_high_score(self) -> Optional[int]:
        return self._platform.read_high_score()

    def write_high_score(self, high_score: int) -> Optional[bool]:
        return self._platform.write_high_score(high_score)

    def log(self, message: str) -> None:
        self._platform.log(message)


class Dinorunner:
    """A running game, drawing through a :class:`Platform`.

    The engine calls the platform from within the constructor (to read the high
    score and draw the first frame), :meth:`update` and the ``key_*`` methods.
    The platform must not call these methods itself, which raises
    :class:`RuntimeError`; reading :attr:`alive` and :attr:`inverted` is allowed.
    """

    def __init__(self, dimension: Union[Dimension, tuple[int, int]], platform: Platform) -> None:
        width, height = dimension
        self._platform = platform
        self._engine = _dinorunner.Engine(width, height, _Adapter(platform))

    @property
    def platform(self) -> Platform:
        """The platform the game draws through."""
        return self._platform

    def update(self) -> None:
        """Advance the game by one frame and draw it. Call it once per displayed frame."""
        self._engine.update()

    def key_up(self) -> None:
        """The jump key was pressed: starts the game, jumps, or restarts after a crash."""
        self._engine.key_up()

    def key_down(self) -> None:
        """The duck key was pressed: ducks, or drops faster while jumping."""
        self._engine.key_down()

    def key_none(self) -> None:
        """All keys were released: stops ducking and ends the jump early for lower jumps."""
        self._engine.key_none()

    @property
    def alive(self) -> bool:
        """Whether the intro is over and the game is running."""
        return self._engine.alive

    @property
    def inverted(self) -> bool:
        """Whether night mode is active."""
        return self._engine.inverted
