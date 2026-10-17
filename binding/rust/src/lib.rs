//! Rust bindings of libdinorunner, a port of Chrome's Dino game in C.
//!
//! The engine does not draw, play sounds or keep time by itself: it asks the
//! application to do so through a [`Platform`]. Implement it for your frontend,
//! create a [`Dinorunner`] and call [`Dinorunner::update`] once per frame.
//!
//! ```no_run
//! use dinorunner::{Dimension, Dinorunner, Platform, Pos, Sprite};
//! use std::time::Instant;
//!
//! struct Frontend {
//!     start: Instant,
//! }
//!
//! impl Platform for Frontend {
//!     fn timestamp(&mut self) -> u64 {
//!         self.start.elapsed().as_millis() as u64
//!     }
//!     fn clear_canvas(&mut self) -> bool {
//!         true
//!     }
//!     fn draw(&mut self, sprite: Sprite, pos: Pos, opacity: u8) -> bool {
//!         println!("{sprite:?} at ({}, {}) with opacity {opacity}", pos.x, pos.y);
//!         true
//!     }
//! }
//!
//! let frontend = Frontend { start: Instant::now() };
//! let mut game = Dinorunner::new(Dimension { width: 600, height: 150 }, frontend)?;
//! game.key_up(); // starts the game
//! loop {
//!     game.update()?;
//! #   break;
//! }
//! # Ok::<(), dinorunner::Error>(())
//! ```

#![warn(missing_docs)]

use std::alloc::{self, Layout};
use std::ffi::{c_char, c_int, c_uchar, c_uint, c_ulong, c_void, CStr};
use std::fmt;
use std::marker::PhantomData;
use std::panic::{self, AssertUnwindSafe};
use std::ptr::NonNull;
use std::sync::{Mutex, MutexGuard};

mod ffi {
    use std::ffi::{c_char, c_int, c_uchar, c_uint, c_ulong, c_ushort, c_void};

    /// `struct dinorunner_s`, only ever handled through pointers
    #[repr(C)]
    pub struct State {
        _private: [u8; 0],
    }

    #[repr(C)]
    #[derive(Clone, Copy)]
    pub struct PosS {
        pub x: c_int,
        pub y: c_int,
    }

    #[repr(C)]
    pub struct DimensionS {
        pub width: c_uint,
        pub height: c_uint,
    }

    #[repr(C)]
    #[derive(Default)]
    pub struct VersionS {
        pub major: c_uchar,
        pub minor: c_uchar,
        pub patch: c_uchar,
    }

    /// `struct dinorunner_callbacks_s`: `None` is a NULL function pointer
    #[repr(C)]
    pub struct Callbacks {
        pub gettimestamp: Option<extern "C" fn(*mut c_void) -> c_ulong>,
        pub clearcanvas: Option<extern "C" fn(*mut c_void) -> c_uchar>,
        pub draw: Option<extern "C" fn(c_int, *const PosS, c_uchar, *mut c_void) -> c_uchar>,
        pub playsound: Option<extern "C" fn(c_int, *mut c_void) -> c_uchar>,
        pub vibrate: Option<extern "C" fn(c_uint, *mut c_void) -> c_uchar>,
        pub readhighscore: Option<extern "C" fn(*mut c_ulong, *mut c_void) -> c_uchar>,
        pub writehighscore: Option<extern "C" fn(c_ulong, *mut c_void) -> c_uchar>,
        pub log: Option<extern "C" fn(*const c_char, *mut c_void) -> c_uchar>,
    }

    /// `struct dinorunner_platform_s`
    #[cfg(test)]
    #[repr(C)]
    pub struct PlatformS {
        pub callbacks: Callbacks,
        pub user_data: *mut c_void,
    }

    extern "C" {
        pub fn dinorunner_init(
            state: *mut State,
            dimension: *const DimensionS,
            callbacks: *const Callbacks,
            user_data: *mut c_void,
        ) -> c_uchar;
        pub fn dinorunner_update(state: *mut State) -> c_uchar;
        pub fn dinorunner_isinverted(state: *const State, night_mode: *mut c_uchar) -> c_uchar;
        pub fn dinorunner_isalive(state: *const State, activation_status: *mut c_uchar) -> c_uchar;
        pub fn dinorunner_onkeyup(state: *mut State);
        pub fn dinorunner_onkeydown(state: *mut State);
        pub fn dinorunner_onkeynone(state: *mut State);
        pub fn dinorunner_seed(random_seed: c_ushort);
        pub fn dinorunner_getversion(version: *mut VersionS) -> c_uchar;

        pub fn dinorunner_rs_state_size() -> usize;
        pub fn dinorunner_rs_state_align() -> usize;
    }

    #[cfg(test)]
    extern "C" {
        pub fn dinorunner_platform_log(
            platform: *const PlatformS,
            format: *const c_char,
            ...
        ) -> c_uchar;
        pub fn dinorunner_rs_callbacks_size() -> usize;
        pub fn dinorunner_rs_pos_size() -> usize;
        pub fn dinorunner_rs_dimension_size() -> usize;
        pub fn dinorunner_rs_version_size() -> usize;
        pub fn dinorunner_rs_sprite_count() -> c_int;
        pub fn dinorunner_rs_sound_count() -> c_int;
    }
}

/// The C engine keeps its pseudo-random generator in a global variable: every call into
/// the engine is serialized, so that games running on different threads stay sound.
static ENGINE: Mutex<()> = Mutex::new(());

fn lock_engine() -> MutexGuard<'static, ()> {
    // Callback panics are caught before they reach the engine, a poisoned lock holds no broken state
    ENGINE
        .lock()
        .unwrap_or_else(|poisoned| poisoned.into_inner())
}

macro_rules! c_enum {
    ($(#[$meta:meta])* $name:ident { $($(#[$variant_meta:meta])* $variant:ident),+ $(,)? }) => {
        $(#[$meta])*
        #[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
        #[repr(i32)]
        pub enum $name {
            $($(#[$variant_meta])* $variant),+
        }

        impl $name {
            /// Every value, in the order of the C enumeration
            pub const ALL: &'static [$name] = &[$($name::$variant),+];

            fn from_raw(raw: c_int) -> Option<Self> {
                usize::try_from(raw).ok().and_then(|index| Self::ALL.get(index).copied())
            }
        }
    };
}

c_enum! {
    /// Sprites the engine asks the [`Platform`] to draw
    Sprite {
        #[allow(missing_docs)] CactusLarge1,
        #[allow(missing_docs)] CactusLarge2,
        #[allow(missing_docs)] CactusLarge3,
        #[allow(missing_docs)] CactusSmall1,
        #[allow(missing_docs)] CactusSmall2,
        #[allow(missing_docs)] CactusSmall3,
        #[allow(missing_docs)] Pterodactyl1,
        #[allow(missing_docs)] Pterodactyl2,
        #[allow(missing_docs)] Pterodactyl3,
        #[allow(missing_docs)] Cloud,
        #[allow(missing_docs)] Horizon1,
        #[allow(missing_docs)] Horizon2,
        #[allow(missing_docs)] Moon1,
        #[allow(missing_docs)] Moon2,
        #[allow(missing_docs)] Moon3,
        #[allow(missing_docs)] Moon4,
        #[allow(missing_docs)] Moon5,
        #[allow(missing_docs)] Moon6,
        #[allow(missing_docs)] Moon7,
        #[allow(missing_docs)] Restart,
        #[allow(missing_docs)] Text,
        #[allow(missing_docs)] TrexStanding1,
        #[allow(missing_docs)] TrexStanding2,
        #[allow(missing_docs)] TrexRunning1,
        #[allow(missing_docs)] TrexRunning2,
        #[allow(missing_docs)] TrexCrashed,
        #[allow(missing_docs)] TrexDucking1,
        #[allow(missing_docs)] TrexDucking2,
        #[allow(missing_docs)] TrexJumping,
        #[allow(missing_docs)] Star1,
        #[allow(missing_docs)] Star2,
        #[allow(missing_docs)] Star3,
        #[allow(missing_docs)] Digit0,
        #[allow(missing_docs)] Digit1,
        #[allow(missing_docs)] Digit2,
        #[allow(missing_docs)] Digit3,
        #[allow(missing_docs)] Digit4,
        #[allow(missing_docs)] Digit5,
        #[allow(missing_docs)] Digit6,
        #[allow(missing_docs)] Digit7,
        #[allow(missing_docs)] Digit8,
        #[allow(missing_docs)] Digit9,
        /// The "HI" label in front of the high score
        Hi,
        #[allow(missing_docs)] GameOver,
    }
}

c_enum! {
    /// Sound effects the engine asks the [`Platform`] to play
    Sound {
        /// Jump
        ButtonPress,
        /// Every 100 points
        Score,
        /// Collision with an obstacle
        Hit,
    }
}

/// Position on the canvas, in pixels from the top left corner
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
pub struct Pos {
    #[allow(missing_docs)]
    pub x: i32,
    #[allow(missing_docs)]
    pub y: i32,
}

/// Size of the game canvas in pixels
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
pub struct Dimension {
    #[allow(missing_docs)]
    pub width: u32,
    #[allow(missing_docs)]
    pub height: u32,
}

/// Semantic version of libdinorunner
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, PartialOrd, Ord)]
pub struct Version {
    #[allow(missing_docs)]
    pub major: u8,
    #[allow(missing_docs)]
    pub minor: u8,
    #[allow(missing_docs)]
    pub patch: u8,
}

impl fmt::Display for Version {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "v{}.{}.{}", self.major, self.minor, self.patch)
    }
}

/// Failures reported by the engine
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Error {
    /// [`Dinorunner::new`] could not initialize the engine
    Init,
    /// [`Dinorunner::update`] failed
    Update,
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Error::Init => f.write_str("could not initialize the dinorunner engine"),
            Error::Update => f.write_str("could not update the dinorunner engine"),
        }
    }
}

impl std::error::Error for Error {}

/// Version of the compiled libdinorunner
pub fn version() -> Version {
    let mut version = ffi::VersionS::default();
    // SAFETY: valid pointer to a local, the function only writes into it
    unsafe { ffi::dinorunner_getversion(&mut version) };
    Version {
        major: version.major,
        minor: version.minor,
        patch: version.patch,
    }
}

/// Reseed the engine's pseudo-random generator, which is shared by all games of the process
pub fn seed(random_seed: u16) {
    let _engine = lock_engine();
    // SAFETY: plain value, access to the global generator is serialized by the lock
    unsafe { ffi::dinorunner_seed(random_seed) };
}

/// Services the engine needs from the application: time, drawing, sound and storage.
///
/// Methods returning `bool` report failure to the engine with `false`. A panic inside a
/// method is caught before it reaches the C engine and reported as failure, except in
/// [`Platform::timestamp`], where it aborts the process as the game cannot continue
/// without a clock.
///
/// The methods are called from within [`Dinorunner::new`], [`Dinorunner::update`] and the
/// `key_*` methods. They must not create or drive another [`Dinorunner`], which would deadlock.
pub trait Platform {
    /// Milliseconds elapsed since an arbitrary, fixed point in time. Must not go backwards.
    fn timestamp(&mut self) -> u64;

    /// Clear the canvas before a new frame is drawn
    fn clear_canvas(&mut self) -> bool;

    /// Draw `sprite` with its top left corner at `pos`; `opacity` goes from 0 (invisible) to 255
    fn draw(&mut self, sprite: Sprite, pos: Pos, opacity: u8) -> bool;

    /// Play a sound effect
    fn play_sound(&mut self, _sound: Sound) -> bool {
        true
    }

    /// Vibrate the device, e.g. a gamepad, for `duration_ms` milliseconds
    fn vibrate(&mut self, _duration_ms: u32) -> bool {
        true
    }

    /// Stored high score, `None` when there is none yet
    fn read_high_score(&mut self) -> Option<u64> {
        None
    }

    /// Store a new high score
    fn write_high_score(&mut self, _high_score: u64) -> bool {
        true
    }

    /// Diagnostic message from the engine
    fn log(&mut self, _message: &str) {}
}

/// What the C callbacks receive as `user_data`: a thin pointer to the platform's fat pointer
struct Dispatch {
    platform: NonNull<dyn Platform + 'static>,
}

/// A running game.
///
/// Owns the engine state and the [`Platform`] it draws through. Neither `Send` nor `Sync`:
/// the engine calls back into the platform from whichever thread drives the game.
pub struct Dinorunner<P: Platform> {
    state: NonNull<ffi::State>,
    state_layout: Layout,
    dispatch: NonNull<Dispatch>,
    platform: NonNull<P>,
    _owns: PhantomData<P>,
}

impl<P: Platform> Dinorunner<P> {
    /// Initialize a game on a canvas of `dimension`, drawing through `platform`.
    ///
    /// The engine reads the high score and draws the first frame during initialization.
    pub fn new(dimension: Dimension, platform: P) -> Result<Self, Error> {
        // SAFETY: the shim functions return compile time constants
        let (size, align) = unsafe {
            (
                ffi::dinorunner_rs_state_size(),
                ffi::dinorunner_rs_state_align(),
            )
        };
        let state_layout =
            Layout::from_size_align(size, align).expect("invalid dinorunner_s layout");
        // The C initialization leaves some fields untouched: start from zeroed memory.
        // SAFETY: `dinorunner_s` is never zero sized
        let state = NonNull::new(unsafe { alloc::alloc_zeroed(state_layout) })
            .unwrap_or_else(|| alloc::handle_alloc_error(state_layout))
            .cast::<ffi::State>();

        let platform = NonNull::from(Box::leak(Box::new(platform)));
        let erased: NonNull<dyn Platform + '_> = platform;
        // SAFETY: only the lifetime is erased. The pointer is only dereferenced by the callbacks,
        // which run during calls on this `Dinorunner`, and `P` outlives it.
        let erased: NonNull<dyn Platform + 'static> = unsafe { std::mem::transmute(erased) };
        let dispatch = NonNull::from(Box::leak(Box::new(Dispatch { platform: erased })));

        let game = Dinorunner {
            state,
            state_layout,
            dispatch,
            platform,
            _owns: PhantomData,
        };
        let c_dimension = ffi::DimensionS {
            width: dimension.width,
            height: dimension.height,
        };
        let _engine = lock_engine();
        // SAFETY: `state` is zeroed memory of the right layout, `dispatch` lives as long as `game`
        let result = unsafe {
            ffi::dinorunner_init(
                game.state.as_ptr(),
                &c_dimension,
                &CALLBACKS,
                game.dispatch.as_ptr().cast(),
            )
        };
        if result == 0 {
            return Err(Error::Init);
        }
        Ok(game)
    }

    /// Advance the game by one frame and draw it. Call it once per displayed frame.
    pub fn update(&mut self) -> Result<(), Error> {
        let _engine = lock_engine();
        // SAFETY: `state` was initialized in `new`
        match unsafe { ffi::dinorunner_update(self.state.as_ptr()) } {
            0 => Err(Error::Update),
            _ => Ok(()),
        }
    }

    /// The jump key was pressed: starts the game, jumps, or restarts after a crash
    pub fn key_up(&mut self) {
        let _engine = lock_engine();
        // SAFETY: `state` was initialized in `new`
        unsafe { ffi::dinorunner_onkeyup(self.state.as_ptr()) }
    }

    /// The duck key was pressed: ducks, or drops faster while jumping
    pub fn key_down(&mut self) {
        let _engine = lock_engine();
        // SAFETY: `state` was initialized in `new`
        unsafe { ffi::dinorunner_onkeydown(self.state.as_ptr()) }
    }

    /// All keys were released: stops ducking and ends the jump early for lower jumps
    pub fn key_none(&mut self) {
        let _engine = lock_engine();
        // SAFETY: `state` was initialized in `new`
        unsafe { ffi::dinorunner_onkeynone(self.state.as_ptr()) }
    }

    /// Whether night mode is active
    pub fn is_inverted(&self) -> bool {
        let mut inverted: c_uchar = 0;
        // SAFETY: `state` was initialized in `new`; reads the state only
        unsafe { ffi::dinorunner_isinverted(self.state.as_ptr(), &mut inverted) };
        inverted != 0
    }

    /// Whether the intro is over and the game is running
    pub fn is_alive(&self) -> bool {
        let mut alive: c_uchar = 0;
        // SAFETY: `state` was initialized in `new`; reads the state only
        unsafe { ffi::dinorunner_isalive(self.state.as_ptr(), &mut alive) };
        alive != 0
    }

    /// The platform the game draws through
    pub fn platform(&self) -> &P {
        // SAFETY: owned by `self`; the engine only accesses it during `&mut self` calls
        unsafe { self.platform.as_ref() }
    }

    /// The platform the game draws through
    pub fn platform_mut(&mut self) -> &mut P {
        // SAFETY: owned by `self`; the engine only accesses it during `&mut self` calls
        unsafe { self.platform.as_mut() }
    }
}

impl<P: Platform> Drop for Dinorunner<P> {
    fn drop(&mut self) {
        // SAFETY: allocated in `new` with these layouts/boxes, never freed elsewhere
        unsafe {
            alloc::dealloc(self.state.as_ptr().cast(), self.state_layout);
            drop(Box::from_raw(self.dispatch.as_ptr()));
            drop(Box::from_raw(self.platform.as_ptr()));
        }
    }
}

impl<P: Platform + fmt::Debug> fmt::Debug for Dinorunner<P> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("Dinorunner")
            .field("platform", self.platform())
            .finish_non_exhaustive()
    }
}

/// # Safety
/// `user_data` must be the `Dispatch` pointer handed to `dinorunner_init`, and no other
/// reference to the platform may be alive. Both hold for engine callbacks.
unsafe fn platform<'a>(user_data: *mut c_void) -> &'a mut dyn Platform {
    match NonNull::new(user_data.cast::<Dispatch>()) {
        Some(dispatch) => &mut *dispatch.as_ref().platform.as_ptr(),
        None => std::process::abort(),
    }
}

/// Runs a callback without letting a panic unwind into the C engine
fn forward(callback: impl FnOnce() -> bool) -> c_uchar {
    match panic::catch_unwind(AssertUnwindSafe(callback)) {
        Ok(true) => 1,
        Ok(false) | Err(_) => 0,
    }
}

extern "C" fn on_timestamp(user_data: *mut c_void) -> c_ulong {
    // SAFETY: called by the engine with the user data given to `dinorunner_init`
    let platform = unsafe { platform(user_data) };
    match panic::catch_unwind(AssertUnwindSafe(|| platform.timestamp())) {
        // Truncates on platforms with a 32-bit `long`; the engine only uses differences
        Ok(timestamp) => timestamp as c_ulong,
        Err(_) => std::process::abort(),
    }
}

extern "C" fn on_clear_canvas(user_data: *mut c_void) -> c_uchar {
    // SAFETY: called by the engine with the user data given to `dinorunner_init`
    let platform = unsafe { platform(user_data) };
    forward(|| platform.clear_canvas())
}

extern "C" fn on_draw(
    sprite: c_int,
    pos: *const ffi::PosS,
    opacity: c_uchar,
    user_data: *mut c_void,
) -> c_uchar {
    // SAFETY: called by the engine with the user data given to `dinorunner_init`
    let platform = unsafe { platform(user_data) };
    // SAFETY: the engine passes a pointer to a local position, or null
    let (Some(sprite), Some(pos)) = (Sprite::from_raw(sprite), unsafe { pos.as_ref() }) else {
        return 0;
    };
    let pos = Pos { x: pos.x, y: pos.y };
    forward(|| platform.draw(sprite, pos, opacity))
}

extern "C" fn on_play_sound(sound: c_int, user_data: *mut c_void) -> c_uchar {
    // SAFETY: called by the engine with the user data given to `dinorunner_init`
    let platform = unsafe { platform(user_data) };
    let Some(sound) = Sound::from_raw(sound) else {
        return 0;
    };
    forward(|| platform.play_sound(sound))
}

extern "C" fn on_vibrate(duration: c_uint, user_data: *mut c_void) -> c_uchar {
    // SAFETY: called by the engine with the user data given to `dinorunner_init`
    let platform = unsafe { platform(user_data) };
    forward(|| platform.vibrate(duration))
}

extern "C" fn on_read_high_score(high_score: *mut c_ulong, user_data: *mut c_void) -> c_uchar {
    // SAFETY: called by the engine with the user data given to `dinorunner_init`
    let platform = unsafe { platform(user_data) };
    if high_score.is_null() {
        return 0;
    }
    forward(|| {
        // 0 tells the engine that there is no high score yet
        let value = platform.read_high_score().unwrap_or(0);
        // SAFETY: checked for null, points to the engine's local variable
        unsafe { *high_score = value as c_ulong };
        true
    })
}

extern "C" fn on_write_high_score(high_score: c_ulong, user_data: *mut c_void) -> c_uchar {
    // SAFETY: called by the engine with the user data given to `dinorunner_init`
    let platform = unsafe { platform(user_data) };
    // `c_ulong` is 32 bits wide on some platforms
    #[allow(clippy::useless_conversion)]
    let high_score = u64::from(high_score);
    forward(|| platform.write_high_score(high_score))
}

extern "C" fn on_log(message: *const c_char, user_data: *mut c_void) -> c_uchar {
    // SAFETY: called by the engine with the user data given to `dinorunner_init`
    let platform = unsafe { platform(user_data) };
    if message.is_null() {
        return 0;
    }
    // SAFETY: the engine passes a NUL terminated buffer
    let message = unsafe { CStr::from_ptr(message) }.to_string_lossy();
    forward(|| {
        platform.log(&message);
        true
    })
}

/// The engine's view of every `Platform`: the callbacks find the platform through the user data
static CALLBACKS: ffi::Callbacks = ffi::Callbacks {
    gettimestamp: Some(on_timestamp),
    clearcanvas: Some(on_clear_canvas),
    draw: Some(on_draw),
    playsound: Some(on_play_sound),
    vibrate: Some(on_vibrate),
    readhighscore: Some(on_read_high_score),
    writehighscore: Some(on_write_high_score),
    log: Some(on_log),
};

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn declarations_match_header() {
        // SAFETY: the shim functions return compile time constants
        unsafe {
            assert_eq!(
                ffi::dinorunner_rs_pos_size(),
                std::mem::size_of::<ffi::PosS>()
            );
            assert_eq!(
                ffi::dinorunner_rs_dimension_size(),
                std::mem::size_of::<ffi::DimensionS>()
            );
            assert_eq!(
                ffi::dinorunner_rs_version_size(),
                std::mem::size_of::<ffi::VersionS>()
            );
            assert_eq!(
                ffi::dinorunner_rs_sprite_count() as usize,
                Sprite::ALL.len()
            );
            assert_eq!(ffi::dinorunner_rs_sound_count() as usize, Sound::ALL.len());
            assert_eq!(
                ffi::dinorunner_rs_callbacks_size(),
                std::mem::size_of::<ffi::Callbacks>()
            );
        }
        assert_eq!(
            Sprite::from_raw(Sprite::GameOver as c_int),
            Some(Sprite::GameOver)
        );
        assert_eq!(Sprite::from_raw(-1), None);
        assert_eq!(Sprite::from_raw(Sprite::ALL.len() as c_int), None);
    }

    #[derive(Default)]
    struct Logger {
        messages: Vec<String>,
    }

    impl Platform for Logger {
        fn timestamp(&mut self) -> u64 {
            0
        }
        fn clear_canvas(&mut self) -> bool {
            true
        }
        fn draw(&mut self, _sprite: Sprite, _pos: Pos, _opacity: u8) -> bool {
            true
        }
        fn log(&mut self, message: &str) {
            self.messages.push(message.to_owned());
        }
    }

    #[test]
    fn log_messages_are_formatted_by_the_engine() {
        let mut logger = Logger::default();
        let platform: NonNull<dyn Platform + '_> = NonNull::from(&mut logger);
        // SAFETY: lifetime erasure only, `logger` outlives every use below
        let platform: NonNull<dyn Platform + 'static> = unsafe { std::mem::transmute(platform) };
        let mut dispatch = Dispatch { platform };
        let engine_platform = ffi::PlatformS {
            callbacks: ffi::Callbacks { ..CALLBACKS },
            user_data: (&mut dispatch as *mut Dispatch).cast::<c_void>(),
        };
        let long = std::ffi::CString::new("x".repeat(1000)).unwrap();
        // SAFETY: valid platform and NUL terminated format strings with matching arguments
        unsafe {
            let log = |format: &std::ffi::CStr| format.as_ptr();
            assert_eq!(
                ffi::dinorunner_platform_log(
                    &engine_platform,
                    log(c"Invalid frame: %d received\n"),
                    7 as c_int
                ),
                1
            );
            assert_eq!(
                ffi::dinorunner_platform_log(
                    &engine_platform,
                    log(c"%s %u %ld"),
                    long.as_ptr(),
                    42 as c_uint,
                    -5 as std::ffi::c_long
                ),
                1
            );
            assert_eq!(
                ffi::dinorunner_platform_log(&engine_platform, std::ptr::null()),
                0
            );
        }
        assert_eq!(logger.messages[0], "Invalid frame: 7 received\n");
        assert_eq!(
            logger.messages[1].len(),
            255,
            "truncated to the engine's buffer"
        );
        assert_eq!(logger.messages.len(), 2);
    }
}
