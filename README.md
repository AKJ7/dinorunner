# ![Dinorunner](assets/dino_sprite.png "Dinorunner") Dinorunner

Chrome's Dinorunner game as a standalone C library.


## Description  
This project contains a port of the famous Chrome's Dinosaur Game written in C.  

It is designed to be used as a configurable backend library accessable to a wide range of programming languages (using i.e. CFFIs) and hardware architectures.

**Features**:
- Hardware-agnostic: No hardware-specific dependencies - Runs everywhere
- Compiler-agnostic: No compiler-specific dependencies - Compile and go
- No `stdlib` or header lib requirement
- No heap allocations
- No `typedef`s
- Supports nightmode
- Only requires 1200 bytes to store data structure on x86_64
- Supports variadic jump heights

The dinorunner was inspired by https://github.com/wayou/t-rex-runner from which the trex-assets were fetched.  
The sound assets used in the demo originates from: https://www.sounds-resource.com/browser_games/googlechromedinosaurgame/sound/18002/ .


## Requirements  
The following functions need to be defined:
```c
unsigned char dinorunner_writehighscore(unsigned long high_score, void* user_data);
unsigned char dinorunner_readhighscore(unsigned long* high_score, void* user_data);
unsigned long dinorunner_gettimestamp(void* user_data);
unsigned char dinorunner_playsound(enum dinorunner_sound_e sound, void* user_data);
unsigned char dinorunner_vibrate(unsigned duration, void* user_data);
unsigned char dinorunner_clearcanvas(void* user_data);
unsigned char dinorunner_draw(enum dinorunner_sprite_e sprite, const struct pos_s* pos, unsigned char opacity, void* user_data);
unsigned char dinorunner_log(void* user_data, const char* format, ...);
```

## Demo  
<p align="center">
  <img src="https://github.com/AKJ7/dinorunner/blob/9f8ccd088d952062bda3dfc1b6aff48846028be9/assets/demo.gif" />
</p>   
<p align="center">
  <img src="https://github.com/AKJ7/dinorunner/blob/fd7c8ae9c22e98f57daca17f15a53548cd8a651f/assets/demo_nightmode.gif" />
</p>

## Usage  

This project is subdivided into two parts:
1. **libdinorunner**: The actual library. It is written without any external dependency and is targetted to be used with any consivable system. It can directly be compiled into a project or as shared and static libraries using:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -S dinorunner -B dinorunner/build && cmake --build dinorunner/build
```
See `dinorunner/lib` for the generated libraries objects. With 
```bash
sudo make install -C dinorunner/build/
```
the libraries can be installed system-wide.

2. **dinorunner-sdl**: This is a running example of the project. It uses sdl2 to process user input and display the output of `libdinorunner` to the screen. 
This project provides a preset docker container into which the program compiles and runs.
Before running the examples in a docker container, the x-server needs to permit access to client outside its host. This is done using: `xhost +`.
The simplest way to run the program is using docker-compose:
```shell
docker compose -f docker-compose.yml up dinorunner
```
Alternaively, using CMake:
```shell
cmake -DCMAKE_BUILD_TYPE=Release -S demo -B demo/build && cmake --build demo/build && demo/bin/dinorunner_sdl
```
while requiring `libsdl2-dev` `libsdl2-image-dev` `libsdl2-gfx-dev` installable with:
```shell
sudo apt -y install libsdl2-dev libsdl2-image-dev libsdl2-gfx-dev
```

## API  
The following functions can be used to interact with the dinorunner engine. See the demo for example.

```c
unsigned char dinorunner_init(struct dinorunner_s* dinorunner, const struct dimension_s* dimension, void* user_data);
unsigned char dinorunner_update(struct dinorunner_s* dinorunner);
unsigned char dinorunner_getversion(struct version_s* version);
unsigned char dinorunner_isinverted(struct dinorunner_s* dinorunner, unsigned char* night_mode);
void dinorunner_onkeyup(struct dinorunner_s* dinorunner);
void dinorunner_onkeydown(struct dinorunner_s* dinorunner);
void dinorunner_onkeynone(struct dinorunner_s* dinorunner);
```

## TODO  
- [x] Add sound and vibration support
- [x] Improve configurability
- [x] Add night mode
