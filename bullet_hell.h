#ifndef BULLET_HELL_H
#define BULLET_HELL_H

#include "third_party/raylib/raylib.h"
#include "third_party/raylib/raymath.h"
#include "third_party/raylib/rlgl.h"

#include "basic.h"
#include "arena.h"
#include "str.h"
#include "context.h"
#include "array.h"
#include "sprite.h"
#include "stb_sprintf.h"


/*
 * macro constants
 */

#define TARGET_FPS 60

#define WINDOW_SCALE 256
#define WINDOW_WIDTH  (3*WINDOW_SCALE)
#define WINDOW_HEIGHT (4*WINDOW_SCALE)

#define WINDOW_RECT ((Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT})

#define MAX_ENTITIES 4096
#define MAX_PARTICLES 1024
#define MAX_BULLET_EMITTER_RINGS 4

#define FRICTION ((float)40.0)


/*
 * tables
 */

#define GAME_STATES             \
  X(NONE)                       \
  X(TITLE_SCREEN)               \
  X(SPAWN_PLAYER)               \
  X(MAIN_LOOP)                  \
  X(GAME_OVER)                  \
  X(DEBUG_SANDBOX)              \
  //X(WAVE_TRANSITION)            \
  //X(SPAWN_ENEMIES)              \

#define GAME_DEBUG_FLAGS       \
  X(DEBUG_UI)                  \
  X(HOT_RELOAD)                \
  X(PLAYER_INVINCIBLE)         \
  X(DRAW_ALL_ENTITY_BOUNDS)    \
  //X(SANDBOX_LOADED)          \

#define GAME_FLAGS      \
  X(PAUSE)              \

#define INPUT_FLAGS         \
  X(ANY)                    \
  X(MOVE_FORWARD)           \
  X(MOVE_BACKWARD)          \
  X(MOVE_LEFT)              \
  X(MOVE_RIGHT)             \
  X(SHOOT)                  \
  X(SLOW_MOVE)              \
  X(PAUSE)                  \

#define ENTITY_KINDS           \
  X(PLAYER)                    \
  X(FORMATION)                 \
  X(BULLET)                    \
  X(CRAB)                      \
  X(FISH)                      \
  X(STINGRAY)                  \
  X(PARROT)                    \
  X(BOSS)                      \

#define ENTITY_ORDERS   \
  X(FIRST)              \
  X(LAST)               \

#define ENTITY_FLAGS                 \
  X(DYNAMICS)                        \
  X(DYNAMICS_HAS_CURVE)              \
  X(HAS_SPRITE)                      \
  X(APPLY_FRICTION)                  \
  X(CLAMP_POS_TO_SCREEN)             \
  X(FILL_BOUNDS)                     \
  X(HAS_BULLET_EMITTER)              \
  X(DIE_IF_OFFSCREEN)                \
  X(APPLY_COLLISION)                 \
  X(RECEIVE_COLLISION)               \
  X(APPLY_COLLISION_DAMAGE)          \
  X(RECEIVE_COLLISION_DAMAGE)        \
  X(DIE_ON_APPLY_COLLISION)          \
  X(HAS_PARTICLE_EMITTER)            \
  X(DAMAGE_BLINK_TINT)               \
  //X(DIE_IF_ABOVE_SCREEN)             \
  //X(DIE_IF_BELOW_SCREEN)             \
  //X(HAS_SHIELDS)                     \
  //X(DEATH_PARTICLES)                 \
  //X(DEATH_SOUND)                     \
  //X(BLINK_TEXT)                      \
  //X(DRAW_TEXT)                       \

#define ENTITY_CONTROLS          \
  X(PLAYER)                      \
  X(CRAB_FORMATION_ENTER)        \
  X(CRAB_ENTER)                  \
  X(CRAB)                        \
  X(FISH_FORMATION_ENTER)        \
  X(FISH_ENTER)                  \
  X(FISH)                        \
  X(STINGRAY_FORMATION_ENTER)    \
  X(STINGRAY_ENTER)              \
  X(STINGRAY)                    \
  X(BOSS_ENTER)                  \
  X(AVENGER_BULLET)              \
  X(CRAB_BULLET)                 \
  X(FISH_BULLET)                 \
  X(STINGRAY_BULLET)             \

#define PARTICLE_FLAGS            \
  X(HAS_SPRITE)                   \
  X(DIE_WHEN_ANIM_FINISH)         \
  X(MULTIPLE_ANIM_CYCLES)         \

#define BULLET_EMITTER_KINDS         \
  X(TRIPLE_THREAT)                   \
  X(AVENGER)                         \
  X(CRAB)                            \
  X(FISH)                            \
  X(STINGRAY)                        \
  X(BOSS)                            \

#define BULLET_EMITTER_FLAGS         \

#define BULLET_EMITTER_RING_FLAGS               \


/*
 * macros
 */

#define entity_kind_in_mask(kind, mask) (!!(mask & (1ull<<kind)))
#define frame_arr_init(array) arr_init((array), gp->frame_scratch)


/*
 * typedefs
 */

typedef struct Game Game;
typedef struct Entity Entity;
typedef Entity* Entity_ptr;
typedef struct Particle_emitter Particle_emitter;
typedef struct Particle Particle;
typedef struct Bullet_emitter Bullet_emitter;
typedef u64 Game_flags;
typedef u64 Game_debug_flags;
typedef u64 Bullet_emitter_kind_mask;
typedef u64 Bullet_emitter_flags;
typedef struct Bullet_emitter_ring Bullet_emitter_ring;
typedef u64 Bullet_emitter_ring_flags;
typedef u64 Input_flags;
typedef u64 Entity_flags;
typedef u64 Entity_kind_mask;
typedef u64 Particle_flags;

typedef enum Game_state {
#define X(state) GAME_STATE_##state,
  GAME_STATES
#undef X
    GAME_STATE_MAX,
} Game_state;

char *Game_state_strings[GAME_STATE_MAX] = {
#define X(state) #state,
  GAME_STATES
#undef X
};

typedef enum Game_flag_index {
  GAME_FLAG_INDEX_INVALID = -1,
#define X(flag) GAME_FLAG_INDEX_##flag,
  GAME_FLAGS
#undef X
    GAME_FLAG_INDEX_MAX,
} Game_flag_index;

STATIC_ASSERT(GAME_FLAG_INDEX_MAX < 64, number_of_game_flags_is_less_than_64);

#define X(flag) const Game_flags GAME_FLAG_##flag = (Game_flags)(1ull<<GAME_FLAG_INDEX_##flag);
GAME_FLAGS
#undef X

typedef enum Game_debug_flag_index {
  GAME_DEBUG_FLAG_INDEX_INVALID = -1,
#define X(flag) GAME_DEBUG_FLAG_INDEX_##flag,
  GAME_DEBUG_FLAGS
#undef X
    GAME_DEBUG_FLAG_INDEX_MAX,
} Game_debug_flag_index;

STATIC_ASSERT(GAME_DEBUG_FLAG_INDEX_MAX < 64, number_of_game_debug_flags_is_less_than_64);

#define X(flag) const Game_debug_flags GAME_DEBUG_FLAG_##flag = (Game_debug_flags)(1ull<<GAME_DEBUG_FLAG_INDEX_##flag);
GAME_DEBUG_FLAGS
#undef X

typedef enum Input_flag_index {
  INPUT_FLAG_INDEX_INVALID = -1,
#define X(flag) INPUT_FLAG_INDEX_##flag,
  INPUT_FLAGS
#undef X
    INPUT_FLAG_INDEX_MAX,
} Input_flag_index;

STATIC_ASSERT(INPUT_FLAG_INDEX_MAX < 64, number_of_input_flags_is_less_than_64);

#define X(flag) const Input_flags INPUT_FLAG_##flag = (Input_flags)(1ull<<INPUT_FLAG_INDEX_##flag);
INPUT_FLAGS
#undef X
const Input_flags INPUT_FLAG_MOVE = INPUT_FLAG_MOVE_FORWARD | INPUT_FLAG_MOVE_LEFT | INPUT_FLAG_MOVE_RIGHT | INPUT_FLAG_MOVE_BACKWARD;

typedef enum Entity_kind {
  ENTITY_KIND_INVALID = 0,
#define X(kind) ENTITY_KIND_##kind,
  ENTITY_KINDS
#undef X
    ENTITY_KIND_MAX,
} Entity_kind;

#define X(kind) const Entity_kind_mask ENTITY_KIND_MASK_##kind = (Entity_kind_mask)(1ull<<ENTITY_KIND_##kind);
ENTITY_KINDS
#undef X

STATIC_ASSERT(ENTITY_KIND_MAX < 64, number_of_entity_kinds_is_less_than_64);

typedef enum Entity_order {
  ENTITY_ORDER_INVALID = -1,
#define X(order) ENTITY_ORDER_##order,
  ENTITY_ORDERS
#undef X
    ENTITY_ORDER_MAX,
} Entity_order;

typedef enum Entity_control {
  ENTITY_CONTROL_NONE = 0,
#define X(control) ENTITY_CONTROL_##control,
  ENTITY_CONTROLS
#undef X
    ENTITY_CONTROL_MAX
} Entity_control;

typedef enum Entity_flag_index {
  ENTITY_FLAG_INDEX_INVALID = -1,
#define X(flag) ENTITY_FLAG_INDEX_##flag,
  ENTITY_FLAGS
#undef X
    ENTITY_FLAG_INDEX_MAX,
} Entity_flag_index;

#define X(flag) const Entity_flags ENTITY_FLAG_##flag = (Entity_flags)(1ull<<ENTITY_FLAG_INDEX_##flag);
ENTITY_FLAGS
#undef X

STATIC_ASSERT(ENTITY_FLAG_INDEX_MAX < 64, number_of_entity_flags_is_less_than_64);

typedef enum Particle_flag_index {
  PARTICLE_FLAG_INDEX_INVALID = -1,
#define X(flag) PARTICLE_FLAG_INDEX_##flag,
  PARTICLE_FLAGS
#undef X
    PARTICLE_FLAG_INDEX_MAX,
} Particle_flag_index;

#define X(flag) const Particle_flags PARTICLE_FLAG_##flag = (Particle_flags)(1ull<<PARTICLE_FLAG_INDEX_##flag);
PARTICLE_FLAGS
#undef X

STATIC_ASSERT(PARTICLE_FLAG_INDEX_MAX < 64, number_of_particle_flags_is_less_than_64);

typedef enum Bullet_emitter_kind {
  BULLET_EMITTER_KIND_INVALID = 0,
#define X(kind) BULLET_EMITTER_KIND_##kind,
  BULLET_EMITTER_KINDS
#undef X
    BULLET_EMITTER_KIND_MAX,
} Bullet_emitter_kind;

#define X(kind) const Bullet_emitter_kind_mask BULLET_EMITTER_KIND_MASK_##kind = (Bullet_emitter_kind_mask)(1ull<<BULLET_EMITTER_KIND_##kind);
BULLET_EMITTER_KINDS
#undef X

STATIC_ASSERT(BULLET_EMITTER_KIND_MAX < 64, number_of_bullet_emitter_kinds_is_less_than_64);

typedef enum Bullet_emitter_flag_index {
  BULLET_EMITTER_FLAG_INDEX_INVALID = -1,
#define X(flag) BULLET_EMITTER_FLAG_INDEX_##flag,
  BULLET_EMITTER_FLAGS
#undef X
    BULLET_EMITTER_FLAG_INDEX_MAX,
} Bullet_emitter_flag_index;

STATIC_ASSERT(BULLET_EMITTER_FLAG_INDEX_MAX < 64, number_of_bullet_emitter_flags_is_less_than_64);

#define X(flag) const Bullet_emitter_flags BULLET_EMITTER_FLAG_##flag = (Bullet_emitter_flags)(1ull<<BULLET_EMITTER_FLAG_INDEX_##flag);
BULLET_EMITTER_FLAGS
#undef X

typedef enum Bullet_emitter_ring_flag_index {
  BULLET_EMITTER_RING_FLAG_INDEX_INVALID = -1,
#define X(flag) BULLET_EMITTER_RING_FLAG_INDEX_##flag,
  BULLET_EMITTER_RING_FLAGS
#undef X
    BULLET_EMITTER_RING_FLAG_INDEX_MAX,
} Bullet_emitter_ring_flag_index;

STATIC_ASSERT(BULLET_EMITTER_RING_FLAG_INDEX_MAX < 64, number_of_bullet_emitter_ring_flags_is_less_than_64);

#define X(flag) const Bullet_emitter_ring_flags BULLET_EMITTER_RING_FLAG_##flag = (Bullet_emitter_ring_flags)(1ull<<BULLET_EMITTER_RING_FLAG_INDEX_##flag);
BULLET_EMITTER_RING_FLAGS
#undef X

DECL_ARR_TYPE(Entity_ptr);
DECL_SLICE_TYPE(Entity_ptr);
DECL_ARR_TYPE(Rectangle);
DECL_SLICE_TYPE(Rectangle);


/*
 * struct bodies
 */

struct Particle {
  b32 live;

  Particle_flags flags;

  Vector2 pos;
  Vector2 vel;
  Vector2 half_size;

  Sprite    sprite;
  Color     sprite_tint;
  float     sprite_scale;
};

struct Particle_emitter {
  int emit_count;

  Particle particle;
};

struct Bullet_emitter_ring {
  Bullet_emitter_ring_flags flags;
  Vector2 dir;
  f32 initial_angle;

  f32 radius;
  f32 spin_cur_angle;
  f32 spin_vel;

  s32 n_arms;
  f32 arms_occupy_circle_sector_percent;

  s32          n_bullets;
  f32          bullet_arm_width;
  f32          bullet_radius;
  f32          bullet_vel;
  f32          bullet_curve;
  f32          bullet_curve_rolloff_vel;
  s32          bullet_damage;
  Color        bullet_bounds_color;
  Color        bullet_fill_color;
  Entity_flags bullet_flags;
  Sprite bullet_sprite;
};

struct Bullet_emitter {
  Bullet_emitter_flags flags;
  Bullet_emitter_kind  kind;
  Entity_kind_mask bullet_collision_mask;

  s32 n_rings;
  Bullet_emitter_ring rings[MAX_BULLET_EMITTER_RINGS];

  b32     shooting;
  float   cooldown_period;
  float   cooldown_timer;
};

struct Entity {
  b32 live;

  Entity_kind    kind;
  Entity_order   update_order;
  Entity_order   draw_order;
  Entity_control control;
  Entity_flags   flags;

  Entity *free_list_next;

  u64 genid;

  Vector2 look_dir;
  Vector2 accel;
  Vector2 vel;
  Vector2 pos;
  f32     radius;
  f32     curve;
  f32     curve_rolloff_vel;

  Color bounds_color;
  Color fill_color;

  Bullet_emitter bullet_emitter;

  Entity_kind_mask apply_collision_mask;

  b32 received_collision;
  s32 received_damage;

  s32 damage_amount;

  s32 health;

  Sprite sprite;
  float sprite_scale;
  Color sprite_tint;

};

struct Game {

  float timestep;

  Game_state state;
  Game_state next_state;

  Game_flags       flags;
  Game_debug_flags debug_flags;

  Input_flags input_flags;

  u64 frame_index;

  Arena *scratch;
  Arena *frame_scratch;

  Font font;

  RenderTexture2D render_texture;

  Texture2D sprite_atlas;

  Entity entities[MAX_ENTITIES];
  u64 entities_allocated;
  Entity *entity_free_list;

  Particle particles[MAX_PARTICLES];
  u64 particles_pos;

  u64 live_entities;
  u64 live_particles;

  Entity *player;

};


/*
 * function headers
 */

void  game_update_and_draw(Game *gp);
void  game_reset(Game *gp);

Entity* entity_spawn(Game *gp);
void    entity_die(Game *gp, Entity *ep);

Entity* entity_spawn_player(Game *gp);
Entity* entity_spawn_crab(Game *gp);
Entity* entity_spawn_fish(Game *gp);
Entity* entity_spawn_stingray(Game *gp);

void entity_emit_bullets(Game *gp, Entity *ep);
b32  entity_check_collision(Game *gp, Entity *a, Entity *b);

void draw_sprite(Game *gp, Sprite sp, Vector2 pos, Color tint);
void draw_sprite_ex(Game *gp, Sprite sp, Vector2 pos, f32 scale, f32 rotation, Color tint);
void sprite_update(Game *gp, Sprite *sp);
Sprite_frame sprite_current_frame(Sprite sp);
b32 sprite_at_keyframe(Sprite sp, s32 keyframe);


/*
 * entity settings
 */

const Vector2 PLAYER_INITIAL_OFFSCREEN_POS = { WINDOW_WIDTH * 0.5f , WINDOW_HEIGHT * 0.5f };
const Vector2 PLAYER_INITIAL_DEBUG_POS = { WINDOW_WIDTH * 0.5f , WINDOW_HEIGHT * 0.5f };
const Vector2 PLAYER_LOOK_DIR = { 0, -1 };
const s32 PLAYER_HEALTH = 100;
const float PLAYER_BOUNDS_RADIUS = 40;
const float PLAYER_SPRITE_SCALE = 1.0f;
const float PLAYER_ACCEL = 1.6e4;
const float PLAYER_SLOW_FACTOR = 1.5e-1;
const Color PLAYER_BOUNDS_COLOR = { 255, 0, 0, 150 };
const Entity_kind_mask PLAYER_APPLY_COLLISION_MASK =
ENTITY_KIND_MASK_CRAB     |
ENTITY_KIND_MASK_FISH     |
ENTITY_KIND_MASK_STINGRAY |
0;

const Entity_flags DEFAULT_BULLET_FLAGS =
ENTITY_FLAG_APPLY_COLLISION |
ENTITY_FLAG_APPLY_COLLISION_DAMAGE |
ENTITY_FLAG_DIE_IF_OFFSCREEN |
ENTITY_FLAG_DIE_ON_APPLY_COLLISION |
ENTITY_FLAG_DYNAMICS |
0;

const float AVENGER_NORMAL_BULLET_VELOCITY = 1400;
const float AVENGER_NORMAL_BULLET_BOUNDS_RADIUS = 5;
//const Vector2 AVENGER_NORMAL_BULLET_SPAWN_OFFSET = { 0, -PLAYER_BOUNDS_SIZE.y * 0.6f - AVENGER_NORMAL_BULLET_BOUNDS_SIZE.y };
const float AVENGER_NORMAL_FIRE_COOLDOWN = 0.05f;
const s32 AVENGER_NORMAL_BULLET_DAMAGE = 5;

const Vector2 CRAB_INITIAL_DEBUG_POS = { WINDOW_WIDTH * 0.5f , WINDOW_HEIGHT * 0.2f };
const s32 CRAB_HEALTH = 15;
const float CRAB_BOUNDS_RADIUS = 50;
//const float CRAB_SPRITE_SCALE;
//const float CRAB_SPRITE_TINT;
const float CRAB_ACCEL = 5.5e3;
const Color CRAB_BOUNDS_COLOR = { 0, 228, 48, 150 };
const Entity_kind_mask CRAB_APPLY_COLLISION_MASK =
ENTITY_KIND_MASK_PLAYER |
0;

const float CRAB_FIRE_COOLDOWN = 0.09f;

#endif
