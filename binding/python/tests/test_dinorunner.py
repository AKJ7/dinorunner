"""Run with: python -m unittest discover -s tests (or pytest)."""

import gc
import threading
import unittest
import weakref

import dinorunner
from dinorunner import Dimension, Dinorunner, DinorunnerError, Platform, Pos, Sound, Sprite, _dinorunner

DIMENSION = Dimension(600, 150)
FRAME_MS = 17


class Recorder(Platform):
    """Platform driven by a fake clock, recording what the engine asked for."""

    def __init__(self, high_score=None):
        self.now = 1_000
        self.draws = []
        self.sounds = []
        self.stored_high_score = high_score
        self.fail_draw = None
        self.game = None

    def timestamp(self):
        return self.now

    def clear_canvas(self):
        return None

    def draw(self, sprite, pos, opacity):
        if self.fail_draw is not None:
            raise self.fail_draw
        self.draws.append((sprite, pos, opacity))

    def play_sound(self, sound):
        self.sounds.append(sound)

    def read_high_score(self):
        return self.stored_high_score

    def write_high_score(self, high_score):
        self.stored_high_score = high_score
        return True


def run_frames(game, frames):
    for _ in range(frames):
        game.platform.now += FRAME_MS
        game.update()


class DeclarationTest(unittest.TestCase):
    def test_enums_match_header(self):
        self.assertEqual(len(Sprite), _dinorunner.SPRITE_COUNT)
        self.assertEqual(len(Sound), _dinorunner.SOUND_COUNT)
        self.assertEqual(list(Sprite), sorted(Sprite))

    def test_version(self):
        version = dinorunner.version()
        self.assertNotEqual(version, (0, 0, 0))
        self.assertRegex(str(version), r"^v\d+\.\d+\.\d+$")

    def test_seed_range(self):
        dinorunner.seed(0)
        dinorunner.seed(65535)
        with self.assertRaises(ValueError):
            dinorunner.seed(65536)
        with self.assertRaises(OverflowError):
            dinorunner.seed(-1)


class GameTest(unittest.TestCase):
    def test_init_draws_typed_values(self):
        game = Dinorunner(DIMENSION, Recorder())
        sprite, pos, opacity = game.platform.draws[0]
        self.assertIsInstance(sprite, Sprite)
        self.assertIsInstance(pos, Pos)
        self.assertEqual(opacity, 255)
        self.assertFalse(game.alive)

    def test_key_up_starts_the_game(self):
        game = Dinorunner(DIMENSION, Recorder())
        game.key_up()
        run_frames(game, 300)
        self.assertTrue(game.alive)
        self.assertFalse(game.inverted)
        self.assertEqual(game.platform.sounds[0], Sound.BUTTON_PRESS)

    def test_high_score_is_read_and_written(self):
        game = Dinorunner(DIMENSION, Recorder(high_score=None))
        game.key_up()
        # Run without jumping until the first crash
        for _ in range(10_000):
            run_frames(game, 1)
            if Sound.HIT in game.platform.sounds:
                break
        self.assertIn(Sound.HIT, game.platform.sounds)
        self.assertGreater(game.platform.stored_high_score, 0)

        restarted = Dinorunner(DIMENSION, Recorder(high_score=12345))
        hi_labels = [sprite for sprite, _, _ in restarted.platform.draws if sprite == Sprite.HI]
        self.assertEqual(len(hi_labels), 1, "the stored high score is shown")

    def test_callback_exception_is_raised_from_the_driving_call(self):
        game = Dinorunner(DIMENSION, Recorder())
        game.key_up()
        game.platform.fail_draw = ValueError("canvas lost")
        with self.assertRaisesRegex(ValueError, "canvas lost"):
            run_frames(game, 1)
        # The game keeps working once the platform recovers
        game.platform.fail_draw = None
        run_frames(game, 300)
        self.assertTrue(game.alive)

    def test_exception_during_init(self):
        platform = Recorder()
        platform.fail_draw = KeyError("no sprite sheet")
        with self.assertRaises(KeyError):
            Dinorunner(DIMENSION, platform)

    def test_falsy_result_is_failure_not_error(self):
        class Failing(Recorder):
            def clear_canvas(self):
                return False

        game = Dinorunner(DIMENSION, Failing())
        run_frames(game, 3)

    def test_bad_timestamp_type(self):
        class BadClock(Recorder):
            def timestamp(self):
                return "now"

        with self.assertRaises(TypeError):
            Dinorunner(DIMENSION, BadClock())

    def test_engine_cannot_be_driven_from_a_callback(self):
        class Reentrant(Recorder):
            def draw(self, sprite, pos, opacity):
                if self.game is not None:
                    self.game.alive  # reading is allowed
                    self.game.update()

        platform = Reentrant()
        game = Dinorunner(DIMENSION, platform)
        platform.game = game
        with self.assertRaisesRegex(RuntimeError, "within one of its callbacks"):
            run_frames(game, 1)

    def test_minimal_platform(self):
        class Minimal(Platform):
            def timestamp(self):
                return 0

            def clear_canvas(self):
                pass

            def draw(self, sprite, pos, opacity):
                pass

        game = Dinorunner((600, 150), Minimal())
        game.key_up()
        game.update()

    def test_reference_cycles_are_collected(self):
        platform = Recorder()
        game = Dinorunner(DIMENSION, platform)
        platform.game = game
        reference = weakref.ref(game)
        del game, platform
        gc.collect()
        self.assertIsNone(reference())

    def test_games_on_threads(self):
        results = []

        def play():
            game = Dinorunner(DIMENSION, Recorder())
            game.key_up()
            run_frames(game, 500)
            results.append(game.alive)

        threads = [threading.Thread(target=play) for _ in range(4)]
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()
        self.assertEqual(results, [True] * 4)


if __name__ == "__main__":
    unittest.main()
