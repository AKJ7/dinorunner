#![allow(dead_code, unused_variables)]

// extern crate dinorunner_api;

mod system;

use system::Hypervisor;


fn main() {
    Hypervisor::new()
        .and_then(|hypervisor| {
            println!("{hypervisor}");
            Ok(hypervisor)
        }).and_then(|mut hypervisor| {
            hypervisor.init().unwrap();
            Ok(hypervisor)
        }) .and_then(|mut hypervisor| {
            hypervisor.run()
        }).unwrap()
}
