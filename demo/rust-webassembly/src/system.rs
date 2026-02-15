use dinorunner_api::dinorunner_api::dinorunner_api::{dinorunner_s, dimension_s, version_s};
use std::fmt;
use std::fmt::Formatter;
use rand::RngExt;

const GAME_DIMENSION: dimension_s = dimension_s { width: 600, height: 150 };

#[derive(Default)]
pub struct Hypervisor {
    dinorunner: dinorunner_s,
}

impl Hypervisor {
    pub fn new() -> Result<Self, &'static str> {
        let mut rng = rand::rng();
        let seed: u16 = rng.random();
        dinorunner_s::seed(seed);
        let hypervisor = Self {
            dinorunner: dinorunner_s::default()
        };
        Ok(hypervisor)
    }

    pub fn init(&mut self) -> Result<(), &'static str> {
        // self.dinorunner.init(dimension, self)
        Ok(())
    }

    pub fn run(&mut self) -> Result<(), &'static str> {
        Ok(())
    }
}

impl fmt::Display for Hypervisor {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        let version = dinorunner_s::lib_version().unwrap();
        write!(f, "Compiled with libdinorunner: v{}.{}.{}", version.major, version.minor, version.patch)
    }
}

impl Drop for Hypervisor {
    fn drop(&mut self) {
        // todo!()
    }
}