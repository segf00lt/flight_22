#include "bullet_hell.h"


/*
 * generated code
 */

#include "sprite_data.c"

//#include "sound_data.c"



/*
 * globals
 */

bool has_started_before = false;
bool at_boss_level = false;

bool  title_screen_key_pressed = false;
int   title_screen_chars_deleted = 0;
float title_screen_type_char_timer = 0;


Sprite colonel_fu = SPRITE_CORONEL_FU;

const float INTRO_SCREEN_FADE_TIME = 1.0f;
float intro_screen_fade_timer = 0.0f;
float intro_screen_colonel_delay = 1.0f;
int   intro_screen_cur_message = 0;
int   intro_screen_chars_typed = 0;
float intro_screen_type_char_timer = 0;
bool  intro_screen_cursor_on = false;
float intro_screen_cursor_blink_timer = 0;
float intro_screen_pre_message_delay = 0.8f;

#define INTRO_SCREEN_MESSAGES               \
  X("SOLDIER, WE'RE ABOUT TO LOSE CONTACT!") \
  X("REMEMBER THE DRILL:\n\n'W A S D' to move\n'J' to shoot\n'B' to drop bombs") \
  X("YOU'RE RIGHT OVER THE BERMUDA TRIANGLE, SON.  \nTRY YOUR BEST TO SURVI..ab%K8^as&kuBZzzzzz#*z\n\n==== TRANSMISSION TERMINATED ====") \


char *intro_screen_messages[] = {
#define X(x) x,
  INTRO_SCREEN_MESSAGES
#undef X
};

int intro_screen_message_lengths[] = {
#define X(x) (int)STRLEN(x),
  INTRO_SCREEN_MESSAGES
#undef X
};

bool  game_over_hint_on = true;
float game_over_hint_timer = 0;

float victory_screen_pre_delay = 1.0f;
float victory_screen_pre_counter_delay = 0.5f;
bool  victory_screen_finished_typing_banner = false;
bool  victory_screen_finished_incrementing_score = false;
int   victory_screen_chars_typed = 0;
int   victory_screen_type_dir = 1;
float victory_screen_show_timer = 2.5f;
int   victory_screen_target_len = STRLEN("VICTORY");
float victory_screen_type_char_timer = 0;
float victory_screen_inc_score_timer = 0;
float victory_screen_score_delay = 0.9f;
int   victory_screen_score_counter = 0;
bool  victory_screen_restart_hint_on = false;
float victory_screen_hint_timer = 0;

Bullet_emitter boss_bullet_emitters[] = {
  { .kind = BULLET_EMITTER_KIND_BOSS_BOOM, },
  { .kind = BULLET_EMITTER_KIND_BOSS_WIDE_SWEEP, },
  { .kind = BULLET_EMITTER_KIND_BOSS_V_SWEEP, },
  //{ .kind = BULLET_EMITTER_KIND_BOSS_TENTACLE, },
  //{ .kind = BULLET_EMITTER_KIND_BOSS_CIRCLES, },
};


float boss_start_shooting_delay = 0;
float boss_shoot_pause = 0;

const int n_boss_bullet_emitters = ARRLEN(boss_bullet_emitters);




/* 
 * function bodies
 */

#define frame_push_array(T, n) (T*)push_array(gp->frame_scratch, T, n)
#define wave_push_array(T, n) (T*)push_array(gp->wave_scratch, T, n)

#define frame_push_struct(T) (T*)push_array(gp->frame_scratch, T, 1)
#define wave_push_struct(T) (T*)push_array(gp->wave_scratch, T, 1)

#define frame_push_struct_no_zero(T) (T*)push_array_no_zero(gp->frame_scratch, T, 1)
#define wave_push_struct_no_zero(T) (T*)push_array_no_zero(gp->wave_scratch, T, 1)

#define frame_push_array_no_zero(T, n) (T*)push_array_no_zero(gp->frame_scratch, T, n)
#define wave_push_array_no_zero(T, n) (T*)push_array_no_zero(gp->wave_scratch, T, n)

#define frame_scope_begin() scope_begin(gp->frame_scratch)
#define frame_scope_end(scope) scope_end(scope)

#define wave_scope_begin() scope_begin(gp->wave_scratch)
#define wave_scope_end(scope) scope_end(scope)

#define entity_is_part_of_list(ep) (ep->list_node && ep->list_node->ep == ep)

Entity *entity_spawn(Game *gp) {
  Entity *ep = 0;

  if(gp->entity_free_list) {
    ep = gp->entity_free_list;
    gp->entity_free_list = ep->free_list_next;
  } else {
    ASSERT(gp->entities_allocated < MAX_ENTITIES);
    ep = &gp->entities[gp->entities_allocated++];
  }

  gp->entity_uid++;

  *ep =
    (Entity){
      .live = 1,
      .uid = gp->entity_uid,
    };

  return ep;
}

void entity_die(Game *gp, Entity *ep) {
  ep->free_list_next = gp->entity_free_list;
  gp->entity_free_list = ep;
  ep->live = 0;

  if(entity_is_part_of_list(ep)) {
    ASSERT(ep->parent_list);
    Entity_list *list = ep->parent_list;
    list->count--;
    dll_remove(list->first, list->last, ep->list_node);
  }

}

force_inline Entity_list* push_entity_list(Game *gp) {
  Entity_list *list = push_array(gp->wave_scratch, Entity_list, 1);
  return list;
}

force_inline Entity_node* push_entity_list_node(Game *gp) {
  Entity_node *e_node = push_array(gp->wave_scratch, Entity_node, 1);
  return e_node;
}

Entity_node* entity_list_append(Game *gp, Entity_list *list, Entity *ep) {
  ASSERT(list);

  Entity_node *e_node = push_entity_list_node(gp);

  e_node->uid = ep->uid;
  e_node->ep = ep;

  ep->parent_list = list;
  ep->list_node = e_node;

  dll_push_back(list->first, list->last, e_node);
  list->count++;

  return e_node;
}

Entity_node* entity_list_push_front(Game *gp, Entity_list *list, Entity *ep) {
  ASSERT(list);

  Entity_node *e_node = push_entity_list_node(gp);

  e_node->uid = ep->uid;
  e_node->ep = ep;

  dll_push_front(list->first, list->last, e_node);
  list->count++;

  return e_node;
}

b32 entity_list_remove(Game *gp, Entity_list *list, Entity *ep) {
  ASSERT(list);

  Entity_node *found = 0;

  for(Entity_node *node = list->first; node; node = node->next) {
    if(node->ep == ep) {
      found = node;
      break;
    }
  }

  if(found) {
    dll_remove(list->first, list->last, found);
  }

  list->count--;

  return (b32)(!!found);
}

Entity* entity_from_uid(Game *gp, u64 uid) {
  Entity *ep = 0;

  for(int i = 0; i < gp->entities_allocated; i++) {
    if(gp->entities[i].uid == uid) {
      ep = gp->entities + i;
      break;
    }
  }

  return ep;
}

Entity* entity_from_handle(Entity_handle handle) {
  ASSERT(handle.ep && handle.uid);

  Entity *ep = 0;

  if(handle.ep->uid == handle.uid) {
    ep = handle.ep;
  }

  return ep;
}

Entity_handle handle_from_entity(Entity *ep) {
  return (Entity_handle){ .uid = ep->uid, .ep = ep };
}

force_inline Waypoint* waypoint_list_append(Game *gp, Waypoint_list *list, Vector2 pos, float radius) {
  return waypoint_list_append_tagged(gp, list, pos, radius, 0);
}

Waypoint* waypoint_list_append_tagged(Game *gp, Waypoint_list *list, Vector2 pos, float radius, u64 tag) {
  Waypoint *wp = push_array_no_zero(gp->wave_scratch, Waypoint, 1);
  wp->pos = pos;
  wp->radius = radius;
  wp->tag = tag;

  dll_push_back(list->first, list->last, wp);

  list->count++;

  return wp;
}

Entity* spawn_player(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->move_control = ENTITY_MOVE_CONTROL_PLAYER;
  ep->kind = ENTITY_KIND_PLAYER;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_HAS_BULLET_EMITTER |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    ENTITY_FLAG_NOT_ON_SCREEN |
    ENTITY_FLAG_EMIT_DEATH_PARTICLES |
    0;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->look_dir = PLAYER_LOOK_DIR;
  ep->pos = PLAYER_INITIAL_POS;

  ep->health = PLAYER_HEALTH;

  ep->hurt_sound = gp->avenger_hurt_sound;

  ep->sprite = SPRITE_SPITFIRE_IDLE;
  ep->sprite_tint = WHITE;
  ep->sprite_scale = PLAYER_SPRITE_SCALE;

  ep->death_particle_emitter = PARTICLE_EMITTER_BIG_PLANE_EXPLOSION;

  ep->bullet_emitter.kind = BULLET_EMITTER_KIND_AVENGER;
  ep->bullet_emitter.flags =
    0;

  ep->bounds_color = PLAYER_BOUNDS_COLOR;

  ep->radius = PLAYER_BOUNDS_RADIUS;
  ep->friction = PLAYER_FRICTION;

  return ep;
}

Entity* spawn_leader(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_LEADER;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_NOT_ON_SCREEN |
    ENTITY_FLAG_DIE_IF_CHILD_LIST_EMPTY |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->radius = 16.0f;
  ep->bounds_color = GREEN;

  return ep;
}

Entity* spawn_crab(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_CRAB;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    //ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    //ENTITY_FLAG_FILL_BOUNDS |
    ENTITY_FLAG_DIE_IF_EXIT_SCREEN |
    ENTITY_FLAG_NOT_ON_SCREEN |
    ENTITY_FLAG_HAS_BULLET_EMITTER |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    ENTITY_FLAG_EMIT_SPAWN_PARTICLES |
    ENTITY_FLAG_EMIT_DEATH_PARTICLES |
    ENTITY_FLAG_DAMAGE_INCREMENTS_SCORE |
    0;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->look_dir = (Vector2){ 0, 1 };

  ep->health = CRAB_HEALTH;

  ep->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_BASIC;

  ep->spawn_particle_emitter = PARTICLE_EMITTER_WHITE_PUFF;
  ep->death_particle_emitter = PARTICLE_EMITTER_BLOOD_PUFF;

  ep->bounds_color = CRAB_BOUNDS_COLOR;
  ep->sprite = SPRITE_CRAB;
  ep->sprite_scale = CRAB_SPRITE_SCALE;
  ep->sprite_tint = ORANGE;
  ep->effect_tint = BLOOD;
  ep->effect_tint_period = 0.08f;
  ep->effect_tint_timer_vel = 1.2f;

  ep->move_control = ENTITY_MOVE_CONTROL_NONE;
  ep->shoot_control = ENTITY_SHOOT_CONTROL_NONE;

  ep->radius = CRAB_BOUNDS_RADIUS;

  ep->hurt_sound = gp->crab_hurt_sound;

  return ep;
}

Entity* spawn_boss_crab(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_BOSS;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_DIE_IF_EXIT_SCREEN |
    ENTITY_FLAG_NOT_ON_SCREEN |
    ENTITY_FLAG_HAS_BULLET_EMITTER |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    ENTITY_FLAG_EMIT_SPAWN_PARTICLES |
    ENTITY_FLAG_EMIT_DEATH_PARTICLES |
    ENTITY_FLAG_DAMAGE_INCREMENTS_SCORE |
    0;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->look_dir = (Vector2){ 0, 1 };

#ifdef DEBUG
  ep->health = 1;
#else
  ep->health = 10000;
#endif

  ep->bullet_emitter.kind = BULLET_EMITTER_KIND_BOSS_BOOM;

  ep->spawn_particle_emitter = PARTICLE_EMITTER_WHITE_PUFF;
  ep->death_particle_emitter = PARTICLE_EMITTER_MASSIVE_BLOOD_PUFF;

  ep->bounds_color = CRAB_BOUNDS_COLOR;
  ep->sprite = SPRITE_BOSS_CRAB;
  ep->sprite_scale = 3.0f;
  ep->sprite_tint = WHITE;
  ep->effect_tint = BLOOD;
  ep->effect_tint_period = 0.08f;
  ep->effect_tint_timer_vel = 1.3f;

  ep->pos = (Vector2){ .x = 0.5f*WINDOW_WIDTH, .y = -0.4f*WINDOW_HEIGHT };
  ep->vel = (Vector2){ .y = 300 };
  ep->friction = 0.56f;

  ep->move_control = ENTITY_MOVE_CONTROL_NONE;
  ep->shoot_control = ENTITY_SHOOT_CONTROL_BOSS;

  ep->radius = 110.0f;

  ep->hurt_sound = gp->crab_hurt_sound;

  return ep;
}

Entity* spawn_health_pack(Game *gp) {

  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_HEALTH_PACK;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_APPLY_COLLISION |
    ENTITY_FLAG_DIE_ON_APPLY_COLLISION |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_DIE_IF_EXIT_SCREEN |
    ENTITY_FLAG_NOT_ON_SCREEN |
    ENTITY_FLAG_EMIT_DEATH_PARTICLES |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->pos = (Vector2){ .x = (float)GetRandomValue(200, WINDOW_WIDTH-200), . y = -0.4*WINDOW_HEIGHT }; 
  ep->vel = (Vector2){ .y = (float)GetRandomValue(780, 800), };
  ep->friction = 0.45f;

  ep->collide_proc = collide_with_health_pack;

  ep->death_particle_emitter = PARTICLE_EMITTER_GREEN_PUFF;

  ep->apply_collision_mask =
    ENTITY_KIND_MASK_PLAYER |
    0;

  ep->bounds_color = GREEN;
  ep->sprite = SPRITE_HEALTH;
  ep->sprite_scale = 3.0f;
  ep->sprite_tint = WHITE;

  ep->radius = 40;

  return ep;
}

Entity* spawn_bomb_pack(Game *gp) {

  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_BOMB_PACK;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_APPLY_COLLISION |
    ENTITY_FLAG_DIE_ON_APPLY_COLLISION |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_DIE_IF_EXIT_SCREEN |
    ENTITY_FLAG_NOT_ON_SCREEN |
    ENTITY_FLAG_EMIT_DEATH_PARTICLES |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->pos = (Vector2){ .x = (float)GetRandomValue(200, WINDOW_WIDTH-200), . y = -0.4*WINDOW_HEIGHT }; 
  ep->vel = (Vector2){ .y = (float)GetRandomValue(780, 800), };
  ep->friction = 0.45f;

  ep->collide_proc = collide_with_bomb_pack;

  ep->death_particle_emitter = PARTICLE_EMITTER_BROWN_PUFF;

  ep->apply_collision_mask =
    ENTITY_KIND_MASK_PLAYER |
    0;

  ep->bounds_color = GREEN;
  ep->sprite = SPRITE_BOOM;
  ep->sprite_scale = 3.0f;
  ep->sprite_tint = WHITE;

  ep->radius = PICKUP_BOUNDS_RADIUS;

  return ep;
}

Entity* spawn_double_bullets_pack(Game *gp) {

  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_DOUBLE_BULLETS_PACK;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_APPLY_COLLISION |
    ENTITY_FLAG_DIE_ON_APPLY_COLLISION |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_DIE_IF_EXIT_SCREEN |
    ENTITY_FLAG_NOT_ON_SCREEN |
    ENTITY_FLAG_EMIT_DEATH_PARTICLES |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->pos = (Vector2){ .x = (float)GetRandomValue(200, WINDOW_WIDTH-200), . y = -0.4*WINDOW_HEIGHT }; 
  ep->vel = (Vector2){ .y = (float)GetRandomValue(780, 800), };
  ep->friction = 0.45f;

  ep->collide_proc = collide_with_double_bullets_pack;

  ep->death_particle_emitter = PARTICLE_EMITTER_BROWN_PUFF;

  ep->apply_collision_mask =
    ENTITY_KIND_MASK_PLAYER |
    0;

  ep->bounds_color = GREEN;
  ep->sprite = SPRITE_DOUBLE_TROUBLE;
  ep->sprite_scale = 3.0f;
  ep->sprite_tint = WHITE;

  ep->radius = PICKUP_BOUNDS_RADIUS;

  return ep;
}

Entity* spawn_triple_bullets_pack(Game *gp) {

  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_TRIPLE_BULLETS_PACK;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_APPLY_COLLISION |
    ENTITY_FLAG_DIE_ON_APPLY_COLLISION |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_DIE_IF_EXIT_SCREEN |
    ENTITY_FLAG_NOT_ON_SCREEN |
    ENTITY_FLAG_EMIT_DEATH_PARTICLES |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->pos = (Vector2){ .x = (float)GetRandomValue(200, WINDOW_WIDTH-200), . y = -0.4*WINDOW_HEIGHT }; 
  ep->vel = (Vector2){ .y = (float)GetRandomValue(780, 800), };
  ep->friction = 0.45f;

  ep->collide_proc = collide_with_triple_bullets_pack;

  ep->death_particle_emitter = PARTICLE_EMITTER_BROWN_PUFF;

  ep->apply_collision_mask =
    ENTITY_KIND_MASK_PLAYER |
    0;

  ep->bounds_color = GREEN;
  ep->sprite = SPRITE_TRIPLE_THREAT;
  ep->sprite_scale = 3.0f;
  ep->sprite_tint = WHITE;

  ep->radius = PICKUP_BOUNDS_RADIUS;

  return ep;
}

Entity* spawn_quinta_bullets_pack(Game *gp) {

  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_QUINTA_BULLETS_PACK;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_APPLY_COLLISION |
    ENTITY_FLAG_DIE_ON_APPLY_COLLISION |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_DIE_IF_EXIT_SCREEN |
    ENTITY_FLAG_NOT_ON_SCREEN |
    ENTITY_FLAG_EMIT_DEATH_PARTICLES |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->pos = (Vector2){ .x = (float)GetRandomValue(200, WINDOW_WIDTH-200), . y = -0.4*WINDOW_HEIGHT }; 
  ep->vel = (Vector2){ .y = (float)GetRandomValue(780, 800), };
  ep->friction = 0.45f;

  ep->collide_proc = collide_with_quinta_bullets_pack;

  ep->death_particle_emitter = PARTICLE_EMITTER_BROWN_PUFF;

  ep->apply_collision_mask =
    ENTITY_KIND_MASK_PLAYER |
    0;

  ep->bounds_color = GREEN;
  ep->sprite = SPRITE_QUINTUS;
  ep->sprite_scale = 3.0f;
  ep->sprite_tint = WHITE;

  ep->radius = PICKUP_BOUNDS_RADIUS;

  return ep;
}

#if 0
Entity* spawn_crab_follow(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->move_control = ENTITY_MOVE_CONTROL_CRAB_FOLLOW_CHAIN;
  ep->kind = ENTITY_KIND_CRAB;

  ep->flags =
    //ENTITY_FLAG_DYNAMICS |
    //ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    //ENTITY_FLAG_FILL_BOUNDS |
    //ENTITY_FLAG_CLAMP_POS_TO_SCREEN |
    ENTITY_FLAG_HAS_BULLET_EMITTER |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    0;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->pos = CRAB_INITIAL_DEBUG_POS;

  ep->health = CRAB_HEALTH;

  //ASSERT(cur_leader_handle.uid && cur_leader_handle.ep);

  //ep->leader_handle = cur_leader_handle;

  //ep->sprite = SPRITE_AVENGER;
  //ep->sprite_tint = WHITE;
  //ep->sprite_scale = PLAYER_SPRITE_SCALE;

  ep->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_BASIC;
  ep->bullet_emitter.flags =
    0;

  ep->bullet_emitter.cooldown_period = CRAB_FIRE_COOLDOWN;

  //ep->fill_color = MAROON;
  ep->bounds_color = CRAB_BOUNDS_COLOR;
  ep->sprite = SPRITE_CRAB;
  ep->sprite_scale = CRAB_SPRITE_SCALE;
  ep->sprite_tint = WHITE;

  ep->radius = CRAB_BOUNDS_RADIUS;

  return ep;
}
#endif

void collide_with_health_pack(Game *gp, Entity *a, Entity *b) {

  ASSERT(a->kind == ENTITY_KIND_HEALTH_PACK);
  ASSERT(b->kind == ENTITY_KIND_PLAYER);

  if(b->health < PLAYER_HEALTH) {
    b->health += 2;
  }

  b->effect_tint = GREEN;

  b->flags |= ENTITY_FLAG_APPLY_EFFECT_TINT;

  b->effect_tint_period = 0.7f;
  b->effect_tint_timer_vel = 1.0f;

  b->effect_tint_timer = b->effect_tint_period;

  SetSoundPan(gp->health_pickup_sound, Normalize(a->pos.x, WINDOW_WIDTH, 0));
  SetSoundVolume(gp->health_pickup_sound, 0.3);
  PlaySound(gp->health_pickup_sound);

}

void collide_with_bomb_pack(Game *gp, Entity *a, Entity *b) {

  if(gp->bomb_count < MAX_BOMBS) {
    gp->bomb_count += 1;
  }

  SetSoundPan(gp->powerup_sound, Normalize(a->pos.x, WINDOW_WIDTH, 0));
  SetSoundVolume(gp->powerup_sound, 0.3);
  PlaySound(gp->powerup_sound);

}

void collide_with_double_bullets_pack(Game *gp, Entity *a, Entity *b) {

  b->bullet_emitter = (Bullet_emitter){ .kind = BULLET_EMITTER_KIND_AVENGER_DOUBLE_TROUBLE };

  SetSoundPan(gp->powerup_sound, Normalize(a->pos.x, WINDOW_WIDTH, 0));
  SetSoundVolume(gp->powerup_sound, 0.3);
  PlaySound(gp->powerup_sound);

}

void collide_with_triple_bullets_pack(Game *gp, Entity *a, Entity *b) {

  b->bullet_emitter = (Bullet_emitter){ .kind = BULLET_EMITTER_KIND_AVENGER_TRIPLE_THREAT };

  SetSoundPan(gp->powerup_sound, Normalize(a->pos.x, WINDOW_WIDTH, 0));
  SetSoundVolume(gp->powerup_sound, 0.3);
  PlaySound(gp->powerup_sound);

}

void collide_with_quinta_bullets_pack(Game *gp, Entity *a, Entity *b) {

  b->bullet_emitter = (Bullet_emitter){ .kind = BULLET_EMITTER_KIND_AVENGER_QUINTUS };

  SetSoundPan(gp->powerup_sound, Normalize(a->pos.x, WINDOW_WIDTH, 0));
  SetSoundVolume(gp->powerup_sound, 0.3);
  PlaySound(gp->powerup_sound);

}

force_inline float get_random_float(float min, float max, int steps) {
  int val = GetRandomValue(0, steps);
  float result = Remap((float)val, 0.0f, (float)steps, min, max);
  return result;
}

void entity_emit_particles(Game *gp, Entity *ep) {

  Particle_emitter emitter = ep->particle_emitter;

  Particle buf[MAX_PARTICLES];
  s32 n_particles = 0;

  switch(emitter) {
    default:
      UNREACHABLE;
    case PARTICLE_EMITTER_BIG_PLANE_EXPLOSION:
      {
        Color tints[] = {
          YELLOW, ORANGE, RED, ColorAlpha(RED, 0.8f),
        };
        int amounts[] = { 300, 200, 40 };

        int base = 0;
        for(int ti = 0; ti < ARRLEN(tints)-1; ti++) {

          n_particles += GetRandomValue(amounts[ti], amounts[ti]+20);
          ASSERT(n_particles <= MAX_PARTICLES);

          int i = base;
          for(; i < n_particles; i++) {
            Particle *p = buf + i;
            *p = (Particle){0};

            p->lifetime = get_random_float(TARGET_FRAME_TIME * 20, TARGET_FRAME_TIME * 30, 5);

            p->pos = ep->pos;
            p->vel =
              Vector2Rotate((Vector2){ 0, -1 },
                  get_random_float(0, 2*PI, 150));

            p->vel = Vector2Scale(p->vel, (float)GetRandomValue(400, 700));

            p->radius = get_random_float(2.9f, 4.7f, 15);
            p->shrink = (0.6*p->radius)/p->lifetime;

            p->friction = (float)GetRandomValue(0, 2);

            p->begin_tint = tints[ti];
            p->end_tint = tints[ti+1];

          }
          base = i;
        }

      } break;
    case PARTICLE_EMITTER_WHITE_PUFF:
      {
        n_particles = GetRandomValue(100, 110);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 20, TARGET_FRAME_TIME * 26, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate((Vector2){ 0, -1 },
                get_random_float(0, 2*PI, 150));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(1400, 1500));

          p->radius = get_random_float(2.0f, 4.0f, 5);
          p->shrink = (0.5*p->radius)/p->lifetime;

          p->friction = (float)GetRandomValue(8, 15);

          p->begin_tint = RAYWHITE;
          p->end_tint = ColorAlpha(p->begin_tint, 0.8);

        }

      } break;
    case PARTICLE_EMITTER_BROWN_PUFF:
      {
        n_particles = GetRandomValue(10, 15);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 30, TARGET_FRAME_TIME * 40, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate((Vector2){ 0, -1 },
                get_random_float(0, 2*PI, 150));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(80, 90));

          p->radius = get_random_float(2.9f, 3.5f, 4);
          p->shrink = (0.36*p->radius)/p->lifetime;


          p->friction = get_random_float(0.05f, 0.1f, 4);

          p->begin_tint = (Color){ 102, 57, 49, 255 };
          p->end_tint = ColorAlpha(p->begin_tint, 0.8);

        }

      } break;
    case PARTICLE_EMITTER_GREEN_PUFF:
      {
        n_particles = GetRandomValue(10, 15);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 30, TARGET_FRAME_TIME * 40, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate((Vector2){ 0, -1 },
                get_random_float(0, 2*PI, 150));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(80, 90));

          p->radius = get_random_float(2.9f, 3.5f, 4);
          p->shrink = (0.36*p->radius)/p->lifetime;


          p->friction = get_random_float(0.05f, 0.1f, 4);

          p->begin_tint = (Color){ 0, 255, 0, 255 };
          p->end_tint = ColorAlpha(p->begin_tint, 0.8);

        }

      } break;
    case PARTICLE_EMITTER_PINK_PUFF:
      {
        n_particles = GetRandomValue(120, 150);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 10, TARGET_FRAME_TIME * 20, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate(Vector2Normalize(Vector2Negate(ep->vel)),
                get_random_float(0, 2*PI, 150));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(1500, 1600));

          p->radius = get_random_float(1.0f, 2.5f, 5);

          p->friction = (float)GetRandomValue(0, 15);

          p->begin_tint = (Color){ 255, 2, 162, 255 };
          p->end_tint = (Color){ 178, 1, 113, 230 };

        }

      } break;
    case PARTICLE_EMITTER_MASSIVE_BLOOD_PUFF:
      {
        n_particles = GetRandomValue(1100, 1300);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 20, TARGET_FRAME_TIME * 30, 5);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate((Vector2){ 0, -1 },
                get_random_float(0, 2*PI, 150));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(400, 600));

          p->radius = get_random_float(3.7f, 4.6f, 10);
          p->shrink = (0.7*p->radius)/p->lifetime;

          p->friction = (float)GetRandomValue(0, 2);

          p->begin_tint = BLOOD;
          p->end_tint = ColorAlpha(BLOOD, 0.75f);

        }

      } break;
    case PARTICLE_EMITTER_BLOOD_PUFF:
      {
        n_particles = GetRandomValue(200, 210);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 20, TARGET_FRAME_TIME * 25, 5);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate((Vector2){ 0, -1 },
                get_random_float(0, 2*PI, 150));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(500, 600));

          p->radius = get_random_float(2.7f, 3.2f, 3);
          p->shrink = (0.7*p->radius)/p->lifetime;

          p->friction = (float)GetRandomValue(0, 2);

          p->begin_tint = BLOOD;
          p->end_tint = ColorAlpha(BLOOD, 0.75f);

        }

      } break;
    case PARTICLE_EMITTER_PURPLE_SPARKS:
      {
        n_particles = GetRandomValue(30, 35);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 10, TARGET_FRAME_TIME * 20, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate((Vector2){ 0, -1 },
                get_random_float(0.0f, 2*PI, 200));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(1500, 1800));

          p->radius = get_random_float(3.2f, 4.5f, 4);
          p->shrink = 12.3f;

          p->friction = (float)GetRandomValue(0, 20);

          p->begin_tint = CRAB_BULLET_SPRITE_TINT;
          p->end_tint = ColorAlpha(p->begin_tint, 0.83);

        }

      } break;
    case PARTICLE_EMITTER_SPARKS:
      {
        n_particles = GetRandomValue(20, 25);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 10, TARGET_FRAME_TIME * 20, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate(Vector2Normalize(Vector2Negate(ep->vel)),
                get_random_float(-PI*0.37f, PI*0.37f, 200));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(1500, 1800));

          p->radius = get_random_float(2.0f, 3.2f, 4);
          p->shrink = 12.3f;

          p->friction = (float)GetRandomValue(0, 20);

          p->begin_tint = (Color){ 255, 188, 3, 255 };
          p->end_tint = ColorAlpha(p->begin_tint, 0.83);

        }

      } break;
    case PARTICLE_EMITTER_BLOOD_SPIT:
      {
        n_particles = GetRandomValue(50, 60);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 10, TARGET_FRAME_TIME * 20, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate(Vector2Normalize(Vector2Negate(ep->vel)),
                get_random_float(-PI*0.1f, PI*0.1f, 200));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(1500, 1800));

          p->radius = get_random_float(1.6f, 2.2f, 4);
          p->shrink = 12.3f;

          p->friction = (float)GetRandomValue(0, 20);

          p->begin_tint = BLOOD;
          p->end_tint = ColorAlpha(BLOOD, 0.85);

        }

      } break;
  }

  for(int i = 0; i < n_particles; i++) {
    if(gp->particles_pos >= MAX_PARTICLES) {
      gp->particles_pos = 0;
    }

    gp->particles[gp->particles_pos++] = buf[i];

  }

}

void entity_emit_bullets(Game *gp, Entity *ep) {
  Bullet_emitter *emitter = &ep->bullet_emitter;

  if(!emitter->cocked) {
    switch(emitter->kind) {
      default:
        UNREACHABLE;
      case BULLET_EMITTER_KIND_AVENGER_QUINTUS:
        {

          emitter->bullet_kind = ENTITY_KIND_PLAYER_BULLET;

          emitter->bullet_collision_mask = AVENGER_BULLET_APPLY_COLLISION_MASK;
          emitter->cooldown_period[0] = 0.12f;
          emitter->active_rings_mask = 0x1;

          emitter->shots[0] = 1;

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->flags |=
              BULLET_EMITTER_RING_FLAG_USE_POINT_BAG |
              0;

            float radius = 15;

            Vector2 point_bag[] = {
              { 0, -2*radius },
              { -1.2*radius, -4.2*radius },
              {           0, -4.2*radius },
              {  1.2*radius, -4.2*radius },
              { 0, -6.2*radius },
            };

            ring->n_bullets = ARRLEN(point_bag);
            memory_copy_array(ring->bullet_point_bag, point_bag);

            ring->sound = gp->avenger_bullet_sound;
            ring->spin_vel = 0.0f;
            ring->radius = 160.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 0;
            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = radius;
            ring->bullet_vel = 1900;
            ring->bullet_friction = 0.1f;
            ring->bullet_damage = 20;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_AVENGER_BULLET;
            ring->bullet_sprite_scale = 3.0f;
            ring->bullet_sprite_tint = WHITE;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_SPARKS;
          }

        } break;
      case BULLET_EMITTER_KIND_AVENGER_TRIPLE_THREAT:
        {

          emitter->bullet_kind = ENTITY_KIND_PLAYER_BULLET;

          emitter->bullet_collision_mask = AVENGER_BULLET_APPLY_COLLISION_MASK;
          emitter->cooldown_period[0] = 0.12f;
          emitter->active_rings_mask = 0x1;

          emitter->shots[0] = 1;
          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->sound = gp->avenger_bullet_sound;
            ring->spin_vel = 0.0f;
            ring->radius = 60.0f;
            ring->n_arms = 3;
            ring->arms_occupy_circle_sector_percent = 0.06f;
            ring->n_bullets = 1;
            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = 15;
            ring->bullet_vel = 1900;
            ring->bullet_friction = 0.1f;
            ring->bullet_damage = 10;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_AVENGER_BULLET;
            ring->bullet_sprite_tint = WHITE;
            ring->bullet_sprite_scale = 3.0f;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_SPARKS;

          }

        } break;
      case BULLET_EMITTER_KIND_AVENGER_DOUBLE_TROUBLE:
        {

          emitter->bullet_kind = ENTITY_KIND_PLAYER_BULLET;

          emitter->bullet_collision_mask = AVENGER_BULLET_APPLY_COLLISION_MASK;
          emitter->cooldown_period[0] = 0.22f;
          emitter->shots[0] = 1;
          emitter->active_rings_mask = 0x1;

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->flags |=
              BULLET_EMITTER_RING_FLAG_BURST |
              0;
            ring->burst_shots = 3;
            ring->burst_cooldown = 0.09f;
            ring->burst_shots_fired = 0;
            ring->burst_timer = 0;

            ring->sound = gp->avenger_bullet_sound;
            ring->spin_vel = 0.0f;
            ring->radius = 60.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 0;
            ring->n_bullets = 2;
            ring->bullet_arm_width = 30.0f;
            ring->bullet_radius = 15;
            ring->bullet_vel = 1900;
            ring->bullet_friction = 0.1f;
            ring->bullet_damage = 7;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_AVENGER_BULLET;
            ring->bullet_sprite_tint = WHITE;
            ring->bullet_sprite_scale = 3.0f;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_SPARKS;

          }

        } break;
      case BULLET_EMITTER_KIND_AVENGER:
        {

          emitter->bullet_kind = ENTITY_KIND_PLAYER_BULLET;

          emitter->bullet_collision_mask = AVENGER_BULLET_APPLY_COLLISION_MASK;
          emitter->cooldown_period[0] = 0.04f;
          emitter->active_rings_mask = 0x1;

          emitter->shots[0] = 1;
          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->sound = gp->avenger_bullet_sound;
            ring->spin_vel = 0.0f;
            ring->radius = 60.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 0;
            ring->n_bullets = 1;
            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = 15;
            ring->bullet_vel = 1900;
            ring->bullet_friction = 0.1f;
            ring->bullet_damage = 5;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_AVENGER_BULLET;
            ring->bullet_sprite_tint = WHITE;
            ring->bullet_sprite_scale = 3.0f;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_SPARKS;

          }


        } break;
      case BULLET_EMITTER_KIND_CRAB_BURST_BOOM:
        {

          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->cooldown_period[0] = 0.0f;
          emitter->active_rings_mask = 0x1;

          emitter->shots[0] = 2;

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->flags |=
              BULLET_EMITTER_RING_FLAG_BURST |
              0;

            ring->burst_shots = 2;
            ring->burst_cooldown = 0.13f;
            ring->burst_shots_fired = 0;
            ring->burst_timer = 0;

            ring->spin_vel = 4*PI;
            ring->radius = 30.0f;
            ring->n_arms = 7;
            ring->arms_occupy_circle_sector_percent = 6.0f/7.0f;
            ring->n_bullets = 2;
            ring->bullet_arm_width = 20.0f;
            ring->bullet_radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;
            ring->bullet_vel = CRAB_NORMAL_BULLET_VELOCITY;
            ring->bullet_curve = 0.016f;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_BULLETS;
            ring->bullet_sprite_scale = CRAB_BULLET_SPRITE_SCALE;
            ring->bullet_sprite_tint = CRAB_BULLET_SPRITE_TINT;
            ring->bullet_lifetime = 6.0f;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_HAS_LIFETIME |
              ENTITY_FLAG_DYNAMICS_HAS_CURVE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_PURPLE_SPARKS;
          }

        } break;
      case BULLET_EMITTER_KIND_CRAB_RADIAL_BOOM:
        {

          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;

          emitter->rings[0].flags |=
            0;

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->cooldown_period[0] = 0.0f;
          emitter->active_rings_mask = 0x1;

          emitter->shots[0] = 1;

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->spin_vel = 4*PI;
            ring->radius = 30.0f;
            ring->n_arms = 27;
            ring->arms_occupy_circle_sector_percent = 26.0f/27.0f;
            ring->n_bullets = 1;
            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;
            ring->bullet_vel = CRAB_NORMAL_BULLET_VELOCITY;
            ring->bullet_curve = 0.01f;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_BULLETS;
            ring->bullet_sprite_scale = CRAB_BULLET_SPRITE_SCALE;
            ring->bullet_sprite_tint = CRAB_BULLET_SPRITE_TINT;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_DYNAMICS_HAS_CURVE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_PURPLE_SPARKS;
          }

        } break;
      case BULLET_EMITTER_KIND_CRAB_BASIC:
        {

          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->active_rings_mask = 0x1;

          emitter->rings[0].flags |=
            BULLET_EMITTER_RING_FLAG_LOOK_AT_PLAYER |
            0;

          emitter->cooldown_period[0] = 0.0f;
          emitter->shots[0] = 1;

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->spin_vel = 0.0f;
            ring->radius = 60.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 0;
            ring->n_bullets = 1;
            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;
            ring->bullet_vel = 500;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_BULLETS;
            ring->bullet_sprite_scale = CRAB_BULLET_SPRITE_SCALE;
            ring->bullet_sprite_tint = CRAB_BULLET_SPRITE_TINT;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_PURPLE_SPARKS;
          }

        } break;
      case BULLET_EMITTER_KIND_CRAB_BATTLE_RIFLE:
        {

          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->active_rings_mask = 0x1;

          emitter->rings[0].flags |=
            BULLET_EMITTER_RING_FLAG_LOOK_AT_PLAYER |
            BULLET_EMITTER_RING_FLAG_BURST |
            0;
          emitter->rings[0].burst_shots = 3;
          emitter->rings[0].burst_cooldown = 0.03f;
          emitter->rings[0].burst_shots_fired = 0;
          emitter->rings[0].burst_timer = 0;

          emitter->cooldown_period[0] = 0.0f;
          emitter->shots[0] = 1;

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->spin_vel = 0.0f;
            ring->radius = 60.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 0;
            ring->n_bullets = 1;
            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;
            ring->bullet_vel = 500;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_BULLETS;
            ring->bullet_sprite_scale = CRAB_BULLET_SPRITE_SCALE;
            ring->bullet_sprite_tint = CRAB_BULLET_SPRITE_TINT;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_PURPLE_SPARKS;

          }

        } break;
      case BULLET_EMITTER_KIND_CRAB_V_SHOT:
        {

          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->active_rings_mask = 0x1;

          emitter->rings[0].flags |=
            BULLET_EMITTER_RING_FLAG_LOOK_AT_PLAYER |
            BULLET_EMITTER_RING_FLAG_USE_POINT_BAG |
            0;

          emitter->cooldown_period[0] = 0.0f;
          emitter->shots[0] = 1;

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->spin_vel = 0.0f;
            ring->radius = 60.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 0;

            float radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;

            Vector2 point_bag[] = {
              { -2.0*radius, 0 },
              { -1.2*radius, 2.2*radius },
              {  0, 4.4*radius },
              {  1.2*radius, 2.2*radius },
              {  2.0*radius, 0 },
            };

            ring->n_bullets = ARRLEN(point_bag);
            memory_copy_array(ring->bullet_point_bag, point_bag);

            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = radius;
            //ring->bullet_vel = 500;
            ring->bullet_vel = 800;
            ring->bullet_friction = 0.47f;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_BULLETS;
            ring->bullet_sprite_scale = CRAB_BULLET_SPRITE_SCALE;
            ring->bullet_sprite_tint = CRAB_BULLET_SPRITE_TINT;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_PURPLE_SPARKS;
          }

        } break;
      case BULLET_EMITTER_KIND_CRAB_TENTACLE_SHOT:
        {
          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->active_rings_mask = 0x1;

          emitter->cooldown_period[0] = 0.12f;
          emitter->shots[0] = 40;

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->flags |=
              BULLET_EMITTER_RING_FLAG_SPIN_WITH_SINE |
              BULLET_EMITTER_RING_FLAG_USE_POINT_BAG |
              0;

            float radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;

            Vector2 point_bag[] = {
              { 0, -2*radius },
              { -1.2*radius, -4.2*radius },
              {  1.2*radius, -4.2*radius },
              { 0, -6.2*radius },
            };

            ring->n_bullets = ARRLEN(point_bag);
            memory_copy_array(ring->bullet_point_bag, point_bag);

            ring->radius = 60.0f;
            ring->n_arms = 5;
            ring->arms_occupy_circle_sector_percent = 4.0f/5.0f;

            ring->spin_sine.t = 0;
            ring->spin_sine.amp = PI*0.5;
            ring->spin_sine.freq = 15*PI;
            ring->spin_sine.phase = 0;

            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = radius;
            ring->bullet_vel = 650;
            //ring->bullet_vel = 800;
            ring->bullet_friction = 0.36f;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_BULLETS;
            ring->bullet_sprite_scale = CRAB_BULLET_SPRITE_SCALE;
            ring->bullet_sprite_tint = CRAB_BULLET_SPRITE_TINT;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_PURPLE_SPARKS;
          }

        } break;
      case BULLET_EMITTER_KIND_CRAB_CLOCK_SHOT:
      case BULLET_EMITTER_KIND_CRAB_COUNTER_CLOCK_SHOT:
        {
          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->active_rings_mask = 0x1;

          emitter->cooldown_period[0] = 0.02f;
          emitter->shots[0] = 40;

          bool counter_clockwise =
            (emitter->kind == BULLET_EMITTER_KIND_CRAB_COUNTER_CLOCK_SHOT);

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->dir = Vector2Rotate((Vector2){ .x = 1 }, PI*0.2f);
            ring->spin_vel = 4*PI;
            ring->radius = 60.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 0;

            float radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;

            ring->n_bullets = 1;

            if(counter_clockwise) {
              ring->spin_vel *= -1;
              ring->dir.x *= -1;
            }

            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = radius;
            //ring->bullet_vel = 500;
            ring->bullet_vel = 800;
            ring->bullet_friction = 0.47f;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_BULLETS;
            ring->bullet_sprite_scale = CRAB_BULLET_SPRITE_SCALE;
            ring->bullet_sprite_tint = CRAB_BULLET_SPRITE_TINT;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_PURPLE_SPARKS;
          }

        } break;
      case BULLET_EMITTER_KIND_CRAB_BOMBER_RUN_BOTTOM:
      case BULLET_EMITTER_KIND_CRAB_BOMBER_RUN_TOP:
        {
          bool is_top = (emitter->kind == BULLET_EMITTER_KIND_CRAB_BOMBER_RUN_TOP);

          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->active_rings_mask = 0x1;

          emitter->cooldown_period[0] = 0.12f;
          emitter->shots[0] = 40;

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->flags |=
              BULLET_EMITTER_RING_FLAG_SPIN_WITH_SINE |
              BULLET_EMITTER_RING_FLAG_MANUALLY_SET_DIR |
              0;

            ring->dir = (Vector2){ .y = is_top ? 1 : -1, };
            ring->radius = 60.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 0;

            float radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;

            ring->n_bullets = 1;

            ring->spin_sine.t = 0;
            ring->spin_sine.amp = PI*0.2;
            ring->spin_sine.freq = 9*PI;
            ring->spin_sine.phase = -PI*0.2;

            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = radius;
            //ring->bullet_vel = 500;
            ring->bullet_vel = 800;
            ring->bullet_friction = 0.47f;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_BULLETS;
            ring->bullet_sprite_scale = CRAB_BULLET_SPRITE_SCALE;
            ring->bullet_sprite_tint = CRAB_BULLET_SPRITE_TINT;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_PURPLE_SPARKS;
          }

        } break;
      case BULLET_EMITTER_KIND_BOSS_BOOM:
        {

          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;

          emitter->rings[0].flags |=
            0;

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->cooldown_period[0] = 0.09f;
          emitter->active_rings_mask = 0x1;

          emitter->shots[0] = GetRandomValue(3, 4);

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->spin_vel = 4*PI;
            ring->radius = 30.0f;
            ring->n_arms = 14;
            ring->arms_occupy_circle_sector_percent = 13.0f/14.0f;
            ring->n_bullets = 1;
            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;
            ring->bullet_vel = CRAB_NORMAL_BULLET_VELOCITY;
            ring->bullet_curve = 0.01f;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_BULLETS;
            ring->bullet_sprite_scale = CRAB_BULLET_SPRITE_SCALE;
            ring->bullet_sprite_tint = CRAB_BULLET_SPRITE_TINT;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_DYNAMICS_HAS_CURVE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_PURPLE_SPARKS;
          }

        } break;
      case BULLET_EMITTER_KIND_BOSS_WIDE_SWEEP:
        {

          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->active_rings_mask = 0x1;

          emitter->rings[0].flags |=
            BULLET_EMITTER_RING_FLAG_LOOK_AT_PLAYER |
            0;

          emitter->cooldown_period[0] = 0.9f;
          emitter->shots[0] = GetRandomValue(3, 5);

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->spin_vel = 0.0f;
            ring->radius = 60.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 0;
            ring->n_bullets = 6;
            ring->bullet_arm_width = 300.0f;
            ring->bullet_radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;
            ring->bullet_vel = 500;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_BULLETS;
            ring->bullet_sprite_scale = CRAB_BULLET_SPRITE_SCALE;
            ring->bullet_sprite_tint = CRAB_BULLET_SPRITE_TINT;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_PURPLE_SPARKS;
          }

        } break;
      case BULLET_EMITTER_KIND_BOSS_V_SWEEP:
        {

          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->active_rings_mask = 0x1;

          emitter->rings[0].flags |=
            BULLET_EMITTER_RING_FLAG_SPIN_WITH_SINE |
            BULLET_EMITTER_RING_FLAG_USE_POINT_BAG |
            0;

          emitter->cooldown_period[0] = 0.093f;
          emitter->shots[0] = GetRandomValue(18, 25);

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];


            float radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;

            Vector2 point_bag[] = {
              { -2.0*radius, 0 },
              { -1.2*radius, 2.2*radius },
              {  0, 4.4*radius },
              {  1.2*radius, 2.2*radius },
              {  2.0*radius, 0 },
            };

            ring->n_bullets = ARRLEN(point_bag);
            memory_copy_array(ring->bullet_point_bag, point_bag);

            ring->radius = 60.0f;
            ring->n_arms = 1;

            ring->spin_sine.t = 0;
            ring->spin_sine.amp = PI*0.5;
            ring->spin_sine.freq = 5*PI;
            ring->spin_sine.phase = 0;

            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = radius;
            //ring->bullet_vel = 500;
            ring->bullet_vel = 700;
            ring->bullet_friction = 0.26f;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_BULLETS;
            ring->bullet_sprite_scale = CRAB_BULLET_SPRITE_SCALE;
            ring->bullet_sprite_tint = CRAB_BULLET_SPRITE_TINT;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              ENTITY_FLAG_EMIT_DEATH_PARTICLES |
              ENTITY_FLAG_APPLY_FRICTION |
              0;

            ring->bullet_death_particle_emitter = PARTICLE_EMITTER_PURPLE_SPARKS;
          }

        } break;
      case BULLET_EMITTER_KIND_BOSS_TENTACLE:
        {
#if 0
int boss_min_and_max_shots[][2] = {
  { 3,  4  },
  { 3,  5  },
  { 14, 25 },
  { 30, 40 },
  { 30, 40 },
};
#endif
          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;
        } break;
      case BULLET_EMITTER_KIND_BOSS_CIRCLES:
        {
          emitter->bullet_kind = ENTITY_KIND_ENEMY_BULLET;
        } break;
    }

    emitter->cocked = 1;

  }

  ASSERT(emitter->bullet_kind != ENTITY_KIND_INVALID);

  { /* flags checks */
  } /* flags checks */

  {
    b8 all_rings_finished = 1;

    for(int i = 0; i < MAX_BULLET_EMITTER_RINGS; i++) {
      if((emitter->active_rings_mask & (1u<<(u32)i)) && emitter->shots[i] > 0) {
        all_rings_finished = 0;
      }
    }

    if(all_rings_finished) {
      emitter->shoot--;
    }

    if(emitter->shoot <= 0) {
      emitter->shoot = 0;
      emitter->cocked = 0;
    }
  }

  if(emitter->shoot) {

    for(int ring_i = 0; ring_i < MAX_BULLET_EMITTER_RINGS; ring_i++) {

      if(!(emitter->active_rings_mask & (1u<<(u32)ring_i))) {
        continue;
      }

      if(emitter->shots[ring_i] <= 0) {
        continue;
      }

      Bullet_emitter_ring *ring = &emitter->rings[ring_i];

      if(ring->flags & BULLET_EMITTER_RING_FLAG_BURST) {

        if(ring->burst_shots_fired >= ring->burst_shots) {

          if(emitter->cooldown_timer[ring_i] <= 0.0f) {
            emitter->cooldown_timer[ring_i] = emitter->cooldown_period[ring_i];

            emitter->shots[ring_i]--;
            ring->burst_timer = 0.0f;
            ring->burst_shots_fired = 0;

          } else {
            emitter->cooldown_timer[ring_i] -= gp->timestep;
          }

          continue;

        } else {

          if(ring->burst_timer <= 0.0f) {
            emitter->cooldown_timer[ring_i] = emitter->cooldown_period[ring_i];
            ring->burst_timer = ring->burst_cooldown;
            ring->burst_shots_fired++;
          } else {
            ring->burst_timer -= gp->timestep;
            continue;
          }

        }

      } else {

        if(emitter->cooldown_timer[ring_i] <= 0.0f) {
          emitter->cooldown_timer[ring_i] = emitter->cooldown_period[ring_i];
          emitter->shots[ring_i]--;
        } else {
          emitter->cooldown_timer[ring_i] -= gp->timestep;
          continue;
        }

      }

      ASSERT(ring->n_arms > 0);
      ASSERT(ring->n_bullets > 0);

      Vector2 arm_dir;
      float arms_occupy_circle_sector_angle;
      float arm_step_angle;
      Vector2 look_at_player_dir = {0};

      if(ring->flags & BULLET_EMITTER_RING_FLAG_LOOK_AT_PLAYER) {

        Entity *player = entity_from_handle(gp->player_handle);
        ASSERT(player);

        look_at_player_dir = Vector2Normalize(Vector2Subtract(player->pos, ep->pos));
        ring->dir = look_at_player_dir;

      } else if(!(ring->flags & BULLET_EMITTER_RING_FLAG_MANUALLY_SET_DIR)) {
        ring->dir = ep->look_dir;
      }

      float arm_angle;
      Matrix arm_transform;

      if(ring->n_arms == 1) {
        arms_occupy_circle_sector_angle = 0;
        arm_step_angle = 0;
        arm_angle = ring->spin_cur_angle + ring->spin_start_angle;
        arm_transform = MatrixRotateZ(arm_angle);
        arm_dir = Vector2Transform(ring->dir, arm_transform);
      } else {
        ASSERT(ring->arms_occupy_circle_sector_percent > 0);

        arms_occupy_circle_sector_angle =
          (2*PI) * ring->arms_occupy_circle_sector_percent;
        arm_step_angle = arms_occupy_circle_sector_angle / (float)(ring->n_arms-1);
        arm_angle = -0.5*arms_occupy_circle_sector_angle + ring->spin_cur_angle + ring->spin_start_angle;
        arm_transform = MatrixRotateZ(arm_angle);
        arm_dir = Vector2Transform(ring->dir, arm_transform);
      }

      for(int arm_i = 0; arm_i < ring->n_arms; arm_i++) {

        Vector2 step_dir = {0};
        Vector2 bullet_pos = Vector2Add(ep->pos, Vector2Scale(arm_dir, ring->radius));
        Vector2 positions[MAX_BULLETS_IN_BAG];

        if(ring->flags & BULLET_EMITTER_RING_FLAG_USE_POINT_BAG) {

          Vector2 dir = ring->dir;
          Vector2 dir_perp =
          {
            .x = dir.y, .y = -dir.x,
          };

          Matrix dir_transform =
          {
            .m0 = dir_perp.x, .m4 = dir.x,
            .m1 = dir_perp.y, .m5 = dir.y,
          };

          arm_transform = MatrixMultiply(arm_transform, dir_transform);

          ASSERT(ring->n_bullets < MAX_BULLETS_IN_BAG);

          for(int i = 0; i < ring->n_bullets; i++) {
            positions[i] = Vector2Add(bullet_pos, Vector2Transform(ring->bullet_point_bag[i], arm_transform));
          }

        } else {
          if(ring->n_bullets > 1) {
            ASSERT(ring->bullet_arm_width > 0);

            Vector2 arm_dir_perp = { arm_dir.y, -arm_dir.x };

            step_dir =
              Vector2Scale(arm_dir_perp, ring->bullet_arm_width/(float)(ring->n_bullets-1));

            bullet_pos =
              Vector2Add(bullet_pos, Vector2Scale(arm_dir_perp, -0.5*ring->bullet_arm_width));
          }
        }

        for(int bullet_i = 0; bullet_i < ring->n_bullets; bullet_i++) {
          Entity *bullet = entity_spawn(gp);

          bullet->kind = emitter->bullet_kind;
          bullet->update_order = ENTITY_ORDER_FIRST;
          bullet->draw_order = ENTITY_ORDER_LAST;

          bullet->flags = DEFAULT_BULLET_FLAGS | ring->bullet_flags;

          bullet->bounds_color = ring->bullet_bounds_color;
          bullet->fill_color = ring->bullet_fill_color;

          bullet->sprite = ring->bullet_sprite;
          bullet->sprite_rotation = ring->bullet_sprite_rotation;
          bullet->sprite_scale = ring->bullet_sprite_scale;
          bullet->sprite_tint = ring->bullet_sprite_tint;

          bullet->scalar_vel = ring->bullet_vel;
          bullet->vel = Vector2Scale(arm_dir, ring->bullet_vel);

          if(IsSoundValid(ring->sound)) {
            SetSoundPan(ring->sound, Normalize(ep->pos.x, WINDOW_WIDTH, 0));
            SetSoundVolume(ring->sound, 0.2);
            SetSoundPitch(ring->sound, get_random_float(0.98, 1.01, 4));
            PlaySound(ring->sound);
          }

          bullet->friction = ring->bullet_friction;
          //bullet->vel = Vector2Add(bullet->vel, Vector2Scale(ring->dir, Vector2DotProduct(ring->dir, ep->vel)));

          bullet->curve = ring->bullet_curve;
          bullet->curve_rolloff_vel = ring->bullet_curve_rolloff_vel;

          bullet->radius = ring->bullet_radius;

          bullet->apply_collision_mask = ep->bullet_emitter.bullet_collision_mask;
          bullet->damage_amount = ring->bullet_damage;

          bullet->spawn_particle_emitter = ring->bullet_spawn_particle_emitter;
          bullet->death_particle_emitter = ring->bullet_death_particle_emitter;

          if(ring->bullet_flags & ENTITY_FLAG_HAS_LIFETIME) {
            bullet->life_time_period = ring->bullet_lifetime;
          }

          if(ring->flags & BULLET_EMITTER_RING_FLAG_USE_POINT_BAG) {
            bullet->pos = positions[bullet_i];
          } else {
            bullet->pos = bullet_pos;
            bullet_pos = Vector2Add(bullet_pos, step_dir);
          }

        } /* bullet loop */

        arm_dir = Vector2Rotate(arm_dir, arm_step_angle);

      } /* arm loop */

      if(ring->flags & BULLET_EMITTER_RING_FLAG_SPIN_WITH_SINE) {
        ring->spin_cur_angle =
          ring->spin_sine.amp * sinf(ring->spin_sine.freq * ring->spin_sine.t + ring->spin_sine.phase);

        ring->spin_sine.t += gp->timestep;
        // TODO sine period
        //if(ring->spin_sine.t >= (1/ring->spin_sine.freq)) {
        //  ring->spin_sine.t = 0;
        //}

      } else {
        ring->spin_cur_angle += ring->spin_vel * gp->timestep;
        if(ring->spin_cur_angle >= 2*PI) {
          ring->spin_cur_angle = 0;
        }
      }

    } /* ring loop */

  }

}

force_inline b32 entity_check_collision(Game *gp, Entity *a, Entity *b) {
  b32 result = 0;

  float sqr_min_dist = SQUARE(a->radius + b->radius);

  if(Vector2DistanceSqr(a->pos, b->pos) < sqr_min_dist) {
    result = 1;
  }

  return result;
}

void sprite_update(Game *gp, Entity *ep) {
  Sprite *sp = &ep->sprite;
  if(!(sp->flags & SPRITE_FLAG_STILL)) {

    ASSERT(sp->fps > 0);

    if(sp->cur_frame < sp->total_frames) {
      sp->frame_counter++;

      if(sp->frame_counter >= (TARGET_FPS / sp->fps)) {
        sp->frame_counter = 0;
        sp->cur_frame++;
      }

    }

    if(sp->cur_frame >= sp->total_frames) {
      if(sp->flags & SPRITE_FLAG_INFINITE_REPEAT) {
        if(sp->flags & SPRITE_FLAG_PINGPONG) {
          sp->cur_frame--;
          sp->flags ^= SPRITE_FLAG_REVERSE;
        } else {
          sp->cur_frame = 0;
        }
      } else {
        sp->cur_frame--;
        sp->flags |= SPRITE_FLAG_AT_LAST_FRAME | SPRITE_FLAG_STILL;
        ASSERT(sp->cur_frame >= 0 && sp->cur_frame < sp->total_frames);
      }
    }

  }

}

Sprite_frame sprite_current_frame(Sprite sp) {
  s32 abs_cur_frame = sp.first_frame + sp.cur_frame;

  if(sp.flags & SPRITE_FLAG_REVERSE) {
    abs_cur_frame = sp.last_frame - sp.cur_frame;
  }

  Sprite_frame frame = __sprite_frames[abs_cur_frame];

  return frame;
}

b32 sprite_at_keyframe(Sprite sp, s32 keyframe) {
  b32 result = 0;

  s32 abs_cur_frame = sp.first_frame + sp.cur_frame;

  if(sp.flags & SPRITE_FLAG_REVERSE) {
    abs_cur_frame = sp.last_frame - sp.cur_frame;
  }

  if(abs_cur_frame == keyframe) {
    result = 1;
  }

  return result;
}

void draw_sprite(Game *gp, Entity *ep) {
  Sprite sp = ep->sprite;
  Vector2 pos = ep->pos;
  f32 scale = ep->sprite_scale;
  f32 rotation = ep->sprite_rotation;
  Color tint = ep->sprite_tint;

  if(ep->flags & ENTITY_FLAG_APPLY_EFFECT_TINT) {
    tint = ColorLerp(ep->sprite_tint, ep->effect_tint, Normalize(ep->effect_tint_timer, 0, ep->effect_tint_period));
  }

  ASSERT(sp.cur_frame >= 0 && sp.cur_frame < sp.total_frames);

  Sprite_frame frame;

  if(sp.flags & SPRITE_FLAG_REVERSE) {
    frame = __sprite_frames[sp.last_frame - sp.cur_frame];
  } else {
    frame = __sprite_frames[sp.first_frame + sp.cur_frame];
  }

  Rectangle source_rec =
  {
    .x = (float)frame.x,
    .y = (float)frame.y,
    .width = (float)frame.w,
    .height = (float)frame.h,
  };

  Rectangle dest_rec =
  {
    .x = (pos.x - scale*0.5f*source_rec.width),
    .y = (pos.y - scale*0.5f*source_rec.height),
    .width = scale*source_rec.width,
    .height = scale*source_rec.height,
  };

  if(sp.flags & SPRITE_FLAG_DRAW_MIRRORED_X) {
    source_rec.width *= -1;
  }

  if(sp.flags & SPRITE_FLAG_DRAW_MIRRORED_Y) {
    source_rec.height *= -1;
  }

  DrawTexturePro(gp->sprite_atlas, source_rec, dest_rec, (Vector2){0}, rotation, tint);
}

void game_init(Game *gp) {
  memory_set(gp, 0, sizeof(Game));

  gp->entities = os_alloc(sizeof(Entity) * MAX_ENTITIES);
  gp->particles = os_alloc(sizeof(Particle) * MAX_PARTICLES);

  gp->scratch = arena_alloc();
  gp->wave_scratch = arena_alloc(.size = KB(16));
  gp->frame_scratch = arena_alloc(.size = KB(8));

  game_load_assets(gp);

  PlayMusicStream(gp->music);

  game_reset(gp);
}

void game_load_assets(Game *gp) {
  gp->font = GetFontDefault();

  gp->render_texture = LoadRenderTexture(WINDOW_WIDTH, WINDOW_HEIGHT);
  gp->background_texture = LoadTexture("./sprites/the_sea.png");
  gp->sprite_atlas = LoadTexture("./aseprite/atlas.png");
  SetTextureFilter(gp->sprite_atlas, TEXTURE_FILTER_POINT);

  gp->avenger_bullet_sound = LoadSound("./sounds/avenger_bullet.wav");
  gp->crab_hurt_sound      = LoadSound("./sounds/crab_hurt_sound.wav");
  gp->health_pickup_sound  = LoadSound("./sounds/health_pickup.wav");
  gp->avenger_hurt_sound   = LoadSound("./sounds/player_damage.wav");
  gp->bomb_sound           = LoadSound("./sounds/bomb_sound.wav");
  gp->powerup_sound        = LoadSound("./sounds/powerup.wav");
  gp->boss_die             = LoadSound("./sounds/boss_die.wav");

  gp->music = LoadMusicStream("./sounds/synthwave.ogg");

}

void game_unload_assets(Game *gp) {

  UnloadRenderTexture(gp->render_texture);
  UnloadTexture(gp->sprite_atlas);
  UnloadTexture(gp->background_texture);

  StopMusicStream(gp->music);
  UnloadMusicStream(gp->music);

  UnloadSound(gp->avenger_bullet_sound);
  UnloadSound(gp->crab_hurt_sound);
  UnloadSound(gp->health_pickup_sound);
  UnloadSound(gp->avenger_hurt_sound);
  UnloadSound(gp->bomb_sound);
  UnloadSound(gp->powerup_sound);

}

void game_reset(Game *gp) {

  gp->state = GAME_STATE_NONE;
  gp->next_state = GAME_STATE_NONE;

  gp->flags = 0;

#ifdef DEBUG
  gp->debug_flags = GAME_DEBUG_FLAG_HOT_RELOAD;
#endif

  gp->player = 0;
  gp->player_handle = (Entity_handle){0};
  gp->entity_free_list = 0;
  gp->entities_allocated = 0;
  gp->live_entities = 0;

  gp->particles_pos = 0;
  gp->live_particles = 0;

  memory_set(gp->entities, 0, sizeof(Entity) * MAX_ENTITIES);
  memory_set(gp->particles, 0, sizeof(Particle) * MAX_PARTICLES);

  gp->frame_index = 0;

  arena_clear(gp->scratch);
  arena_clear(gp->frame_scratch);
  arena_clear(gp->wave_scratch);

  gp->background_y_offset = 0;

  memory_set(&gp->phase, 0, sizeof(gp->phase));
  gp->phase_index = 0;
  gp->wave = 0;

  gp->score = 0;

  SeekMusicStream(gp->music, 0);

  gp->gameover_pre_delay = 0;
  gp->gameover_type_char_timer = 0;
  gp->gameover_chars_typed = 0;

}

void game_wave_end(Game *gp) {
  arena_clear(gp->wave_scratch);
  memory_set(&gp->phase, 0, sizeof(gp->phase));
  gp->phase_index = 0;
  gp->wave++;
  gp->next_state = GAME_STATE_WAVE_TRANSITION;
  gp->flags |= GAME_FLAG_PLAYER_CANNOT_SHOOT;
  gp->wave_timer = WAVE_DELAY_TIME;
  gp->wave_type_char_timer = TYPING_SPEED;
  gp->wave_chars_typed = 0;
  gp->wave_banner_msg = push_str8f(gp->wave_scratch, "WAVE %i", gp->wave+1);
  gp->wave_banner_type_dir = 1;
  gp->wave_banner_target_msg_len = gp->wave_banner_msg.len;
}

void game_main_loop(Game *gp) {

  switch(gp->wave) {
    case 0:
      switch(gp->phase_index) {
        case 0:
          {

            if(gp->phase.check_if_finished == 0) {
              gp->phase.check_if_finished = 1;

              const int crabs_per_row = 2;
              const float strafe_speed = 300;

              {
                Entity *leader = spawn_leader(gp);
                Entity_handle leader_handle = handle_from_entity(leader);
                leader->child_list = push_entity_list(gp);
                leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                leader->vel = (Vector2){ .x = strafe_speed };
                leader->leader_strafe_padding = 50;
                leader->pos =
                  (Vector2){
                    .x = -400 + (0.5*(float)(crabs_per_row-1))*3.0*CRAB_BOUNDS_RADIUS,
                    .y = WINDOW_HEIGHT * 0.13f,
                  };

                for(int i = 0; i < crabs_per_row; i++) {
                  Vector2 pos = 
                  {
                    -400 + i*3.0*CRAB_BOUNDS_RADIUS,
                    WINDOW_HEIGHT * 0.13f,
                  };

                  Entity *crab = spawn_crab(gp);

                  crab->shoot_control = ENTITY_SHOOT_CONTROL_PERIODIC_1;
                  crab->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_BASIC;

                  crab->pos = pos;
                  crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                  crab->leader_handle = leader_handle;
                  entity_list_append(gp, leader->child_list, crab);

                }

              }

              {
                Entity *leader = spawn_leader(gp);
                Entity_handle leader_handle = handle_from_entity(leader);
                leader->child_list = push_entity_list(gp);
                leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                leader->vel = (Vector2){ .x = -strafe_speed };
                leader->leader_strafe_padding = 50;
                leader->pos =
                  (Vector2){
                    .x = WINDOW_WIDTH+(400-2.5*CRAB_BOUNDS_RADIUS) + (0.5*(float)(crabs_per_row-1))*3.0*CRAB_BOUNDS_RADIUS,
                    .y = WINDOW_HEIGHT * 0.26f,
                  };

                for(int i = 0; i < crabs_per_row; i++) {
                  Vector2 pos = 
                  {
                    WINDOW_WIDTH+(400-2.5*CRAB_BOUNDS_RADIUS) + i*3.0*CRAB_BOUNDS_RADIUS,
                    WINDOW_HEIGHT * 0.26f,
                  };

                  Entity *crab = spawn_crab(gp);

                  crab->shoot_control = ENTITY_SHOOT_CONTROL_PERIODIC_1;
                  crab->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_BASIC;

                  crab->pos = pos;
                  crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                  crab->leader_handle = leader_handle;
                  entity_list_append(gp, leader->child_list, crab);

                }

              }

            } else {
              if(gp->live_enemies == 0) {
                goto phase_end;
              }
            }

          } break;
        case 1:
          {

            if(gp->phase.check_if_finished == 0) {
              gp->phase.check_if_finished = 1;

              const int crabs_per_row = 2;
              const float strafe_speed = 300;

              {
                Entity *leader = spawn_leader(gp);
                Entity_handle leader_handle = handle_from_entity(leader);
                leader->child_list = push_entity_list(gp);
                leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                leader->vel = (Vector2){ .x = strafe_speed };
                leader->leader_strafe_padding = 50;
                leader->pos =
                  (Vector2){
                    .x = -400 + (0.5*(float)(crabs_per_row-1))*3.0*CRAB_BOUNDS_RADIUS,
                    .y = WINDOW_HEIGHT * 0.26f,
                  };

                for(int i = 0; i < crabs_per_row; i++) {
                  Vector2 pos = 
                  {
                    -400 + i*3.0*CRAB_BOUNDS_RADIUS,
                    WINDOW_HEIGHT * 0.26f,
                  };

                  Entity *crab = spawn_crab(gp);

                  crab->shoot_control = ENTITY_SHOOT_CONTROL_PERIODIC_1;
                  crab->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_BASIC;

                  crab->pos = pos;
                  crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                  crab->leader_handle = leader_handle;
                  entity_list_append(gp, leader->child_list, crab);

                }

              }

              {
                Entity *leader = spawn_leader(gp);
                Entity_handle leader_handle = handle_from_entity(leader);
                leader->child_list = push_entity_list(gp);
                leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                leader->vel = (Vector2){ .x = -strafe_speed };
                leader->leader_strafe_padding = 50;
                leader->pos =
                  (Vector2){
                    .x = WINDOW_WIDTH+(400-2.5*CRAB_BOUNDS_RADIUS) + (0.5*(float)(crabs_per_row-1))*3.0*CRAB_BOUNDS_RADIUS,
                    .y = WINDOW_HEIGHT * 0.13f,
                  };

                for(int i = 0; i < crabs_per_row; i++) {
                  Vector2 pos = 
                  {
                    WINDOW_WIDTH+(400-2.5*CRAB_BOUNDS_RADIUS) + i*3.0*CRAB_BOUNDS_RADIUS,
                    WINDOW_HEIGHT * 0.13f,
                  };

                  Entity *crab = spawn_crab(gp);

                  crab->shoot_control = ENTITY_SHOOT_CONTROL_PERIODIC_1;
                  crab->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_BASIC;

                  crab->pos = pos;
                  crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                  crab->leader_handle = leader_handle;
                  entity_list_append(gp, leader->child_list, crab);

                }

              }

            } else {
              if(gp->live_enemies == 0) {

                goto phase_end;

              }
            }

          } break;
        case 2:
          {

            const u32 SPAWNED_FIRST_GROUP  = 1u<<0;
            const u32 SPAWNED_SECOND_GROUP = 1u<<1;
            const u32 SPAWNED_THIRD_GROUP  = 1u<<2;

            const float FIRST_GROUP_DELAY  = 1.0f;
            const float SECOND_GROUP_DELAY = 2.4f;
            const float THIRD_GROUP_DELAY  = 3.15f;
            const float leader_fall_speed = 1600;

            f32 *first_enemy_group_spawn_delay = &gp->phase.timer[0];
            f32 *second_enemy_group_spawn_delay = &gp->phase.timer[1];
            f32 *third_enemy_group_spawn_delay = &gp->phase.timer[2];

            const s32 crab_health = 25;
            const float crab_radius = CRAB_BOUNDS_RADIUS;
            const float padding = 20;
            const Vector2 square_of_4_crabs_points[] = {
              { -crab_radius - padding, -crab_radius - padding },
              {  crab_radius + padding, -crab_radius - padding },
              { -crab_radius - padding,  crab_radius + padding },
              {  crab_radius + padding,  crab_radius + padding },
            };

            if(gp->phase.check_if_finished) {

              if(gp->live_enemies == 0) {

                goto phase_end;

              }

            } else {

              if(!(gp->phase.flags & SPAWNED_FIRST_GROUP)) {
                if(*first_enemy_group_spawn_delay >= FIRST_GROUP_DELAY) {

                  gp->phase.flags |= SPAWNED_FIRST_GROUP;

                  Entity *leader = spawn_leader(gp);
                  Entity_handle leader_handle = handle_from_entity(leader);

                  leader->look_dir = (Vector2){ 0, 1 };

                  leader->flags |=
                    ENTITY_FLAG_HAS_BULLET_EMITTER |
                    ENTITY_FLAG_APPLY_FRICTION |
                    0;
                  leader->friction = 1.5f;
                  leader->child_list = push_entity_list(gp);
                  leader->move_control = ENTITY_MOVE_CONTROL_NONE;
                  leader->shoot_control = ENTITY_SHOOT_CONTROL_ONCE;
                  leader->start_shooting_delay_period = 1.6f;
                  leader->number_of_shots = 1;
                  leader->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_RADIAL_BOOM;
                  leader->vel = (Vector2){ .y = leader_fall_speed };
                  leader->pos =
                    (Vector2){
                      .x = WINDOW_WIDTH * 0.3f,
                      .y = WINDOW_HEIGHT * -0.7f,
                    };

                  for(int i = 0; i < 4; i++) {
                    Vector2 pos = Vector2Add(leader->pos, square_of_4_crabs_points[i]);

                    Entity *crab = spawn_crab(gp);

                    crab->pos = pos;
                    crab->health = crab_health;
                    crab->shoot_control = ENTITY_SHOOT_CONTROL_NONE;
                    crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                    crab->leader_handle = leader_handle;
                    entity_list_append(gp, leader->child_list, crab);

                  }

                } else {
                  *first_enemy_group_spawn_delay += gp->timestep;
                }

              } else if(!(gp->phase.flags & SPAWNED_SECOND_GROUP)) {
                if(*second_enemy_group_spawn_delay >= SECOND_GROUP_DELAY) {

                  gp->phase.flags |= SPAWNED_SECOND_GROUP;

                  Entity *leader = spawn_leader(gp);
                  Entity_handle leader_handle = handle_from_entity(leader);

                  leader->look_dir = (Vector2){ 0, 1 };

                  leader->flags |=
                    ENTITY_FLAG_HAS_BULLET_EMITTER |
                    ENTITY_FLAG_APPLY_FRICTION |
                    0;
                  leader->friction = 1.5f;
                  leader->child_list = push_entity_list(gp);
                  leader->move_control = ENTITY_MOVE_CONTROL_NONE;
                  leader->shoot_control = ENTITY_SHOOT_CONTROL_ONCE;
                  leader->start_shooting_delay_period = 1.6f;
                  leader->number_of_shots = 1;
                  leader->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_RADIAL_BOOM;
                  leader->vel = (Vector2){ .y = leader_fall_speed };
                  leader->pos =
                    (Vector2){
                      .x = WINDOW_WIDTH * 0.76f,
                      .y = WINDOW_HEIGHT * -0.7f,
                    };

                  for(int i = 0; i < 4; i++) {
                    Vector2 pos = Vector2Add(leader->pos, square_of_4_crabs_points[i]);

                    Entity *crab = spawn_crab(gp);

                    crab->pos = pos;
                    crab->health = crab_health;
                    crab->shoot_control = ENTITY_SHOOT_CONTROL_NONE;
                    crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                    crab->leader_handle = leader_handle;
                    entity_list_append(gp, leader->child_list, crab);

                  }

                } else {
                  *second_enemy_group_spawn_delay += gp->timestep;
                }
              } else if(!(gp->phase.flags & SPAWNED_THIRD_GROUP)) {
                if(*third_enemy_group_spawn_delay >= THIRD_GROUP_DELAY) {

                  gp->phase.flags |= SPAWNED_THIRD_GROUP;

                  Entity *leader = spawn_leader(gp);
                  Entity_handle leader_handle = handle_from_entity(leader);

                  leader->look_dir = (Vector2){ 0, 1 };

                  leader->flags |=
                    ENTITY_FLAG_HAS_BULLET_EMITTER |
                    ENTITY_FLAG_APPLY_FRICTION |
                    0;
                  leader->friction = 1.5f;
                  leader->child_list = push_entity_list(gp);
                  leader->move_control = ENTITY_MOVE_CONTROL_NONE;
                  leader->shoot_control = ENTITY_SHOOT_CONTROL_ONCE;
                  leader->start_shooting_delay_period = 1.6f;
                  leader->number_of_shots = 1;
                  leader->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_RADIAL_BOOM;
                  leader->vel = (Vector2){ .y = leader_fall_speed };
                  leader->pos =
                    (Vector2){
                      .x = WINDOW_WIDTH * 0.54f,
                      .y = WINDOW_HEIGHT * -0.8f,
                    };

                  for(int i = 0; i < 4; i++) {
                    Vector2 pos = Vector2Add(leader->pos, square_of_4_crabs_points[i]);

                    Entity *crab = spawn_crab(gp);

                    crab->pos = pos;
                    crab->health = crab_health;
                    crab->shoot_control = ENTITY_SHOOT_CONTROL_NONE;
                    crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                    crab->leader_handle = leader_handle;
                    entity_list_append(gp, leader->child_list, crab);

                  }

                } else {
                  *third_enemy_group_spawn_delay += gp->timestep;
                }
              } else {
                gp->phase.check_if_finished = 1;
              }

            }
          } break;
        default:
          goto wave_end;
      }
      break;

    case 1:
      switch(gp->phase_index) {
        case 0:
          {

            const s32 N_CRABS = 5; 
            const f32 SPAWN_CLOCK_CRAB_DELAY = 1.3f;
            s32 *n_crabs = &gp->phase.accumulator[0];
            f32 *spawn_clock_crab_timer = &gp->phase.timer[0];

            if(*n_crabs >= N_CRABS) {
              if(gp->live_enemies == 0) {
                goto phase_end;
              }
            } else {

              if(*spawn_clock_crab_timer >= SPAWN_CLOCK_CRAB_DELAY) {
                if(gp->live_enemies == 0) {
                  *n_crabs += 1;

                  *spawn_clock_crab_timer = 0;

                  Entity *crab = spawn_crab(gp);

                  crab->death_particle_emitter = PARTICLE_EMITTER_WHITE_PUFF;
                  crab->flags |=
                    //ENTITY_FLAG_HAS_LIFETIME |
                    0;
                  //crab->life_time_period = 2.7f;

                  crab->accel = (Vector2){ .y = 1800 };
                  crab->vel = (Vector2){ .y = -1000 };
                  crab->pos =
                    (Vector2){
                      .x = get_random_float(150, WINDOW_WIDTH-150, 20),
                      .y = get_random_float(WINDOW_HEIGHT*0.3f, WINDOW_HEIGHT*0.5f, 5),
                    }; 

                  crab->sprite_scale = 4.0f;
                  crab->sprite_tint = ColorAlpha(WHITE, 0.7);
                  crab->health = 15;
                  crab->shoot_control = ENTITY_SHOOT_CONTROL_ONCE;
                  crab->bullet_emitter.kind =
                    (*n_crabs & 0x1) ? BULLET_EMITTER_KIND_CRAB_CLOCK_SHOT : BULLET_EMITTER_KIND_CRAB_COUNTER_CLOCK_SHOT;
                  crab->start_shooting_delay_period = 0.3f;
                }

              } else {
                *spawn_clock_crab_timer += gp->timestep;
              }

            }

          } break;
        case 1:
          {

            const s32 N_CRABS = 5; 
            const f32 SPAWN_CLOCK_CRAB_DELAY = 1.5f;
            s32 *n_crabs = &gp->phase.accumulator[0];
            f32 *spawn_clock_crab_timer = &gp->phase.timer[0];

            if(*n_crabs >= N_CRABS) {
              if(gp->live_enemies == 0) {
                goto phase_end;
              }
            } else {

              if(*spawn_clock_crab_timer >= SPAWN_CLOCK_CRAB_DELAY) {
                if(gp->live_enemies == 0) {

                  *n_crabs += 1;

                  *spawn_clock_crab_timer = 0;

                  Entity *crab = spawn_crab(gp);

                  crab->flags |=
                    ENTITY_FLAG_HAS_LIFETIME |
                    ENTITY_FLAG_APPLY_FRICTION |
                    0;
                  crab->life_time_period = 1.8f;

                  crab->death_particle_emitter = PARTICLE_EMITTER_WHITE_PUFF;

                  float sign = (*n_crabs & 0x1) ? -1 : 1;

                  crab->vel = (Vector2){ .x = sign * 600.0f };

                  if(GetRandomValue(0, 1)) {
                    crab->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_BOMBER_RUN_TOP;
                    if(sign < 0) {
                      crab->pos =
                        (Vector2){
                          .x = get_random_float(WINDOW_WIDTH*0.8f, WINDOW_WIDTH*0.9f, 2),
                          .y = get_random_float(WINDOW_HEIGHT*0.08, WINDOW_HEIGHT*0.18, 4),
                        }; 
                    } else {
                      crab->pos =
                        (Vector2){
                          .x = get_random_float(WINDOW_WIDTH*0.2f, WINDOW_WIDTH*0.3f, 2),
                          .y = get_random_float(WINDOW_HEIGHT*0.08, WINDOW_HEIGHT*0.18, 4),
                        }; 
                    }
                  } else {
                    crab->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_BOMBER_RUN_BOTTOM;
                    if(sign < 0) {
                      crab->pos =
                        (Vector2){
                          .x = get_random_float(WINDOW_WIDTH*0.8f, WINDOW_WIDTH*0.9f, 2),
                          .y = get_random_float(WINDOW_HEIGHT*0.85, WINDOW_HEIGHT*0.93, 4),
                        }; 
                    } else {
                      crab->pos =
                        (Vector2){
                          .x = get_random_float(WINDOW_WIDTH*0.2f, WINDOW_WIDTH*0.3f, 2),
                          .y = get_random_float(WINDOW_HEIGHT*0.85, WINDOW_HEIGHT*0.93, 4),
                        }; 
                    }
                  }

                  crab->sprite_scale = 4.0f;
                  crab->sprite_tint = ColorAlpha(WHITE, 0.7);
                  crab->health = 15;
                  crab->friction = 0.5f;
                  crab->shoot_control = ENTITY_SHOOT_CONTROL_ONCE;
                  crab->start_shooting_delay_period = 0.1f;

                }
              } else {
                *spawn_clock_crab_timer += gp->timestep;
              }

            }

          } break;
        case 2:
          { /* last ghost */
            const s32 N_CRABS = 6; 
            const f32 SPAWN_CLOCK_CRAB_DELAY = 2.6f;
            const u32 DROPPED_BOMB = 1<<0;
            s32 *n_crabs = &gp->phase.accumulator[0];
            f32 *spawn_clock_crab_timer = &gp->phase.timer[0];
            u32 *flags = &gp->phase.flags;

            if(!(*flags & DROPPED_BOMB)) {
              if(GetRandomValue(0, 10000000) == 27) {
                *flags |= DROPPED_BOMB;
                spawn_bomb_pack(gp);
              }
            }

            if(*spawn_clock_crab_timer < SPAWN_CLOCK_CRAB_DELAY) {
              *spawn_clock_crab_timer += gp->timestep;
            } else {
              if(*n_crabs >= N_CRABS) {
                if(gp->live_enemies == 0) {
                  goto phase_end;
                }
              } else {

                if(gp->live_enemies == 0) {
                  *n_crabs += 1;

                  *spawn_clock_crab_timer = 0;

                  Entity *crab = spawn_crab(gp);

                  crab->pos =
                    (Vector2){
                      Lerp(100, WINDOW_WIDTH-100, get_random_float(0, 1, 25)),
                      Lerp(100, WINDOW_HEIGHT-100, get_random_float(0, 1, 25)),
                    };

                  crab->flags |=
                    ENTITY_FLAG_HAS_LIFETIME |
                    0;
                  crab->life_time_period = 2.6f;

                  crab->death_particle_emitter = PARTICLE_EMITTER_WHITE_PUFF;

                  crab->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_BURST_BOOM;

                  crab->sprite_scale = 4.0f;
                  crab->sprite_tint = ColorAlpha(WHITE, 0.7);

                  crab->health = 30;
                  crab->shoot_control = ENTITY_SHOOT_CONTROL_ONCE;
                  crab->start_shooting_delay_period = 0.032f;
                }

              }
            }

          } break;
        default:
          goto wave_end;
      }
      break;

    case 2:
      switch(gp->phase_index) {
        case 0:
          {

            if(gp->phase.check_if_finished == 0) {
              gp->phase.check_if_finished = 1;
              gp->phase.timer[0] = 1.0f;

              { /* spawn_orbiting_crabs */
                Entity *leader = spawn_leader(gp);
                Entity_handle leader_handle = handle_from_entity(leader);
                leader->child_list = push_entity_list(gp);
                leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                leader->vel = (Vector2){ .x = -850 };
                //leader->friction = 1.8f;
                leader->leader_strafe_padding = 100;
                leader->flags |=
                  //ENTITY_FLAG_APPLY_FRICTION |
                  0;
                leader->pos =
                  (Vector2){
                    .x = WINDOW_WIDTH*0.5,
                    .y = WINDOW_HEIGHT *0.19,
                  };

                int crab_count = 6;
                float radius = 120;
                float step_angle = (2*PI) / (float)crab_count;
                Vector2 arm = ORBIT_ARM;

                for(int i = 0; i < 6; i++) {
                  Vector2 pos = Vector2Add(leader->pos, Vector2Scale(Vector2Rotate(arm, (float)i*step_angle), radius));

                  Entity *crab = spawn_crab(gp);

                  crab->shoot_control = ENTITY_SHOOT_CONTROL_NONE;

                  crab->pos = pos;
                  crab->move_control = ENTITY_MOVE_CONTROL_ORBIT_LEADER;

                  crab->orbit_cur_angle = (float)i*step_angle;
                  crab->orbit_speed = PI*0.15;
                  crab->orbit_radius = radius;
                  crab->sprite_tint = MAROON;
                  crab->health = CRAB_HEALTH*1.5f;

                  crab->leader_handle = leader_handle;
                  entity_list_append(gp, leader->child_list, crab);

                }

                Entity *crab = spawn_crab(gp);
                crab->pos = leader->pos;
                crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                crab->sprite_tint = MAROON;
                crab->shoot_control = ENTITY_SHOOT_CONTROL_PERIODIC_1;
                crab->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_TENTACLE_SHOT;
                crab->leader_handle = leader_handle;
                entity_list_append(gp, leader->child_list, crab);


              } /* spawn_orbiting_crabs */

            } else {

              if(gp->live_enemies == 0) {
                if(gp->phase.timer[0] < 0) {
                  gp->phase.timer[0] = 0;
                  goto phase_end;
                } else {
                  gp->phase.timer[0] -= gp->timestep;
                }
              }

            }

          } break;
        case 1:
          {
            const int SNAKE_ENEMIES_COUNT = 32;
            const Vector2 INITIAL_POS = { .x = WINDOW_WIDTH * 0.2f, .y = WINDOW_HEIGHT * 0.2f };
            const float ENEMY_SPAWN_DELAY = 0.6f;
            const float SECOND_GROUP_DELAY = 2.6f;
            const float THIRD_GROUP_DELAY = 2.95f;
            const float crab_snake_speed = 350.0f;

            const u32 CREATED_WAYPOINTS    = 1u<<0;
            const u32 SPAWNED_SECOND_GROUP = 1u<<1;
            const u32 SPAWNED_THIRD_GROUP  = 1u<<2;

            f32 *enemy_snake_spawn_timer = &gp->phase.timer[0];
            f32 *second_enemy_group_spawn_delay = &gp->phase.timer[1];
            f32 *third_enemy_group_spawn_delay = &gp->phase.timer[2];
            s32 *enemy_snake_count = &gp->phase.accumulator[0];


            if(!(gp->phase.flags & CREATED_WAYPOINTS)) {
              gp->phase.flags |= CREATED_WAYPOINTS;

              Waypoint_list *waypoints = wave_push_struct(Waypoint_list);

              float radius = 10;
              Vector2 positions[] = {
                { WINDOW_WIDTH * 0.8f, WINDOW_HEIGHT * 0.2f },
                { WINDOW_WIDTH * 0.8f, WINDOW_HEIGHT * 0.30f },
                { WINDOW_WIDTH * 0.2f, WINDOW_HEIGHT * 0.30f },
                { WINDOW_WIDTH * 0.2f, WINDOW_HEIGHT * 0.40f },
                { WINDOW_WIDTH * 0.8f, WINDOW_HEIGHT * 0.40f },
              };

              for(int i = 0; i < ARRLEN(positions); i++) {
                waypoint_list_append(gp, waypoints, positions[i], radius);
              }

              waypoints->action = entity_die;

              gp->phase.waypoints = waypoints;

            } else {

              Waypoint_list *waypoints = gp->phase.waypoints;
              ASSERT(waypoints);

              if(gp->phase.check_if_finished) {

                if(gp->live_enemies == 0) {
                  goto phase_end;
                }

              } else {

                if(*enemy_snake_count >= SNAKE_ENEMIES_COUNT) {
                  gp->phase.check_if_finished = 1;
                } else {

                  if(*enemy_snake_spawn_timer >= ENEMY_SPAWN_DELAY) {
                    *enemy_snake_spawn_timer = 0;

                    *enemy_snake_count += 1;

                    Entity *crab = spawn_crab(gp);
                    //crab->health = BIG_CRAB_HEALTH;
                    //crab->sprite_scale = 2.5f;
                    //crab->radius = 55;
                    crab->pos = INITIAL_POS;
                    crab->waypoints = *waypoints;
                    crab->scalar_vel = crab_snake_speed;
                    crab->move_control = ENTITY_MOVE_CONTROL_GOTO_WAYPOINT;
                    crab->shoot_control = ENTITY_SHOOT_CONTROL_PERIODIC_2;
                    crab->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_V_SHOT;
                    crab->sprite_tint = MAROON;

                  } else {
                    *enemy_snake_spawn_timer += gp->timestep;
                  }

                  if(!(gp->phase.flags & SPAWNED_SECOND_GROUP)) {
                    if(*second_enemy_group_spawn_delay >= SECOND_GROUP_DELAY) {

                      gp->phase.flags |= SPAWNED_SECOND_GROUP;

                      { /* spawn_orbiting_crabs */
                        float leader_scalar_vel = 150;

                        Entity *leader = spawn_leader(gp);
                        Entity_handle leader_handle = handle_from_entity(leader);
                        leader->child_list = push_entity_list(gp);
                        leader->vel = (Vector2){ .x = 1, .y = 0 };
                        leader->vel = Vector2Scale(Vector2Rotate(leader->vel, (5.0f*PI)/7.0f), leader_scalar_vel);
                        leader->pos =
                          (Vector2){
                            .x = WINDOW_WIDTH * 1.4,
                            .y = WINDOW_HEIGHT * -0.63,
                          };

                        int crab_count = 4;
                        float radius = 110;
                        float step_angle = (2*PI) / (float)crab_count;
                        Vector2 arm = ORBIT_ARM;

                        for(int i = 0; i < 6; i++) {
                          Vector2 pos =
                            Vector2Add(leader->pos, Vector2Scale(Vector2Rotate(arm, (float)i*step_angle), radius));

                          Entity *crab = spawn_crab(gp);

                          crab->health = 2*CRAB_HEALTH;

                          crab->pos = pos;
                          crab->move_control = ENTITY_MOVE_CONTROL_ORBIT_LEADER;
                          crab->shoot_control = ENTITY_SHOOT_CONTROL_NONE;

                          crab->orbit_cur_angle = (float)i*step_angle;
                          crab->orbit_speed = -PI*0.15;
                          crab->orbit_radius = radius;
                          crab->sprite_tint = MAROON;

                          crab->leader_handle = leader_handle;
                          entity_list_append(gp, leader->child_list, crab);

                        }

                        Entity *crab = spawn_crab(gp);
                        crab->pos = leader->pos;
                        crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                        crab->shoot_control = ENTITY_SHOOT_CONTROL_PERIODIC_3;
                        crab->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_RADIAL_BOOM;
                        crab->leader_handle = leader_handle;
                        crab->sprite_tint = MAROON;
                        entity_list_append(gp, leader->child_list, crab);

                      } /* spawn_orbiting_crabs */

                    } else {
                      *second_enemy_group_spawn_delay += gp->timestep;
                    }

                  }

                  if(gp->phase.flags & SPAWNED_SECOND_GROUP && !(gp->phase.flags & SPAWNED_THIRD_GROUP)) {
                    if(*third_enemy_group_spawn_delay >= THIRD_GROUP_DELAY) {

                      gp->phase.flags |= SPAWNED_THIRD_GROUP;

                      { /* spawn_orbiting_crabs */
                        float leader_scalar_vel = 150;

                        Entity *leader = spawn_leader(gp);
                        Entity_handle leader_handle = handle_from_entity(leader);
                        leader->child_list = push_entity_list(gp);
                        leader->vel = (Vector2){ .x = -1, .y = 0 };
                        leader->vel = Vector2Scale(Vector2Rotate(leader->vel, (-5.0f*PI)/6.0f), leader_scalar_vel);
                        leader->pos =
                          (Vector2){
                            .x = WINDOW_WIDTH * -0.87,
                            .y = WINDOW_HEIGHT * -0.60,
                          };

                        int crab_count = 4;
                        float radius = 110;
                        float step_angle = (2*PI) / (float)crab_count;
                        Vector2 arm = ORBIT_ARM;

                        for(int i = 0; i < 6; i++) {
                          Vector2 pos =
                            Vector2Add(leader->pos, Vector2Scale(Vector2Rotate(arm, (float)i*step_angle), radius));

                          Entity *crab = spawn_crab(gp);

                          crab->health = 2*CRAB_HEALTH;

                          crab->pos = pos;
                          crab->move_control = ENTITY_MOVE_CONTROL_ORBIT_LEADER;
                          crab->shoot_control = ENTITY_SHOOT_CONTROL_NONE;

                          crab->orbit_cur_angle = (float)i*step_angle;
                          crab->orbit_speed = PI*0.15;
                          crab->orbit_radius = radius;
                          crab->sprite_tint = MAROON;

                          crab->leader_handle = leader_handle;
                          entity_list_append(gp, leader->child_list, crab);

                        }

                        Entity *crab = spawn_crab(gp);
                        crab->pos = leader->pos;
                        crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                        crab->shoot_control = ENTITY_SHOOT_CONTROL_PERIODIC_4;
                        crab->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_RADIAL_BOOM;
                        crab->leader_handle = leader_handle;
                        crab->sprite_tint = MAROON;
                        entity_list_append(gp, leader->child_list, crab);

                      } /* spawn_orbiting_crabs */

                    } else {
                      *third_enemy_group_spawn_delay += gp->timestep;
                    }

                  }

                }

              }

            }
          } break;
        case 2:
          {

            const float ENEMY_SPAWN_DELAY = 1.6f;
            const u32 SPAWNED_ENEMY = 1<<0;
            const float PICKUP_TIME = 7.7f;

            u32 *flags = &gp->phase.flags;
            f32 *enemy_spawn_delay = &gp->phase.timer[0];
            f32 *pickup_timer = &gp->phase.timer[1];

            if(*enemy_spawn_delay < ENEMY_SPAWN_DELAY) {
              *enemy_spawn_delay += gp->timestep;
            } else {

              if(!(*flags & SPAWNED_ENEMY)) {

                *flags |= SPAWNED_ENEMY;
                spawn_boss_crab(gp);
                at_boss_level = true;

              } else {

                if(*pickup_timer >= PICKUP_TIME) {
                  *pickup_timer = 0;

                  int v = GetRandomValue(0, 5);

                  if(v == 0) {
                    spawn_health_pack(gp);
                  } else if(v == 1) {
                    spawn_health_pack(gp);
                    spawn_health_pack(gp);
                  } else if(v == 2) {
                    spawn_double_bullets_pack(gp);
                  } else if(v == 3) {
                    spawn_triple_bullets_pack(gp);
                  } else if(v == 4) {
                    spawn_quinta_bullets_pack(gp);
                  } else if(v == 5) {
                    spawn_bomb_pack(gp);
                  }
                } else {
                  *pickup_timer += gp->timestep;
                }

                if(gp->live_enemies == 0) {
                  goto phase_end;
                }
              }

            }

          } break;
        default:
          goto wave_end;
      }
      break;

    default:
      {
        Entity *player = entity_from_handle(gp->player_handle);
        gp->input_flags = 0;
        player->flags &= ~ENTITY_FLAG_DYNAMICS & ~ENTITY_FLAG_APPLY_FRICTION & ~ENTITY_FLAG_RECEIVE_COLLISION;
        player->accel = (Vector2){0};
        player->vel = (Vector2){0};
        gp->next_state = GAME_STATE_VICTORY;
      } break;
  }

  goto end;

phase_end:
  if(!at_boss_level) {
    int v = GetRandomValue(0, 5);

    if(v == 0) {
      spawn_health_pack(gp);
    } else if(v == 1) {
      spawn_health_pack(gp);
      spawn_health_pack(gp);
    } else if(v == 2) {
      spawn_double_bullets_pack(gp);
    } else if(v == 3) {
      spawn_triple_bullets_pack(gp);
    } else if(v == 4) {
      spawn_quinta_bullets_pack(gp);
    } else if(v == 5) {
      spawn_bomb_pack(gp);
    }
  }
  memory_set(&gp->phase, 0, sizeof(gp->phase));
  gp->phase_index++;
  goto end;

wave_end:
  if(!at_boss_level) {
    spawn_health_pack(gp);
    game_wave_end(gp);
  } else {
    gp->next_state = GAME_STATE_VICTORY;
  }
  goto end;

end:
  if(!gp->player->live) {
    gp->next_state = GAME_STATE_GAME_OVER;
  }

}

void game_update_and_draw(Game *gp) {

  if(IsMusicStreamPlaying(gp->music)) {
    if(GetMusicTimePlayed(gp->music) >= 160.58f) {
      SeekMusicStream(gp->music, 32.630f);
    }

    SetMusicVolume(gp->music, 0.10f);
    UpdateMusicStream(gp->music);
  }

#ifdef DEBUG
  gp->timestep = Clamp(1.0f/50.0f, 1.0f/TARGET_FPS, GetFrameTime());
#else
  gp->timestep = Clamp(1.0f/10.0f, 1.0f/TARGET_FPS, GetFrameTime());
#endif

  gp->next_state = gp->state;

  { /* get input */
    gp->input_flags = 0;

    if(IsKeyDown(KEY_W)) {
      gp->input_flags |= INPUT_FLAG_MOVE_FORWARD;
    }

    if(IsKeyDown(KEY_A)) {
      gp->input_flags |= INPUT_FLAG_MOVE_LEFT;
    }

    if(IsKeyDown(KEY_S)) {
      gp->input_flags |= INPUT_FLAG_MOVE_BACKWARD;
    }

    if(IsKeyDown(KEY_D)) {
      gp->input_flags |= INPUT_FLAG_MOVE_RIGHT;
    }

    if(IsKeyDown(KEY_J)) {
      gp->input_flags |= INPUT_FLAG_SHOOT;
    }

    if(IsKeyPressed(KEY_B)) {
      gp->input_flags |= INPUT_FLAG_BOMB;
    }

    if(IsKeyDown(KEY_LEFT_SHIFT)) {
      gp->input_flags |= INPUT_FLAG_SLOW_MOVE;
    }

    if(IsKeyPressed(KEY_ESCAPE)) {
      if(gp->state >= GAME_STATE_SPAWN_PLAYER) {
        gp->input_flags |= INPUT_FLAG_PAUSE;
      }
    }

#ifdef DEBUG

    if(IsKeyPressed(KEY_F1)) {
      if(gp->debug_flags & GAME_DEBUG_FLAG_MUTE) {
        SetMasterVolume(1);
      } else {
        SetMasterVolume(0);
      }
      gp->debug_flags ^= GAME_DEBUG_FLAG_MUTE;
    }

    if(IsKeyPressed(KEY_F5)) {
      game_reset(gp);
    }

    if(IsKeyPressed(KEY_F3)) {
      gp->debug_flags ^= GAME_DEBUG_FLAG_HOT_RELOAD;
    }

    if(IsKeyPressed(KEY_F11)) {
      gp->debug_flags  ^= GAME_DEBUG_FLAG_DEBUG_UI;
    }

    if(IsKeyPressed(KEY_F10)) {
      gp->debug_flags ^= GAME_DEBUG_FLAG_DRAW_ALL_ENTITY_BOUNDS;
    }

    if(IsKeyPressed(KEY_F7)) {
      gp->debug_flags  ^= GAME_DEBUG_FLAG_PLAYER_INVINCIBLE;
    }

#endif

    int key = GetCharPressed();
    if(key != 0) {
      gp->input_flags |= INPUT_FLAG_ANY;
    }

  } /* get input */


  if(gp->input_flags & INPUT_FLAG_PAUSE) {
    gp->flags ^= GAME_FLAG_PAUSE;
  }

  { /* update */

    if(gp->flags & GAME_FLAG_PAUSE) {
      if(gp->input_flags & INPUT_FLAG_ANY) {
        ResumeSound(gp->avenger_bullet_sound);
        ResumeSound(gp->crab_hurt_sound);
        ResumeSound(gp->health_pickup_sound);
        gp->flags ^= GAME_FLAG_PAUSE;
      } else {
        PauseSound(gp->avenger_bullet_sound);
        PauseSound(gp->crab_hurt_sound);
        PauseSound(gp->health_pickup_sound);
        goto update_end;
      }
    }

    if(gp->background_y_offset >= WINDOW_HEIGHT) {
      gp->background_y_offset -= WINDOW_HEIGHT;
    } else {
      gp->background_y_offset += gp->timestep * 100;
    }

    if(gp->bomb_cooldown > 0.0f) {
      gp->bomb_cooldown -= gp->timestep;
    }

    switch(gp->state) {
      default:
        UNREACHABLE;
      case GAME_STATE_NONE:
        {
          /* start settings */

          SetMusicVolume(gp->music, 1.0);

          {
#ifdef DEBUG

            gp->wave = 0;
            gp->phase_index = 0;

            gp->debug_flags |=
              GAME_DEBUG_FLAG_DEBUG_UI |
              //GAME_DEBUG_FLAG_PLAYER_INVINCIBLE |
              //GAME_DEBUG_FLAG_SKIP_TRANSITIONS |
              0;

            //gp->next_state = GAME_STATE_GAME_OVER;
            gp->next_state = GAME_STATE_SPAWN_PLAYER;
            //gp->bomb_count = 1;
            //gp->score = 90;
            //gp->next_state = GAME_STATE_TITLE_SCREEN;
#else

            gp->wave = 0;
            gp->phase_index = 0;

            if(!has_started_before) {
              gp->next_state = GAME_STATE_TITLE_SCREEN;
              has_started_before = true;
            } else {
              gp->next_state = GAME_STATE_SPAWN_PLAYER;
            }
#endif

            { /* init globals */

              at_boss_level = false;

              title_screen_key_pressed = false;
              title_screen_chars_deleted = 0;
              title_screen_type_char_timer = 0;

              intro_screen_fade_timer = 0;
              intro_screen_colonel_delay = 1.0f;
              intro_screen_cur_message = 0;
              intro_screen_chars_typed = 0;
              intro_screen_type_char_timer = 0;
              intro_screen_pre_message_delay = 0.8f;

              victory_screen_pre_delay = 1.0f;
              victory_screen_pre_counter_delay = 0.5f;
              victory_screen_finished_typing_banner = false;
              victory_screen_finished_incrementing_score = false;
              victory_screen_chars_typed = 0;
              victory_screen_type_dir = 1;
              victory_screen_show_timer = 2.5f;
              victory_screen_target_len = STRLEN("VICTORY");
              victory_screen_type_char_timer = 0;
              victory_screen_inc_score_timer = 0;
              victory_screen_score_delay = 1.4f;
              victory_screen_score_counter = 0;
              victory_screen_restart_hint_on = true;
              victory_screen_hint_timer = 0;

              boss_start_shooting_delay = 0;
              boss_shoot_pause = 0;

            } /* init globals */

            memory_set(&gp->phase, 0, sizeof(gp->phase));

            gp->flags |= GAME_FLAG_PLAYER_CANNOT_SHOOT;
            gp->wave_timer = WAVE_DELAY_TIME;
            gp->wave_type_char_timer = TYPING_SPEED;
            gp->wave_chars_typed = 0;
            gp->wave_banner_msg = push_str8f(gp->wave_scratch, "WAVE %i", gp->wave+1);
            gp->wave_banner_type_dir = 1;
            gp->wave_banner_target_msg_len = gp->wave_banner_msg.len;
          }

          goto update_end;

        } break;
      case GAME_STATE_TITLE_SCREEN:
        {

          if(title_screen_key_pressed) {
            if(title_screen_chars_deleted >= STRLEN("FLIGHT 22")) {
              gp->next_state = GAME_STATE_INTRO_SCREEN;
              gp->intro_scope = scope_begin(gp->scratch);
            } else {

              if(title_screen_type_char_timer >= TYPING_SPEED) {
                title_screen_type_char_timer = 0;
                title_screen_chars_deleted += 1;
              } else {
                title_screen_type_char_timer += gp->timestep;
              }

            }
          }

          if(gp->input_flags & INPUT_FLAG_ANY) {
            title_screen_key_pressed = true;
          }

        } break;
      case GAME_STATE_INTRO_SCREEN:
        {

        } break;
      case GAME_STATE_SPAWN_PLAYER:
        {

          if(!gp->player) {
            gp->player = spawn_player(gp);
            gp->player->friction = 3.0f;
            gp->player->vel.y = -6e2;
            gp->player_handle = handle_from_entity(gp->player);
          } else {
            if(gp->player->pos.y < WINDOW_HEIGHT*0.8f) {
              gp->next_state = GAME_STATE_WAVE_TRANSITION;
              gp->player->friction = PLAYER_FRICTION;
              gp->player->vel.y = 0;
            }
          }

        } break;
      case GAME_STATE_WAVE_TRANSITION:
        {

#ifdef DEBUG
          if(gp->debug_flags & GAME_DEBUG_FLAG_SKIP_TRANSITIONS) {
            gp->next_state = GAME_STATE_MAIN_LOOP;
            gp->flags &= ~GAME_FLAG_PLAYER_CANNOT_SHOOT;
          }
#endif
          if(gp->live_entities == 1) {

            ASSERT(entity_from_handle(gp->player_handle));

            if(gp->wave_chars_typed == gp->wave_banner_target_msg_len) {

              if(gp->wave_timer < 0) {
                if(gp->wave_chars_typed == 0) {
                  gp->next_state = GAME_STATE_MAIN_LOOP;
                  gp->flags &= ~GAME_FLAG_PLAYER_CANNOT_SHOOT;
                } else {
                  ASSERT(gp->wave_chars_typed == gp->wave_banner_msg.len);
                  gp->wave_banner_target_msg_len = 0;
                  gp->wave_banner_type_dir = -1;
                }
              } else {
                gp->wave_timer -= gp->timestep;
              }

            } else {

              if(gp->wave_type_char_timer < 0) {
                gp->wave_type_char_timer = TYPING_SPEED;
                gp->wave_chars_typed += gp->wave_banner_type_dir;
              } else {
                gp->wave_type_char_timer -= gp->timestep;
              }

            }
          }

        } break;
      case GAME_STATE_MAIN_LOOP:
        game_main_loop(gp);
        if(gp->next_state == GAME_STATE_VICTORY) {
          goto update_end;
        }
        break;
      case GAME_STATE_VICTORY:
        {

          if(victory_screen_finished_incrementing_score) {
            if(IsKeyPressed(KEY_ENTER)) {
              game_reset(gp);
              goto update_end;
            }
          }

        } break;
      case GAME_STATE_GAME_OVER:
        {

          if(gp->gameover_pre_delay >= 0.6) {
            if(gp->gameover_chars_typed >= STRLEN("GAME OVER")) {

              if(gp->input_flags & INPUT_FLAG_ANY) {
                game_reset(gp);
                goto update_end;
              }

            } else {

              if(gp->gameover_type_char_timer < 0) {
                gp->gameover_type_char_timer = TYPING_SPEED;
                gp->gameover_chars_typed += 1;
              } else {
                gp->gameover_type_char_timer -= gp->timestep;
              }

            }
          } else {
            gp->gameover_pre_delay += gp->timestep;
          }

        } break;
#ifdef DEBUG
      case GAME_STATE_DEBUG_SANDBOX:
        {
        } break;
#endif
    }

    gp->live_entities = 0;
    gp->live_enemies = 0;

    for(Entity_order order = ENTITY_ORDER_FIRST; order < ENTITY_ORDER_MAX; order++) {

      for(int i = 0; i < gp->entities_allocated; i++)
      { /* update_entities */

        Entity *ep = &gp->entities[i];

        if(ep->live && ep->update_order == order)
        { /* entity_update */

          gp->live_entities++;

          if(entity_kind_in_mask(ep->kind, ENEMY_KIND_MASK)) {
            gp->live_enemies++;
          }

          b8 applied_collision = 0;
          b8 is_on_screen = CheckCollisionCircleRec(ep->pos, ep->radius, WINDOW_RECT);

          switch(ep->move_control) {
            default:
              UNREACHABLE;
            case ENTITY_MOVE_CONTROL_NONE:
              break;
            case ENTITY_MOVE_CONTROL_PLAYER:
              if(gp->state >= GAME_STATE_WAVE_TRANSITION) {

                b8 was_moving_left = 0;
                b8 was_moving_right = 0;

                if(ep->accel.x < 0) {
                  was_moving_left = 1;
                } else if(ep->accel.x > 0) {
                  was_moving_right = 1;
                }

                b8 was_not_moving = !was_moving_right && !was_moving_left;

                ep->accel = (Vector2){0};

                if(gp->input_flags & INPUT_FLAG_MOVE){

                  if(gp->input_flags & INPUT_FLAG_MOVE_LEFT) {
                    ep->accel.x = -PLAYER_ACCEL;
                    
                    if(was_not_moving) {
                      //ep->sprite = SPRITE_SHIP_STRAFE;
                      //ep->sprite.flags |= SPRITE_FLAG_DRAW_MIRRORED_X;
                    }

                  }

                  if(gp->input_flags & INPUT_FLAG_MOVE_RIGHT) {
                    ep->accel.x += PLAYER_ACCEL;

                    if(was_not_moving) {
                      //ep->sprite = SPRITE_SHIP_STRAFE;
                    }

                  }

                  if(gp->input_flags & INPUT_FLAG_MOVE_FORWARD) {
                    ep->accel.y = -PLAYER_ACCEL;
                  }

                  if(gp->input_flags & INPUT_FLAG_MOVE_BACKWARD) {
                    ep->accel.y += PLAYER_ACCEL;
                  }

                  if(gp->input_flags & INPUT_FLAG_SLOW_MOVE) {
                    ep->accel = Vector2Scale(ep->accel, PLAYER_SLOW_FACTOR);
                  }

                } else {

                  // TODO strafing animation for the spitfire
#if 0
                  if(was_moving_left) {
                    ep->sprite = SPRITE_SHIP_STRAFE;
                    ep->sprite.flags |= SPRITE_FLAG_REVERSE;
                    ep->sprite.flags |= SPRITE_FLAG_DRAW_MIRRORED_X;
                  } else if(was_moving_right) {
                    ep->sprite = SPRITE_SHIP_STRAFE;
                    ep->sprite.flags |= SPRITE_FLAG_REVERSE;
                  }
#endif

                  ep->vel = (Vector2){0};
                }

                if(ep->received_damage > 0) {
                  ep->effect_tint = BLOOD;

                  if(ep->invulnerability_timer > 0) {
                    ep->health += ep->received_damage;
                  } else {
                    ep->invulnerability_timer = 1.2f;
                  }

                }

                if(ep->invulnerability_timer != 0) {
                  if(ep->invulnerability_timer > 0) {
                    ep->invulnerability_timer -= gp->timestep;

                    if(ep->effect_tint_timer <= 0) {
                      ep->flags |= ENTITY_FLAG_APPLY_EFFECT_TINT;

                      ep->effect_tint_period = 0.1f;
                      ep->effect_tint_timer_vel = 1.0f;

                      ep->effect_tint_timer = ep->effect_tint_period;
                    }

                  } else {
                    ep->invulnerability_timer = 0;
                    ep->effect_tint = BLANK;
                  }
                }

                if(gp->flags & GAME_FLAG_DO_THE_CRAB_WALK) {
                  ep->accel.y = 0;
                  ep->vel.y = 0;
                }

                if(gp->input_flags & INPUT_FLAG_SHOOT) {
                  if(!(gp->flags & GAME_FLAG_PLAYER_CANNOT_SHOOT)) {
                    ep->bullet_emitter.shoot = 1;
                  }
                }

                if(gp->input_flags & INPUT_FLAG_BOMB) {

                  if(gp->bomb_count > 0) {
                    if(gp->bomb_cooldown <= 0) {

                      SetSoundVolume(gp->bomb_sound, 0.2f);
                      PlaySound(gp->bomb_sound);

                      gp->bomb_cooldown = BOMB_COOLDOWN_TIME;
                      gp->bomb_count--;

                      for(int i = 0; i < gp->entities_allocated; i++) {
                        Entity *check = &gp->entities[i];

                        if(check->live) {
                          if(CheckCollisionCircleRec(check->pos, check->radius, WINDOW_RECT)) {
                            if(entity_kind_in_mask(check->kind, ENTITY_KIND_MASK_CRAB | ENTITY_KIND_MASK_ENEMY_BULLET)) {
                              check->flags |= ENTITY_FLAG_DIE_NOW;
                            }
                          }
                        }
                      }

                    }
                  }

                }

                if(gp->debug_flags & GAME_DEBUG_FLAG_PLAYER_INVINCIBLE) {
                  ep->received_damage = 0;
                  ep->bullet_emitter.shoot = 0;
                }

              } break;
            case ENTITY_MOVE_CONTROL_COPY_LEADER:
              {
                Entity *leader = entity_from_handle(ep->leader_handle);
                ASSERT(leader);

                ep->vel = leader->vel;

              } break;
            case ENTITY_MOVE_CONTROL_ORBIT_LEADER:
              {
                Entity *leader = entity_from_handle(ep->leader_handle);
                ASSERT(leader);

                Vector2 arm = ORBIT_ARM;
                ep->pos =
                  Vector2Add(leader->pos,
                      Vector2Scale(Vector2Rotate(arm, ep->orbit_cur_angle), ep->orbit_radius));

                if(ep->orbit_cur_angle >= 2*PI) {
                  ep->orbit_cur_angle = 0;
                } else {
                  ep->orbit_cur_angle += ep->orbit_speed * gp->timestep;
                }

                ep->vel = leader->vel;

              } break;
            case ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE:
              {

                if(ep->flags & ENTITY_FLAG_ON_SCREEN) {
                  if(ep->child_list->count <= 0) {
                    entity_die(gp, ep);
                    goto entity_update_end;
                  }

                  const float padding = ep->leader_strafe_padding;

                  Entity *furthest_left = ep->child_list->first->ep;
                  Entity *furthest_right = ep->child_list->last->ep;

                  for(Entity_node *node = ep->child_list->first; node; node = node->next) {
                    Entity *child = node->ep;

                    if(child->pos.x < furthest_left->pos.x) {
                      furthest_left = child;
                    }

                    if(child->pos.x > furthest_right->pos.x) {
                      furthest_right = child;
                    }

                  }

                  if(furthest_left->flags & ENTITY_FLAG_ON_SCREEN) {
                    if(furthest_left->vel.x < 0 && furthest_left->pos.x < padding) { // right
                      ep->vel.x *= -1;
                    }
                  }

                  if(furthest_right->flags & ENTITY_FLAG_ON_SCREEN) {
                    if(furthest_right->vel.x > 0 && furthest_right->pos.x > WINDOW_WIDTH-padding) { // left
                      ep->vel.x *= -1;
                    }
                  }

                }

              } break;
            case ENTITY_MOVE_CONTROL_FOLLOW_LEADER:
              {
                Entity *leader = entity_from_handle(ep->leader_handle);
                ASSERT(leader);

                Vector2 dir = Vector2Normalize(Vector2Subtract(leader->pos, ep->pos));
                ep->vel = Vector2Scale(dir, ep->scalar_vel);

              } break;
            case ENTITY_MOVE_CONTROL_GOTO_WAYPOINT:
              {
                ASSERT(ep->waypoints.first && ep->waypoints.last);
                Waypoint *wp = ep->cur_waypoint;
                if(!wp) {
                  wp = ep->waypoints.first;
                }

                Vector2 dir = Vector2Subtract(wp->pos, ep->pos);
                float dir_len_sqr = Vector2LengthSqr(dir);

                if(dir_len_sqr < SQUARE(wp->radius)) {
                  if(!wp->next) {
                    if(ep->waypoints.action) {
                      ep->waypoints.action(gp, ep);
                    }
                  } else {
                    ep->cur_waypoint = wp->next;
                  }
                } else {
                  float dir_len = sqrtf(dir_len_sqr);
                  dir = Vector2Scale(dir, ep->scalar_vel/dir_len);
                  ep->vel = dir;
                }

              } break;
            case ENTITY_MOVE_CONTROL_FOLLOW_LEADER_CHAIN:
              {

                ASSERT(ep->parent_list);

                Entity_node *next_node_in_chain = ep->list_node->next;
                ASSERT(next_node_in_chain);

                Entity *leader = entity_from_handle(next_node_in_chain->handle);
                float ideal_dist = leader->radius + 20.0f;

                Vector2 delta = Vector2Subtract(leader->pos, ep->pos);
                float dist = Vector2Length(delta);
                if(dist >= 0.0001f) {
                  Vector2 dir = Vector2Scale(delta, 1.0f/dist);
                  Vector2 ideal_pos = Vector2Subtract(leader->pos, Vector2Scale(dir, ideal_dist));
                  float snap_speed = 4.0f * gp->timestep;
                  ep->pos = Vector2Lerp(ep->pos, ideal_pos, snap_speed);
                }

              } break;
          }

          if(is_on_screen) {
            switch(ep->shoot_control) {
              default:
                UNREACHABLE;
              case ENTITY_SHOOT_CONTROL_NONE:
                break;
              case ENTITY_SHOOT_CONTROL_BOSS:
                {
                  if(boss_start_shooting_delay < 1.8f) {
                    boss_start_shooting_delay += gp->timestep;
                  } else {

                    if(boss_shoot_pause >= get_random_float(1.0f, 2.0f, 4)) {
                      if(ep->bullet_emitter.shoot <= 0) {
                        boss_shoot_pause = 0;
                        int i = GetRandomValue(0, n_boss_bullet_emitters-1);
                        ep->bullet_emitter = boss_bullet_emitters[i];
                        ep->bullet_emitter.shoot = 1;
                      }
                    } else {
                      boss_shoot_pause += gp->timestep;
                    }

                  }
                } break;
              case ENTITY_SHOOT_CONTROL_ONCE:
                {
                  ASSERT(ep->start_shooting_delay_period > 0);

                  if(ep->start_shooting_delay < ep->start_shooting_delay_period) {
                    ep->start_shooting_delay += gp->timestep;
                  } else {
                    ep->bullet_emitter.shoot = 1;
                    ep->shoot_control = ENTITY_SHOOT_CONTROL_NONE;
                  }

                } break;
              case ENTITY_SHOOT_CONTROL_PERIODIC_1:
                {

                  if(ep->start_shooting_delay < 0.6f) {
                    ep->start_shooting_delay += gp->timestep;
                  } else {

                    if(ep->bullet_emitter.shoot <= 0) {
                      if(ep->shooting_pause_timer <= 0) {
                        ep->shooting_pause_timer = 1.0f;
                        ep->bullet_emitter.shoot = 1;
                      } else {
                        ep->shooting_pause_timer -= gp->timestep;
                      }
                    }

                  }

                } break;

              case ENTITY_SHOOT_CONTROL_PERIODIC_2:
                {

                  if(ep->start_shooting_delay < 0.3f) {
                    ep->start_shooting_delay += gp->timestep;
                  } else {

                    if(ep->bullet_emitter.shoot <= 0) {
                      if(ep->shooting_pause_timer <= 0) {
                        ep->shooting_pause_timer = 2.5f;
                        ep->bullet_emitter.shoot = 1;
                      } else {
                        ep->shooting_pause_timer -= gp->timestep;
                      }
                    }

                  }

                } break;

              case ENTITY_SHOOT_CONTROL_PERIODIC_3:
                {

                  if(ep->start_shooting_delay < 2.0f) {
                    ep->start_shooting_delay += gp->timestep;
                  } else {

                    if(ep->bullet_emitter.shoot <= 0) {
                      if(ep->shooting_pause_timer <= 0) {
                        ep->shooting_pause_timer = 2.0f;
                        ep->bullet_emitter.shoot = 1;
                      } else {
                        ep->shooting_pause_timer -= gp->timestep;
                      }
                    }

                  }

                } break;

              case ENTITY_SHOOT_CONTROL_PERIODIC_4:
                {

                  if(ep->start_shooting_delay < 1.5f) {
                    ep->start_shooting_delay += gp->timestep;
                  } else {

                    if(ep->bullet_emitter.shoot <= 0) {
                      if(ep->shooting_pause_timer <= 0) {
                        ep->shooting_pause_timer = 2.5f;
                        ep->bullet_emitter.shoot = 1;
                      } else {
                        ep->shooting_pause_timer -= gp->timestep;
                      }
                    }

                  }

                } break;

            }
          }

          /* flags stuff */

          if(ep->flags & ENTITY_FLAG_APPLY_FRICTION) {
            ep->vel = Vector2Subtract(ep->vel, Vector2Scale(ep->vel, ep->friction*gp->timestep));
          }

          if(ep->flags & ENTITY_FLAG_DYNAMICS) {
            Vector2 a_times_t = Vector2Scale(ep->accel, gp->timestep);
            ep->vel = Vector2Add(ep->vel, a_times_t);
            ep->pos = Vector2Add(ep->pos, Vector2Add(Vector2Scale(ep->vel, gp->timestep), Vector2Scale(a_times_t, 0.5*gp->timestep)));

            if(ep->flags & ENTITY_FLAG_DYNAMICS_HAS_CURVE) {
              ep->vel = Vector2Rotate(ep->vel, ep->curve);
            }

          }

          if(ep->kind == ENTITY_KIND_PLAYER) {
            Vector2 pos_min = { ep->radius, ep->radius };
            Vector2 pos_max =
              Vector2Subtract(WINDOW_SIZE, pos_min);
            ep->pos = Vector2Clamp(ep->pos, pos_min, pos_max);
          }

          if(ep->flags & ENTITY_FLAG_HAS_BULLET_EMITTER) {
            if(gp->state != GAME_STATE_GAME_OVER) {
              entity_emit_bullets(gp, ep);
            }
          }

          if(ep->flags & ENTITY_FLAG_APPLY_COLLISION) {

            for(int i = 0; i < gp->entities_allocated; i++) {
              Entity *colliding = &gp->entities[i];

              if(ep != colliding && colliding->live) {

                if(entity_kind_in_mask(colliding->kind, ep->apply_collision_mask)) {
                  if(entity_check_collision(gp, ep, colliding)) {
                    applied_collision = 1;
                    colliding->received_collision = 1;

                    if(ep->collide_proc) {
                      ep->collide_proc(gp, ep, colliding);
                    }

                    if(ep->flags & ENTITY_FLAG_APPLY_COLLISION_DAMAGE) {
                      colliding->received_damage += ep->damage_amount;
                    }

                  }

                }

              }

            }

          }

          if(ep->flags & ENTITY_FLAG_DIE_IF_CHILD_LIST_EMPTY) {
            ASSERT(ep->child_list);
            if(ep->child_list->count <= 0) {
              ep->flags |= ENTITY_FLAG_DIE_NOW;
            }
          }

          if(ep->flags & ENTITY_FLAG_DIE_ON_APPLY_COLLISION) {
            if(applied_collision) {
              ep->flags |= ENTITY_FLAG_DIE_NOW;
            }
          }

          if(ep->flags & ENTITY_FLAG_APPLY_EFFECT_TINT) {
            if(ep->effect_tint_timer < 0) {
              ep->effect_tint_timer = 0;
              ep->flags &= ~ENTITY_FLAG_APPLY_EFFECT_TINT;
            } else {
              ep->effect_tint_timer -= ep->effect_tint_timer_vel*gp->timestep;
            }
          }

          if(ep->flags & ENTITY_FLAG_RECEIVE_COLLISION) {
            if(is_on_screen) {
              if(ep->received_collision) {
                ep->received_collision = 0;

                if(ep->flags & ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE) {

                  if(ep->received_damage > 0) {
                    if(IsSoundValid(ep->hurt_sound)) {
                      SetSoundPan(ep->hurt_sound, Normalize(ep->pos.x, WINDOW_WIDTH, 0));
                      if(ep->kind == ENTITY_KIND_PLAYER) {
                        SetSoundVolume(ep->hurt_sound, 0.17f);
                      } else {
                        SetSoundVolume(ep->hurt_sound, 0.5f);
                      }
                      PlaySound(ep->hurt_sound);
                    }
                  }

                  ep->health -= ep->received_damage;

                  if(ep->flags & ENTITY_FLAG_DAMAGE_INCREMENTS_SCORE) {
                    gp->score += ep->received_damage;
                  }

                  ep->received_damage = 0;

                  if(ep->kind != ENTITY_KIND_PLAYER) {
                    ep->flags |= ENTITY_FLAG_APPLY_EFFECT_TINT;

                    ep->effect_tint = BLOOD;

                    if(ep->effect_tint_period == 0) {
                      ep->effect_tint_period = 0.02f;
                    }
                    if(ep->effect_tint_timer_vel == 0) {
                      ep->effect_tint_timer_vel = 1.0f;
                    }

                    ep->effect_tint_timer = ep->effect_tint_period;
                  }

                  if(ep->health <= 0) {
                    ep->flags |= ENTITY_FLAG_DIE_NOW;
                  }

                }

              }
            }
          }

          if(ep->flags & ENTITY_FLAG_HAS_SPRITE) {
            sprite_update(gp, ep);
          }

          if(ep->flags & ENTITY_FLAG_NOT_ON_SCREEN) {
            if(is_on_screen) {
              ep->flags ^= ENTITY_FLAG_NOT_ON_SCREEN | ENTITY_FLAG_ON_SCREEN;
            }
          }

          if(ep->flags & ENTITY_FLAG_DIE_IF_EXIT_SCREEN) {
            if(is_on_screen) {
              ep->flags ^= ENTITY_FLAG_DIE_IF_EXIT_SCREEN | ENTITY_FLAG_DIE_IF_OFFSCREEN;
            }
          }

          if(ep->flags & ENTITY_FLAG_DIE_IF_OFFSCREEN) {
            if(!is_on_screen) {
              if(!(ep->flags & ENTITY_FLAG_NOT_ON_SCREEN)) {
                entity_die(gp, ep);
                goto entity_update_end;
              }
            }
          }

          if(ep->flags & ENTITY_FLAG_HAS_LIFETIME) {
            ASSERT(ep->life_time_period > 0);

            if(ep->life_timer >= ep->life_time_period) {
              ep->life_timer = 0;
              ep->flags |= ENTITY_FLAG_DIE_NOW;
            } else {
              ep->life_timer += gp->timestep;
            }

          }

          if(ep->flags & ENTITY_FLAG_EMIT_SPAWN_PARTICLES) {
            ep->flags &= ~ENTITY_FLAG_EMIT_SPAWN_PARTICLES;

            if(is_on_screen) {

              if(IsSoundValid(ep->spawn_sound)) {
                PlaySound(ep->spawn_sound);
              }

              Particle_emitter tmp = ep->particle_emitter;
              ep->particle_emitter = ep->spawn_particle_emitter;
              entity_emit_particles(gp, ep);
              ep->particle_emitter = tmp;
            }

          }

          if(ep->flags & ENTITY_FLAG_DIE_NOW) {

            if(is_on_screen) {
              if(ep->flags & ENTITY_FLAG_EMIT_DEATH_PARTICLES) {
                Particle_emitter tmp = ep->particle_emitter;
                ep->particle_emitter = ep->death_particle_emitter;
                entity_emit_particles(gp, ep);
                ep->particle_emitter = tmp;
              }

              if(ep->kind == ENTITY_KIND_BOSS) {
                SetSoundVolume(gp->boss_die, 1.0);
                PlaySound(gp->boss_die);
              }
            }

            entity_die(gp, ep);
            goto entity_update_end;
          }

entity_update_end:;
        } /* entity_update */

      } /* update_entities */

      if(gp->state >= GAME_STATE_WAVE_TRANSITION) {
        Entity *player = entity_from_handle(gp->player_handle);

        if(!player) {
          gp->next_state = GAME_STATE_GAME_OVER;
        }
      }

    }

    gp->live_particles = 0;

    for(int i = 0; i < MAX_PARTICLES; i++) {
      Particle *p = &gp->particles[i];

      if(p->live >= p->lifetime) {
        p->live = 0;
        p->lifetime = 0;
        continue;
      }

      gp->live_particles++;

      { /* particle_update */

        if(!CheckCollisionCircleRec(p->pos, p->radius, WINDOW_RECT)) {
          p->live = 0;
          p->lifetime = 0;
          gp->live_particles--;
          continue;
        }

        p->pos = Vector2Add(p->pos, Vector2Scale(p->vel, gp->timestep));
        p->vel = Vector2Subtract(p->vel, Vector2Scale(p->vel, p->friction*gp->timestep));

        if(p->radius > 0) {
          p->radius -= p->shrink * gp->timestep;
        }

        p->live += gp->timestep;

      } /* particle_update */

    }

update_end:;
  } /* update */

  defer_loop(BeginTextureMode(gp->render_texture), EndTextureMode())
  { /* draw */
    ClearBackground(BLACK);

    { /* draw_background */

      float height = 3*0.25*gp->background_texture.width;

      // TODO better background

      Rectangle source =
      {
        0, 0, gp->background_texture.width, height,
      };

      Rectangle dest1 =
      {
        0, gp->background_y_offset - WINDOW_HEIGHT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
      };

      Rectangle dest2 =
      {
        0, gp->background_y_offset,
        WINDOW_WIDTH, WINDOW_HEIGHT,
      };

      DrawTexturePro(gp->background_texture, source, dest1, VEC2_ZERO, 0, WHITE);
      DrawTexturePro(gp->background_texture, source, dest2, VEC2_ZERO, 0, WHITE);

    } /* draw_background */

    for(Entity_order order = ENTITY_ORDER_FIRST; order < ENTITY_ORDER_MAX; order++) {
      for(int i = 0; i < gp->entities_allocated; i++)
      { /* entity_draw */

        Entity *ep = &gp->entities[i];

        if(!ep->live || ep->draw_order != order) continue;

        if(ep->flags & ENTITY_FLAG_FILL_BOUNDS) {
          Color tint = ep->fill_color;
          DrawCircleV(ep->pos, ep->radius, tint);
        }

        if(ep->flags & ENTITY_FLAG_HAS_SPRITE) {
          Vector2 pos = ep->pos;

          // big hacks
          if(ep->kind == ENTITY_KIND_PLAYER) {
            ep->pos.y += PLAYER_SPRITE_Y_OFFSET;
          }

          if(ep->kind == ENTITY_KIND_BOSS) {
            ep->pos.y += 110;
          }

          draw_sprite(gp, ep);

          ep->pos = pos;
        }

        if(gp->debug_flags & GAME_DEBUG_FLAG_DRAW_ALL_ENTITY_BOUNDS) {
          Color bounds_color = ep->bounds_color;
          bounds_color.a = 150;
          DrawCircleLinesV(ep->pos, ep->radius, ep->bounds_color);

          DrawCircleV(ep->pos, 4.0f, bounds_color);
        }

      } /* entity_draw */
    }

    for(int i = 0; i < MAX_PARTICLES; i++) {
      Particle *p = &gp->particles[i];

      if(p->live >= p->lifetime) continue;

      { /* particle_draw */

        Rectangle rec = { p->pos.x - p->radius, p->pos.y - p->radius, TIMES2(p->radius), TIMES2(p->radius) };

        Color tint = ColorLerp(p->begin_tint, p->end_tint, Normalize(p->live, 0, p->lifetime));

        DrawRectangleRec(rec, tint);

      } /* particle_draw */

    }

    if(gp->state >= GAME_STATE_SPAWN_PLAYER)
      defer_loop(SetTextLineSpacing(5), SetTextLineSpacing(10)) { /* HUD */

        Entity *player = entity_from_handle(gp->player_handle);

        gp->player_health = 0;
        if(player) {
          gp->player_health = player->health;
        }

        /* TODO(jfd 09/05/2025):
         *
         * There is apparently in emscripten (or maybe my flags are wrong)
         * which cause long integers not to format correctly.
         * Investigate this later.
         * For the meantime, don't try to print anything with %li or %lu.
         *
         *
         * SOLVED(jfd 09/05/2025, a few moments later...):
         *
         * Emscripten is 32bit only!!!!!!!!!!!!!!!
         * Which leaves us kind of screwed for now, unless we do some sketchy #defines
         */

        Str8 msg =
          push_str8f(gp->frame_scratch,
              "SCORE: %i\n"
              "HEALTH: %i\n"
              "BOMBS: %i\n",
              gp->score,
              gp->player_health,
              gp->bomb_count);

        DrawTextEx(
            gp->font,
            (char*)msg.s,
            (Vector2){ 10, 10 },
            20,
            2,
            BLACK);

      } /* HUD */

    if(gp->state == GAME_STATE_WAVE_TRANSITION) {
      Arena_scope scope = wave_scope_begin();

      s64 tmp = gp->wave_banner_msg.len;
      gp->wave_banner_msg.len = gp->wave_chars_typed;
      const char *msg = push_cstr_copy_str8(gp->wave_scratch, gp->wave_banner_msg);
      gp->wave_banner_msg.len = tmp;

      const float width = WINDOW_WIDTH;
      const float height = WINDOW_HEIGHT;

      Vector2 pos = { width * 0.5, height * 0.3f };
      Vector2 size = MeasureTextEx(gp->font, msg, WAVE_BANNER_FONT_SIZE, 10);
      DrawTextEx(
          gp->font,
          msg,
          Vector2Subtract(pos, Vector2Scale(size, 0.5)),
          WAVE_BANNER_FONT_SIZE,
          WAVE_BANNER_FONT_SPACING,
          WAVE_BANNER_FONT_COLOR);

      scope_end(scope);
    } else if(gp->state == GAME_STATE_GAME_OVER) {

      Arena_scope scope = frame_scope_begin();

      char msg[] = "GAME OVER";
      msg[gp->gameover_chars_typed] = 0;

      const float width = WINDOW_WIDTH;
      const float height = WINDOW_HEIGHT;

      Vector2 pos = { width * 0.5, height * 0.4f };
      Vector2 size = MeasureTextEx(gp->font, msg, GAME_OVER_FONT_SIZE, 10);
      DrawTextEx(
          gp->font,
          msg,
          Vector2Subtract(pos, Vector2Scale(size, 0.5)),
          GAME_OVER_FONT_SIZE,
          GAME_OVER_FONT_SPACING,
          GAME_OVER_FONT_COLOR);

      if(gp->gameover_chars_typed >= STRLEN(msg)) {
        if(game_over_hint_timer >= 0.5) {
          game_over_hint_timer = 0;
          game_over_hint_on = !game_over_hint_on;
        } else {
          game_over_hint_timer += gp->timestep;
        }

        if(game_over_hint_on) {
          char *hint = "press any key to restart";

          Vector2 pos = { WINDOW_WIDTH * 0.5, WINDOW_HEIGHT * 0.5f };
          Vector2 size = MeasureTextEx(gp->font, hint, 20, 2);
          DrawTextEx(
              gp->font,
              hint,
              Vector2Subtract(pos, Vector2Scale(size, 0.5)),
              20,
              2,
              BLACK);
        }

      }

      scope_end(scope);

    } else if(gp->state == GAME_STATE_TITLE_SCREEN) {

      char title[] = "FLIGHT 22";
      char *hint = "press any key to start";
      int title_font_size = 100;
      int hint_font_size = 20;

      Color title_color = ColorBrightness(MUSTARD, 0.1);
      Color hint_color = ColorBrightness(BLACK, 0.1);

      static bool hint_on = true;
      static float hint_blink_time = 0.0f;

      Vector2 title_pos = { WINDOW_WIDTH * 0.5, WINDOW_HEIGHT * 0.4f };
      Vector2 title_size = MeasureTextEx(gp->font, title, title_font_size, title_font_size/10);

      Vector2 hint_pos = { WINDOW_WIDTH * 0.5, WINDOW_HEIGHT * 0.5f };
      Vector2 hint_size = MeasureTextEx(gp->font, hint, hint_font_size, hint_font_size/10);

      if(title_screen_key_pressed) {
        title[STRLEN(title)-title_screen_chars_deleted] = 0;

        DrawTextEx(
            gp->font,
            title,
            Vector2Subtract(title_pos, Vector2Scale(title_size, 0.5)),
            title_font_size,
            title_font_size/10,
            title_color);

      } else {

        DrawTextEx(
            gp->font,
            title,
            Vector2Subtract(title_pos, Vector2Scale(title_size, 0.5)),
            title_font_size,
            title_font_size/10,
            title_color);

        if(hint_on) {
          DrawTextEx(
              gp->font,
              hint,
              Vector2Subtract(hint_pos, Vector2Scale(hint_size, 0.5)),
              hint_font_size,
              hint_font_size/10,
              hint_color);
        }

        if((hint_on && hint_blink_time >= 0.75f) || (!hint_on && hint_blink_time >= 0.5f)) {
          hint_on = !hint_on;
          hint_blink_time = 0;
        } else {
          hint_blink_time += gp->timestep;
        }
      }

    } else if(gp->state == GAME_STATE_INTRO_SCREEN) {

      DrawRectangleRec(WINDOW_RECT, ColorAlpha(BLACK, Lerp(0.0f, 0.4f, Normalize(intro_screen_fade_timer, 0, INTRO_SCREEN_FADE_TIME))));

      if(intro_screen_fade_timer < INTRO_SCREEN_FADE_TIME) {
        intro_screen_fade_timer += gp->timestep;
      } else if(intro_screen_colonel_delay > 0.0f) {
        intro_screen_colonel_delay -= gp->timestep;
      } else {

        { /* update colonel fu */
          Sprite *sp = &colonel_fu;
          if(!(sp->flags & SPRITE_FLAG_STILL)) {

            ASSERT(sp->fps > 0);

            if(sp->cur_frame < sp->total_frames) {
              sp->frame_counter++;

              if(sp->frame_counter >= (TARGET_FPS / sp->fps)) {
                sp->frame_counter = 0;
                sp->cur_frame++;
              }

            }

            if(sp->cur_frame >= sp->total_frames) {
              if(sp->flags & SPRITE_FLAG_INFINITE_REPEAT) {
                if(sp->flags & SPRITE_FLAG_PINGPONG) {
                  sp->cur_frame--;
                  sp->flags ^= SPRITE_FLAG_REVERSE;
                } else {
                  sp->cur_frame = 0;
                }
              } else {
                sp->cur_frame--;
                sp->flags |= SPRITE_FLAG_AT_LAST_FRAME | SPRITE_FLAG_STILL;
                ASSERT(sp->cur_frame >= 0 && sp->cur_frame < sp->total_frames);
              }
            }

          }

        } /* update colonel fu */

        { /* draw colonel fu */
          Sprite sp = colonel_fu;
          Vector2 pos = { WINDOW_WIDTH * 0.16, WINDOW_HEIGHT * 0.3 };
          f32 scale = 8.0f;

          ASSERT(sp.cur_frame >= 0 && sp.cur_frame < sp.total_frames);

          Sprite_frame frame;

          if(sp.flags & SPRITE_FLAG_REVERSE) {
            frame = __sprite_frames[sp.last_frame - sp.cur_frame];
          } else {
            frame = __sprite_frames[sp.first_frame + sp.cur_frame];
          }

          Rectangle source_rec =
          {
            .x = (float)frame.x,
            .y = (float)frame.y,
            .width = (float)frame.w,
            .height = (float)frame.h,
          };

          Rectangle dest_rec =
          {
            .x = (pos.x - scale*0.5f*source_rec.width),
            .y = (pos.y - scale*0.5f*source_rec.height),
            .width = scale*source_rec.width,
            .height = scale*source_rec.height,
          };

          if(sp.flags & SPRITE_FLAG_DRAW_MIRRORED_X) {
            source_rec.width *= -1;
          }

          if(sp.flags & SPRITE_FLAG_DRAW_MIRRORED_Y) {
            source_rec.height *= -1;
          }

          DrawTexturePro(gp->sprite_atlas, source_rec, dest_rec, (Vector2){0}, 0, WHITE);
          DrawRectangleLinesEx(dest_rec, 2.0f, MUSTARD);
        } /* draw colonel fu */

        if(intro_screen_pre_message_delay > 0) {
          intro_screen_pre_message_delay -= gp->timestep;
        } else if(intro_screen_cur_message >= ARRLEN(intro_screen_messages)) {
          gp->next_state = GAME_STATE_SPAWN_PLAYER;
        } else {

          if(intro_screen_chars_typed >= intro_screen_message_lengths[intro_screen_cur_message]) {
            if(gp->input_flags & INPUT_FLAG_ANY) {
              intro_screen_cur_message += 1;
              intro_screen_chars_typed = 0;
            }
          } else {

            if(intro_screen_type_char_timer >= TYPING_SPEED) {
              intro_screen_type_char_timer = 0;
              intro_screen_chars_typed += 1;
            } else {
              intro_screen_type_char_timer += gp->timestep;
            }

          }

          {

            Arena_scope scope = scope_begin(gp->scratch);

            Str8 s =
            {
              .s = (u8*)intro_screen_messages[intro_screen_cur_message],
              .len = intro_screen_chars_typed,
            };


            bool message_complete = false;
            if(intro_screen_chars_typed < intro_screen_message_lengths[intro_screen_cur_message]) {
              intro_screen_cursor_on = true;
            } else {
              message_complete = true;
              if(intro_screen_cursor_blink_timer > 0.5f) {
                intro_screen_cursor_blink_timer = 0;
                intro_screen_cursor_on = !intro_screen_cursor_on;
              } else {
                intro_screen_cursor_blink_timer += gp->timestep;
              }
            }

            char *msg;

            if(intro_screen_cursor_on) {
              msg = (char*)push_str8f(gp->scratch, "%S_", s).s;
            } else {
              msg = push_cstr_copy_str8(gp->scratch, s);
            }

            Vector2 pos = { WINDOW_WIDTH * 0.28, WINDOW_HEIGHT * 0.18f };
            DrawTextEx(
                gp->font,
                msg,
                pos,
                30,
                3,
                MUSTARD);

            if(message_complete) {

              Vector2 pos = { WINDOW_WIDTH * 0.28, WINDOW_HEIGHT * 0.40f };
              DrawTextEx(
                  gp->font,
                  "press any key to continue",
                  pos,
                  20,
                  2,
                  ColorAlpha(LIGHTGRAY, 0.7f));
            }

            scope_end(scope);

          }

        }

      }

    } else if(gp->state == GAME_STATE_VICTORY) {

      if(victory_screen_pre_delay > 0) {
        victory_screen_pre_delay -= gp->timestep;
      } else {

        Arena_scope scope = scope_begin(gp->scratch);

        float victory_banner_font_size = 80;

        char victory_banner[] = "VICTORY";
        victory_banner[victory_screen_chars_typed] = 0;



        if(!victory_screen_finished_typing_banner) {

          if(victory_screen_chars_typed == victory_screen_target_len) {
            if(victory_screen_type_dir == 1) {
              if(victory_screen_show_timer <= 0) {
                victory_screen_type_dir = -1;
                victory_screen_target_len = 0;
              } else {
                victory_screen_show_timer -= gp->timestep;
              }
            } else {
              ASSERT(victory_screen_type_dir == -1);
              if(victory_screen_chars_typed <= 0) {
                victory_screen_finished_typing_banner = true;
              }
            }
          } else {
            if(victory_screen_type_char_timer >= TYPING_SPEED) {
              victory_screen_chars_typed += victory_screen_type_dir;
              victory_screen_type_char_timer = 0;
            } else {
              victory_screen_type_char_timer += gp->timestep;
            }

          }

          Vector2 victory_banner_pos = { WINDOW_WIDTH * 0.5, WINDOW_HEIGHT * 0.4f };
          Vector2 victory_banner_size = MeasureTextEx(gp->font, victory_banner, victory_banner_font_size, victory_banner_font_size/10);

          DrawTextEx(
              gp->font,
              victory_banner,
              Vector2Subtract(victory_banner_pos, Vector2Scale(victory_banner_size, 0.5)),
              victory_banner_font_size,
              victory_banner_font_size/10,
              ColorBrightness(MUSTARD, 0.1));


        } else {
          if(victory_screen_score_delay > 0) {
            victory_screen_score_delay -= gp->timestep;
          } else {

            Str8 score_banner = push_str8f(gp->scratch,
                "SCORE: %i", victory_screen_score_counter);

            if(victory_screen_pre_counter_delay > 0) {
              victory_screen_pre_counter_delay -= gp->timestep;
            } else {
              //if(victory_screen_inc_score_timer >= COUNTING_SPEED) {
              //  victory_screen_inc_score_timer = 0;

                if(victory_screen_score_counter >= gp->score) {
                  victory_screen_finished_incrementing_score = true;
                  victory_screen_score_counter = gp->score;
                } else {
                  victory_screen_score_counter += 95;
                }

              //} else {
              //  victory_screen_inc_score_timer += gp->timestep;
              //}
            }

            char *score_cstr = push_cstr_copy_str8(gp->scratch, score_banner);

            Vector2 victory_score_pos = { WINDOW_WIDTH * 0.5, WINDOW_HEIGHT * 0.4f };
            Vector2 victory_score_size = MeasureTextEx(gp->font, score_cstr, 100, 10);

            DrawTextEx(
                gp->font,
                score_cstr,
                Vector2Subtract(victory_score_pos, Vector2Scale(victory_score_size, 0.5)),
                100,
                10,
                MUSTARD);

            if(victory_screen_finished_incrementing_score) {
              if(victory_screen_hint_timer >= 0.5) {
                victory_screen_hint_timer = 0;
                victory_screen_restart_hint_on = !victory_screen_restart_hint_on;
              } else {
                victory_screen_hint_timer += gp->timestep;
              }

              char *hint = "press enter to play again";

              Vector2 hint_pos = { WINDOW_WIDTH * 0.5, WINDOW_HEIGHT * 0.6f };
              Vector2 hint_size = MeasureTextEx(gp->font, hint, 20, 2);

              if(victory_screen_restart_hint_on) {
                DrawTextEx(
                    gp->font,
                    hint,
                    Vector2Subtract(hint_pos, Vector2Scale(hint_size, 0.5)),
                    20,
                    2,
                    BLACK);
              }

            }

          }

        }


        scope_end(scope);

      }

    }

    if(gp->flags & GAME_FLAG_PAUSE) {
      DrawRectangleRec(WINDOW_RECT, (Color){ .a = 100 });

      char *paused_msg = "PAUSED";
      char *hint_msg = "press any key to resume";
      int pause_font_size = 90;
      int hint_font_size = 20;

      Color pause_color = MUSTARD;
      Color hint_color = LIGHTGRAY;

      static b8 hint_on = 1;
      static f32 hint_blink_time = 0.0f;

      Vector2 pause_pos = { WINDOW_WIDTH * 0.5, WINDOW_HEIGHT * 0.4f };
      Vector2 pause_size = MeasureTextEx(gp->font, paused_msg, pause_font_size, pause_font_size/10);

      Vector2 hint_pos = { WINDOW_WIDTH * 0.5, WINDOW_HEIGHT * 0.5f };
      Vector2 hint_size = MeasureTextEx(gp->font, hint_msg, hint_font_size, hint_font_size/10);

      DrawTextEx(
          gp->font,
          paused_msg,
          Vector2Subtract(pause_pos, Vector2Scale(pause_size, 0.5)),
          pause_font_size,
          pause_font_size/10,
          pause_color);

      if(hint_on) {
        DrawTextEx(
            gp->font,
            hint_msg,
            Vector2Subtract(hint_pos, Vector2Scale(hint_size, 0.5)),
            hint_font_size,
            hint_font_size/10,
            hint_color);
      }

      if((hint_on && hint_blink_time >= 0.75f) || (!hint_on && hint_blink_time >= 0.5f)) {
        hint_on = !hint_on;
        hint_blink_time = 0;
      } else {
        hint_blink_time += gp->timestep;
      }

    }

  } /* draw */

  defer_loop(BeginDrawing(), EndDrawing())
  { /* draw to screen */

    ClearBackground(BLACK);

    float scale = fminf((float)GetScreenWidth() / WINDOW_WIDTH,
        (float)GetScreenHeight() / WINDOW_HEIGHT);
    if(scale < 1.0) {
      scale = 0.5;
    } else {
      scale = roundf(scale);
    }
    int offset_x = (GetScreenWidth() - (int)(WINDOW_WIDTH * scale)) >> 1;
    int offset_y = (GetScreenHeight() - (int)(WINDOW_HEIGHT * scale)) >> 1;

    {
      Rectangle dest = { (float)offset_x, (float)offset_y, WINDOW_WIDTH * scale, WINDOW_HEIGHT * scale };
      DrawRectangleRec(dest, BLACK);
      DrawTexturePro(gp->render_texture.texture,
          (Rectangle){ 0, 0, (float)gp->render_texture.texture.width, -(float)gp->render_texture.texture.height },
          dest,
          (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

#ifdef DEBUG
    if(gp->debug_flags & GAME_DEBUG_FLAG_DEBUG_UI) { /* debug overlay */
      char *debug_text = frame_push_array(char, 256);
      char *debug_text_fmt =
        "sound: %s\n"
        "auto_hot_reload: %s\n"
        "player_is_invincible: %s\n"
        "frame time: %.7f\n"
        "live entities count: %i\n"
        "live enemies count: %i\n"
        "live particles count: %i\n"
        "most entities allocated: %li\n"
        "particle_pos: %i\n"
        "screen width: %i\n"
        "screen height: %i\n"
        "player pos: { x = %.1f, y = %.1f }\n"
        "wave: %i\n"
        "phase: %i\n"
        "game state: %s";
      stbsp_sprintf(debug_text,
          debug_text_fmt,
          (gp->debug_flags & GAME_DEBUG_FLAG_MUTE) ? "off" : "on",
          (gp->debug_flags & GAME_DEBUG_FLAG_HOT_RELOAD) ? "on" : "off",
          (gp->debug_flags & GAME_DEBUG_FLAG_PLAYER_INVINCIBLE) ? "on" : "off",
          gp->timestep,
          gp->live_entities,
          gp->live_enemies,
          gp->live_particles,
          gp->entities_allocated,
          gp->particles_pos,
          GetScreenWidth(),
          GetScreenHeight(),
          gp->player ? gp->player->pos.x : 0,
          gp->player ? gp->player->pos.y : 0,
          gp->wave+1,
          gp->phase_index+1,
          Game_state_strings[gp->state]);
      Vector2 debug_text_size = MeasureTextEx(gp->font, debug_text, 18, 1.0);
      DrawText(debug_text, 10, GetScreenHeight() - debug_text_size.y - 10, 18, GREEN);
    } /* debug overlay */
#endif

  } /* draw to screen */


  gp->state = gp->next_state;

  gp->frame_index++;

  arena_clear(gp->frame_scratch);

}
