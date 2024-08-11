# ![Dinorunner](assets/dino_sprite.png "Dinorunner") Dinorunner

Chrome's Dinorunner game as a standalone C library.


## Description  
This project contains a port of the famous Chrome's Dinosaur Game written in C.  

It is designed to be used as a configurable backend library accessable to a wide range of programming languages (using i.e. CFFIs) and hardware architectures.

**Features**:
- No hardware-specific dependencies: Runs everywhere
- No compiler-specific dependencies: Compile and go
- No `stdlib` or header lib requirement
- No `typedef`

The dinorunner was inspired by https://github.com/wayou/t-rex-runner from which the trex-assets were fetched.


## Requirements  
The following functions need to be defined:
```c
unsigned char dinorunner_writehighscore(unsigned long high_score, void* user_data);
unsigned char dinorunner_readhighscore(unsigned long* high_score, void* user_data);
unsigned long dinorunner_gettimestamp(void);
unsigned char dinorunner_sound_play(enum dinorunner_sound_e sound);
unsigned char dinorunner_vibrate(unsigned duration, void* user_data);
unsigned char dinorunner_canvas_clear(void* user_data);
unsigned char dinorunner_draw(enum dinorunner_sprite_e sprite, const struct pos_s* pos, void* user_data);
unsigned char dinorunner_log(const char* format, ...);
```

## Demo  
<p align="center">
  <img src="https://github.com/AKJ7/dinorunner/blob/9f8ccd088d952062bda3dfc1b6aff48846028be9/assets/demo.gif" />
</p>   

## Usage  

```shell
docker compose -f docker-compose.yml up
```

## TODO  
- [ ] Add sound and vibration support
- [ ] Improve configurability
