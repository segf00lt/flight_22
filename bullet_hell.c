#include "bullet_hell.h"


/*
 * generated code
 */

#include "sprite_data.c"


/*
 * globals
 */



/* 
 * function bodies
 */

#define frame_push_array(T, n) (T*)push_array(gp->frame_scratch, T, n)

#define entity_is_part_of_list(ep) (ep->list_node && ep->list_node->ep == ep)

Entity* entity_spawn(Game *gp) {
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
  Entity_list *list = gp->entity_lists + gp->entity_lists_count++;
  return list;
}

force_inline Entity_node* push_entity_list_node(Game *gp) {
  Entity_node *e_node = push_array_no_zero(gp->entity_node_arena, Entity_node, 1);
  return e_node;
}

Entity_list* get_entity_list_by_id(Game *gp, int list_id) {
  Entity_list *list = 0;

  ASSERT(list_id >= 0 && list_id < MAX_ENTITY_LISTS);

  list = gp->entity_lists + list_id;

  return list;
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

Entity* get_entity_by_uid(Game *gp, u64 uid) {
  Entity *ep = 0;

  for(int i = 0; i < gp->entities_allocated; i++) {
    if(gp->entities[i].uid == uid) {
      ep = gp->entities + i;
      break;
    }
  }

  return ep;
}

Entity* get_entity_by_handle(Entity_handle handle) {
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

Entity* spawn_player(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->move_control = ENTITY_MOVE_CONTROL_PLAYER;
  ep->kind = ENTITY_KIND_PLAYER;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_CLAMP_POS_TO_SCREEN |
    ENTITY_FLAG_HAS_BULLET_EMITTER |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    0;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->look_dir = PLAYER_LOOK_DIR;
  ep->pos = PLAYER_INITIAL_OFFSCREEN_POS;

  ep->health = PLAYER_HEALTH;

  ep->sprite = SPRITE_AVENGER;
  ep->sprite_tint = WHITE;
  ep->sprite_scale = PLAYER_SPRITE_SCALE;

  ep->bullet_emitter.kind = BULLET_EMITTER_KIND_AVENGER;
  ep->bullet_emitter.flags =
    0;
  ep->bullet_emitter.cooldown_period = AVENGER_NORMAL_FIRE_COOLDOWN;

  ep->bounds_color = PLAYER_BOUNDS_COLOR;

  ep->radius = PLAYER_BOUNDS_RADIUS;

  return ep;
}

Entity* spawn_crab_leader(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_LEADER;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_NOT_ON_SCREEN |
    ENTITY_FLAG_DIE_IF_CHILD_LIST_EMPTY |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->pos = CRAB_LEADER_INITIAL_DEBUG_POS;

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
    0;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->look_dir = (Vector2){ 0, 1 };
  ep->pos = CRAB_INITIAL_DEBUG_POS;

  ep->health = CRAB_HEALTH;

  ep->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB;
  ep->bullet_emitter.flags =
    0;

  ep->bullet_emitter.cooldown_period = CRAB_FIRE_COOLDOWN;

  ep->bounds_color = CRAB_BOUNDS_COLOR;
  ep->sprite = SPRITE_CRAB;
  ep->sprite_scale = CRAB_SPRITE_SCALE;
  ep->sprite_tint = WHITE;

  ep->radius = CRAB_BOUNDS_RADIUS;

  return ep;
}

#if !1
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

  //ASSERT(cur_crab_leader_handle.uid && cur_crab_leader_handle.ep);

  //ep->leader_handle = cur_crab_leader_handle;

  //ep->sprite = SPRITE_AVENGER;
  //ep->sprite_tint = WHITE;
  //ep->sprite_scale = PLAYER_SPRITE_SCALE;

  ep->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB;
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

Entity* spawn_fish(Game *gp) {
  UNIMPLEMENTED;
  return 0;
}

Entity* spawn_stingray(Game *gp) {
  UNIMPLEMENTED;
  return 0;
}

void entity_emit_bullets(Game *gp, Entity *ep) {
  Bullet_emitter *emitter = &ep->bullet_emitter;

  switch(ep->bullet_emitter.kind) {
    default:
      UNREACHABLE;
    case BULLET_EMITTER_KIND_AVENGER:
      {

        emitter->bullet_collision_mask = PLAYER_APPLY_COLLISION_MASK;
        emitter->n_rings = 1;

        {
          Bullet_emitter_ring *ring = &emitter->rings[0];

          ring->spin_vel = 0.0f;
          ring->radius = 60.0f;
          ring->n_arms = 1;
          ring->arms_occupy_circle_sector_percent = 0;
          ring->n_bullets = 1;
          ring->bullet_arm_width = 0.0f;
          ring->bullet_radius = AVENGER_NORMAL_BULLET_BOUNDS_RADIUS;
          ring->bullet_vel = AVENGER_NORMAL_BULLET_VELOCITY*.5f;
          ring->bullet_damage = AVENGER_NORMAL_BULLET_DAMAGE;
          ring->bullet_bounds_color = GREEN;
          ring->bullet_fill_color = ORANGE;
          ring->bullet_flags =
            ENTITY_FLAG_FILL_BOUNDS |
            0;

        }

#if !1
        {
          Bullet_emitter_ring *ring = &emitter->rings[0];

          ring->spin_vel = 0.1f;
          ring->radius = 50.0f;
          ring->n_arms = 1;
          ring->arms_occupy_circle_sector_percent = 2.0f/3.0f;
          ring->n_bullets = 3;
          ring->bullet_arm_width = 50.0f;
          ring->bullet_radius = AVENGER_NORMAL_BULLET_BOUNDS_RADIUS;
          ring->bullet_vel = AVENGER_NORMAL_BULLET_VELOCITY*.5f;
          ring->bullet_damage = AVENGER_NORMAL_BULLET_DAMAGE;
          ring->bullet_bounds_color = GREEN;
          ring->bullet_fill_color = ORANGE;
          ring->bullet_flags =
            ENTITY_FLAG_FILL_BOUNDS |
            0;

        }

        {
          Bullet_emitter_ring *ring = &emitter->rings[1];

          //ring->spin_start_angle = (PI);
          ring->spin_vel = 0.1f;
          ring->radius = AVENGER_NORMAL_BULLET_BOUNDS_RADIUS+70.0f;
          ring->n_arms = 1;
          ring->arms_occupy_circle_sector_percent = 2.0f/3.0f;
          ring->n_bullets = 1;
          ring->bullet_arm_width = 20.0f;
          ring->bullet_radius = AVENGER_NORMAL_BULLET_BOUNDS_RADIUS;
          ring->bullet_vel = AVENGER_NORMAL_BULLET_VELOCITY*.5f;
          ring->bullet_damage = AVENGER_NORMAL_BULLET_DAMAGE;
          ring->bullet_bounds_color = GREEN;
          ring->bullet_fill_color = ORANGE;
          ring->bullet_flags =
            ENTITY_FLAG_FILL_BOUNDS |
            0;

        }

        {
          Bullet_emitter_ring *ring = &emitter->rings[2];

          ring->spin_vel = 0.1f;
          ring->radius = AVENGER_NORMAL_BULLET_BOUNDS_RADIUS+100.0f;
          ring->n_arms = 1;
          ring->arms_occupy_circle_sector_percent = 2.0f/3.0f;
          ring->n_bullets = 3;
          ring->bullet_arm_width = 50.0f;
          ring->bullet_radius = AVENGER_NORMAL_BULLET_BOUNDS_RADIUS;
          ring->bullet_vel = AVENGER_NORMAL_BULLET_VELOCITY*.5f;
          ring->bullet_damage = AVENGER_NORMAL_BULLET_DAMAGE;
          ring->bullet_bounds_color = GREEN;
          ring->bullet_fill_color = ORANGE;
          ring->bullet_flags =
            ENTITY_FLAG_FILL_BOUNDS |
            0;

        }

#endif

      } break;
    case BULLET_EMITTER_KIND_CRAB:
      {

        UNIMPLEMENTED;

      } break;
  }

  { /* flags checks */
  } /* flags checks */

  {

    ASSERT(ep->bullet_emitter.n_rings > 0);
    ASSERT(ep->bullet_emitter.n_rings <= MAX_BULLET_EMITTER_RINGS);

    for(int ring_i = 0; ring_i < ep->bullet_emitter.n_rings; ring_i++) {
      Bullet_emitter_ring *ring = &ep->bullet_emitter.rings[ring_i];

      ASSERT(ring->n_arms > 0);
      ASSERT(ring->n_bullets > 0);

      Vector2 arm_dir;
      float arms_occupy_circle_sector_angle;
      float arm_step_angle;

      if(ring->n_arms == 1) {
        arms_occupy_circle_sector_angle = 0;
        arm_step_angle = 0;
        arm_dir = Vector2Rotate(ep->look_dir, ring->spin_cur_angle + ring->spin_start_angle);
      } else {
        ASSERT(ring->arms_occupy_circle_sector_percent > 0);

        arms_occupy_circle_sector_angle =
          (2*PI) * ring->arms_occupy_circle_sector_percent;
        arm_step_angle = arms_occupy_circle_sector_angle / (float)(ring->n_arms-1);
        arm_dir =
          Vector2Rotate(
              ep->look_dir,
              -0.5*arms_occupy_circle_sector_angle + ring->spin_cur_angle + ring->spin_start_angle);
      }

      for(int arm_i = 0; arm_i < ring->n_arms; arm_i++) {

        Vector2 step_dir = {0};
        Vector2 bullet_pos = Vector2Add(ep->pos, Vector2Scale(arm_dir, ring->radius));

        if(ring->n_bullets > 1) {
          ASSERT(ring->bullet_arm_width > 0);

          Vector2 arm_dir_perp = { arm_dir.y, -arm_dir.x };

          step_dir =
            Vector2Scale(arm_dir_perp, ring->bullet_arm_width/(float)(ring->n_bullets-1));

          bullet_pos =
            Vector2Add(bullet_pos, Vector2Scale(arm_dir_perp, -0.5*ring->bullet_arm_width));
        }

        for(int bullet_i = 0; bullet_i < ring->n_bullets; bullet_i++) {
          Entity *bullet = entity_spawn(gp);

          bullet->kind = ENTITY_KIND_BULLET;
          bullet->update_order = ENTITY_ORDER_FIRST;
          bullet->draw_order = ENTITY_ORDER_LAST;

          bullet->flags = DEFAULT_BULLET_FLAGS | ring->bullet_flags;

          bullet->bounds_color = ring->bullet_bounds_color;
          bullet->fill_color = ring->bullet_fill_color;

          bullet->vel = Vector2Scale(arm_dir, ring->bullet_vel);

          bullet->vel = Vector2Add(bullet->vel, Vector2Scale(ep->look_dir, Vector2DotProduct(ep->look_dir, ep->vel)));

          bullet->curve = ring->bullet_curve;
          bullet->curve_rolloff_vel = ring->bullet_curve_rolloff_vel;

          bullet->radius = ring->bullet_radius;

          bullet->apply_collision_mask = ep->bullet_emitter.bullet_collision_mask;
          bullet->damage_amount = ring->bullet_damage;

          bullet->pos = bullet_pos;

          bullet_pos = Vector2Add(bullet_pos, step_dir);

        }

        arm_dir = Vector2Rotate(arm_dir, arm_step_angle);

      } /* for(int arm_i = 0; arm_i < ring->n_arms; arm_i++) */

      ring->spin_cur_angle += ring->spin_vel;
      if(ring->spin_cur_angle >= 2*PI) {
        ring->spin_cur_angle = 0;
      }


    } /* for(int ring_i = 0; ring_i < ep->bullet_emitter.n_rings; ring_i++) */

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

void draw_sprite(Game *gp, Sprite sp, Vector2 pos, Color tint) {
  draw_sprite_ex(gp, sp, pos, 1.0f, 0.0f, tint);
}

void sprite_update(Game *gp, Sprite *sp) {
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

void draw_sprite_ex(Game *gp, Sprite sp, Vector2 pos, f32 scale, f32 rotation, Color tint) {
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

void game_reset(Game *gp) {

  gp->state = GAME_STATE_NONE;
  gp->next_state = GAME_STATE_NONE;

  gp->flags = 0;

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

  arena_clear(gp->entity_node_arena);

  gp->entity_lists_count = 0;

  gp->phase_started = 0;
  gp->phase = 0;
  gp->wave = 0;

}

void game_main_loop(Game *gp) {

  switch(gp->wave) {
    case 0:
      switch(gp->phase) {
        case 0:
          {

            if(gp->phase_started == 0) {
              gp->phase_started = 1;

              { /* init phase */

                const int crabs_per_row = 2;

                {
                  Entity *leader = spawn_crab_leader(gp);
                  Entity_handle leader_handle = handle_from_entity(leader);
                  leader->child_list = push_entity_list(gp);
                  leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                  leader->vel = (Vector2){ .x = CRAB_NORMAL_STRAFE_SPEED };
                  leader->leader_strafe_padding = 100;
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

                    crab->pos = pos;
                    crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                    crab->leader_handle = leader_handle;
                    entity_list_append(gp, leader->child_list, crab);

                  }

                }

                {
                  Entity *leader = spawn_crab_leader(gp);
                  Entity_handle leader_handle = handle_from_entity(leader);
                  leader->child_list = push_entity_list(gp);
                  leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                  leader->vel = (Vector2){ .x = -CRAB_NORMAL_STRAFE_SPEED };
                  leader->leader_strafe_padding = 100;
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

                    crab->pos = pos;
                    crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                    crab->leader_handle = leader_handle;
                    entity_list_append(gp, leader->child_list, crab);

                  }

                }

              } /* init phase */

            } else {
              if(gp->live_entities == 1) {
                goto phase_end;
              }
            }

          } break;
        case 1:
          {

            if(gp->phase_started == 0) {
              gp->phase_started = 1;

              { /* init phase */

                const int crabs_per_row = 2;

                {
                  Entity *leader = spawn_crab_leader(gp);
                  Entity_handle leader_handle = handle_from_entity(leader);
                  leader->child_list = push_entity_list(gp);
                  leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                  leader->vel = (Vector2){ .x = CRAB_NORMAL_STRAFE_SPEED };
                  leader->leader_strafe_padding = 100;
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

                    crab->pos = pos;
                    crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                    crab->leader_handle = leader_handle;
                    entity_list_append(gp, leader->child_list, crab);

                  }

                }

                {
                  Entity *leader = spawn_crab_leader(gp);
                  Entity_handle leader_handle = handle_from_entity(leader);
                  leader->child_list = push_entity_list(gp);
                  leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                  leader->vel = (Vector2){ .x = -CRAB_NORMAL_STRAFE_SPEED };
                  leader->leader_strafe_padding = 100;
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

                    crab->pos = pos;
                    crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                    crab->leader_handle = leader_handle;
                    entity_list_append(gp, leader->child_list, crab);

                  }

                }

              } /* init phase */

            } else {
              if(gp->live_entities == 1) {
                goto phase_end;
              }
            }

          } break;
        case 2:
          {

            if(gp->phase_started == 0) {
              gp->phase_started = 1;

              { /* init phase */

                Entity *leader = spawn_crab_leader(gp);
                Entity_handle leader_handle = handle_from_entity(leader);
                leader->child_list = push_entity_list(gp);
                leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                leader->vel = (Vector2){ .x = -CRAB_NORMAL_STRAFE_SPEED };
                leader->leader_strafe_padding = 50;
                leader->pos =
                  (Vector2){
                    .x = WINDOW_WIDTH*0.5,
                    .y = WINDOW_HEIGHT *0.5,
                  };

                int crab_count = 6;
                float radius = 120;
                float step_angle = (2*PI) / (float)crab_count;
                Vector2 arm = ORBIT_ARM;

                for(int i = 0; i < 6; i++) {
                  Vector2 pos = Vector2Add(leader->pos, Vector2Scale(Vector2Rotate(arm, (float)i*step_angle), radius));

                  Entity *crab = spawn_crab(gp);

                  crab->pos = pos;
                  crab->move_control = ENTITY_MOVE_CONTROL_ORBIT_LEADER;

                  crab->orbit_cur_angle = (float)i*step_angle;
                  crab->orbit_speed = PI*0.15;
                  crab->orbit_radius = radius;

                  crab->leader_handle = leader_handle;
                  entity_list_append(gp, leader->child_list, crab);

                }


              } /* init phase */

            } else {
            }

          } break;
        default:
          goto wave_end;
      }
      break;

    default:
      gp->next_state = GAME_STATE_VICTORY;
  }

  goto end;

phase_end:
  gp->phase_started = 0;
  gp->phase++;
  goto end;

wave_end:
  gp->phase_started = 0;
  gp->phase = 0;
  gp->wave++;
  gp->next_state = GAME_STATE_WAVE_TRANSITION;

  gp->entity_lists_count = 0;
  arena_clear(gp->entity_node_arena);

  goto end;

end:
  if(!gp->player->live) {
    gp->next_state = GAME_STATE_GAME_OVER;
  }

}

void game_update_and_draw(Game *gp) {

  gp->timestep = Clamp(1.0f/10.0f, 1.0f/TARGET_FPS, GetFrameTime());

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

    //if(IsKeyDown(KEY_LEFT_SHIFT)) {
    //  gp->input_flags |= INPUT_FLAG_SLOW_MOVE;
    //}

    if(IsKeyPressed(KEY_ESCAPE)) {
      gp->input_flags |= INPUT_FLAG_PAUSE;
    }

#ifdef DEBUG

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
        gp->flags ^= GAME_FLAG_PAUSE;
      } else {
        goto update_end;
      }
    }

    switch(gp->state) {
      default:
        UNREACHABLE;
      case GAME_STATE_NONE:
        {
          //gp->next_state = GAME_STATE_DEBUG_SANDBOX;
          gp->next_state = GAME_STATE_MAIN_LOOP;

          gp->player = spawn_player(gp);

#if 0
          Entity_list *list = push_entity_list(gp);

          Vector2 crab_pos = { .x = WINDOW_WIDTH * 0.4f, .y = WINDOW_HEIGHT * 0.1f };

          for(int i = 0; i < 5; i++) {
            Entity *crab = spawn_crab(gp);
            crab->pos = crab_pos;
            Entity_node *node = entity_list_push_front(gp, list, crab);
            crab->parent_list = list;
            crab->list_node = node;

            crab_pos.x -= 5;
          }

          Entity *leader_crab = spawn_crab_leader(gp);
          leader_crab->pos = (Vector2){ .x =  WINDOW_WIDTH * 0.5f, .y = WINDOW_HEIGHT * 0.1f };
          leader_crab->flags |=
            ENTITY_FLAG_DYNAMICS |
            ENTITY_FLAG_DYNAMICS_HAS_CURVE |
            0;
          leader_crab->radius = CRAB_BOUNDS_RADIUS;
          leader_crab->vel = (Vector2){ .x = 300, .y = 0 };
          leader_crab->curve = 0.015f;
          entity_list_append(gp, list, leader_crab);
#endif


          goto update_end;

        } break;
      case GAME_STATE_TITLE_SCREEN:
        {
        } break;
      case GAME_STATE_SPAWN_PLAYER:
        {
        } break;
      case GAME_STATE_WAVE_TRANSITION:
        {
        } break;
      case GAME_STATE_MAIN_LOOP:
        game_main_loop(gp);
        if(gp->next_state == GAME_STATE_VICTORY) {
          goto update_end;
        }
        break;
      case GAME_STATE_VICTORY:
        {
        } break;
      case GAME_STATE_GAME_OVER:
        {
        } break;
#ifdef DEBUG
      case GAME_STATE_DEBUG_SANDBOX:
        {
        } break;
#endif
    }

    gp->live_entities = 0;

    for(Entity_order order = ENTITY_ORDER_FIRST; order < ENTITY_ORDER_MAX; order++) {

      for(int i = 0; i < gp->entities_allocated; i++)
      { /* update_entities */

        Entity *ep = &gp->entities[i];

        if(ep->live && ep->update_order == order)
        { /* entity_update */

          gp->live_entities++;

          b8 applied_collision = 0;

          switch(ep->move_control) {
            default:
              UNREACHABLE;
            case ENTITY_MOVE_CONTROL_NONE:
              break;
            case ENTITY_MOVE_CONTROL_PLAYER:
              {

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

                  // TODO strafing animation for the avenger
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

                if(gp->input_flags & INPUT_FLAG_SHOOT) {
                  ep->bullet_emitter.shooting = 1;
                }

                if(gp->debug_flags & GAME_DEBUG_FLAG_PLAYER_INVINCIBLE) {
                  ep->received_damage = 0;
                }

              } break;
            case ENTITY_MOVE_CONTROL_COPY_LEADER:
              {
                Entity *leader = get_entity_by_handle(ep->leader_handle);
                ASSERT(leader);

                ep->vel = leader->vel;

              } break;
            case ENTITY_MOVE_CONTROL_ORBIT_LEADER:
              {
                Entity *leader = get_entity_by_handle(ep->leader_handle);
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
                    if(furthest_left->vel.x < 0 && furthest_left->pos.x < padding) {
                      ep->vel.x *= -1;
                    }
                  }

                  if(furthest_right->flags & ENTITY_FLAG_ON_SCREEN) {
                    if(furthest_right->vel.x > 0 && furthest_right->pos.x > WINDOW_WIDTH-padding) {
                      ep->vel.x *= -1;
                    }
                  }

                }

              } break;
            case ENTITY_MOVE_CONTROL_FOLLOW_CHAIN:
              {

                ASSERT(ep->parent_list);

                Entity_node *next_node_in_chain = ep->list_node->next;
                ASSERT(next_node_in_chain);

                Entity *leader = get_entity_by_handle(next_node_in_chain->handle);
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

          switch(ep->shoot_control) {
            default:
              UNREACHABLE;
            case ENTITY_SHOOT_CONTROL_NONE:
              break;
          }

          /* flags stuff */

          if(ep->flags & ENTITY_FLAG_DYNAMICS) {
            Vector2 a_times_t = Vector2Scale(ep->accel, gp->timestep);
            ep->vel = Vector2Add(ep->vel, a_times_t);
            ep->pos = Vector2Add(ep->pos, Vector2Add(Vector2Scale(ep->vel, gp->timestep), Vector2Scale(a_times_t, 0.5*gp->timestep)));

            if(ep->flags & ENTITY_FLAG_DYNAMICS_HAS_CURVE) {
              ep->vel = Vector2Rotate(ep->vel, ep->curve);
            }

          }

          if(ep->flags & ENTITY_FLAG_APPLY_FRICTION) {
            ep->vel = Vector2Subtract(ep->vel, Vector2Scale(ep->vel, FRICTION*gp->timestep));
          }

          if(ep->flags & ENTITY_FLAG_CLAMP_POS_TO_SCREEN) {
            Vector2 pos_min = { ep->radius, ep->radius };
            Vector2 pos_max =
              Vector2Subtract((Vector2){ WINDOW_WIDTH, WINDOW_HEIGHT }, pos_min);
            ep->pos = Vector2Clamp(ep->pos, pos_min, pos_max);
          }

          if(ep->flags & ENTITY_FLAG_HAS_BULLET_EMITTER) {

            if(gp->state == GAME_STATE_MAIN_LOOP || gp->state == GAME_STATE_DEBUG_SANDBOX) {

              if(ep->bullet_emitter.shooting) {
                ep->bullet_emitter.shooting = 0;

                if(ep->bullet_emitter.cooldown_timer == 0.0f) {
                  ep->bullet_emitter.cooldown_timer = ep->bullet_emitter.cooldown_period;
                  entity_emit_bullets(gp, ep);
                }

              }

              if(ep->bullet_emitter.cooldown_timer < 0.0f) {
                ep->bullet_emitter.cooldown_timer = 0.0f;
              } else {
                ep->bullet_emitter.cooldown_timer -= gp->timestep;
              }

            }

          }

          if(ep->flags & ENTITY_FLAG_APPLY_COLLISION) {

            for(int i = 0; i < gp->entities_allocated; i++) {
              Entity *colliding = &gp->entities[i];

              if(colliding->live) {

                if(entity_kind_in_mask(colliding->kind, ep->apply_collision_mask)) {
                  if(entity_check_collision(gp, ep, colliding)) {
                    applied_collision = 1;
                    colliding->received_collision = 1;

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
              entity_die(gp, ep);
              goto entity_update_end;
            }
          }

          if(ep->flags & ENTITY_FLAG_DIE_ON_APPLY_COLLISION) {
            if(applied_collision) {
              entity_die(gp, ep);
              goto entity_update_end;
            }
          }

          if(ep->flags & ENTITY_FLAG_RECEIVE_COLLISION) {
            if(ep->received_collision) {
              ep->received_collision = 0;

              if(ep->flags & ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE) {
                ep->health -= ep->received_damage;
                ep->received_damage = 0;
                if(ep->health <= 0) {
                  entity_die(gp, ep);
                  goto entity_update_end;
                }
              }

            }
          }

          if(ep->flags & ENTITY_FLAG_HAS_SPRITE) {
            sprite_update(gp, &ep->sprite);
          }

          if(ep->flags & ENTITY_FLAG_NOT_ON_SCREEN) {
            if(CheckCollisionCircleRec(ep->pos, ep->radius, WINDOW_RECT)) {
              ep->flags ^= ENTITY_FLAG_NOT_ON_SCREEN | ENTITY_FLAG_ON_SCREEN;
            }
          }

          if(ep->flags & ENTITY_FLAG_DIE_IF_EXIT_SCREEN) {
            if(CheckCollisionCircleRec(ep->pos, ep->radius, WINDOW_RECT)) {
              ep->flags ^= ENTITY_FLAG_DIE_IF_EXIT_SCREEN | ENTITY_FLAG_DIE_IF_OFFSCREEN;
            }
          }

          if(ep->flags & ENTITY_FLAG_DIE_IF_OFFSCREEN) {
            if(!CheckCollisionCircleRec(ep->pos, ep->radius, WINDOW_RECT)) {
              entity_die(gp, ep);
              goto entity_update_end;
            }
          }

entity_update_end:;
        } /* entity_update */

      } /* update_entities */
    }

    gp->live_particles = 0;

    for(int i = 0; i < MAX_PARTICLES; i++) {
      Particle *p = &gp->particles[i];

      if(!p->live) continue;

      gp->live_particles++;

      { /* particle_update */
      } /* particle_update */

    }

update_end:;
  } /* update */

  defer_loop(BeginTextureMode(gp->render_texture), EndTextureMode())
  { /* draw */
    ClearBackground(SKYBLUE);

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
          Color tint = ep->sprite_tint;
          draw_sprite_ex(gp, ep->sprite, ep->pos, ep->sprite_scale, 0, tint);
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

      if(!p->live) continue;

      gp->live_particles++;

      { /* particle_draw */
      } /* particle_draw */

    }

  } /* draw */

  defer_loop(BeginDrawing(), EndDrawing())
  { /* draw to screen */

    ClearBackground(BLACK);

    int scale = (int)fminf((float)GetScreenWidth() / WINDOW_WIDTH,
        (float)GetScreenHeight() / WINDOW_HEIGHT);
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

    if(gp->debug_flags & GAME_DEBUG_FLAG_DEBUG_UI) { /* debug overlay */
      char *debug_text = frame_push_array(char, 256);
      char *debug_text_fmt =
        "auto_hot_reload: %s\n"
        "player_is_invincible: %s\n"
        "frame time: %.7f\n"
        "live entities count: %li\n"
        "most entities allocated: %li\n"
        "live particles count: %li\n"
        "particle_pos: %i\n"
        "screen width: %i\n"
        "screen height: %i\n"
        "player pos: { x = %f, y = %f }\n"
        "game state: %s";
      stbsp_sprintf(debug_text,
          debug_text_fmt,
          (gp->debug_flags & GAME_DEBUG_FLAG_HOT_RELOAD) ? "on" : "off",
          (gp->debug_flags & GAME_DEBUG_FLAG_PLAYER_INVINCIBLE) ? "on" : "off",
          gp->timestep,
          gp->live_entities,
          gp->entities_allocated,
          gp->live_particles,
          gp->particles_pos,
          GetScreenWidth(),
          GetScreenHeight(),
          gp->player->pos.x,
          gp->player->pos.y,
          Game_state_strings[gp->state]);
      Vector2 debug_text_size = MeasureTextEx(gp->font, debug_text, 18, 1.0);
      DrawText(debug_text, 10, GetScreenHeight() - debug_text_size.y - 10, 18, GREEN);
    } /* debug overlay */

  } /* draw to screen */


  gp->state = gp->next_state;

  gp->frame_index++;

  arena_clear(gp->frame_scratch);

}
