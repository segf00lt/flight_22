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

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1000, 800, "invaders");
  InitAudioDevice();

  SetTargetFPS(TARGET_FPS);
  SetTextLineSpacing(10);
  SetExitKey(0);

  context_init();

  Game *gp = os_alloc(sizeof(Game));

  { /* init game */
    memory_set(gp, 0, sizeof(Game));

    gp->state = GAME_STATE_NONE;
    gp->frame_scratch = arena_alloc();

    load_assets(gp);
  } /* init game */

  SetMasterVolume(GetMasterVolume() * 0.5);

  while(!WindowShouldClose()) {
    game_update_and_draw(gp);
  }

  unload_assets(gp);

  CloseWindow();
  CloseAudioDevice();

  return 0;
}
