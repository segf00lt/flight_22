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

Entity* entity_spawn(Game *gp) {
  Entity *ep = 0;

  if(gp->entity_free_list) {
    ep = gp->entity_free_list;
    gp->entity_free_list = ep->free_list_next;
  } else {
    ASSERT(gp->entities_allocated < MAX_ENTITIES);
    ep = &gp->entities[gp->entities_allocated++];
  }

  *ep =
    (Entity){
      .live = 1,
      .genid = gp->frame_index,
    };

  return ep;
}

void entity_die(Game *gp, Entity *ep) {
  ep->free_list_next = gp->entity_free_list;
  gp->entity_free_list = ep;
  ep->live = 0;
}

Entity* entity_spawn_player(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->control = ENTITY_CONTROL_PLAYER;
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

Entity* entity_spawn_crab(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->control = ENTITY_CONTROL_CRAB;
  ep->kind = ENTITY_KIND_CRAB;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_APPLY_FRICTION |
    //ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_FILL_BOUNDS |
    //ENTITY_FLAG_CLAMP_POS_TO_SCREEN |
    ENTITY_FLAG_HAS_BULLET_EMITTER |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    0;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->pos = CRAB_INITIAL_DEBUG_POS;

  ep->health = CRAB_HEALTH;

  //ep->sprite = SPRITE_AVENGER;
  //ep->sprite_tint = WHITE;
  //ep->sprite_scale = PLAYER_SPRITE_SCALE;

  ep->bullet_emitter.kind = BULLET_EMITTER_KIND_CRAB;
  ep->bullet_emitter.flags =
    0;

  ep->bullet_emitter.cooldown_period = CRAB_FIRE_COOLDOWN;

  ep->fill_color = MAROON;
  ep->bounds_color = CRAB_BOUNDS_COLOR;

  ep->radius = CRAB_BOUNDS_RADIUS;

  return ep;
}

Entity* entity_spawn_fish(Game *gp) {
  UNIMPLEMENTED;
  return 0;
}

Entity* entity_spawn_stingray(Game *gp) {
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

        for(int i = 0; i < emitter->n_rings; i++) {
          Bullet_emitter_ring *ring = &emitter->rings[i];

          // TODO test more
          ring->spin_vel = 0.3f;
          ring->radius = 70.0f;
          ring->n_arms = 2;
          ring->arms_occupy_circle_sector_percent = 1.0f/2.0f;
          ring->n_bullets = 6;
          ring->bullet_arm_width = 60.0f;
          ring->bullet_radius = AVENGER_NORMAL_BULLET_BOUNDS_RADIUS;
          ring->bullet_vel = AVENGER_NORMAL_BULLET_VELOCITY;
          ring->bullet_damage = AVENGER_NORMAL_BULLET_DAMAGE;
          ring->bullet_bounds_color = GREEN;
          ring->bullet_fill_color = ORANGE;
          ring->bullet_flags =
            ENTITY_FLAG_FILL_BOUNDS |
            0;

        }

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
        arm_dir = Vector2Rotate(ep->look_dir, ring->spin_cur_angle);
      } else {
        ASSERT(ring->arms_occupy_circle_sector_percent > 0);

        arms_occupy_circle_sector_angle =
          (2*PI) * ring->arms_occupy_circle_sector_percent;
        arm_step_angle = arms_occupy_circle_sector_angle / (float)(ring->n_arms-1);
        arm_dir =
          Vector2Rotate(
              ep->look_dir,
              -0.5*arms_occupy_circle_sector_angle + ring->spin_cur_angle);
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
          bullet->control = ENTITY_CONTROL_AVENGER_BULLET;

          bullet->flags = DEFAULT_BULLET_FLAGS | ring->bullet_flags;

          bullet->bounds_color = ring->bullet_bounds_color;
          bullet->fill_color = ring->bullet_fill_color;

          bullet->vel = Vector2Scale(arm_dir, ring->bullet_vel);
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
    .x = pos.x - scale*0.5f*source_rec.width,
    .y = pos.y - scale*0.5f*source_rec.height,
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

  arena_clear(gp->frame_scratch);

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

    switch(gp->state) {
      default:
        UNREACHABLE;
      case GAME_STATE_NONE:
        {
          gp->next_state = GAME_STATE_DEBUG_SANDBOX;

          gp->player = entity_spawn_player(gp);

          entity_spawn_crab(gp);

          goto update_end;

        } break;
      case GAME_STATE_TITLE_SCREEN:
        {
        } break;
      case GAME_STATE_SPAWN_PLAYER:
        {
        } break;
      case GAME_STATE_MAIN_LOOP:
        {
        } break;
      case GAME_STATE_GAME_OVER:
        {
        } break;
      case GAME_STATE_DEBUG_SANDBOX:
        {
        } break;
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

          switch(ep->control) {
            default:
              UNREACHABLE;
            case ENTITY_CONTROL_AVENGER_BULLET:
              {

              } break;
            case ENTITY_CONTROL_CRAB:
              {
              } break;
            case ENTITY_CONTROL_PLAYER:
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

  defer_loop((BeginTextureMode(gp->render_texture), ClearBackground(DARKBLUE)), EndTextureMode())
  { /* draw */

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
          bounds_color.a = 255;
          DrawCircleLinesV(ep->pos, ep->radius, bounds_color);

          DrawCircleV(ep->pos, 2.0f, ep->bounds_color);
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

    float scale = fminf((float)GetScreenWidth() / WINDOW_WIDTH,
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
