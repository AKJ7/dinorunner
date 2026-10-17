//! Compiles libdinorunner's C sources (from `../../dinorunner`) together with the Rust shim.

use std::env;
use std::fs;
use std::path::{Path, PathBuf};

const C_SOURCES: &[&str] = &[
    "clouds.c",
    "dinorunner.c",
    "distance_meter.c",
    "game_over_panel.c",
    "horizon.c",
    "horizon_line.c",
    "nightmode.c",
    "obstacle.c",
    "platform.c",
    "trex.c",
    "utils.c",
];

/// Reads `x.y.z` from `project(dinorunner VERSION x.y.z ...)`
fn project_version(cmake_lists: &Path) -> (String, String, String) {
    let content = fs::read_to_string(cmake_lists)
        .unwrap_or_else(|e| panic!("Could not read {}: {e}", cmake_lists.display()));
    let project = content
        .find("project(")
        .unwrap_or_else(|| panic!("No project() in {}", cmake_lists.display()));
    let version = content[project..]
        .split_whitespace()
        .skip_while(|word| *word != "VERSION")
        .nth(1)
        .unwrap_or_else(|| panic!("No project VERSION in {}", cmake_lists.display()));
    let parts: Vec<&str> = version.trim_end_matches(')').split('.').collect();
    match parts.as_slice() {
        [major, minor, patch] => (major.to_string(), minor.to_string(), patch.to_string()),
        _ => panic!("Unexpected project version '{version}'"),
    }
}

fn main() {
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let lib_dir = manifest_dir.join("../../dinorunner");
    let src_dir = lib_dir.join("src");
    let include_dir = lib_dir.join("include");
    let cmake_lists = lib_dir.join("CMakeLists.txt");
    let version_template = src_dir.join("version.c.in");
    let shim = manifest_dir.join("src/shim.c");

    let (major, minor, patch) = project_version(&cmake_lists);
    let template = fs::read_to_string(&version_template)
        .unwrap_or_else(|e| panic!("Could not read {}: {e}", version_template.display()));
    let version_c = out_dir.join("version.c");
    fs::write(
        &version_c,
        template
            .replace("@PROJECT_VERSION_MAJOR@", &major)
            .replace("@PROJECT_VERSION_MINOR@", &minor)
            .replace("@PROJECT_VERSION_PATCH@", &patch),
    )
    .expect("Could not write version.c");

    let mut build = cc::Build::new();
    if !build.get_compiler().is_like_msvc() {
        build.std("c99");
    }
    build
        .include(&include_dir)
        .files(C_SOURCES.iter().map(|source| src_dir.join(source)))
        .file(&version_c)
        .file(&shim)
        .warnings(true)
        .compile("dinorunner_rs");

    for path in C_SOURCES.iter().map(|source| src_dir.join(source)) {
        println!("cargo:rerun-if-changed={}", path.display());
    }
    for path in [
        &include_dir.join("dinorunner.h"),
        &src_dir.join("platform.h"),
        &cmake_lists,
        &version_template,
        &shim,
    ] {
        println!("cargo:rerun-if-changed={}", path.display());
    }
}
