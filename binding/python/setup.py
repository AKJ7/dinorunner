"""Builds the dinorunner._dinorunner extension from libdinorunner's C sources in ../../dinorunner."""

import re
import shutil
from pathlib import Path

from setuptools import Extension, setup

HERE = Path(__file__).resolve().parent
LIBRARY = (HERE / ".." / ".." / "dinorunner").resolve()
C_HEADERS = ["platform.h"]
C_SOURCES = [
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
]
# setuptools places object files next to the relative source path: copy the sources
# inside this directory instead of compiling them from ../../dinorunner
GENERATED = Path("build") / "dinorunner-src"


def project_version() -> tuple[str, str, str]:
    """Reads x.y.z from project(dinorunner VERSION x.y.z ...) in the C library's CMakeLists.txt."""
    cmake_lists = (LIBRARY / "CMakeLists.txt").read_text()
    match = re.search(r"project\s*\([^)]*?VERSION\s+(\d+)\.(\d+)\.(\d+)", cmake_lists)
    if match is None:
        raise RuntimeError(f"No project VERSION in {LIBRARY / 'CMakeLists.txt'}")
    return match.group(1), match.group(2), match.group(3)


def prepare_sources() -> list[str]:
    if not LIBRARY.is_dir():
        raise RuntimeError(f"libdinorunner sources not found in {LIBRARY}")
    destination = HERE / GENERATED
    destination.mkdir(parents=True, exist_ok=True)
    for source in [*C_HEADERS, *C_SOURCES]:
        shutil.copyfile(LIBRARY / "src" / source, destination / source)
    major, minor, patch = project_version()
    template = (LIBRARY / "src" / "version.c.in").read_text()
    (destination / "version.c").write_text(
        template.replace("@PROJECT_VERSION_MAJOR@", major)
        .replace("@PROJECT_VERSION_MINOR@", minor)
        .replace("@PROJECT_VERSION_PATCH@", patch)
    )
    return [str(GENERATED / source) for source in [*C_SOURCES, "version.c"]]


setup(
    ext_modules=[
        Extension(
            "dinorunner._dinorunner",
            sources=["src/dinorunner/_dinorunner.c", *prepare_sources()],
            include_dirs=[str(LIBRARY / "include")],
        )
    ],
)
