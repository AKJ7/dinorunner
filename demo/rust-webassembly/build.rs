fn main() {
    println!("cargo:rustc-link-lib=dinorunner");
    println!("cargo::rustc-link-search=./../../dinorunner/lib")
}
