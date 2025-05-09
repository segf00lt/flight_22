#include "bullet_hell.h"
#include <dlfcn.h>


#if defined(OS_MAC)
#define GAME_MODULE_PATH "./bullet_hell.dylib"
#elif defined(OS_LINUX)
#define GAME_MODULE_PATH "./bullet_hell.so"
#else
#error unsupported OS
#endif


typedef void (*Game_proc)(Game *);


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

  void *game_module = dlopen(GAME_MODULE_PATH, RTLD_NOW);
  s64 game_module_modtime = GetFileModTime(GAME_MODULE_PATH);

  Game_proc game_update_and_draw_proc = (Game_proc)dlsym(game_module, "game_update_and_draw");
  Game_proc game_init_proc            = (Game_proc)dlsym(game_module, "game_init");
  Game_proc game_unload_assets_proc   = (Game_proc)dlsym(game_module, "game_unload_assets");
  Game_proc game_load_assets_proc     = (Game_proc)dlsym(game_module, "game_load_assets");

  Game *gp = os_alloc(sizeof(Game));

  game_init_proc(gp);

  while(!WindowShouldClose()) {
    game_update_and_draw_proc(gp);

    if(gp->debug_flags & GAME_DEBUG_FLAG_HOT_RELOAD) {
      s64 modtime = GetFileModTime(GAME_MODULE_PATH);
      if(game_module_modtime != modtime) {
        game_module_modtime = modtime;
        TraceLog(LOG_DEBUG, "reloading game code");

        WaitTime(0.17f);
        ASSERT(!dlclose(game_module));

        game_module = dlopen(GAME_MODULE_PATH, RTLD_NOW);
        game_update_and_draw_proc = (Game_proc)dlsym(game_module, "game_update_and_draw");
        game_unload_assets_proc   = (Game_proc)dlsym(game_module, "game_unload_assets");
        game_load_assets_proc     = (Game_proc)dlsym(game_module, "game_load_assets");

        game_unload_assets_proc(gp);
        game_load_assets_proc(gp);

      }
    }

  }

  game_unload_assets_proc(gp);

  CloseWindow();
  CloseAudioDevice();

  return 0;
}
