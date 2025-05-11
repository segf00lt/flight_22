#include <emscripten/emscripten.h>


#include "bullet_hell.h"


#include "bullet_hell.c"

void wasm_main_loop(void *gp) {
  game_update_and_draw(gp);
}

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Flight 22");
  InitAudioDevice();

  //SetMasterVolume(GetMasterVolume() * 0.5);

  SetTargetFPS(TARGET_FPS);
  SetTextLineSpacing(10);
  SetTraceLogLevel(LOG_DEBUG);
  SetExitKey(0);

  //context_init();

  Game *gp = os_alloc(sizeof(Game));

  game_init(gp);

  if(!IsAudioDeviceReady()) {
    TraceLog(LOG_WARNING, "audio not initialized");
  }

  emscripten_set_main_loop_arg(wasm_main_loop, (void*)gp, TARGET_FPS, 1);

  game_unload_assets(gp);

  CloseWindow();
  CloseAudioDevice();

  return 0;
}
