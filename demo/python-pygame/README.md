# dinorunner with pygame

Chrome's Dino game in a desktop window: libdinorunner's C engine, driven through the
[Python binding](../../binding/python) and drawn with [pygame](https://pyga.me).

It uses the original sprite sheet and sounds, keeps the high score in a file and supports
night mode, the keyboard, the mouse, touch screens and gamepads (with rumble on crash).

## Requirements

- Python 3.10 or newer
- A C compiler and the Python development headers, to build the binding
  (e.g. `sudo apt install build-essential python3-dev`)
- pygame: `pygame-ce` is recommended, it provides wheels for the latest Python versions. The
  classic `pygame` package works as well, where it is available for your Python version.

## Run

From the repository root:

```shell
python3 -m venv .venv && . .venv/bin/activate
pip install ./binding/python -r demo/python-pygame/requirements.txt
python demo/python-pygame/main.py
```

The window can be resized, the game is scaled to fit.

| Option | Default |
| --- | --- |
| `--high-score-file PATH` | `highscore.txt` next to `main.py` |

## Controls

| Action | Keyboard | Mouse / touch | Gamepad |
| --- | --- | --- | --- |
| Start, jump, restart | <kbd>Space</kbd>, <kbd>↑</kbd> or <kbd>W</kbd> | Click / tap | A (button 0) |
| Duck, drop faster while jumping | <kbd>↓</kbd> or <kbd>S</kbd> | | D-pad or left stick down |
| Quit | <kbd>Esc</kbd> | Close the window | |

Releasing the jump key early makes a lower jump.

## How it works

`main.py` implements `dinorunner.Platform` with pygame:

| Engine request | pygame |
| --- | --- |
| `timestamp` | `pygame.time.get_ticks()` |
| `clear_canvas`, `draw` | Blits sprites cut out of the sprite sheet into an off-screen frame |
| `play_sound` | `pygame.mixer.Sound` |
| `vibrate` | `Joystick.rumble()` of the connected gamepad |
| `read_high_score`, `write_high_score` | The high score file |
| `log` | Standard error |

Each frame, the main loop handles the input events, calls `Dinorunner.update()` once and shows
the frame, at 60 frames per second.

Night mode fades the background from light to dark. Half way through the fade, the sprites switch
to an inverted copy of the sprite sheet, so that they always contrast with the background.

## Troubleshooting

**`Sound disabled: ...`** on start: no audio device could be opened. The game runs without sound.

**`No matching distribution found for pygame`**: the classic `pygame` package has no wheels for
your Python version yet. Install `pygame-ce` instead, as listed in `requirements.txt`.
