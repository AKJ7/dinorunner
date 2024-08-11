#ifndef DINORUNNER_DINORUNNER_H
#define DINORUNNER_DINORUNNER_H

#ifdef __cplusplus
extern "C" {
#endif

#define TREX_UNUSED(VAL) (void)VAL

#define DINORUNNER_CONFIG_CORE_FPS 60
#define DINORUNNER_CONFIG_CORE_CLEARTIME 3000
#define DINORUNNER_CONFIG_CORE_MAXSPEED 13
#define DINORUNNER_CONFIG_CORE_ACCELERATION 0.001
#define DINORUNNER_CONFIG_MAX_BLINK_COUNT 3
#define DINORUNNER_CONFIG_CORE_INVERTFADEDURATION 12000
#define DINORUNNER_CONFIG_CORE_INVERTDISTANCE 700
#define DINORUNNER_CONFIG_SPEED 6

#define DINORUNNER_CONFIG_CLOUD_WIDTH 46
#define DINORUNNER_CONFIG_CLOUD_HEIGHT 16
#define DINORUNNER_CONFIG_CLOUD_MAX_GAP 400
#define DINORUNNER_CONFIG_CLOUD_MIN_GAP 150
#define DINORUNNER_CONFIG_CLOUD_MIN_SKY_LEVEL 71
#define DINORUNNER_CONFIG_CLOUD_MAX_SKY_LEVEL 30
#define DINORUNNER_CONFIG_CLOUD_MAX_COUNT 6
#define DINORUNNER_CONFIG_CLOUD_SPEED 0.2f
#define DINORUNNER_CONFIG_CLOUD_FREQUENCY 0.5f

#define DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT 10
#define DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_DUPLICATION 2

#define DINORUNNER_CONFIG_GAMEOVERPANEL_TEXT_X 0
#define DINORUNNER_CONFIG_GAMEOVERPANEL_TEXT_Y 13
#define DINORUNNER_CONFIG_GAMEOVERPANEL_TEXT_WIDTH 191
#define DINORUNNER_CONFIG_GAMEOVERPANEL_TEXT_HEIGHT 11
#define DINORUNNER_CONFIG_GAMEOVERPANEL_RESTART_WIDTH 36
#define DINORUNNER_CONFIG_GAMEOVERPANEL_RESTART_HEIGHT 32

#define DINORUNNER_CONFIG_DISTANCEMETER_MAXSCOREUNIT 5
#define DINORUNNER_CONFIG_DISTANCEMETER_ACHIEVEMENT_DISTANCE 100
#define DINORUNNER_CONFIG_DISTANCEMETER_COEFFICIENT 0.025
#define DINORUNNER_CONFIG_DISTANCEMETER_FLASHDURATION 1000.0f / 4
#define DINORUNNER_CONFIG_DISTANCEMETER_FLASHITERATION 3

#define DINORUNNER_CONFIG_MAX_OBSTACLE_LENGTH 3
#define DINORUNNER_CONFIG_MAX_GAP_COEFFICIENT 1.5f

#define DINORUNNER_CONFIG_HORIZON_GAP_COEFFICIENT 0.5f

#define DINORUNNER_CONFIG_TREX_DROP_VELOCIY -5.0f
#define DINORUNNER_CONFIG_TREX_GRAVITY 0.6f
#define DINORUNNER_CONFIG_TREX_HEIGHT 47
#define DINORUNNER_CONFIG_TREX_HEIGHT_DUCK 25
#define DINORUNNER_CONFIG_TREX_INITIAL_JUMP_VELOCITY -10
#define DINORUNNER_CONFIG_TREX_INTRO_DURATION 1500
#define DINORUNNER_CONFIG_TREX_MAX_JUMP_HEIGHT 30
#define DINORUNNER_CONFIG_TREX_MIN_JUMP_HEIGHT 30
#define DINORUNNER_CONFIG_TREX_SPEED_DROP_COEFFICIENT 3
#define DINORUNNER_CONFIG_TREX_SPRITE_WIDTH 262
#define DINORUNNER_CONFIG_TREX_START_X_POS 50
#define DINORUNNER_CONFIG_TREX_WIDTH 44
#define DINORUNNER_CONFIG_TREX_WIDTH_DUCK 59
#define DINORUNNER_CONFIG_TREX_BLINK_TIMING 7000
#define DINORUNNER_CONFIG_TREX_BOTTOM_PAD 5

struct canvas_s;

struct canvas_ctx_s;

struct pos_s {
  int x;
  int y;
};

struct dimension_s {
  unsigned width;
  unsigned height;
};

struct cloud_s {
  int x;
  int y;
  int cloud_gap;
  unsigned char is_alive;
  unsigned char remove;
  unsigned container_width;
  unsigned frame_counter;
  struct pos_s sprite_pos;
};

enum obstacle_type_e {
  OBSTACLE_TYPE_CACTUS_SMALL,
  OBSTACLE_TYPE_CACTUS_LARGE,
  OBSTACLE_TYPE_PTERODACTYL,
  OBSTACLE_TYPE_NONE
};

enum dinorunner_sprite_e {
  DINORUNNER_SPRITE_CACTUSLARGE1,
  DINORUNNER_SPRITE_CACTUSLARGE2,
  DINORUNNER_SPRITE_CACTUSLARGE3,
  DINORUNNER_SPRITE_CACTUSSMALL1,
  DINORUNNER_SPRITE_CACTUSSMALL2,
  DINORUNNER_SPRITE_CACTUSSMALL3,
  DINORUNNER_SPRITE_PTERODACTYL1,
  DINORUNNER_SPRITE_PTERODACTYL2,
  DINORUNNER_SPRITE_PTERODACTYL3,
  DINORUNNER_SPRITE_CLOUD,
  DINORUNNER_SPRITE_HORIZON1,
  DINORUNNER_SPRITE_HORIZON2,
  DINORUNNER_SPRITE_MOON,
  DINORUNNER_SPRITE_RESTART,
  DINORUNNER_SPRITE_TEXT,
  DINORUNNER_SPRITE_TREX_STANDING1,
  DINORUNNER_SPRITE_TREX_STANDING2,
  DINORUNNER_SPRITE_TREX_RUNNING1,
  DINORUNNER_SPRITE_TREX_RUNNING2,
  DINORUNNER_SPRITE_TREX_CRASHED,
  DINORUNNER_SPRITE_TREX_DUCKING1,
  DINORUNNER_SPRITE_TREX_DUCKING2,
  DINORUNNER_SPRITE_TREX_JUMPING,
  DINORUNNER_SPRITE_STAR,
  DINORUNNER_SPRITE_0,
  DINORUNNER_SPRITE_1,
  DINORUNNER_SPRITE_2,
  DINORUNNER_SPRITE_3,
  DINORUNNER_SPRITE_4,
  DINORUNNER_SPRITE_5,
  DINORUNNER_SPRITE_6,
  DINORUNNER_SPRITE_7,
  DINORUNNER_SPRITE_8,
  DINORUNNER_SPRITE_9,
  DINORUNNER_SPRITE_HI,
  DINORUNNER_SPRITE_GAMEOVER
};

struct obstacle_s {
  unsigned char is_alive;
  unsigned char remove;
  int x_pos;
  int y_pos;
  float speed_offset;
  int current_frame;
  int timer;
  int width;
  unsigned char following_obstacle_created;
  struct obstacle_config_s {
    enum obstacle_type_e type_config;
    int width;
    int height;
    int y_pos;
    float multiple_speed;
    float min_gap;
    float min_speed;
    int num_frames;
    int frame_rate;
    float speed_offset;
    struct collision_box_s {
      int x, y, width, height;
    } collision_box[5];
    unsigned collision_box_size;
  } config;
  struct pos_s sprite_pos;
  int size;
  struct dimension_s dimension;
  float gap;
  float gap_coefficient;
  void* user_data;
};

struct nightmode_s {
  int x_pos;
  int y_pos;
  int current_phase;
  float opacity;
  unsigned container_width;
  void* stars;
  unsigned char draw_stars;
};

struct horizonline_s {
  struct pos_s sprite_pos;
  struct pos_s source_x_pos;
  struct dimension_s src_dimensions;
  struct dimension_s dimensions;
  int y_pos;
  int x_pos[2];
  enum dinorunner_sprite_e sprite[2];
  float bump_threshold;
  void* user_data;
};

struct horizon_s {
  struct config_s {
    float bumpy_threshold;
    float cloud_frequency;
    float cloud_speed;
    int horizon_height;
    unsigned max_cloud;
  } config;
  struct dimension_s dimension;
  float gap_coefficient;
  enum obstacle_type_e obstacle_history[DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT];
  struct obstacle_s obstacles[DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT];
  struct pos_s horizon_offset_s;
  float cloud_frequency;
  struct cloud_s clouds[DINORUNNER_CONFIG_CLOUD_MAX_COUNT];
  float cloud_speed;
  struct horizonline_s horizon_line;
  struct nightmode_s nightmode;
  float running_time;
  int index;
  int frame_counter;
  unsigned char first;
  int obstacle_index;
};

struct sprite_s {
  unsigned width;
  unsigned height;
  const unsigned char* bitmap;
};

struct distance_meter_s {
  unsigned short achievement;
  unsigned int high_score;
  struct pos_s sprite_pos;
  unsigned int x;
  unsigned int y;
  unsigned long current_distance;
  unsigned long max_score;
  unsigned flash_timer;
  unsigned flash_iterations;
  unsigned short invert_timer;
  unsigned long max_score_units;
  unsigned int width;
  void* user_data;
};

struct gameoverpanel_s {
  struct pos_s text_image_pos_;
  struct pos_s restart_image_pos;
  struct dimension_s canvas_dimension;
};

enum dinorunner_sound_e { DINORUNNER_SOUND_BUTTON_PRESS, DINORUNNER_SOUND_SCORE, DINORUNNER_SOUND_HIT };

enum trex_status_e {
  TREX_STATUS_NONE,
  TREX_STATUS_CRASHED,
  TREX_STATUS_DUCKING,
  TREX_STATUS_JUMPING,
  TREX_STATUS_RUNNING,
  TREX_STATUS_WAITING,
};

struct trex_s {
  unsigned char playing_intro;
  unsigned char speed_drop;
  unsigned char jumping;
  unsigned char ducking;
  unsigned jump_count;
  int blink_count;
  int x_pos;
  int y_pos;
  struct pos_s sprite_pos;
  int ground_y_pos;
  int current_frame;
  enum dinorunner_sprite_e current_animation_frame[2];
  float blink_delay;
  float animation_start_time;
  unsigned long timer;
  float ms_per_frame;
  enum trex_status_e status;
  float jump_velocity;
  unsigned char reached_min_height;
  int jump_spot_x;
  unsigned char mid_air;
  int initial_jump_velocity;
  float drop_velocity;
  int min_jump_height;
  void* user_data;
};

struct dinorunner_s {
  struct dimension_s dimension;
  struct distance_meter_s distance_meter;
  struct horizon_s horizon;
  struct trex_s trex;
  struct gameoverpanel_s gameoverpanel;
  unsigned char playing;
  unsigned char crashed;
  float distance_ran;
  unsigned long highest_score;
  float time;
  float running_time;
  float ms_per_frame;
  float current_speed;
  unsigned char activated;
  unsigned char paused;
  unsigned char inverte_trigger;
  unsigned char playing_intro;
  unsigned long invert_timer;
  unsigned long play_count;
  unsigned char update_pending;
  float raq_id;
  int collision_count;
  int score_read;
  unsigned char inverted;
};

struct version_s {
  unsigned char major;
  unsigned char minor;
  unsigned char patch;
};

void dinorunner_memcpy(void* dest, const void* src, unsigned n);
float dinorunner_ceilf(float f);
float dinorunner_floorf(float x);
float dinorunner_pow(float x, int y);
float dinorunner_roundf(float x);
unsigned dinorunner_rand(void);
float dinorunner_srand(void);
unsigned dinorunner_getrandomnumb(unsigned min_value, unsigned max_value);
unsigned char dinorunner_getversion(struct version_s* version);

unsigned char dinorunner_draw_text(char* data, int x, int y);
unsigned char dinorunner_vibrate(unsigned duration, void* user_data);

unsigned char dinorunner_draw(enum dinorunner_sprite_e sprite, const struct pos_s* pos, void* user_data);

unsigned long dinorunner_gettimestamp(void);
unsigned char dinorunner_sound_play(enum dinorunner_sound_e sound);
unsigned char dinorunner_writehighscore(unsigned long high_score, void* user_data);
unsigned char dinorunner_readhighscore(unsigned long* high_score, void* user_data);
unsigned char dinorunner_canvas_clear(void* user_data);
unsigned char dinorunner_log(const char* message, ...);

void dinorunner_cloud_init(struct cloud_s* cloud, const struct pos_s* sprite_pos, unsigned container_width);
unsigned char dinorunner_cloud_update(struct cloud_s* cloud, float speed, void* user_data);

unsigned char dinorunner_horizon_init(struct horizon_s* horizon, const struct pos_s* sprite_position,
                                      const struct dimension_s* dimension, float gap_coefficient);
unsigned char dinorunner_horizon_update(struct horizon_s* horizon, float delta_time, float current_speed,
                                        unsigned char update_obstacles_request, unsigned char show_night_mode);
void dinorunner_horizon_reset(struct horizon_s* horizon);

void dinorunner_horizonline_init(struct horizonline_s* horizoneline, const struct dimension_s* src_dimension);
void dinorunner_horizonline_reset(struct horizonline_s* horizonline);
void dinorunner_horizonline_update(struct horizonline_s* horizonline, float delta_time, float speed);

float dinorunner_obstacle_getMinSpeed(enum obstacle_type_e obstacle_type_e);
enum obstacle_type_e dinorunner_obstacle_fromIndex(int obstacle_index);
void dinorunner_obstacle_init(struct obstacle_s* obstacle, enum obstacle_type_e obstacle_type,
                              const struct dimension_s* dimension, float gap_coefficient, float speed, int opt_x_offset,
                              void* user_data);
void dinorunner_obstacle_update(struct obstacle_s* obstacle, float delta_time, float speed);

unsigned char dinorunner_gameoverpanel_init(struct gameoverpanel_s* gameoverpanel, const struct pos_s* text_image_pos,
                                            const struct pos_s* restart_image_pos, const struct dimension_s* dimension);
unsigned dinorunner_gameoverpanel_draw(struct gameoverpanel_s* gameoverpanel, void* user_data);

unsigned char dinorunner_distancemeter_init(struct distance_meter_s* distance_meter,
                                            const struct dimension_s* sprite_info, unsigned int canvas_width,
                                            void* user_data);
unsigned char dinorunner_distancemeter_draw(const struct distance_meter_s* distance_meter, unsigned digit_pos,
                                            unsigned value, unsigned short opt_high, void* user_data);
void dinorunner_distancemeter_reset(struct distance_meter_s* distance_meter, unsigned long score, void* user_data);
void dinorunner_distancemeter_writehighscore(struct distance_meter_s* distance_meter, unsigned distance);
unsigned dinorunner_distancemeter_getactualdistance(unsigned distance);
unsigned char dinorunner_distancemeter_update(struct distance_meter_s* distance_meter, float delta_time,
                                              unsigned long distance, void* user_data);

unsigned char dinorunner_nightmode_init(struct nightmode_s* nightmode, const struct pos_s* pos, unsigned width);
void dinorunner_nightmode_reset(struct nightmode_s* nightmode);
unsigned char dinorunner_nightmode_update(struct nightmode_s* nightmode, unsigned char show_nightmode);

unsigned char dinorunner_trex_init(struct trex_s* trex, const struct pos_s* sprite_pos, unsigned container_width,
                                   unsigned container_height);
unsigned char dinorunner_trex_update(struct trex_s* trex, unsigned long delta_time, enum trex_status_e status);
void dinorunner_trex_reset(struct trex_s* trex);
void dinorunner_trex_setduck(struct trex_s* trex, unsigned char is_ducking);
void dinorunner_trex_startjump(struct trex_s* trex, float speed);
void dinorunner_trex_setspeeddrop(struct trex_s* trex);
void dinorunner_trex_updatejump(struct trex_s* trex, float delta_time);
void dinorunner_trex_endjump(struct trex_s* trex);

unsigned char dinorunner_init(struct dinorunner_s* dinorunner, const struct dimension_s* dimension, void* user_data);
unsigned char dinorunner_update(struct dinorunner_s* dinorunner);
void dinorunner_onkeyup(struct dinorunner_s* dinorunner);
void dinorunner_onkeydown(struct dinorunner_s* dinorunner);
void dinorunner_onkeynone(struct dinorunner_s* dinorunner);

#ifdef __cplusplus
}
#endif

#endif  // DINORUNNER_DINORUNNER_H