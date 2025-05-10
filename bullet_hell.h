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
#define TARGET_FRAME_TIME ((float)(1.0f / (float)TARGET_FPS))

#define WINDOW_SCALE 320
#define WINDOW_WIDTH  (4*WINDOW_SCALE)
#define WINDOW_HEIGHT (3*WINDOW_SCALE)

#define WINDOW_SIZE ((Vector2){ WINDOW_WIDTH, WINDOW_HEIGHT })
#define WINDOW_RECT ((Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT})

#define MAX_ENTITIES 4096
#define MAX_PARTICLES 8192
#define MAX_BULLET_EMITTER_RINGS 4
#define MAX_BULLETS_IN_BAG 8
#define MAX_LEADERS 32
#define MAX_ENTITY_LISTS 16

#define BLOOD ((Color){ 255, 0, 0, 255 })


/*
 * tables
 */

#define GAME_STATES             \
  X(NONE)                       \
  X(TITLE_SCREEN)               \
  X(SPAWN_PLAYER)               \
  X(MAIN_LOOP)                  \
  X(WAVE_TRANSITION)            \
  X(VICTORY)                    \
  X(GAME_OVER)                  \
  X(DEBUG_SANDBOX)              \

#define GAME_DEBUG_FLAGS       \
  X(DEBUG_UI)                  \
  X(HOT_RELOAD)                \
  X(PLAYER_INVINCIBLE)         \
  X(DRAW_ALL_ENTITY_BOUNDS)    \
  X(SANDBOX_LOADED)            \

#define GAME_FLAGS         \
  X(PAUSE)                 \
  X(PLAYER_CANNOT_SHOOT)   \
  X(DO_THE_CRAB_WALK)      \

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
  X(LEADER)                    \
  X(BULLET)                    \
  X(CRAB)                      \
  X(SHIELD)                    \
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
  X(NOT_ON_SCREEN)                   \
  X(ON_SCREEN)                       \
  X(DIE_IF_OFFSCREEN)                \
  X(DIE_IF_EXIT_SCREEN)              \
  X(DIE_IF_CHILD_LIST_EMPTY)         \
  X(DIE_IF_ORPHANED)                 \
  X(DIE_NOW)                         \
  X(APPLY_COLLISION)                 \
  X(RECEIVE_COLLISION)               \
  X(APPLY_COLLISION_DAMAGE)          \
  X(RECEIVE_COLLISION_DAMAGE)        \
  X(DAMAGE_INCREMENTS_SCORE)         \
  X(DIE_ON_APPLY_COLLISION)          \
  X(HAS_PARTICLE_EMITTER)            \
  X(CHILDREN_ON_SCREEN)              \
  X(HAS_SHIELDS)                     \
  X(EMIT_SPAWN_PARTICLES)            \
  X(EMIT_DEATH_PARTICLES)            \
  X(APPLY_EFFECT_TINT)               \

#define ENTITY_MOVE_CONTROLS          \
  X(PLAYER)                           \
  X(FOLLOW_LEADER)                    \
  X(FOLLOW_LEADER_CHAIN)              \
  X(COPY_LEADER)                      \
  X(ORBIT_LEADER)                     \
  X(LEADER_HORIZONTAL_STRAFE)         \
  X(GOTO_WAYPOINT)                    \

#define ENTITY_SHOOT_CONTROLS         \
  X(ONCE)                             \
  X(PERIODIC_1)                       \
  X(PERIODIC_2)                       \
  X(PERIODIC_3)                       \
  X(PERIODIC_4)                       \

#define PARTICLE_EMITTERS        \
  X(SPARKS)                      \
  X(BLOOD_SPIT)                  \
  X(BLOOD_PUFF)                  \
  X(PINK_PUFF)                   \
  X(GREEN_PUFF)                  \
  X(WHITE_PUFF)                  \

#define PARTICLE_FLAGS            \

#define BULLET_EMITTER_KINDS         \
  X(AVENGER_DOUBLE_BARREL)           \
  X(AVENGER_TRIPLE_THREAT)           \
  X(AVENGER)                         \
  X(CRAB_BASIC)                      \
  X(CRAB_BATTLE_RIFLE)               \
  X(CRAB_RADIAL_BOOM)                \

#define BULLET_EMITTER_FLAGS         \

#define BULLET_EMITTER_RING_FLAGS   \
  X(LOOK_AT_PLAYER)                 \
  X(USE_POINT_BAG)                  \
  X(BURST)                          \


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
typedef struct Particle Particle;
typedef struct Bullet_emitter Bullet_emitter;
typedef u64 Game_flags;
typedef u64 Game_debug_flags;
typedef u64 Particle_emitter_kind_mask;
typedef u64 Bullet_emitter_kind_mask;
typedef u64 Bullet_emitter_flags;
typedef struct Bullet_emitter_ring Bullet_emitter_ring;
typedef u64 Bullet_emitter_ring_flags;
typedef u64 Input_flags;
typedef u64 Entity_flags;
typedef u64 Entity_kind_mask;
typedef u32 Particle_flags;
typedef struct Entity_handle Entity_handle;
typedef struct Entity_node Entity_node;
typedef struct Entity_list Entity_list;
typedef struct Waypoint Waypoint;
typedef struct Waypoint_list Waypoint_list;
typedef void (*Entity_modifier_proc)(Game*, Entity*);

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

char *Entity_kind_strings[ENTITY_KIND_MAX] = {
  "",
#define X(kind) #kind,
  ENTITY_KINDS
#undef X
};

STATIC_ASSERT(ENTITY_KIND_MAX < 64, number_of_entity_kinds_is_less_than_64);

typedef enum Entity_order {
  ENTITY_ORDER_INVALID = -1,
#define X(order) ENTITY_ORDER_##order,
  ENTITY_ORDERS
#undef X
    ENTITY_ORDER_MAX,
} Entity_order;

typedef enum Entity_move_control {
  ENTITY_MOVE_CONTROL_NONE = 0,
#define X(move_control) ENTITY_MOVE_CONTROL_##move_control,
  ENTITY_MOVE_CONTROLS
#undef X
    ENTITY_MOVE_CONTROL_MAX
} Entity_move_control;

typedef enum Entity_shoot_control {
  ENTITY_SHOOT_CONTROL_NONE = 0,
#define X(shoot_control) ENTITY_SHOOT_CONTROL_##shoot_control,
  ENTITY_SHOOT_CONTROLS
#undef X
    ENTITY_SHOOT_CONTROL_MAX
} Entity_shoot_control;

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

#define X(flag) const Particle_flags PARTICLE_FLAG_##flag = (Particle_flags)(1u<<PARTICLE_FLAG_INDEX_##flag);
PARTICLE_FLAGS
#undef X

STATIC_ASSERT(PARTICLE_FLAG_INDEX_MAX < 32, number_of_particle_flags_is_less_than_32);

typedef enum Particle_emitter {
  PARTICLE_EMITTER_INVALID = 0,
#define X(kind) PARTICLE_EMITTER_##kind,
  PARTICLE_EMITTERS
#undef X
    PARTICLE_EMITTER_MAX,
} Particle_emitter;

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
  Particle_flags flags;

  f32 live;
  f32 lifetime;

  Vector2 pos;
  Vector2 vel;
  f32     radius; /* this says radius, but the particles are squares */
  f32     friction;
  Color   begin_tint;
  Color   end_tint;
};

struct Bullet_emitter_ring {
  Bullet_emitter_ring_flags flags;
  Vector2 dir;
  f32 initial_angle;

  f32 radius;
  f32 spin_cur_angle;
  f32 spin_start_angle;
  f32 spin_vel;

  s32 n_arms;
  f32 arms_occupy_circle_sector_percent;

  Vector2 bullet_point_bag[MAX_BULLETS_IN_BAG];

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

  Particle_emitter bullet_spawn_particle_emitter;
  Particle_emitter bullet_death_particle_emitter;

  Sprite bullet_sprite;
  f32    bullet_sprite_scale;
  f32    bullet_sprite_rotation;
  Color  bullet_sprite_tint;

  f32 burst_cooldown;
  f32 burst_timer;
  s32 burst_shots;
  s32 burst_shots_fired;
};

struct Bullet_emitter {
  Bullet_emitter_flags flags;
  Entity_kind_mask bullet_collision_mask;
  Bullet_emitter_kind  kind;

  Bullet_emitter_ring rings[MAX_BULLET_EMITTER_RINGS];

  s32 shots[MAX_BULLET_EMITTER_RINGS];
  f32 cooldown_period[MAX_BULLET_EMITTER_RINGS];
  f32 cooldown_timer[MAX_BULLET_EMITTER_RINGS];

  u32 active_rings_mask;
  b16 cocked;
  s16 shoot;
};

struct Waypoint_list {
  Waypoint *first;
  Waypoint *last;
  s64 count;
  Entity_modifier_proc action;
};

struct Waypoint {
  Waypoint *next;
  Waypoint *prev;
  Vector2   pos;
  f32       radius;
  u32       tag;
};

struct Entity_list {
  Entity_node *first;
  Entity_node *last;
  s64 count;
};

struct Entity_handle {
  u64     uid;
  Entity *ep;
};

struct Entity_node {
  Entity_node *next;
  Entity_node *prev;
  union {
    Entity_handle handle;
    struct {
      u64     uid;
      Entity *ep;
    };
  };
};

struct Entity {
  b32 live;

  Entity_kind    kind;
  Entity_order   update_order;
  Entity_order   draw_order;

  Entity_move_control move_control;
  Entity_shoot_control shoot_control;

  Entity_flags   flags;

  Entity *free_list_next;

  u64 uid;
  u64 tag;

  Entity_handle leader_handle;

  Entity_list *child_list;
  Entity_list *parent_list;
  Entity_node *list_node;

  Vector2 look_dir;
  Vector2 accel;
  Vector2 vel;
  Vector2 pos;
  f32     radius;
  f32     curve;
  f32     curve_rolloff_vel;
  f32     scalar_vel;
  f32     friction;

  f32     orbit_cur_angle;
  f32     orbit_speed;
  f32     orbit_radius;

  f32 leader_strafe_padding;

  f32 shooting_pause_timer;
  f32 start_shooting_delay;

  b32 received_collision;
  s32 received_damage;

  s32 damage_amount;

  s32 health;

  Color bounds_color;
  Color fill_color;

  Waypoint_list waypoints;
  Waypoint     *cur_waypoint;

  Particle_emitter particle_emitter;
  Particle_emitter spawn_particle_emitter;
  Particle_emitter death_particle_emitter;

  Bullet_emitter bullet_emitter;

  Entity_kind_mask apply_collision_mask;

  Sprite sprite;
  f32    sprite_scale;
  f32    sprite_rotation;
  Color  sprite_tint;
  Color  effect_tint;
  f32    effect_tint_timer_vel;
  f32    effect_tint_period;
  f32    effect_tint_timer;

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
  Arena *wave_scratch;

  Font font;

  RenderTexture2D render_texture;

  Texture2D particle_atlas;
  Texture2D sprite_atlas;
  Texture2D background_texture;

  f32 background_y_offset;

  u64 entity_uid;
  Entity *entities;
  u64 entities_allocated;
  Entity *entity_free_list;

  Particle *particles;
  u64 particles_pos;

  u32 live_entities;
  u32 live_particles;
  u32 live_enemies;

  Entity *player;
  Entity_handle player_handle;

  s16 wave;

  s16 phase_index;

  struct {
    u32 flags;
    b16 check_if_finished;
    b16 set_timer;
    s32 accumulator;
    f32 timer[4];
    union {
      Waypoint_list *waypoints;
    };
  } phase;

  s32 score;
  s32 player_health;

  f32  wave_timer;
  f32  wave_type_char_timer;
  s32  wave_chars_typed;
  s32  wave_banner_type_dir;
  s32  wave_banner_target_msg_len;
  Str8 wave_banner_msg;

};


/*
 * function headers
 */

float get_random_float(float min, float max, int steps);

void game_init(Game *gp);
void game_load_assets(Game *gp);
void game_unload_assets(Game *gp);
void game_update_and_draw(Game *gp);
void game_reset(Game *gp);
void game_main_loop(Game *gp);
void game_wave_end(Game *gp);

Entity* entity_spawn(Game *gp);
void    entity_die(Game *gp, Entity *ep);

Entity_list* push_entity_list(Game *gp);
Entity_node* push_entity_list_node(Game *gp);
Entity_list* get_entity_list_by_id(Game *gp, int list_id);
Entity_node* entity_list_append(Game *gp, Entity_list *list, Entity *ep);
Entity_node* entity_list_push_front(Game *gp, Entity_list *list, Entity *ep);
b32          entity_list_remove(Game *gp, Entity_list *list, Entity *ep);

Waypoint* waypoint_list_append(Game *gp, Waypoint_list *list, Vector2 pos, float radius);
Waypoint* waypoint_list_append_tagged(Game *gp, Waypoint_list *list, Vector2 pos, float radius, u64 tag);

Entity* entity_from_uid(Game *gp, u64 uid);
Entity* entity_from_handle(Entity_handle handle);

Entity_handle handle_from_entity(Entity *ep);

Entity* spawn_player(Game *gp);
Entity* spawn_crab(Game *gp);
Entity* spawn_fish(Game *gp);
Entity* spawn_stingray(Game *gp);
Entity* spawn_crab_leader(Game *gp);

void entity_emit_particles(Game *gp, Entity *ep);

void entity_emit_bullets(Game *gp, Entity *ep);

b32  entity_check_collision(Game *gp, Entity *a, Entity *b);

void draw_sprite(Game *gp, Entity *ep);
void sprite_update(Game *gp, Entity *ep);
Sprite_frame sprite_current_frame(Sprite sp);
b32 sprite_at_keyframe(Sprite sp, s32 keyframe);


/*
 * entity settings
 */

const Vector2 PLAYER_INITIAL_POS = { WINDOW_WIDTH * 0.5f , WINDOW_HEIGHT * 0.8f };
const Vector2 PLAYER_INITIAL_DEBUG_POS = { WINDOW_WIDTH * 0.5f , WINDOW_HEIGHT * 0.8f };
const Vector2 PLAYER_LOOK_DIR = { 0, -1 };
const s32 PLAYER_HEALTH = 100;
const float PLAYER_BOUNDS_RADIUS = 22;
const float PLAYER_SPRITE_SCALE = 2.0f;
const float PLAYER_ACCEL = 1.6e4;
const float PLAYER_SLOW_FACTOR = 0.5f;
const Color PLAYER_BOUNDS_COLOR = { 255, 0, 0, 255 };
const Entity_kind_mask PLAYER_APPLY_COLLISION_MASK =
ENTITY_KIND_MASK_CRAB     |
0;

const Entity_flags DEFAULT_BULLET_FLAGS =
ENTITY_FLAG_APPLY_COLLISION |
ENTITY_FLAG_APPLY_COLLISION_DAMAGE |
ENTITY_FLAG_DIE_IF_OFFSCREEN |
ENTITY_FLAG_DIE_ON_APPLY_COLLISION |
ENTITY_FLAG_DYNAMICS |
0;

const float AVENGER_NORMAL_BULLET_VELOCITY = 1800;
const float AVENGER_NORMAL_BULLET_BOUNDS_RADIUS = 10;
const float AVENGER_NORMAL_FIRE_COOLDOWN = 0.04f;
const s32 AVENGER_NORMAL_BULLET_DAMAGE = 5;

const Vector2 CRAB_LOOK_DIR = { 0, 1 };

const s32 CRAB_HEALTH = 10;
const s32 BIG_CRAB_HEALTH = 25;
const s32 LARGE_CRAB_HEALTH = 35;
const s32 HUGE_CRAB_HEALTH = 30;

const float CRAB_FOLLOW_LEADER_SPEED = 300;
const float CRAB_BOUNDS_RADIUS = 40;
const float CRAB_SPRITE_SCALE = 2.0f;
const float CRAB_NORMAL_STRAFE_SPEED = 400;

const float CRAB_ACCEL = 5.5e3;

const Color CRAB_BOUNDS_COLOR = { 0, 228, 48, 255 };
const Entity_kind_mask CRAB_APPLY_COLLISION_MASK =
ENTITY_KIND_MASK_PLAYER |
0;
const float CRAB_FIRE_COOLDOWN = 0.03f;
const float CRAB_BURST_PAUSE = 1.0f;
const float CRAB_NORMAL_BULLET_BOUNDS_RADIUS = 10;
const float CRAB_NORMAL_BULLET_VELOCITY = 400;
const float CRAB_NORMAL_BULLET_DAMAGE = 1;

const Vector2 ORBIT_ARM = { 0, -1 };

const Entity_kind_mask ENEMY_KIND_MASK =
ENTITY_KIND_MASK_CRAB |
ENTITY_KIND_MASK_SHIELD |
ENTITY_KIND_MASK_LEADER |
0;

const float WAVE_DELAY_TIME = 1.4f;
const float WAVE_BANNER_TYPE_SPEED   = 0.1f;
const float WAVE_BANNER_FONT_SIZE    = 80.0f;
const float WAVE_BANNER_FONT_SPACING = 10.0f;
const Color WAVE_BANNER_FONT_COLOR   = GOLD;

#endif
