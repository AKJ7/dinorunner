use dinorunner_api::dinorunner_api::dinorunner_api::{dimension_s, dinorunner_s};
use libc::clone;
use openal::Position;
use rand::RngExt;
use sfml::cpp::FBox;
use std::env::consts::ARCH;
use std::fmt;
use std::fmt::Formatter;
use std::rc::Rc;
use std::sync::{Arc, Mutex, MutexGuard, Weak};

extern crate sfml;

use sfml::graphics::glsl::Vec2;
use sfml::graphics::{
    CircleShape, Color, RectangleShape, RenderTarget, RenderWindow, Shape, Sprite, Texture,
    Transformable,
};
use sfml::system::{Vector2, Vector2f};
use sfml::window;
use sfml::window::{ContextSettings, Event, Key, Style, VideoMode, Window};

const GAME_DIMENSION: dimension_s = dimension_s {
    width: 600,
    height: 150,
};
const BITS_PER_PIXELS: u32 = 32;
const PADDING: u32 = 20;
const WINDOW_WIDTH: u32 = GAME_DIMENSION.width + PADDING;
const WINDOW_HEIGHT: u32 = GAME_DIMENSION.height + PADDING;
const TITLE: &str = "Dinorunner SFML Rust Demo";
const HIGHSCORE_STORE: &str = "dinorunner_highscore.txt";
const SPRITE_FILENAME: &str = "../../demo/assets/100-offline-sprite.png";
const DIE_SOUND_FILE: &str = "../../demo/assets/die.wav";
const JUMP_SOUND_FILE: &str = "../../demo/assets/jump.wav";
const POINT_SOUND_FILE: &str = "../../demo/assets/point.wav";

const FRAME_RATE: u32 = 60;
const BACKGROUND_COLOR: u8 = 0xF2;

// #[derive(Default)]
pub struct Hypervisor {
    weak: Weak<Mutex<Hypervisor>>,
    dinorunner: dinorunner_s,
    window: FBox<RenderWindow>,
    // sprite: Sprite<'static>,
    // inverted_sprite: Sprite<'static>,
}

impl Hypervisor {
    pub fn create_window() -> FBox<RenderWindow> {
        let settings = ContextSettings::default();
        let mode = VideoMode::new(WINDOW_WIDTH, WINDOW_HEIGHT, BITS_PER_PIXELS);
        let style = Style::TITLEBAR | Style::CLOSE;
        let mut window = RenderWindow::new(mode, TITLE, style, &settings).unwrap();
        window.set_framerate_limit(FRAME_RATE);
        window.set_key_repeat_enabled(false);
        window.set_mouse_cursor_visible(false);
        // let mut rect = RectangleShape::default();
        // rect.set_size(Vector2f {
        //     x: GAME_DIMENSION.width as f32,
        //     y: GAME_DIMENSION.height as f32,
        // });
        // rect.set_position(Vector2f {
        //     x: PADDING as f32,
        //     y: PADDING as f32,
        // });
        // rect.set_fill_color(Color::rgb(
        //     BACKGROUND_COLOR,
        //     BACKGROUND_COLOR,
        //     BACKGROUND_COLOR,
        // ));
        window
    }
    pub fn new() -> Arc<Mutex<Hypervisor>> {
        let mut rng = rand::rng();
        let seed: u16 = rng.random();
        dinorunner_s::seed(seed);

        let settings = ContextSettings::default();
        let mode = VideoMode::new(WINDOW_WIDTH, WINDOW_HEIGHT, BITS_PER_PIXELS);
        let style = Style::TITLEBAR | Style::CLOSE;
        let mut window = RenderWindow::new(mode, TITLE, style, &settings).unwrap();
        window.set_framerate_limit(FRAME_RATE);
        window.set_key_repeat_enabled(false);
        window.set_mouse_cursor_visible(false);

        let texture = Texture::from_file(SPRITE_FILENAME).expect("Could not load sprite");

        Arc::new_cyclic(|weak| {
            Mutex::from(Hypervisor {
                weak: weak.clone(),
                dinorunner: dinorunner_s::default(),
                // window: Self::create_window(),
                window: window,
                // sprite: Sprite::new(),
                // inverted_sprite: Sprite::new(),
            })
        })
    }

    // pub fn arc(&self) -> Arc<Hypervisor> {
    //     self.weak.upgrade().unwrap()
    // }

    pub fn init(&mut self) -> Result<(), &'static str> {
        self.dinorunner
            .init(&GAME_DIMENSION, &mut self.weak.clone())
    }

    pub fn run(&mut self) -> Result<(), &'static str> {
        let mut running: bool = true;
        let mut up_pressed: bool = false;
        let mut down_pressed: bool = false;
        while self.window.is_open() && running {
            for event in self.window.poll_event() {
                match event {
                    Event::Closed
                    | Event::KeyPressed {
                        code: Key::Escape, ..
                    } => running = false,
                    Event::KeyPressed { code: Key::Up, .. }
                    | Event::KeyPressed {
                        code: Key::Space, ..
                    } => {
                        self.dinorunner.onkeyup();
                    }
                    Event::KeyPressed {
                        code: Key::Down, ..
                    } => {
                        self.dinorunner.onkeydown();
                    }
                    _ => {}
                }
            }
            self.window.clear(Color::rgb(
                BACKGROUND_COLOR,
                BACKGROUND_COLOR,
                BACKGROUND_COLOR,
            ));
            self.window.display();
            self.dinorunner.update()?;
        }
        Ok(())
    }
}

impl fmt::Display for Hypervisor {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        let version = dinorunner_s::lib_version().unwrap();
        write!(f, "Compiled with libdinorunner: {}", version)
    }
}

impl Drop for Hypervisor {
    fn drop(&mut self) {
        if self.window.is_open() {
            self.window.close();
        }
        println!("Dropping");
        // todo!()
    }
}
