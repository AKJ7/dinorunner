use dinorunner::{Dimension, Dinorunner, Platform, Pos, Sound, Sprite};
use std::cell::Cell;
use std::rc::Rc;

const DIMENSION: Dimension = Dimension {
    width: 600,
    height: 150,
};
const FRAME_MS: u64 = 17;

/// Platform driven by a fake clock, recording what the engine asked for
#[derive(Default)]
struct Recorder {
    clock: Rc<Cell<u64>>,
    draws: usize,
    sounds: Vec<Sound>,
    panic_on_draw: bool,
}

impl Platform for Recorder {
    fn timestamp(&mut self) -> u64 {
        self.clock.get()
    }

    fn clear_canvas(&mut self) -> bool {
        true
    }

    fn draw(&mut self, _sprite: Sprite, _pos: Pos, _opacity: u8) -> bool {
        assert!(!self.panic_on_draw, "draw failed");
        self.draws += 1;
        true
    }

    fn play_sound(&mut self, sound: Sound) -> bool {
        self.sounds.push(sound);
        true
    }
}

fn run_frames<P: Platform>(game: &mut Dinorunner<P>, clock: &Cell<u64>, frames: usize) {
    for _ in 0..frames {
        clock.set(clock.get() + FRAME_MS);
        game.update().unwrap();
    }
}

#[test]
fn version_is_set() {
    assert_ne!(dinorunner::version().to_string(), "v0.0.0");
}

#[test]
fn key_up_starts_the_game() {
    let clock = Rc::new(Cell::new(1_000));
    let mut game = Dinorunner::new(
        DIMENSION,
        Recorder {
            clock: clock.clone(),
            ..Default::default()
        },
    )
    .unwrap();
    assert!(game.platform().draws > 0, "init draws the first frame");
    assert!(!game.is_alive());

    game.key_up();
    run_frames(&mut game, &clock, 300);

    assert!(game.is_alive());
    assert!(!game.is_inverted());
    assert_eq!(game.platform().sounds.first(), Some(&Sound::ButtonPress));
}

#[test]
fn panicking_callback_does_not_reach_the_engine() {
    let clock = Rc::new(Cell::new(0));
    let platform = Recorder {
        clock: clock.clone(),
        panic_on_draw: true,
        ..Default::default()
    };
    let mut game = Dinorunner::new(DIMENSION, platform).unwrap();
    game.key_up();
    run_frames(&mut game, &clock, 10);

    game.platform_mut().panic_on_draw = false;
    run_frames(&mut game, &clock, 1);
    assert!(game.platform().draws > 0);
}

#[test]
fn games_can_be_dropped_and_recreated() {
    for _ in 0..100 {
        let clock = Rc::new(Cell::new(0));
        let mut game = Dinorunner::new(
            DIMENSION,
            Recorder {
                clock: clock.clone(),
                ..Default::default()
            },
        )
        .unwrap();
        game.key_up();
        run_frames(&mut game, &clock, 5);
    }
}

#[test]
fn games_run_on_separate_threads() {
    let threads: Vec<_> = (0..4)
        .map(|_| {
            std::thread::spawn(|| {
                let clock = Rc::new(Cell::new(0));
                let mut game = Dinorunner::new(
                    DIMENSION,
                    Recorder {
                        clock: clock.clone(),
                        ..Default::default()
                    },
                )
                .unwrap();
                game.key_up();
                run_frames(&mut game, &clock, 500);
                game.is_alive()
            })
        })
        .collect();
    for thread in threads {
        assert!(thread.join().unwrap());
    }
}
