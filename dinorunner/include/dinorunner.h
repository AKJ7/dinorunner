/**
 * @file dinorunner.h
 * 
 * @copyright Copyright (C) 2025 - All Rights Reserved 
 *  You may use, distribute and modify this code under the 
 *  terms of the GPL license.
 */

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

#define DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT 4
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

#define DINORUNNER_CONFIG_HORIZON_GAP_COEFFICIENT 0.6f

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

#define DINORUNNER_CONFIG_NIGHTMODE_FADESPEED 0.035
#define DINORUNNER_CONFIG_NIGHTMODE_HEIGHT 40
#define DINORUNNER_CONFIG_NIGHTMODE_WIDTH 20
#define DINORUNNER_CONFIG_NIGHTMODE_MOONSPEED 0.25
#define DINORUNNER_CONFIG_NIGHTMODE_NUMBSTARS 2
#define DINORUNNER_CONFIG_NIGHTMODE_STARSIZE 9
#define DINORUNNER_CONFIG_NIGHTMODE_STARSPEED 0.3
#define DINORUNNER_CONFIG_NIGHTMODE_STARMAXY 70
#define DINORUNNER_CONFIG_NIGHTMODE_MOONPHASES 7

/**
 * @brief Container to store coordinates of sprites
 */
struct pos_s {
  int x;
  int y;
};

/**
 * @brief Container to store the Width and height of 
 * various objects
 */
struct dimension_s {
  unsigned width;
  unsigned height;
};

/**
 * @brief Structure to store a cloud's informations
 */
struct cloud_s {
  int x;
  int y;
  int cloud_gap;
  unsigned container_width;
  unsigned frame_counter;
  unsigned char is_alive;
  unsigned char remove;
};

/**
 * @brief Enumeration of all various obstacle types used in the game
 */
enum obstacle_type_e {
  OBSTACLE_TYPE_CACTUS_SMALL,
  OBSTACLE_TYPE_CACTUS_LARGE,
  OBSTACLE_TYPE_PTERODACTYL,
  OBSTACLE_TYPE_NONE
};

/**
 * @brief Enumeration of various sprites used in the game
 */
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
  DINORUNNER_SPRITE_MOON1,
  DINORUNNER_SPRITE_MOON2,
  DINORUNNER_SPRITE_MOON3,
  DINORUNNER_SPRITE_MOON4,
  DINORUNNER_SPRITE_MOON5,
  DINORUNNER_SPRITE_MOON6,
  DINORUNNER_SPRITE_MOON7,
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
  DINORUNNER_SPRITE_STAR1,
  DINORUNNER_SPRITE_STAR2,
  DINORUNNER_SPRITE_STAR3,
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

/**
 * @brief Enumeration of the sound effects used in the game
 */
enum dinorunner_sound_e { DINORUNNER_SOUND_BUTTON_PRESS, DINORUNNER_SOUND_SCORE, DINORUNNER_SOUND_HIT };

/**
 * @brief Enumeration of the state of the trex
 */
enum trex_status_e {
  TREX_STATUS_NONE,
  TREX_STATUS_CRASHED,
  TREX_STATUS_DUCKING,
  TREX_STATUS_JUMPING,
  TREX_STATUS_RUNNING,
  TREX_STATUS_WAITING,
};

/**
 * @brief Structure containing informations of an obstacle
 */
struct obstacle_s {
  struct obstacle_config_s {
    struct collision_box_s {
      int x, y, width, height;
    } collision_box[5];
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
    unsigned collision_box_size;
  } config;
  int x_pos;
  int y_pos;
  float speed_offset;
  int current_frame;
  int timer;
  int width;
  int size;
  float gap;
  float gap_coefficient;
  unsigned char is_alive;
  unsigned char following_obstacle_created;
};

/**
 * @brief Structure defining a star property
 */
struct star_s {
  float x;
  float y;
};

/**
 * @brief Structure containing information of the nightmode state
 */
struct nightmode_s {
  struct star_s stars[DINORUNNER_CONFIG_NIGHTMODE_NUMBSTARS];
  struct pos_s sprite_pos;
  float x_pos;
  float y_pos;
  float opacity;
  unsigned char draw_stars;
  unsigned char current_phase;
};

/**
 * @brief Structure containing information of the horizonline object
 */
struct horizonline_s {
  struct dimension_s dimensions;
  enum dinorunner_sprite_e sprite[2];
  int x_pos[2];
  int y_pos;
  float bump_threshold;
};

/**
 * @brief Structure containing information of the horizon object
 */
struct horizon_s {
  struct obstacle_s obstacles[DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT];
  struct cloud_s clouds[DINORUNNER_CONFIG_CLOUD_MAX_COUNT];
  enum obstacle_type_e obstacle_history[DINORUNNER_CONFIG_OBSTACLE_MAX_OBSTACLE_COUNT];
  struct horizonline_s horizon_line;
  struct nightmode_s nightmode;
  struct dimension_s dimension;
  struct pos_s horizon_offset_s;
  float gap_coefficient;
  float cloud_frequency;
  float cloud_speed;
  float running_time;
  int index;
  int frame_counter;
  int obstacle_index;
  unsigned char first;
};

/**
 * @brief Structure containing the information of the distance counter
 */
struct distance_meter_s {
  struct pos_s sprite_pos;
  unsigned long max_score_units;
  unsigned long current_distance;
  unsigned long max_score;
  unsigned int high_score;
  unsigned int x;
  unsigned int y;
  unsigned flash_timer;
  unsigned flash_iterations;
  unsigned int width;
  unsigned short invert_timer;
  unsigned short achievement;
};

/**
 * @brief Structure representing the gameover sprite
 */
struct gameoverpanel_s {
  struct dimension_s canvas_dimension;
};

/**
 * @brief Structure representing the information of the trex object
 */
struct trex_s {
  struct pos_s sprite_pos;
  enum dinorunner_sprite_e current_animation_frame[2];
  unsigned long timer;
  unsigned jump_count;
  int blink_count;
  int x_pos;
  int y_pos;
  int ground_y_pos;
  int current_frame;
  float blink_delay;
  float animation_start_time;
  float ms_per_frame;
  enum trex_status_e status;
  float jump_velocity;
  unsigned char reached_min_height;
  int jump_spot_x;
  int initial_jump_velocity;
  float drop_velocity;
  int min_jump_height;
  unsigned char mid_air;
  unsigned char playing_intro;
  unsigned char speed_drop;
  unsigned char jumping;
  unsigned char ducking;
};

/**
 * @brief Structure containing the dinorunner game info
 */
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
  unsigned char invert_trigger;
  unsigned char playing_intro;
  unsigned long invert_timer;
  unsigned long play_count;
  unsigned char update_pending;
  float raq_id;
  int collision_count;
  int score_read;
  unsigned char inverted;
  void* user_data;
};

/**
 * @brief Structure to store the game version
 */
struct version_s {
  unsigned char major;
  unsigned char minor;
  unsigned char patch;
};

/**
 * @brief Port of memcpy as there are not standard library routines available
 * 
 * @param dest Destination buffer to copy into
 * @param src Source buffer to copy from
 * @param n Number of bytes to copy
 * @internal
 */
void dinorunner_memcpy(void* dest, const void* src, unsigned n);

/**
 * @brief Computes the smallest integer value not less than f
 * 
 * @param f Value to ceil
 * @return float Smallest integer value not less than f
 * @internal
 */
float dinorunner_ceilf(float f);

/**
 * @brief Computes the largest integer value not greater than x
 * 
 * @param x Value to floor
 * @return float Largest integer value not greater than x
 * @internal
 */
float dinorunner_floorf(float x);

/**
 * @brief Compute the result of the first argument raised to the power
 * of the second
 * 
 * @param x Base value
 * @param y Exponent of base
 * @return float Power of x raised to y
 * @internal 
 */
float dinorunner_pow(float x, int y);

/**
 * @brief Computes the nearest integer value to x
 * 
 * @param x Value to round
 * @return float Rounded value
 * @internal
 */
float dinorunner_roundf(float x);

/**
 * @brief Pseudo-random 16-bit number generator
 * @ref https://en.wikipedia.org/wiki/Lehmer_random_number_generator
 * 
 * @return unsigned Random number between 0 and 0xFFFF
 * @internal
 */
unsigned dinorunner_rand(void);

/**
 * @brief Pseudo-random float number generator
 * 
 * @return float Random float between 0.0f and 1.0f
 * @internal
 */
float dinorunner_srand(void);

/**
 * @brief Generate a random integer given an interval 
 * 
 * @param min_value Minimum value the random integer is allowed to have
 * @param max_value Maximum value the random integer is allowed to have
 * @return unsigned Random value between min_value and max_value
 * @internal
 */
unsigned dinorunner_getrandomnumb(unsigned min_value, unsigned max_value);

/**
 * @brief Get the current version of the library
 * 
 * @param version Structure to store the semantic version into
 * @return unsigned char 1 on success, 0 otherwise.
 * The only reason for failure is caused when passing a null pointer
 * as parameter 
 */
unsigned char dinorunner_getversion(struct version_s* version);

/**
 * @brief Called by the lib to request the vibration of the target device
 * 
 * @param duration Amount of time to vibrate
 * @param user_data Provided userdata
 * @return unsigned char 1 on success, 0 otherwise as the user sees fit
 * @interface
 */
unsigned char dinorunner_vibrate(unsigned duration, void* user_data);

/**
 * @brief Draw request to blit a specific sprite onto the canvas
 * 
 * @param sprite Sprite to draw
 * @param pos Position on the canvas to draw on
 * @param user_data Given userdata
 * @param opacity Sprite opacity: 0 (Not visible) -> 255 (Fully visible)
 * @return unsigned char 1 on succes, 0 otherwise as the user sees fit
 * @interface
 */
unsigned char dinorunner_draw(enum dinorunner_sprite_e sprite, const struct pos_s* pos, unsigned char opacity,
                              void* user_data);

/**
 * @brief Get the game's running time
 * 
 * @return unsigned long Amount of milliseconds that have passed since the game started
 * @interface
 */
unsigned long dinorunner_gettimestamp(void* user_data);

/**
 * @brief Game's request to play a specific sound 
 * 
 * @param sound Sound to play
 * @param Stored data of the user to pass back
 * @return unsigned char 1 on success, 0 otherwise as the user sees fit
 * @interface
 */
unsigned char dinorunner_playsound(enum dinorunner_sound_e sound, void* user_data);

/**
 * @brief Game's request to store/write the current highscore
 * 
 * @param high_score Value of the highscore to write
 * @param user_data Given userdata
 * @return unsigned char 1 on success, 0 otherwise as the user sees fit
 * @interface
 */
unsigned char dinorunner_writehighscore(unsigned long high_score, void* user_data);

/**
 * @brief Game's request to read the current highscore
 * 
 * @param high_score Pointer to the variable location to store the value into
 * @param user_data Given user data
 * @return unsigned char 1 on success, 0 otherwise as the user sees fit
 * @interface
 */
unsigned char dinorunner_readhighscore(unsigned long* high_score, void* user_data);

/**
 * @brief Game's request to clear the drawing canvas
 * 
 * @param user_data Given user data
 * @return unsigned char 1 on success, 0 otherwise as the user sees fit
 */
unsigned char dinorunner_clearcanvas(void* user_data);

/**
 * @brief Game's request to log messages
 * 
 * @param user_data Given user data to pass
 * @param format format of the message to log
 * @param ... arguments
 * @return unsigned char 1 on success, 0 otherwise as the user sees fit 
 * @interface
 */
unsigned char dinorunner_log(void* user_data, const char* format, ...);

/**
 * @brief Initialize the dinorunner instance
 * 
 * @param dinorunner Instance of the dinorunner structure to prepare
 * @param dimension Measurements of the canvas
 * @param user_data userdata to pass back to the request routines
 * @return unsigned char 1 on success, 0 otherwise
 */
unsigned char dinorunner_init(struct dinorunner_s* dinorunner, const struct dimension_s* dimension, void* user_data);

/**
 * @brief Update the dinorunner install
 * Should be triggered betwwen frame updates
 * 
 * @param dinorunner Instance of the running dinorunner object
 * @return unsigned char 1 on success, 0 otherwise
 */
unsigned char dinorunner_update(struct dinorunner_s* dinorunner);

/**
 * @brief Get the inverted/night-mode state
 * 
 * @param dinorunner Instance of the running dinorunner object
 * @param night_mode Variable into which to store the actual nightmode state
 * @return unsigned char 1 on success, 0 otherwise
 */
unsigned char dinorunner_isinverted(struct dinorunner_s* dinorunner, unsigned char* night_mode);

/**
 * @brief Trigger the "key up" input
 * 
 * @param dinorunner Instance of the running dinorunner object
 */
void dinorunner_onkeyup(struct dinorunner_s* dinorunner);

/**
 * @brief Trigger the "key down" input
 * 
 * @param dinorunner Instance of the running dinorunner object
 */
void dinorunner_onkeydown(struct dinorunner_s* dinorunner);

/**
 * @brief Cancel of key input
 * 
 * @param dinorunner Instance of the running dinorunner object
 */
void dinorunner_onkeynone(struct dinorunner_s* dinorunner);

/**
 * @defgroup Internal functions
 * Used internally by the dinorunner routines
 * @{
 */
void dinorunner_cloud_init(struct cloud_s* cloud, unsigned container_width);
unsigned char dinorunner_cloud_update(struct cloud_s* cloud, float speed, void* user_data);

unsigned char dinorunner_horizon_init(struct horizon_s* horizon, const struct dimension_s* dimension,
                                      float gap_coefficient);
unsigned char dinorunner_horizon_update(struct horizon_s* horizon, float delta_time, float current_speed,
                                        unsigned char update_obstacles_request, unsigned char show_night_mode,
                                        void* user_data);
void dinorunner_horizon_reset(struct horizon_s* horizon, void* user_data);

void dinorunner_horizonline_init(struct horizonline_s* horizoneline, const struct dimension_s* src_dimension);
void dinorunner_horizonline_reset(struct horizonline_s* horizonline);
void dinorunner_horizonline_update(struct horizonline_s* horizonline, float delta_time, float speed, void* user_data);

float dinorunner_obstacle_getMinSpeed(enum obstacle_type_e obstacle_type_e);
enum obstacle_type_e dinorunner_obstacle_fromIndex(int obstacle_index);
void dinorunner_obstacle_init(struct obstacle_s* obstacle, enum obstacle_type_e obstacle_type,
                              const struct dimension_s* dimension, float gap_coefficient, float speed, int opt_x_offset,
                              void* user_data);
void dinorunner_obstacle_update(struct obstacle_s* obstacle, float delta_time, float speed, void* user_data);

unsigned char dinorunner_gameoverpanel_init(struct gameoverpanel_s* gameoverpanel, const struct dimension_s* dimension);
unsigned dinorunner_gameoverpanel_draw(struct gameoverpanel_s* gameoverpanel, void* user_data);

unsigned char dinorunner_distancemeter_init(struct distance_meter_s* distance_meter,
                                            const struct dimension_s* sprite_info, unsigned int canvas_width,
                                            void* user_data);
unsigned char dinorunner_distancemeter_draw(const struct distance_meter_s* distance_meter, unsigned digit_pos,
                                            unsigned value, unsigned short opt_high, void* user_data);
void dinorunner_distancemeter_reset(struct distance_meter_s* distance_meter, unsigned long score, void* user_data);
void dinorunner_distancemeter_writehighscore(struct distance_meter_s* distance_meter, unsigned distance,
                                             void* user_data);
unsigned dinorunner_distancemeter_getactualdistance(unsigned distance);
unsigned char dinorunner_distancemeter_update(struct distance_meter_s* distance_meter, float delta_time,
                                              unsigned long distance, void* user_data);

void dinorunner_nightmode_init(struct nightmode_s* nightmode, unsigned width);
void dinorunner_nightmode_reset(struct nightmode_s* nightmode, unsigned container_width, void* user_data);
unsigned char dinorunner_nightmode_update(struct nightmode_s* nightmode, unsigned char show_nightmode,
                                          unsigned container_width, void* user_data);

unsigned char dinorunner_trex_init(struct trex_s* trex, unsigned container_width, unsigned container_height,
                                   void* user_data);
unsigned char dinorunner_trex_update(struct trex_s* trex, unsigned long delta_time, enum trex_status_e status,
                                     void* user_data);
void dinorunner_trex_reset(struct trex_s* trex, void* user_data);
void dinorunner_trex_setduck(struct trex_s* trex, unsigned char is_ducking, void* user_data);
void dinorunner_trex_startjump(struct trex_s* trex, float speed, void* user_data);
void dinorunner_trex_setspeeddrop(struct trex_s* trex);
void dinorunner_trex_updatejump(struct trex_s* trex, float delta_time, void* user_data);
void dinorunner_trex_endjump(struct trex_s* trex);
/**
 * @} 
 */

#ifdef __cplusplus
}
#endif

#endif  // DINORUNNER_DINORUNNER_H