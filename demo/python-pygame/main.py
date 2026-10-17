"""libdinorunner with pygame: the original sprites and sounds in a desktop window.

Usage: python main.py [--high-score-file PATH]
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Optional

import pygame

import dinorunner
from dinorunner import Pos, Sound, Sprite

GAME_DIMENSION = dinorunner.Dimension(600, 150)
PADDING = 20
WINDOW_SIZE = (GAME_DIMENSION.width + 2 * PADDING, GAME_DIMENSION.height + 2 * PADDING)
FPS = 60
BACKGROUND = (247, 247, 247)
NIGHT_BACKGROUND = tuple(255 - channel for channel in BACKGROUND)
ASSETS = Path(__file__).resolve().parent.parent / "assets"
DEFAULT_HIGH_SCORE_FILE = Path(__file__).resolve().parent / "highscore.txt"
#: The intro reveals the landscape from the T-Rex's width to the full canvas in this time
INTRO_DURATION_MS = 400
TREX_WIDTH = 44
#: Night mode fades in and out over this time
NIGHT_FADE_MS = 1500
JUMP_KEYS = {pygame.K_SPACE, pygame.K_UP, pygame.K_w}
DUCK_KEYS = {pygame.K_DOWN, pygame.K_s}
#: Gamepad: A/cross jumps, the stick or D-pad pointing down ducks
GAMEPAD_JUMP_BUTTON = 0
GAMEPAD_RUMBLE = 1.0
#: Horizontal offset of each moon phase inside the sprite sheet
MOON_PHASES = (140, 120, 100, 60, 40, 20, 0)


def sprite_sources() -> dict[Sprite, pygame.Rect]:
    """Area of each sprite in the sprite sheet"""
    sources = {
        Sprite.CACTUS_LARGE_1: (332, 2, 25, 50),
        Sprite.CACTUS_LARGE_2: (357, 2, 50, 50),
        Sprite.CACTUS_LARGE_3: (407, 2, 75, 50),
        Sprite.CACTUS_SMALL_1: (228, 2, 17, 35),
        Sprite.CACTUS_SMALL_2: (245, 2, 34, 35),
        Sprite.CACTUS_SMALL_3: (279, 2, 51, 35),
        Sprite.PTERODACTYL_1: (134, 2, 46, 40),
        Sprite.PTERODACTYL_2: (180, 2, 46, 40),
        Sprite.CLOUD: (86, 2, 46, 14),
        Sprite.HORIZON_1: (2, 54, 600, 12),
        Sprite.HORIZON_2: (602, 54, 600, 12),
        Sprite.RESTART: (2, 2, 36, 32),
        Sprite.TEXT: (655, 2, 191, 11),
        Sprite.GAME_OVER: (654, 15, 191, 11),
        Sprite.TREX_STANDING_1: (848, 2, 44, 47),
        Sprite.TREX_JUMPING: (848, 2, 44, 47),
        Sprite.TREX_STANDING_2: (892, 2, 44, 47),
        Sprite.TREX_RUNNING_1: (936, 2, 44, 47),
        Sprite.TREX_RUNNING_2: (980, 2, 44, 47),
        Sprite.TREX_CRASHED: (1068, 2, 44, 47),
        Sprite.TREX_DUCKING_1: (1112, 2, 59, 47),
        Sprite.TREX_DUCKING_2: (1171, 2, 59, 47),
        Sprite.HI: (754, 2, 20, 13),
    }
    for phase, sprite in enumerate(range(Sprite.MOON_1, Sprite.MOON_7 + 1)):
        # The full moon is twice as wide
        width = 40 if phase == 3 else 20
        sources[Sprite(sprite)] = (484 + MOON_PHASES[phase], 2, width, 40)
    for index, sprite in enumerate((Sprite.STAR_1, Sprite.STAR_2, Sprite.STAR_3)):
        sources[sprite] = (645, 2 + 9 * index, 8, 9)
    for digit in range(10):
        sources[Sprite(Sprite.DIGIT_0 + digit)] = (654 + 10 * digit, 2, 10, 13)
    return {sprite: pygame.Rect(rect) for sprite, rect in sources.items()}


def inverted(sheet: pygame.Surface) -> pygame.Surface:
    """The sprite sheet with inverted colors and the same transparency, for night mode"""
    if hasattr(pygame.transform, "invert"):  # pygame-ce
        return pygame.transform.invert(sheet)
    result = sheet.copy()
    for x in range(result.get_width()):
        for y in range(result.get_height()):
            red, green, blue, alpha = result.get_at((x, y))
            result.set_at((x, y), (255 - red, 255 - green, 255 - blue, alpha))
    return result


def vertical_padding(sprite: Sprite) -> int:
    """The engine places the horizon and the clouds relative to the whole canvas, not the game area"""
    return 0 if sprite in (Sprite.HORIZON_1, Sprite.HORIZON_2, Sprite.CLOUD) else PADDING


class NightMode:
    """Fades the background between day and night. Sprites switch to inverted colors half way,
    so that they always contrast with the background (fading the whole frame to its inverse
    would turn everything into a uniform gray half way)."""

    def __init__(self) -> None:
        #: 0.0 is day, 1.0 is night
        self.level = 0.0

    def advance(self, active: bool, elapsed_ms: int) -> None:
        step = elapsed_ms / NIGHT_FADE_MS
        self.level = min(1.0, self.level + step) if active else max(0.0, self.level - step)

    def background(self) -> tuple[int, int, int]:
        return tuple(
            round(day + (night - day) * self.level) for day, night in zip(BACKGROUND, NIGHT_BACKGROUND)
        )


class PygameFrontend(dinorunner.Platform):
    """Draws into an off-screen frame, plays sounds and stores the high score"""

    def __init__(self, high_score_file: Path) -> None:
        self.frame = pygame.Surface(WINDOW_SIZE)
        sheet = pygame.image.load(ASSETS / "100-offline-sprite.png").convert_alpha()
        night_sheet = inverted(sheet)
        # Each sprite is its own surface, so that its opacity can be set independently
        self.sprites = {sprite: sheet.subsurface(rect).copy() for sprite, rect in sprite_sources().items()}
        self.night_sprites = {sprite: night_sheet.subsurface(rect).copy() for sprite, rect in sprite_sources().items()}
        self.night_mode = NightMode()
        self.sounds = self._load_sounds()
        self.gamepad: Optional[pygame.joystick.JoystickType] = None
        self.high_score_file = high_score_file
        #: Width of the visible game area, grows during the intro
        self.visible_width = TREX_WIDTH

    @staticmethod
    def _load_sounds() -> dict[Sound, pygame.mixer.Sound]:
        try:
            pygame.mixer.init()
            return {
                Sound.BUTTON_PRESS: pygame.mixer.Sound(ASSETS / "jump.wav"),
                Sound.SCORE: pygame.mixer.Sound(ASSETS / "point.wav"),
                Sound.HIT: pygame.mixer.Sound(ASSETS / "die.wav"),
            }
        except pygame.error as error:
            print(f"Sound disabled: {error}", file=sys.stderr)
            return {}

    def timestamp(self) -> int:
        return pygame.time.get_ticks()

    def clear_canvas(self) -> None:
        self.frame.fill(self.night_mode.background())

    def draw(self, sprite: Sprite, pos: Pos, opacity: int) -> bool:
        # Light sprites once the background is more dark than light
        sprites = self.night_sprites if self.night_mode.level >= 0.5 else self.sprites
        image = sprites.get(sprite)
        if image is None:
            return False
        image.set_alpha(opacity)
        self.frame.set_clip(pygame.Rect(PADDING, PADDING, self.visible_width, GAME_DIMENSION.height))
        self.frame.blit(image, (pos.x + PADDING, pos.y + vertical_padding(sprite)))
        self.frame.set_clip(None)
        return True

    def play_sound(self, sound: Sound) -> bool:
        effect = self.sounds.get(sound)
        if effect is None:
            return False
        effect.play()
        return True

    def vibrate(self, duration_ms: int) -> bool:
        if self.gamepad is None:
            return False
        return self.gamepad.rumble(GAMEPAD_RUMBLE, GAMEPAD_RUMBLE, duration_ms)

    def read_high_score(self) -> Optional[int]:
        try:
            return int(self.high_score_file.read_text().strip())
        except (OSError, ValueError):
            return None

    def write_high_score(self, high_score: int) -> bool:
        try:
            self.high_score_file.write_text(f"{high_score}\n")
        except OSError as error:
            print(f"Could not store the high score: {error}", file=sys.stderr)
            return False
        return True

    def log(self, message: str) -> None:
        print(f"[libdinorunner] {message}", end="" if message.endswith("\n") else "\n", file=sys.stderr)


class Demo:
    def __init__(self, high_score_file: Path) -> None:
        pygame.init()
        pygame.display.set_caption("Dinorunner")
        # SCALED: a small logical resolution, scaled up to fit the screen
        self.screen = pygame.display.set_mode(WINDOW_SIZE, pygame.SCALED | pygame.RESIZABLE)
        self.clock = pygame.time.Clock()
        self.frontend = PygameFrontend(high_score_file)
        self.intro_start: Optional[int] = None
        dinorunner.seed(pygame.time.get_ticks() & 0xFFFF)
        self.game = dinorunner.Dinorunner(GAME_DIMENSION, self.frontend)
        self.running = True

    def handle(self, event: pygame.event.Event) -> None:
        game = self.game
        if event.type == pygame.QUIT:
            self.running = False
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                self.running = False
            elif event.key in JUMP_KEYS:
                game.key_up()
            elif event.key in DUCK_KEYS:
                game.key_down()
        elif event.type == pygame.KEYUP and event.key in JUMP_KEYS | DUCK_KEYS:
            game.key_none()
        # SDL also reports taps on touch screens as mouse clicks
        elif event.type == pygame.MOUSEBUTTONDOWN:
            game.key_up()
        elif event.type == pygame.MOUSEBUTTONUP:
            game.key_none()
        elif event.type == pygame.JOYDEVICEADDED and self.frontend.gamepad is None:
            self.frontend.gamepad = pygame.joystick.Joystick(event.device_index)
        elif event.type == pygame.JOYDEVICEREMOVED:
            gamepad = self.frontend.gamepad
            if gamepad is not None and gamepad.get_instance_id() == event.instance_id:
                self.frontend.gamepad = None
        elif event.type == pygame.JOYBUTTONDOWN and event.button == GAMEPAD_JUMP_BUTTON:
            game.key_up()
        elif event.type == pygame.JOYBUTTONUP and event.button == GAMEPAD_JUMP_BUTTON:
            game.key_none()
        elif event.type == pygame.JOYHATMOTION:
            if event.value[1] < 0:
                game.key_down()
            else:
                game.key_none()
        elif event.type == pygame.JOYAXISMOTION and event.axis == 1:
            if event.value > 0.5:
                game.key_down()
            elif abs(event.value) < 0.2:
                game.key_none()

    def step(self, elapsed_ms: int) -> None:
        """Advances the game by one frame and shows it"""
        for event in pygame.event.get():
            self.handle(event)
        if self.game.alive:
            now = pygame.time.get_ticks()
            if self.intro_start is None:
                self.intro_start = now
            progress = min(1.0, (now - self.intro_start) / INTRO_DURATION_MS)
            self.frontend.visible_width = round(TREX_WIDTH + (GAME_DIMENSION.width - TREX_WIDTH) * progress)
        self.frontend.night_mode.advance(self.game.inverted, elapsed_ms)
        self.game.update()
        self.screen.blit(self.frontend.frame, (0, 0))
        pygame.display.flip()

    def run(self) -> None:
        elapsed_ms = 0
        while self.running:
            self.step(elapsed_ms)
            elapsed_ms = self.clock.tick(FPS)
        pygame.quit()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument(
        "--high-score-file",
        type=Path,
        default=DEFAULT_HIGH_SCORE_FILE,
        help=f"where the high score is kept (default: {DEFAULT_HIGH_SCORE_FILE})",
    )
    arguments = parser.parse_args()
    print(f"libdinorunner {dinorunner.version()}, pygame {pygame.version.ver}")
    Demo(arguments.high_score_file).run()


if __name__ == "__main__":
    main()
