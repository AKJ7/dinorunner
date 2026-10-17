//! libdinorunner in the browser: draws on a `<canvas id="dinorunner">` with the
//! original sprite sheet, plays the original sounds and keeps the high score in
//! `localStorage`. See `build.sh` and `www/index.html`.

use std::cell::RefCell;
use std::rc::Rc;

use dinorunner::{Dimension, Dinorunner, Platform, Pos, Sound, Sprite};
use wasm_bindgen::prelude::*;
use wasm_bindgen::JsCast;
use web_sys::{
    CanvasRenderingContext2d, HtmlAudioElement, HtmlCanvasElement, HtmlImageElement, KeyboardEvent,
    Navigator, Performance, Storage, Window,
};

const GAME_DIMENSION: Dimension = Dimension {
    width: 600,
    height: 150,
};
const PADDING: f64 = 20.0;
const CANVAS_ID: &str = "dinorunner";
const BACKGROUND_COLOR: &str = "#f7f7f7";
const SPRITE_SHEET: &str = "assets/100-offline-sprite.png";
const JUMP_SOUND: &str = "assets/jump.wav";
const SCORE_SOUND: &str = "assets/point.wav";
const HIT_SOUND: &str = "assets/die.wav";
const HIGH_SCORE_KEY: &str = "dinorunner-high-score";
/// The intro reveals the landscape from the T-Rex's width to the full canvas in this time
const INTRO_DURATION_MS: f64 = 400.0;
const TREX_WIDTH: f64 = 44.0;
/// Horizontal offset of each moon phase inside the sprite sheet
const MOON_PHASES: [f64; 7] = [140.0, 120.0, 100.0, 60.0, 40.0, 20.0, 0.0];

/// Area of the sprite sheet: x, y, width, height
#[derive(Clone, Copy)]
struct Rect(f64, f64, f64, f64);

fn sprite_source(sprite: Sprite) -> Option<Rect> {
    let digit = |index: f64| Rect(654.0 + 10.0 * index, 2.0, 10.0, 13.0);
    let star = |index: f64| Rect(645.0, 2.0 + 9.0 * index, 8.0, 9.0);
    let moon = |phase: usize| {
        // The full moon is twice as wide
        let width = if phase == 3 { 40.0 } else { 20.0 };
        Rect(484.0 + MOON_PHASES[phase], 2.0, width, 40.0)
    };
    let rect = match sprite {
        Sprite::CactusLarge1 => Rect(332.0, 2.0, 25.0, 50.0),
        Sprite::CactusLarge2 => Rect(357.0, 2.0, 50.0, 50.0),
        Sprite::CactusLarge3 => Rect(407.0, 2.0, 75.0, 50.0),
        Sprite::CactusSmall1 => Rect(228.0, 2.0, 17.0, 35.0),
        Sprite::CactusSmall2 => Rect(245.0, 2.0, 34.0, 35.0),
        Sprite::CactusSmall3 => Rect(279.0, 2.0, 51.0, 35.0),
        Sprite::Pterodactyl1 => Rect(134.0, 2.0, 46.0, 40.0),
        Sprite::Pterodactyl2 => Rect(180.0, 2.0, 46.0, 40.0),
        Sprite::Cloud => Rect(86.0, 2.0, 46.0, 14.0),
        Sprite::Horizon1 => Rect(2.0, 54.0, 600.0, 12.0),
        Sprite::Horizon2 => Rect(602.0, 54.0, 600.0, 12.0),
        Sprite::Moon1 => moon(0),
        Sprite::Moon2 => moon(1),
        Sprite::Moon3 => moon(2),
        Sprite::Moon4 => moon(3),
        Sprite::Moon5 => moon(4),
        Sprite::Moon6 => moon(5),
        Sprite::Moon7 => moon(6),
        Sprite::Restart => Rect(2.0, 2.0, 36.0, 32.0),
        Sprite::Text => Rect(655.0, 2.0, 191.0, 11.0),
        Sprite::GameOver => Rect(654.0, 15.0, 191.0, 11.0),
        Sprite::TrexStanding1 | Sprite::TrexJumping => Rect(848.0, 2.0, 44.0, 47.0),
        Sprite::TrexStanding2 => Rect(892.0, 2.0, 44.0, 47.0),
        Sprite::TrexRunning1 => Rect(936.0, 2.0, 44.0, 47.0),
        Sprite::TrexRunning2 => Rect(980.0, 2.0, 44.0, 47.0),
        Sprite::TrexCrashed => Rect(1068.0, 2.0, 44.0, 47.0),
        Sprite::TrexDucking1 => Rect(1112.0, 2.0, 59.0, 47.0),
        Sprite::TrexDucking2 => Rect(1171.0, 2.0, 59.0, 47.0),
        Sprite::Star1 => star(0.0),
        Sprite::Star2 => star(1.0),
        Sprite::Star3 => star(2.0),
        Sprite::Digit0 => digit(0.0),
        Sprite::Digit1 => digit(1.0),
        Sprite::Digit2 => digit(2.0),
        Sprite::Digit3 => digit(3.0),
        Sprite::Digit4 => digit(4.0),
        Sprite::Digit5 => digit(5.0),
        Sprite::Digit6 => digit(6.0),
        Sprite::Digit7 => digit(7.0),
        Sprite::Digit8 => digit(8.0),
        Sprite::Digit9 => digit(9.0),
        Sprite::Hi => Rect(754.0, 2.0, 20.0, 13.0),
        // Not used by the engine
        Sprite::Pterodactyl3 => return None,
    };
    Some(rect)
}

/// The engine places the horizon and the clouds relative to the whole canvas, not the game area
fn vertical_padding(sprite: Sprite) -> f64 {
    match sprite {
        Sprite::Horizon1 | Sprite::Horizon2 | Sprite::Cloud => 0.0,
        _ => PADDING,
    }
}

/// The browser side of the game: canvas, sounds, clock and storage
struct Browser {
    context: CanvasRenderingContext2d,
    canvas_width: f64,
    canvas_height: f64,
    sprite_sheet: HtmlImageElement,
    jump_sound: HtmlAudioElement,
    score_sound: HtmlAudioElement,
    hit_sound: HtmlAudioElement,
    performance: Performance,
    navigator: Navigator,
    storage: Option<Storage>,
    /// Width of the visible game area, grows during the intro
    visible_width: f64,
}

impl Platform for Browser {
    fn timestamp(&mut self) -> u64 {
        self.performance.now() as u64
    }

    fn clear_canvas(&mut self) -> bool {
        self.context.set_fill_style_str(BACKGROUND_COLOR);
        self.context
            .fill_rect(0.0, 0.0, self.canvas_width, self.canvas_height);
        true
    }

    fn draw(&mut self, sprite: Sprite, pos: Pos, opacity: u8) -> bool {
        let Some(Rect(x, y, width, height)) = sprite_source(sprite) else {
            return false;
        };
        let context = &self.context;
        context.save();
        context.begin_path();
        context.rect(
            PADDING,
            PADDING,
            self.visible_width,
            f64::from(GAME_DIMENSION.height),
        );
        context.clip();
        context.set_global_alpha(f64::from(opacity) / 255.0);
        let result = context
            .draw_image_with_html_image_element_and_sw_and_sh_and_dx_and_dy_and_dw_and_dh(
                &self.sprite_sheet,
                x,
                y,
                width,
                height,
                f64::from(pos.x) + PADDING,
                f64::from(pos.y) + vertical_padding(sprite),
                width,
                height,
            );
        context.restore();
        result.is_ok()
    }

    fn play_sound(&mut self, sound: Sound) -> bool {
        let audio = match sound {
            Sound::ButtonPress => &self.jump_sound,
            Sound::Score => &self.score_sound,
            Sound::Hit => &self.hit_sound,
        };
        audio.set_current_time(0.0);
        // Browsers refuse to play before the first user interaction: no sound then, but no error either
        audio.play().is_ok()
    }

    fn vibrate(&mut self, duration_ms: u32) -> bool {
        self.navigator.vibrate_with_duration(duration_ms)
    }

    fn read_high_score(&mut self) -> Option<u64> {
        let storage = self.storage.as_ref()?;
        storage.get_item(HIGH_SCORE_KEY).ok()??.parse().ok()
    }

    fn write_high_score(&mut self, high_score: u64) -> bool {
        match &self.storage {
            Some(storage) => storage
                .set_item(HIGH_SCORE_KEY, &high_score.to_string())
                .is_ok(),
            None => false,
        }
    }

    fn log(&mut self, message: &str) {
        web_sys::console::log_1(&format!("[libdinorunner] {}", message.trim_end()).into());
    }
}

type Game = Rc<RefCell<Dinorunner<Browser>>>;
type FrameCallback = Rc<RefCell<Option<Closure<dyn FnMut(f64)>>>>;

enum Key {
    Jump,
    Duck,
}

fn game_key(event: &KeyboardEvent) -> Option<Key> {
    match event.code().as_str() {
        "ArrowUp" | "Space" | "KeyW" => Some(Key::Jump),
        "ArrowDown" | "KeyS" => Some(Key::Duck),
        _ => None,
    }
}

fn add_listener<E: JsCast + 'static>(
    target: &web_sys::EventTarget,
    event: &str,
    mut handler: impl FnMut(E) + 'static,
) -> Result<(), JsValue> {
    let closure = Closure::<dyn FnMut(web_sys::Event)>::new(move |event: web_sys::Event| {
        if let Ok(event) = event.dyn_into::<E>() {
            handler(event);
        }
    });
    target.add_event_listener_with_callback(event, closure.as_ref().unchecked_ref())?;
    // The listeners live as long as the page
    closure.forget();
    Ok(())
}

fn bind_input(window: &Window, canvas: &HtmlCanvasElement, game: &Game) -> Result<(), JsValue> {
    let keys = game.clone();
    add_listener(window, "keydown", move |event: KeyboardEvent| {
        let Some(key) = game_key(&event) else { return };
        // Keep the page from scrolling
        event.prevent_default();
        if event.repeat() {
            return;
        }
        match key {
            Key::Jump => keys.borrow_mut().key_up(),
            Key::Duck => keys.borrow_mut().key_down(),
        }
    })?;

    let keys = game.clone();
    add_listener(window, "keyup", move |event: KeyboardEvent| {
        if game_key(&event).is_some() {
            keys.borrow_mut().key_none();
        }
    })?;

    // Touch screens and mice: tap to jump
    let pointer = game.clone();
    add_listener(canvas, "pointerdown", move |event: web_sys::Event| {
        event.prevent_default();
        pointer.borrow_mut().key_up();
    })?;
    let pointer = game.clone();
    add_listener(canvas, "pointerup", move |_: web_sys::Event| {
        pointer.borrow_mut().key_none()
    })?;
    Ok(())
}

fn request_frame(window: &Window, callback: &Closure<dyn FnMut(f64)>) -> Result<(), JsValue> {
    window.request_animation_frame(callback.as_ref().unchecked_ref())?;
    Ok(())
}

/// Calls `Dinorunner::update` on every animation frame
fn run(window: Window, canvas: HtmlCanvasElement, game: Game) -> Result<(), JsValue> {
    let frame: FrameCallback = Rc::new(RefCell::new(None));
    let next_frame = frame.clone();
    let frame_window = window.clone();
    let mut inverted = false;
    let mut intro_start: Option<f64> = None;
    *frame.borrow_mut() = Some(Closure::new(move |now: f64| {
        {
            let mut game = game.borrow_mut();
            if game.is_alive() {
                let start = *intro_start.get_or_insert(now);
                let progress = ((now - start) / INTRO_DURATION_MS).min(1.0);
                let full_width = f64::from(GAME_DIMENSION.width);
                game.platform_mut().visible_width =
                    TREX_WIDTH + (full_width - TREX_WIDTH) * progress;
            }
            if let Err(error) = game.update() {
                web_sys::console::error_1(&error.to_string().into());
                return;
            }
            // Night mode inverts the canvas, like the original game
            if game.is_inverted() != inverted {
                inverted = game.is_inverted();
                let _ = canvas.class_list().toggle_with_force("inverted", inverted);
            }
        }
        if let Some(callback) = next_frame.borrow().as_ref() {
            let _ = request_frame(&frame_window, callback);
        }
    }));
    let first_frame = frame.borrow();
    request_frame(
        &window,
        first_frame.as_ref().expect("frame callback is set"),
    )
}

fn create_game(
    window: &Window,
    canvas: &HtmlCanvasElement,
    sprite_sheet: HtmlImageElement,
) -> Result<Game, JsValue> {
    let context = canvas
        .get_context("2d")?
        .ok_or("canvas has no 2d context")?
        .dyn_into::<CanvasRenderingContext2d>()?;
    let browser = Browser {
        context,
        canvas_width: f64::from(canvas.width()),
        canvas_height: f64::from(canvas.height()),
        sprite_sheet,
        jump_sound: HtmlAudioElement::new_with_src(JUMP_SOUND)?,
        score_sound: HtmlAudioElement::new_with_src(SCORE_SOUND)?,
        hit_sound: HtmlAudioElement::new_with_src(HIT_SOUND)?,
        performance: window
            .performance()
            .ok_or("window.performance is not available")?,
        navigator: window.navigator(),
        // Unavailable e.g. in private windows of some browsers: the high score is then not kept
        storage: window.local_storage().ok().flatten(),
        visible_width: TREX_WIDTH,
    };
    dinorunner::seed(random_seed(&browser.performance));
    let game = Dinorunner::new(GAME_DIMENSION, browser).map_err(|error| error.to_string())?;
    Ok(Rc::new(RefCell::new(game)))
}

/// Seed from the sub-millisecond page load time
fn random_seed(performance: &Performance) -> u16 {
    (performance.now() * 1000.0) as u64 as u16
}

#[wasm_bindgen(start)]
pub fn start() -> Result<(), JsValue> {
    std::panic::set_hook(Box::new(|info| {
        web_sys::console::error_1(&info.to_string().into())
    }));
    web_sys::console::log_1(&format!("libdinorunner {}", dinorunner::version()).into());

    let window = web_sys::window().ok_or("no window")?;
    let document = window.document().ok_or("no document")?;
    let canvas = document
        .get_element_by_id(CANVAS_ID)
        .ok_or_else(|| format!("no <canvas id=\"{CANVAS_ID}\">"))?
        .dyn_into::<HtmlCanvasElement>()?;
    canvas.set_width(GAME_DIMENSION.width + 2 * PADDING as u32);
    canvas.set_height(GAME_DIMENSION.height + 2 * PADDING as u32);

    // Start once the sprite sheet is loaded: the first frame is drawn during initialization
    let sprite_sheet = HtmlImageElement::new()?;
    let on_load = {
        let (window, canvas, image) = (window.clone(), canvas.clone(), sprite_sheet.clone());
        Closure::once(move || {
            let result = create_game(&window, &canvas, image).and_then(|game| {
                bind_input(&window, &canvas, &game)?;
                run(window, canvas, game)
            });
            if let Err(error) = result {
                web_sys::console::error_1(&error);
            }
        })
    };
    sprite_sheet.set_onload(Some(on_load.as_ref().unchecked_ref()));
    on_load.forget();
    let on_error = Closure::<dyn FnMut()>::new(|| {
        web_sys::console::error_1(&format!("Could not load {SPRITE_SHEET}").into());
    });
    sprite_sheet.set_onerror(Some(on_error.as_ref().unchecked_ref()));
    on_error.forget();
    sprite_sheet.set_src(SPRITE_SHEET);
    Ok(())
}
