//! Runs a few frames without a window and prints what the engine asks to draw.
//!
//! cargo run --example headless

use dinorunner::{Dimension, Dinorunner, Platform, Pos, Sound, Sprite};
use std::time::Instant;

struct Console {
    start: Instant,
    high_score: Option<u64>,
}

impl Platform for Console {
    fn timestamp(&mut self) -> u64 {
        self.start.elapsed().as_millis() as u64
    }

    fn clear_canvas(&mut self) -> bool {
        println!("--- clear canvas");
        true
    }

    fn draw(&mut self, sprite: Sprite, pos: Pos, opacity: u8) -> bool {
        println!(
            "draw {sprite:?} at ({}, {}), opacity {opacity}",
            pos.x, pos.y
        );
        true
    }

    fn play_sound(&mut self, sound: Sound) -> bool {
        println!("play {sound:?}");
        true
    }

    fn read_high_score(&mut self) -> Option<u64> {
        self.high_score
    }

    fn write_high_score(&mut self, high_score: u64) -> bool {
        self.high_score = Some(high_score);
        true
    }

    fn log(&mut self, message: &str) {
        eprint!("[libdinorunner] {message}");
    }
}

fn main() -> Result<(), dinorunner::Error> {
    println!("libdinorunner {}", dinorunner::version());
    dinorunner::seed(std::process::id() as u16);
    let console = Console {
        start: Instant::now(),
        high_score: None,
    };
    let mut game = Dinorunner::new(
        Dimension {
            width: 600,
            height: 150,
        },
        console,
    )?;
    // Press "up" to start the game, then run for 3 frames.
    // In real applications, keep running until the window is closed.
    game.key_up();
    for _ in 0..3 {
        game.update()?;
        std::thread::sleep(std::time::Duration::from_millis(16));
    }
    Ok(())
}
