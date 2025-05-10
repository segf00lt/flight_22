#include "bullet_hell.h"


#include "bullet_hell.c"


int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1000, 800, "Flight 22");
  InitAudioDevice();

  SetMasterVolume(GetMasterVolume() * 0.5);

  SetTargetFPS(TARGET_FPS);
  SetTextLineSpacing(10);
  SetExitKey(0);

  //context_init();

  Game *gp = os_alloc(sizeof(Game));

  game_init(gp);

  while(!WindowShouldClose()) {
    game_update_and_draw(gp);
  }

  game_unload_assets(gp);

  CloseWindow();
  CloseAudioDevice();

  return 0;
}
