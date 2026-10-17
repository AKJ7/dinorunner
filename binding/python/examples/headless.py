"""Runs a few frames without a window and prints what the engine asks to draw.

python examples/headless.py
"""

import os
import sys
import time

import dinorunner


class Console(dinorunner.Platform):
    def __init__(self):
        self.high_score = None

    def timestamp(self):
        return time.monotonic_ns() // 1_000_000

    def clear_canvas(self):
        print("--- clear canvas")

    def draw(self, sprite, pos, opacity):
        print(f"draw {sprite.name} at ({pos.x}, {pos.y}), opacity {opacity}")

    def play_sound(self, sound):
        print(f"play {sound.name}")

    def read_high_score(self):
        return self.high_score

    def write_high_score(self, high_score):
        self.high_score = high_score

    def log(self, message):
        print(f"[libdinorunner] {message}", end="", file=sys.stderr)


def main():
    print(f"libdinorunner {dinorunner.version()}")
    dinorunner.seed(os.getpid() & 0xFFFF)
    game = dinorunner.Dinorunner(dinorunner.Dimension(600, 150), Console())
    # Press "up" to start the game, then run for 3 frames.
    # In real applications, keep running until the window is closed.
    game.key_up()
    for _ in range(3):
        game.update()
        time.sleep(1 / 60)


if __name__ == "__main__":
    main()
