#![allow(dead_code, unused_variables)]

// extern crate dinorunner_api;

mod system;

use std::sync::Arc;
use system::Hypervisor;

fn main() {
    let hypervisor = Hypervisor::new();
    let value = hypervisor.lock();
    if value.is_ok() {
        let mut s = value.unwrap();
        s.init().expect("TODO: panic message");
        s.run().expect("TODO: ");
    }
}
