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
    ENTITY_FLAG_CLAMP_POS_TO_SCREEN |
    ENTITY_FLAG_HAS_BULLET_EMITTER |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    0;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->look_dir = PLAYER_LOOK_DIR;
  ep->pos = PLAYER_INITIAL_POS;

  ep->health = PLAYER_HEALTH;

  ep->sprite = SPRITE_AVENGER;
  ep->sprite_tint = WHITE;
  ep->sprite_scale = PLAYER_SPRITE_SCALE;

  ep->bullet_emitter.kind = BULLET_EMITTER_KIND_AVENGER;
  ep->bullet_emitter.flags =
    0;

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

  ep->shoot_control = ENTITY_SHOOT_CONTROL_STRAIGHT_PERIODIC_BURSTS;
  ep->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_BASIC;

  ep->shooting_pause = CRAB_BURST_PAUSE;

  ep->bounds_color = CRAB_BOUNDS_COLOR;
  ep->sprite = SPRITE_CRAB;
  ep->sprite_scale = CRAB_SPRITE_SCALE;
  ep->sprite_tint = YELLOW;

  ep->radius = CRAB_BOUNDS_RADIUS;

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

// TODO particles
void entity_emit_particles(Game *gp, Entity *ep) {
  UNIMPLEMENTED;

  Particle_emitter *emitter = &ep->particle_emitter;

  switch(emitter->kind) {
    default:
      UNREACHABLE;
    case PARTICLE_EMITTER_KIND_PUFF:
      {
        s32 n_particles = GetRandomValue(40, 60);

        for(int i = 0; i < n_particles; i++) {

        }

      } break;
  }


}

void entity_emit_bullets(Game *gp, Entity *ep) {
  Bullet_emitter *emitter = &ep->bullet_emitter;

  if(!emitter->cocked) {
    switch(emitter->kind) {
      default:
        UNREACHABLE;
      case BULLET_EMITTER_KIND_AVENGER:
        {

          emitter->bullet_collision_mask = PLAYER_APPLY_COLLISION_MASK;
          emitter->cooldown_period[0] = AVENGER_NORMAL_FIRE_COOLDOWN;
          emitter->active_rings_mask = 0x1;

          emitter->shots[0] = 1;
          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->spin_vel = 0.0f;
            ring->radius = 60.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 0;
            ring->n_bullets = 1;
            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = 15;
            ring->bullet_vel = AVENGER_NORMAL_BULLET_VELOCITY;
            ring->bullet_damage = AVENGER_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_sprite = SPRITE_AVENGER_BULLET;
            ring->bullet_sprite_tint = WHITE;
            ring->bullet_sprite_scale = 3.0f;
            ring->bullet_flags =
              ENTITY_FLAG_HAS_SPRITE |
              0;

          }

#if 0
          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->spin_vel = 0.1f;
            ring->radius = 50.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 2.0f/3.0f;
            ring->n_bullets = 3;
            ring->bullet_arm_width = 50.0f;
            ring->bullet_radius = AVENGER_NORMAL_BULLET_BOUNDS_RADIUS;
            ring->bullet_vel = AVENGER_NORMAL_BULLET_VELOCITY;
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
            ring->bullet_vel = AVENGER_NORMAL_BULLET_VELOCITY;
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
            ring->bullet_vel = AVENGER_NORMAL_BULLET_VELOCITY;
            ring->bullet_damage = AVENGER_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_fill_color = ORANGE;
            ring->bullet_flags =
              ENTITY_FLAG_FILL_BOUNDS |
              0;

          }

#endif

        } break;
      case BULLET_EMITTER_KIND_ORBIT_AND_SNIPE:
        {
          Entity *player = entity_from_handle(gp->player_handle);
          ASSERT(player);

          emitter->rings[0].flags |=
            BULLET_EMITTER_RING_FLAG_MANUALLY_SET_DIR |
            BULLET_EMITTER_RING_FLAG_BURST |
            0;
          emitter->rings[0].burst_shots = 3;
          emitter->rings[0].burst_cooldown = 0.03f;
          emitter->rings[0].burst_shots_fired = 0;
          emitter->rings[0].burst_timer = 0;
          emitter->rings[0].dir = Vector2Normalize(Vector2Subtract(player->pos, ep->pos));

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->cooldown_period[0] = 1.0f;
          emitter->cooldown_period[1] = 2.0f;
          emitter->active_rings_mask = 0x3;

          emitter->shots[0] = 5;
          emitter->shots[1] = 2;

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->spin_vel = 0.0f;
            ring->radius = 60.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 0;
            ring->n_bullets = 1;
            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;
            ring->bullet_vel = CRAB_NORMAL_BULLET_VELOCITY;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_fill_color = ORANGE;
            ring->bullet_flags =
              ENTITY_FLAG_FILL_BOUNDS |
              0;
          }

          {
            Bullet_emitter_ring *ring = &emitter->rings[1];

            ring->spin_vel = 0.2f;
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
            ring->bullet_fill_color = ORANGE;
            ring->bullet_flags =
              ENTITY_FLAG_FILL_BOUNDS |
              ENTITY_FLAG_DYNAMICS_HAS_CURVE |
              0;
          }

        } break;
      case BULLET_EMITTER_KIND_CRAB_BASIC:
        {

          Entity *player = entity_from_handle(gp->player_handle);
          ASSERT(player);

          emitter->bullet_collision_mask = ENTITY_KIND_MASK_PLAYER;
          emitter->active_rings_mask = 0x1;

          emitter->rings[0].flags |=
            BULLET_EMITTER_RING_FLAG_MANUALLY_SET_DIR |
            0;
          emitter->rings[0].dir = Vector2Normalize(Vector2Subtract(player->pos, ep->pos));

          emitter->cooldown_period[0] = 0.8f;
          emitter->shots[0] = 4;

          {
            Bullet_emitter_ring *ring = &emitter->rings[0];

            ring->spin_vel = 0.0f;
            ring->radius = 60.0f;
            ring->n_arms = 1;
            ring->arms_occupy_circle_sector_percent = 0;
            ring->n_bullets = 1;
            ring->bullet_arm_width = 0.0f;
            ring->bullet_radius = CRAB_NORMAL_BULLET_BOUNDS_RADIUS;
            ring->bullet_vel = CRAB_NORMAL_BULLET_VELOCITY;
            ring->bullet_damage = CRAB_NORMAL_BULLET_DAMAGE;
            ring->bullet_bounds_color = GREEN;
            ring->bullet_fill_color = ORANGE;
            ring->bullet_flags =
              ENTITY_FLAG_FILL_BOUNDS |
              0;

          }

        } break;
    }

    emitter->cocked = 1;

  }

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

      if(ring->flags & BULLET_EMITTER_RING_FLAG_MANUALLY_SET_DIR) {
        ASSERT(Vector2LengthSqr(ring->dir) > 0);
      } else {
        ring->dir = ep->look_dir;
      }

      if(ring->n_arms == 1) {
        arms_occupy_circle_sector_angle = 0;
        arm_step_angle = 0;
        arm_dir = Vector2Rotate(ring->dir, ring->spin_cur_angle + ring->spin_start_angle);
      } else {
        ASSERT(ring->arms_occupy_circle_sector_percent > 0);

        arms_occupy_circle_sector_angle =
          (2*PI) * ring->arms_occupy_circle_sector_percent;
        arm_step_angle = arms_occupy_circle_sector_angle / (float)(ring->n_arms-1);
        arm_dir =
          Vector2Rotate(
              ring->dir,
              -0.5*arms_occupy_circle_sector_angle + ring->spin_cur_angle + ring->spin_start_angle);
      }

      for(int arm_i = 0; arm_i < ring->n_arms; arm_i++) {

        Vector2 step_dir = {0};
        Vector2 bullet_pos = Vector2Add(ep->pos, Vector2Scale(arm_dir, ring->radius));
        Vector2 positions[MAX_BULLETS_IN_BAG];

        if(ring->flags & BULLET_EMITTER_RING_FLAG_USE_POINT_BAG) {

          // TODO test bullet point bag
          TraceLog(LOG_WARNING, "flag USE_POINT_BAG is untested");

          ASSERT(ring->n_bullets < MAX_BULLETS_IN_BAG);

          Vector2 arm_dir_perp = { arm_dir.y, -arm_dir.x };
          Matrix m =
          {
            .m0 = arm_dir_perp.x, .m4 = arm_dir.x,
            .m1 = arm_dir_perp.x, .m5 = arm_dir.x,
          };

          for(int i = 0; i < ring->n_bullets; i++) {
            positions[i] = Vector2Transform(ring->bullet_point_bag[i], m);
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

          bullet->kind = ENTITY_KIND_BULLET;
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

          bullet->vel = Vector2Add(bullet->vel, Vector2Scale(ring->dir, Vector2DotProduct(ring->dir, ep->vel)));

          bullet->curve = ring->bullet_curve;
          bullet->curve_rolloff_vel = ring->bullet_curve_rolloff_vel;

          bullet->radius = ring->bullet_radius;

          bullet->apply_collision_mask = ep->bullet_emitter.bullet_collision_mask;
          bullet->damage_amount = ring->bullet_damage;

          bullet->pos = bullet_pos;

          bullet_pos = Vector2Add(bullet_pos, step_dir);

        } /* bullet loop */

        arm_dir = Vector2Rotate(arm_dir, arm_step_angle);

      } /* arm loop */

      ring->spin_cur_angle += ring->spin_vel;
      if(ring->spin_cur_angle >= 2*PI) {
        ring->spin_cur_angle = 0;
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
  arena_clear(gp->wave_scratch);

  gp->background_y_offset = 0;

  memory_set(&gp->phase, 0, sizeof(gp->phase));
  gp->phase_index = 0;
  gp->wave = 0;

}

void game_wave_end(Game *gp) {
  memory_set(&gp->phase, 0, sizeof(gp->phase));
  gp->phase_index = 0;
  gp->wave++;
  gp->next_state = GAME_STATE_WAVE_TRANSITION;
  gp->flags |= GAME_FLAG_PLAYER_CANNOT_SHOOT;
  gp->wave_timer = WAVE_DELAY_TIME;
  gp->wave_type_char_timer = WAVE_BANNER_TYPE_SPEED;
  gp->wave_chars_typed = 0;
  gp->wave_banner_msg = push_str8f(gp->wave_scratch, "WAVE %i", gp->wave+1);
  gp->wave_banner_type_dir = 1;
  gp->wave_banner_target_msg_len = gp->wave_banner_msg.len;

  arena_clear(gp->wave_scratch);
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

              {
                Entity *leader = spawn_crab_leader(gp);
                Entity_handle leader_handle = handle_from_entity(leader);
                leader->child_list = push_entity_list(gp);
                leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                leader->vel = (Vector2){ .x = CRAB_NORMAL_STRAFE_SPEED };
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
              gp->phase.timer[0] = 2.0f;

              const int crabs_per_row = 2;

              {
                Entity *leader = spawn_crab_leader(gp);
                Entity_handle leader_handle = handle_from_entity(leader);
                leader->child_list = push_entity_list(gp);
                leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                leader->vel = (Vector2){ .x = CRAB_NORMAL_STRAFE_SPEED };
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

                  crab->pos = pos;
                  crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                  crab->leader_handle = leader_handle;
                  entity_list_append(gp, leader->child_list, crab);

                }

              }

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
        case 2:
          {

            if(gp->phase.check_if_finished == 0) {
              gp->phase.check_if_finished = 1;
              gp->phase.timer[0] = 1.0f;

              { /* spawn_orbiting_crabs */
                Entity *leader = spawn_crab_leader(gp);
                Entity_handle leader_handle = handle_from_entity(leader);
                leader->child_list = push_entity_list(gp);
                leader->move_control = ENTITY_MOVE_CONTROL_LEADER_HORIZONTAL_STRAFE;
                leader->vel = (Vector2){ .x = -CRAB_NORMAL_STRAFE_SPEED };
                leader->leader_strafe_padding = 50;
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

                  crab->leader_handle = leader_handle;
                  entity_list_append(gp, leader->child_list, crab);

                }

                Entity *crab = spawn_crab(gp);
                crab->pos = leader->pos;
                crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                crab->shoot_control = ENTITY_SHOOT_CONTROL_ORBIT_AND_SNIPE;
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
        default:
          goto wave_end;
      }
      break;

    case 1:
      switch(gp->phase_index) {
        case 0:
          {
            const int ENEMIES_COUNT = 20;
            const Vector2 INITIAL_POS = { .x = WINDOW_WIDTH * 0.2f, .y = WINDOW_HEIGHT * 0.2f };
            const float ENEMY_SPAWN_DELAY = 0.6f;
            const float SECOND_GROUP_DELAY = 2.6f;
            const float THIRD_GROUP_DELAY = 1.1f;

            const u32 CREATED_WAYPOINTS    = 1u<<0;
            const u32 SPAWNED_SECOND_GROUP = 1u<<1;
            const u32 SPAWNED_THIRD_GROUP  = 1u<<2;

            f32 *enemy_snake_spawn_timer = &gp->phase.timer[0];
            f32 *second_enemy_group_spawn_delay = &gp->phase.timer[1];
            f32 *third_enemy_group_spawn_delay = &gp->phase.timer[2];


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
                gp->phase.accumulator = 0;

                if(gp->live_enemies == 0) {
                  goto phase_end;
                }

              } else {

                if(gp->phase.accumulator >= ENEMIES_COUNT) {
                  gp->phase.check_if_finished = 1;
                } else {

                  if(*enemy_snake_spawn_timer >= ENEMY_SPAWN_DELAY) {
                    *enemy_snake_spawn_timer = 0;
                    gp->phase.accumulator++;

                    Entity *crab = spawn_crab(gp);
                    crab->pos = INITIAL_POS;
                    crab->move_control = ENTITY_MOVE_CONTROL_GOTO_WAYPOINT;
                    crab->waypoints = *waypoints;
                    crab->scalar_vel = 200.0f;

                  } else {
                    *enemy_snake_spawn_timer += gp->timestep;
                  }

                  if(!(gp->phase.flags & SPAWNED_SECOND_GROUP)) {
                    if(*second_enemy_group_spawn_delay >= SECOND_GROUP_DELAY) {

                      gp->phase.flags |= SPAWNED_SECOND_GROUP;

                      { /* spawn_orbiting_crabs */
                        Entity *leader = spawn_crab_leader(gp);
                        Entity_handle leader_handle = handle_from_entity(leader);
                        leader->child_list = push_entity_list(gp);
                        leader->vel = (Vector2){ .x = 1, .y = 0 };
                        leader->vel = Vector2Scale(Vector2Rotate(leader->vel, (5.0f*PI)/7.0f), 200);
                        leader->pos =
                          (Vector2){
                            .x = WINDOW_WIDTH * 1.4,
                            .y = WINDOW_HEIGHT * -0.50,
                          };

                        int crab_count = 4;
                        float radius = 110;
                        float step_angle = (2*PI) / (float)crab_count;
                        Vector2 arm = ORBIT_ARM;

                        for(int i = 0; i < 6; i++) {
                          Vector2 pos =
                            Vector2Add(leader->pos, Vector2Scale(Vector2Rotate(arm, (float)i*step_angle), radius));

                          Entity *crab = spawn_crab(gp);

                          crab->pos = pos;
                          crab->move_control = ENTITY_MOVE_CONTROL_ORBIT_LEADER;
                          crab->shoot_control = ENTITY_SHOOT_CONTROL_NONE;

                          crab->orbit_cur_angle = (float)i*step_angle;
                          crab->orbit_speed = -PI*0.15;
                          crab->orbit_radius = radius;

                          crab->leader_handle = leader_handle;
                          entity_list_append(gp, leader->child_list, crab);

                        }

                        Entity *crab = spawn_crab(gp);
                        crab->pos = leader->pos;
                        crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                        crab->shoot_control = ENTITY_SHOOT_CONTROL_ORBIT_AND_SNIPE;
                        crab->leader_handle = leader_handle;
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
                        Entity *leader = spawn_crab_leader(gp);
                        Entity_handle leader_handle = handle_from_entity(leader);
                        leader->child_list = push_entity_list(gp);
                        leader->vel = (Vector2){ .x = -1, .y = 0 };
                        leader->vel = Vector2Scale(Vector2Rotate(leader->vel, (-5.0f*PI)/6.0f), 200);
                        leader->pos =
                          (Vector2){
                            .x = WINDOW_WIDTH * -0.6,
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

                          crab->pos = pos;
                          crab->move_control = ENTITY_MOVE_CONTROL_ORBIT_LEADER;
                          crab->shoot_control = ENTITY_SHOOT_CONTROL_NONE;

                          crab->orbit_cur_angle = (float)i*step_angle;
                          crab->orbit_speed = PI*0.15;
                          crab->orbit_radius = radius;

                          crab->leader_handle = leader_handle;
                          entity_list_append(gp, leader->child_list, crab);

                        }

                        Entity *crab = spawn_crab(gp);
                        crab->pos = leader->pos;
                        crab->move_control = ENTITY_MOVE_CONTROL_COPY_LEADER;
                        crab->shoot_control = ENTITY_SHOOT_CONTROL_ORBIT_AND_SNIPE;
                        crab->leader_handle = leader_handle;
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
        case 1:
          {
          } break;
        case 2:
          {
          } break;
      }
      break;

    default:
      gp->next_state = GAME_STATE_VICTORY;
  }

  goto end;

phase_end:
  memory_set(&gp->phase, 0, sizeof(gp->phase));
  gp->phase_index++;
  goto end;

wave_end:
  game_wave_end(gp);
  goto end;

end:
  if(!gp->player->live) {
    gp->next_state = GAME_STATE_GAME_OVER;
  }

}

void game_update_and_draw(Game *gp) {

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

    if(IsKeyDown(KEY_LEFT_SHIFT)) {
      gp->input_flags |= INPUT_FLAG_SLOW_MOVE;
    }

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

    if(gp->background_y_offset >= WINDOW_HEIGHT) {
      gp->background_y_offset -= WINDOW_HEIGHT;
    } else {
      gp->background_y_offset += gp->timestep * 100;
    }

    switch(gp->state) {
      default:
        UNREACHABLE;
      case GAME_STATE_NONE:
        {
          //gp->next_state = GAME_STATE_DEBUG_SANDBOX;
          //gp->next_state = GAME_STATE_MAIN_LOOP;

          gp->player = spawn_player(gp);
          gp->player_handle = handle_from_entity(gp->player);

          {
#ifdef DEBUG
            gp->wave = 0;
            gp->phase_index = 0;

            gp->debug_flags |=
              GAME_DEBUG_FLAG_DEBUG_UI |
              GAME_DEBUG_FLAG_PLAYER_INVINCIBLE |
              0;
#else

            gp->wave = 0;
            gp->phase_index = 0;

#endif
            gp->next_state = GAME_STATE_WAVE_TRANSITION;

            memory_set(&gp->phase, 0, sizeof(gp->phase));

            gp->flags |= GAME_FLAG_PLAYER_CANNOT_SHOOT;
            gp->wave_timer = WAVE_DELAY_TIME;
            gp->wave_type_char_timer = WAVE_BANNER_TYPE_SPEED;
            gp->wave_chars_typed = 0;
            gp->wave_banner_msg = push_str8f(gp->wave_scratch, "WAVE %i", gp->wave+1);
            gp->wave_banner_type_dir = 1;
            gp->wave_banner_target_msg_len = gp->wave_banner_msg.len;
          }

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
                gp->wave_type_char_timer = WAVE_BANNER_TYPE_SPEED;
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

                if(gp->flags & GAME_FLAG_DO_THE_CRAB_WALK) {
                  ep->accel.y = 0;
                  ep->vel.y = 0;
                }

                if(gp->input_flags & INPUT_FLAG_SHOOT) {
                  if(!(gp->flags & GAME_FLAG_PLAYER_CANNOT_SHOOT)) {
                    ep->bullet_emitter.shoot = 1;
                  }
                }

                if(gp->debug_flags & GAME_DEBUG_FLAG_PLAYER_INVINCIBLE) {
                  ep->received_damage = 0;
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
              case ENTITY_SHOOT_CONTROL_ORBIT_AND_SNIPE:
                {
                  if(ep->start_shooting_delay < 0.6f) {
                    ep->start_shooting_delay += gp->timestep;
                  } else {
                    ep->bullet_emitter.kind = BULLET_EMITTER_KIND_ORBIT_AND_SNIPE;

                    if(ep->bullet_emitter.shoot <= 0) {
                      if(ep->shooting_timer <= 0) {
                        ep->shooting_timer = 1.0f;
                        ep->bullet_emitter.shoot = 5;
                      } else {
                        ep->shooting_timer -= gp->timestep;
                      }
                    }
                  }

                } break;
              case ENTITY_SHOOT_CONTROL_STRAIGHT_PERIODIC_BURSTS:
                {

                  if(ep->start_shooting_delay < 0.6f) {
                    ep->start_shooting_delay += gp->timestep;
                  } else {
                    ep->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB_BASIC;

                    if(ep->bullet_emitter.shoot <= 0) {
                      if(ep->shooting_timer <= 0) {
                        ep->shooting_timer = 2.6f;
                        ep->bullet_emitter.shoot = 1;
                      } else {
                        ep->shooting_timer -= gp->timestep;
                      }
                    }
                  }

                } break;
            }
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
              Vector2Subtract(WINDOW_SIZE, pos_min);
            ep->pos = Vector2Clamp(ep->pos, pos_min, pos_max);
          }

          if(ep->flags & ENTITY_FLAG_HAS_BULLET_EMITTER) {
            entity_emit_bullets(gp, ep);
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
              entity_die(gp, ep);
              goto entity_update_end;
            }
          }

          if(ep->flags & ENTITY_FLAG_DIE_NOW) {
            entity_die(gp, ep);
            goto entity_update_end;
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
    ClearBackground(BLACK);

    {
      Rectangle source =
      {
        0, 0, gp->background_texture.width, gp->background_texture.height,
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
    }

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
          //Color tint = ep->sprite_tint;
          draw_sprite(gp, ep);
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

      { /* particle_draw */
      } /* particle_draw */

    }

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

      wave_scope_end(scope);
    }

    if(gp->flags & GAME_FLAG_PAUSE) {
      Color c = { .a = 100 };
      DrawRectangleRec(WINDOW_RECT, c);

      char *paused_msg = "PAUSED";
      char *hint_msg = "press any key to resume";
      int pause_font_size = 90;
      int hint_font_size = 20;

      Color pause_color = GOLD;
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
        "player pos: { x = %f, y = %f }\n"
        "wave: %i\n"
        "phase: %i\n"
        "game state: %s";
      stbsp_sprintf(debug_text,
          debug_text_fmt,
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
          gp->player->pos.x,
          gp->player->pos.y,
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
