#![allow(dead_code, unused_variables)]

// extern crate dinorunner_api;

mod system;
use system::Hypervisor;

fn main() {
    let mut hypervisor = Hypervisor::new().and_then(|v| {
        println!("{v}");
        Ok(v)
    }).unwrap();
    let hypervisor = hypervisor.init().unwrap();
    hypervisor.run().expect("TODO: panic message");
}
