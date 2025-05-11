
/////////////////////////
/// BEGIN GENERATED


/* sprite frames array */

const Sprite_frame __sprite_frames[41] =
{
  [0] = { .x = 480, .y = 256, .w = 16, .h = 16, },
  [1] = { .x = 480, .y = 272, .w = 16, .h = 16, },
  [2] = { .x = 496, .y = 256, .w = 16, .h = 16, },
  [3] = { .x = 128, .y = 256, .w = 32, .h = 32, },
  [4] = { .x = 160, .y = 256, .w = 32, .h = 32, },
  [5] = { .x = 192, .y = 256, .w = 32, .h = 32, },
  [6] = { .x = 0, .y = 0, .w = 128, .h = 128, },
  [7] = { .x = 128, .y = 0, .w = 128, .h = 128, },
  [8] = { .x = 256, .y = 0, .w = 128, .h = 128, },
  [9] = { .x = 384, .y = 0, .w = 128, .h = 128, },
  [10] = { .x = 0, .y = 288, .w = 10, .h = 10, },
  [11] = { .x = 496, .y = 282, .w = 10, .h = 10, },
  [12] = { .x = 496, .y = 272, .w = 10, .h = 10, },
  [13] = { .x = 224, .y = 256, .w = 32, .h = 32, },
  [14] = { .x = 256, .y = 256, .w = 32, .h = 32, },
  [15] = { .x = 288, .y = 256, .w = 32, .h = 32, },
  [16] = { .x = 320, .y = 256, .w = 32, .h = 32, },
  [17] = { .x = 352, .y = 256, .w = 32, .h = 32, },
  [18] = { .x = 384, .y = 256, .w = 32, .h = 32, },
  [19] = { .x = 416, .y = 256, .w = 32, .h = 32, },
  [20] = { .x = 32, .y = 256, .w = 32, .h = 32, },
  [21] = { .x = 96, .y = 256, .w = 32, .h = 32, },
  [22] = { .x = 448, .y = 256, .w = 32, .h = 32, },
  [23] = { .x = 0, .y = 256, .w = 32, .h = 32, },
  [24] = { .x = 64, .y = 128, .w = 64, .h = 64, },
  [25] = { .x = 128, .y = 128, .w = 64, .h = 64, },
  [26] = { .x = 192, .y = 128, .w = 64, .h = 64, },
  [27] = { .x = 256, .y = 128, .w = 64, .h = 64, },
  [28] = { .x = 320, .y = 128, .w = 64, .h = 64, },
  [29] = { .x = 384, .y = 128, .w = 64, .h = 64, },
  [30] = { .x = 448, .y = 128, .w = 64, .h = 64, },
  [31] = { .x = 0, .y = 192, .w = 64, .h = 64, },
  [32] = { .x = 64, .y = 192, .w = 64, .h = 64, },
  [33] = { .x = 128, .y = 192, .w = 64, .h = 64, },
  [34] = { .x = 0, .y = 128, .w = 64, .h = 64, },
  [35] = { .x = 192, .y = 192, .w = 64, .h = 64, },
  [36] = { .x = 256, .y = 192, .w = 64, .h = 64, },
  [37] = { .x = 320, .y = 192, .w = 64, .h = 64, },
  [38] = { .x = 384, .y = 192, .w = 64, .h = 64, },
  [39] = { .x = 448, .y = 192, .w = 64, .h = 64, },
  [40] = { .x = 64, .y = 256, .w = 32, .h = 32, },
};


/* keyframes */



/* sprites */

const Sprite SPRITE_SPITFIRE_IDLE = { .flags = 0 | SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 24, .last_frame = 27, .fps = 100, .total_frames = 4 };
const Sprite SPRITE_SPITFIRE_STRAFE_3 = { .flags = 0 | SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 28, .last_frame = 31, .fps = 100, .total_frames = 4 };
const Sprite SPRITE_SPITFIRE_STRAFE_2 = { .flags = 0 | SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 32, .last_frame = 35, .fps = 100, .total_frames = 4 };
const Sprite SPRITE_SPITFIRE_STRAFE_1 = { .flags = 0 | SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 36, .last_frame = 39, .fps = 100, .total_frames = 4 };
const Sprite SPRITE_AVENGER_BULLET = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 0, .last_frame = 2, .fps = 16, .total_frames = 3 };
const Sprite SPRITE_BOOM = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 3, .last_frame = 5, .fps = 10, .total_frames = 3 };
const Sprite SPRITE_BOSS_CRAB = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 6, .last_frame = 9, .fps = 8, .total_frames = 4 };
const Sprite SPRITE_BULLETS = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 10, .last_frame = 12, .fps = 8, .total_frames = 3 };
const Sprite SPRITE_CORONEL_FU = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 13, .last_frame = 16, .fps = 8, .total_frames = 4 };
const Sprite SPRITE_CRAB = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 17, .last_frame = 20, .fps = 10, .total_frames = 4 };
const Sprite SPRITE_DOUBLE_TROUBLE = { .flags = SPRITE_FLAG_STILL, .first_frame = 21, .last_frame = 21, .total_frames = 1 };
const Sprite SPRITE_HEALTH = { .flags = SPRITE_FLAG_STILL, .first_frame = 22, .last_frame = 22, .total_frames = 1 };
const Sprite SPRITE_QUINTUS = { .flags = SPRITE_FLAG_STILL, .first_frame = 23, .last_frame = 23, .total_frames = 1 };
const Sprite SPRITE_TRIPLE_THREAT = { .flags = SPRITE_FLAG_STILL, .first_frame = 40, .last_frame = 40, .total_frames = 1 };


/////////////////////////
/// END GENERATED

