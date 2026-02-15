// #![allow(dead_code, unused_variables)]

pub mod dinorunner_api {
    
    use libc::{c_uint, c_uchar, c_void, user};

    #[repr(C)]
    #[derive(Default)]
    pub struct dinorunner_s;

    #[repr(C)]
    pub struct pos_s;

    #[repr(C)]
    pub struct dimension_s {
        pub width: c_uint,
        pub height: c_uint
    }

    #[repr(C)]
    #[derive(Default, Debug)]
    pub struct version_s {
        pub major: c_uchar,
        pub minor: c_uchar,
        pub patch: c_uchar
    }


    #[link(name = "dinorunner", kind="static")]
    unsafe extern "C" {
        fn dinorunner_init(dinorunner: *mut dinorunner_s, dimension: *const dimension_s, user_data: *mut libc::c_void) -> c_uchar;
        fn dinorunner_update(dinorunner: *mut dinorunner_s) -> c_uchar;
        fn dinorunner_getversion(version: *mut version_s) -> c_uchar;
        fn dinorunner_isinverted(dinorunner: *const dinorunner_s, night_mode: *mut c_uchar) -> c_uchar;
        fn dinorunner_isalive(dinorunner: *const dinorunner_s, activation_status: *mut c_uchar) -> c_uchar;
        fn dinorunner_seed(random_seed: libc::c_ushort) -> c_void;
        fn dinorunner_onkeyup(dinorunner: *mut dinorunner_s) -> c_void;
        fn dinorunner_onkeydown(dinorunner: *mut dinorunner_s) -> c_void;
        fn dinorunner_onkeynone(dinorunner: *mut dinorunner_s) -> c_void;
    }

    impl dinorunner_s {
        pub fn init<T>(&mut self, dimension: &dimension_s, user_data: &mut T) -> Result<(), &'static str> {
            unsafe {
                // let casted_data = user_data as *mut c_void;
                // let mut casted_data = user_data);
                // match dinorunner_init(self as *mut dinorunner_s, dimension as *const dimension_s, casted_data) {
                //     0 => Err("Could not initialize engine."),
                //     _ => Ok(())
                // }
                Ok(())
            }        
        }

        pub fn update(&mut self) -> Result<(), &'static str> {
            unsafe {
                match dinorunner_update(self as *mut dinorunner_s) {
                    0 => Err("Could not update engine."),
                    _ => Ok(())
                }
            }
        }

        pub fn lib_version() -> Result<version_s, &'static str> {
            unsafe {
                let mut version = version_s::default();
                match dinorunner_getversion(&mut version as *mut version_s) {
                    0 => Err("Could not read libdinorunner version"),
                    _ => Ok(version)
                }
            }
        }

        pub fn is_inverted(&self) -> Result<bool, &'static str> {
            unsafe {
                let mut night_mode: c_uchar = 0;
                match dinorunner_isinverted(self as *const dinorunner_s, &mut night_mode as *mut c_uchar) {
                    0 => Err("Could not retrieve night mode status"),
                    _ => Ok(night_mode == 1)
                }
            }
        }

        pub fn is_alive(&self) -> Result<bool, &'static str> {
            unsafe {
                let mut activation_status: c_uchar = 0;
                match dinorunner_isalive(self as *const dinorunner_s, &mut activation_status as *mut c_uchar) {
                    0 => Err("Could not fetch alive status"),
                    _ => Ok(activation_status == 1)
                }
            }
        }

        pub fn seed(random_seed: u16) {
            unsafe { 
                dinorunner_seed(random_seed);
            }
        }

        pub fn onkeyup(&mut self) {
            unsafe {
                dinorunner_onkeyup(self as *mut dinorunner_s);
            }
        }

        pub fn onkeydown(&mut self) {
            unsafe {
                dinorunner_onkeydown(self as *mut dinorunner_s);
            }
        }

        pub fn onkeynone(&mut self) {
            unsafe {
                dinorunner_onkeynone(self as *mut dinorunner_s);
            }
        }

        #[unsafe(no_mangle)]
        pub extern "C" fn dinorunner_gettimestamp(user_data: * mut libc::c_void) -> libc::c_ulong {
            return 0;
        }
    }


    #[unsafe(no_mangle)]
    pub extern "C" fn dinorunner_writehighscore(high_score: libc::c_ulong, user_data: * mut libc::c_void) -> libc::c_uchar {
        return 0;
    }

    #[unsafe(no_mangle)]
    pub extern "C" fn dinorunner_readhighscore(high_score: * mut libc::c_ulong, user_data: *mut libc::c_void) -> libc::c_uchar {
        return 0;
    }

    #[unsafe(no_mangle)]
    pub extern "C" fn dinorunner_vibrate(duration: libc::c_uint, user_data: *mut libc::c_void) -> libc::c_uchar {
        return 0;
    }

    #[unsafe(no_mangle)]
    pub extern "C" fn dinorunner_clearcanvas(user_data: *mut libc::c_void) -> libc::c_uchar {
        return 0;
    }

    #[unsafe(no_mangle)]
    pub extern "C" fn dinorunner_playsound(sound: libc::c_int, user_data: *mut libc::c_void) -> libc::c_uchar {
        return 0;
    }

    #[unsafe(no_mangle)]
    pub extern "C" fn dinorunner_draw(sprite: libc::c_int, pos: *const pos_s, opacity: libc::c_uchar, user_data: *mut libc::c_void) -> libc::c_uchar {
        return 0;
    }

    #[unsafe(no_mangle)]
    pub extern "C" fn dinorunner_log(user_data: libc::c_void, format: *const libc::c_char) -> libc::c_uchar {
        return 0;
    }
}
