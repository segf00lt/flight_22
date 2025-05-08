#include "bullet_hell.h"
#include <dlfcn.h>


#if defined(OS_MAC)
#define GAME_MODULE_PATH "./bullet_hell.dylib"
#elif defined(OS_LINUX)
#define GAME_MODULE_PATH "./bullet_hell.so"
#else
#error unsupported OS
#endif


typedef void (*Game_update_and_draw_proc)(Game *);


void load_assets(Game *gp) {
  gp->font = GetFontDefault();

  gp->render_texture = LoadRenderTexture(WINDOW_WIDTH, WINDOW_HEIGHT);
  gp->background_texture = LoadTexture("./sprites/islands.png");
  gp->sprite_atlas = LoadTexture("./aseprite/atlas.png");
  SetTextureFilter(gp->sprite_atlas, TEXTURE_FILTER_POINT);
}

void unload_assets(Game *gp) {
  UnloadRenderTexture(gp->render_texture);
  UnloadTexture(gp->sprite_atlas);
  UnloadTexture(gp->background_texture);
}

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1000, 800, "Flight 22");
  InitAudioDevice();

  SetMasterVolume(GetMasterVolume() * 0.5);

  SetTargetFPS(TARGET_FPS);
  SetTextLineSpacing(10);
  SetTraceLogLevel(LOG_DEBUG);
  SetExitKey(0);

  //context_init();

  Game *gp = os_alloc(sizeof(Game));

  { /* TODO game_init() */
    memory_set(gp, 0, sizeof(Game));

    gp->entities = os_alloc(sizeof(Entity) * MAX_ENTITIES);
    gp->particles = os_alloc(sizeof(Particle) * MAX_PARTICLES);
    memory_set(gp->entities, 0, sizeof(Entity) * MAX_ENTITIES);
    memory_set(gp->particles, 0, sizeof(Particle) * MAX_PARTICLES);

    gp->state = GAME_STATE_NONE;
    gp->scratch = arena_alloc();
    gp->wave_scratch = arena_alloc(.size = KB(16));
    gp->frame_scratch = arena_alloc(.size = KB(8));

    load_assets(gp);

    gp->debug_flags |= GAME_DEBUG_FLAG_HOT_RELOAD;
  } /* game_init() */

  void *game_module = dlopen(GAME_MODULE_PATH, RTLD_NOW);
  Game_update_and_draw_proc game_update_and_draw_proc = (Game_update_and_draw_proc)dlsym(game_module, "game_update_and_draw");
  s64 game_module_modtime = GetFileModTime(GAME_MODULE_PATH);


  while(!WindowShouldClose()) {
    game_update_and_draw_proc(gp);

    if(gp->debug_flags & GAME_DEBUG_FLAG_HOT_RELOAD) {
      s64 modtime = GetFileModTime(GAME_MODULE_PATH);
      if(game_module_modtime != modtime) {
        game_module_modtime = modtime;
        TraceLog(LOG_DEBUG, "reloading game code");

        unload_assets(gp);
        load_assets(gp);

        WaitTime(0.17f);
        ASSERT(!dlclose(game_module));
        game_module = dlopen(GAME_MODULE_PATH, RTLD_NOW);
        game_update_and_draw_proc = (Game_update_and_draw_proc)dlsym(game_module, "game_update_and_draw");

      }
    }

  }

  unload_assets(gp);

  CloseWindow();
  CloseAudioDevice();

  return 0;
}
