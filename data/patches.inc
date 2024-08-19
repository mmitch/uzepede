/*
 * uzepede - a centipede clone for uzebox
 *
 * Copyright (C) 2012-2024 by  Christian Garbs <mitch@cgarbs.de>
 * licensed under the GNU GPL v3 or later
 *
 * see https://github.com/mmitch/uzepede
 *
 */

const char fx_titlescreen[] PROGMEM ={ 
0,PC_WAVE,0,
0,PC_NOTE_DOWN,24,
0,PC_ENV_VOL,0,
0,PC_TREMOLO_LEVEL,64,
0,PC_TREMOLO_RATE,64,
0,PC_ENV_SPEED,1,
128,PC_ENV_SPEED,0,
32,PC_ENV_SPEED,-1,
0,PATCH_END
};

const char fx_shot[] PROGMEM ={ 
0,PC_WAVE,2,
0,PC_ENV_SPEED,-16,
0,PC_SLIDE_SPEED,12,
0,PC_SLIDE,-24,
8,PC_ENV_SPEED,-32,
0,PATCH_END
};

const char fx_mushroom[] PROGMEM ={ 
0,PC_WAVE,4,
0,PC_NOTE_DOWN,36,
2,PC_NOTE_CUT,0,
0,PATCH_END
};

const char fx_start[] PROGMEM ={ 
0,PC_WAVE,8,
0,PC_ENV_SPEED,-2,
8,PC_NOTE_DOWN,12,
8,PC_NOTE_UP,12,
8,PC_NOTE_DOWN,12,
8,PC_NOTE_UP,12,
8,PC_NOTE_DOWN,12,
8,PC_NOTE_UP,12,
8,PC_NOTE_DOWN,12,
8,PC_NOTE_UP,12,
8,PC_NOTE_DOWN,12,
8,PC_NOTE_UP,12,
8,PC_NOTE_DOWN,12,
8,PC_NOTE_UP,12,
8,PC_NOTE_DOWN,12,
8,PC_NOTE_UP,12,
0,PATCH_END
};

const char fx_gameover1[] PROGMEM ={ 
0,PC_WAVE,1,
0,PC_NOTE_DOWN,40,
0,PC_SLIDE_SPEED,64,
0,PC_SLIDE,-4,
0,PC_ENV_SPEED,-1,
64,PC_NOTE_CUT,0,
0,PATCH_END
};

const char fx_gameover2[] PROGMEM ={ 
0,PC_NOISE_PARAMS,1,
0,PC_ENV_SPEED,-4,
64,PC_NOTE_CUT,0,
0,PATCH_END
};

const char fx_wormhead[] PROGMEM ={ 
0,PC_NOISE_PARAMS,1,
0,PC_ENV_VOL,0,
0,PC_ENV_SPEED,8,
32,PC_NOTE_CUT,0,
0,PATCH_END
};

const char fx_wormbody[] PROGMEM ={ 
0,PC_NOISE_PARAMS,1,
0,PC_ENV_SPEED,-32,
0,PATCH_END
};

const char fx_spider_fall[] PROGMEM ={ 
0,PC_WAVE,4,
0,PC_NOTE_UP,24,
0,PC_SLIDE_SPEED,64,
0,PC_SLIDE,-24,
0,PC_ENV_SPEED,-4,
0,PATCH_END
};

const char fx_spider[] PROGMEM ={ 
0,PC_NOISE_PARAMS,1,
0,PC_ENV_VOL,0,
0,PC_ENV_SPEED,64,
4,PC_ENV_SPEED,-8,
0,PATCH_END
};

#define FX_TITLESCREEN 0
#define FX_START 1
#define FX_SHOT 2
#define FX_MUSHROOM 3
#define FX_GAMEOVER1 4
#define FX_GAMEOVER2 5
#define FX_WORMHEAD 6
#define FX_WORMBODY 7
#define FX_SPIDERFALL 8
#define FX_SPIDER 9

const struct PatchStruct patches[] PROGMEM = {
{0,NULL,fx_titlescreen,0,0},
{0,NULL,fx_start,0,0},
{0,NULL,fx_shot,0,0},
{0,NULL,fx_mushroom,0,0},
{0,NULL,fx_gameover1,0,0},
{1,NULL,fx_gameover2,0,0},
{1,NULL,fx_wormhead,0,0},
{1,NULL,fx_wormbody,0,0},
{0,NULL,fx_spider_fall,0,0},
{1,NULL,fx_spider,0,0},
};
