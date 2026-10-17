#!/usr/bin/env bash
# Builds the browser demo into www/: the wasm module and its JS glue in www/pkg, the game assets in www/assets.
#
# Requirements:
#   rustup target add wasm32-unknown-unknown
#   cargo install wasm-bindgen-cli --version 0.2.129   (must match wasm-bindgen in Cargo.toml)
#   clang and llvm-ar, to compile libdinorunner's C sources to WebAssembly (e.g. sudo apt install clang llvm)
#
# Usage: ./build.sh [--debug] [--serve]
set -euo pipefail

cd "$(dirname "$0")"
profile=release
serve=false
for argument in "$@"; do
  case "$argument" in
    --debug) profile=debug ;;
    --serve) serve=true ;;
    *) echo "Unknown argument: $argument" >&2; exit 1 ;;
  esac
done

wasm_bindgen="$(command -v wasm-bindgen || echo "${CARGO_HOME:-$HOME/.cargo}/bin/wasm-bindgen")"
if [[ ! -x "$wasm_bindgen" ]]; then
  echo "wasm-bindgen not found: cargo install wasm-bindgen-cli --version 0.2.129" >&2
  exit 1
fi
# cc compiles the C sources with clang for wasm32 targets; gcc cannot emit WebAssembly
export CC_wasm32_unknown_unknown="${CC_wasm32_unknown_unknown:-clang}"
export AR_wasm32_unknown_unknown="${AR_wasm32_unknown_unknown:-$(command -v llvm-ar || echo ar)}"
if ! command -v "$CC_wasm32_unknown_unknown" > /dev/null; then
  echo "$CC_wasm32_unknown_unknown not found: install clang and llvm (sudo apt install clang llvm)" >&2
  exit 1
fi

cargo_flags=(--target wasm32-unknown-unknown)
if [[ "$profile" == release ]]; then
  cargo_flags+=(--release)
fi
cargo build "${cargo_flags[@]}"

"$wasm_bindgen" --target web --no-typescript --out-dir www/pkg \
  "target/wasm32-unknown-unknown/$profile/dinorunner_web.wasm"

mkdir -p www/assets
cp ../assets/100-offline-sprite.png ../assets/jump.wav ../assets/point.wav ../assets/die.wav www/assets/

echo "Built into $(pwd)/www"
if [[ "$serve" == true ]]; then
  echo "Serving on http://localhost:8000"
  python3 -m http.server 8000 --directory www
fi
