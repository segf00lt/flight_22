#include <emscripten/emscripten.h>


#include "bullet_hell.h"


#include "bullet_hell.c"



void load_assets(Game *gp) {

  gp->font = GetFontDefault();

  gp->sprite_atlas = LoadTexture("./aseprite/atlas.png");

  gp->render_texture = LoadRenderTexture(WINDOW_WIDTH, WINDOW_HEIGHT);

}

void unload_assets(Game *gp) {

  UnloadRenderTexture(gp->render_texture);

  UnloadTexture(gp->sprite_atlas);

}

void wasm_main_loop(void *gp) {
  game_update_and_draw(gp);
}

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Flight 22");
  InitAudioDevice();

  SetTargetFPS(TARGET_FPS);
  SetTextLineSpacing(10);
  SetExitKey(0);

  context_init();

  Game *gp = MemAlloc(sizeof(Game));

  { /* init game */
    memset(gp, 0, sizeof(Game));

    gp->state = GAME_STATE_NONE;
    gp->frame_scratch = arena_alloc();

    load_assets(gp);
  } /* init game */

  SetMasterVolume(GetMasterVolume() * 0.5);

  if(!IsAudioDeviceReady()) {
    TraceLog(LOG_WARNING, "audio not initialized");
  }

  emscripten_set_main_loop_arg(wasm_main_loop, (void*)gp, TARGET_FPS, 1);

  unload_assets(gp);

  CloseWindow();
  CloseAudioDevice();

  return 0;
}
