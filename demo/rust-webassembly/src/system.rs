use dinorunner_api::dinorunner_api::dinorunner_api::{LibDinorunner, dimension_s, dinorunner_s};
use openal::{Device, Position};
use rand::RngExt;
use sfml::cpp::FBox;
use std::env::consts::ARCH;
use std::ffi::c_void;
use std::fmt;
use std::fmt::Formatter;
use std::ops::{Deref, DerefMut};
use std::rc::Rc;
use std::sync::{Arc, Mutex, MutexGuard, Weak};

extern crate sfml;

use sfml::graphics::glsl::Vec2;
use sfml::graphics::{
    CircleShape, Color, Drawable, RectangleShape, RenderStates, RenderTarget, RenderWindow, Shape,
    Sprite, Texture, Transformable,
};
use sfml::system::{Clock, Vector2, Vector2f};
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
const HIGHSCORE_STORE: &'static str = "dinorunner_highscore.txt";
const SPRITE_FILENAME: &'static str = "../../demo/assets/100-offline-sprite.png";
const DIE_SOUND_FILE: &'static str = "../../demo/assets/die.wav";
const JUMP_SOUND_FILE: &'static str = "../../demo/assets/jump.wav";
const POINT_SOUND_FILE: &'static str = "../../demo/assets/point.wav";
const FRAME_RATE: u32 = 60;
const BACKGROUND_COLOR: u8 = 0xF2;

// #[derive(Default)]
pub struct Hypervisor<'a> {
    // window: FBox<RenderWindow>,
    dinorunner: dinorunner_s,
    weak: Weak<Mutex<Hypervisor<'a>>>,
    // window: FBox<&'static mut RenderWindow>,
    // sprite: Sprite<'a>,
    // inverted_sprite: Sprite<'static>,
}

impl<'a> Hypervisor<'a> {
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
    pub fn new() -> Arc<Mutex<Hypervisor<'a>>> {
        let mut rng = rand::rng();
        let seed: u16 = rng.random();
        dinorunner_s::seed(seed);

        let settings = ContextSettings::default();
        let mode = VideoMode::new(WINDOW_WIDTH, WINDOW_HEIGHT, BITS_PER_PIXELS);
        let style = Style::TITLEBAR | Style::CLOSE;
        let mut window = RenderWindow::new(mode, TITLE, style, &settings)
            .expect("Could not create SFML window!");
        window.set_framerate_limit(FRAME_RATE);
        window.set_key_repeat_enabled(false);
        window.set_mouse_cursor_visible(false);
        // RenderWindow::
        // std::mem::drop(window);

        // let texture = Texture::from_file(SPRITE_FILENAME).expect("Could not load sprite");

        Arc::new_cyclic(|weak| {
            Mutex::from(Hypervisor {
                weak: weak.clone(),
                dinorunner: dinorunner_s::default(),
                // window: Self::create_window(),
                // window: window,
                // window: RenderWindow::new(mode, TITLE, style, &settings).unwrap(), // .as_ref().unwrap()
                // sprite: Sprite::new(),
                // inverted_sprite: Sprite::new(),
            })
        })
    }

    // pub fn arc(&self) -> Arc<Hypervisor> {
    //     self.weak.upgrade().unwrap()
    // }

    pub fn init(&mut self) -> Result<(), &'static str> {
        let strong = self.weak.strong_count();
        let weak = self.weak.weak_count();
        println!("Strong: {strong}, Weak: {weak}");
        self.dinorunner
            .init(&GAME_DIMENSION, &mut self.weak.clone())
    }

    pub fn run(&mut self) -> Result<(), &'static str> {
        let mut running: bool = true;
        // while self.window.is_open() && running {
        // while running {
        //     for event in self.window.poll_event() {
        //         match event {
        //             Event::Closed
        //             | Event::KeyPressed {
        //                 code: Key::Escape, ..
        //             } => running = false,
        //             Event::KeyPressed { code, .. } => match code {
        //                 Key::Escape => running = false,
        //                 Key::Up | Key::Space => self.dinorunner.onkeyup(),
        //                 Key::Down => self.dinorunner.onkeydown(),
        //                 _ => {}
        //             },
        //             _ => {}
        //         }
        //     }
        //     self.window.clear(Color::rgb(
        //         BACKGROUND_COLOR,
        //         BACKGROUND_COLOR,
        //         BACKGROUND_COLOR,
        //     ));
        //     self.dinorunner.update()?;
        //     self.window.display();
        // }
        Ok(())
    }
}

impl fmt::Display for Hypervisor<'static> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        let version = dinorunner_s::lib_version().unwrap();
        write!(f, "Compiled with libdinorunner: {}", version)
    }
}

impl LibDinorunner<'_> for Hypervisor<'_> {
    // fn get_timestamp<T>(userdata: &mut T) -> Result<u64, &'static str> {
    fn get_timestamp<T>() -> Result<u64, &'static str> {
        let mut clock = Clock::start().unwrap();
        Ok(clock.elapsed_time().as_milliseconds() as u64)
    }
    // #[unsafe(no_mangle)]
    // extern "C" fn dinorunner_gettimestamp(user_data: *mut c_void) -> libc::c_ulong {
    //     println!("Getting timestamp");
    //     let mut a = user_data as *mut Weak<Mutex<Hypervisor<'_>>>;
    //     unsafe {
    //         let mut hypervisor = (*a).upgrade().unwrap();
    //         // let mut value = hypervisor.lock().unwrap();
    //         //     let is_inverted = value.dinorunner.is_inverted().unwrap();
    //         //     println!("Is inverted: {is_inverted}")
    //         //     // .lock().unwrap();
    //     }
    //     // a.as_ptr().lock().unwrap().dinorunner.
    //     // let mut hypervisor = user_data as Weak<Mutex<Hypervisor<'_>>>;
    //     1
    // }
}

// impl Drop for Hypervisor<'_> {
//     fn drop(&mut self) {
//         // self.window.drop();
//         // self.window.drop();
//         if self.window.is_open() {
//             self.window.close();
//             // std::mem::take(&mut self.window);
//             // std::mem::take(*self.window)
//             // drop(self.window);
//             // let mut val = self.weak.clone();
//             // drop(val.upgrade().unwrap().lock().unwrap().window);
//             println!("Closing window");
//         }
//         println!("Dropping");
//         // todo!()
//     }
// }
