/*
 * uzepede - a centipede clone for uzebox
 *
 * Copyright (C) 2012-2024 by  Christian Garbs <mitch@cgarbs.de>
 * licensed under the GNU GPL v3 or later
 *
 * see https://github.com/mmitch/uzepede
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// map Uzebox types to Linux/x86
typedef uint8_t  u8;
typedef uint16_t u16;
typedef u16* VRAM_PTR_TYPE;

// define our constants
// FIXME: use original values? extract from uzepede.c to extra file?

#define MINX 3
#define MINY 1
#define MAXX 37
#define MAXY 27

#define MINX_SCREEN 2
#define MINY_SCREEN 0
#define MAXX_SCREEN 38
#define MAXY_SCREEN 28

#define OFFSCREEN 0xAA
#define MAXWORMLEN 16
#define MAXWORMCOUNT 16

#define SCORE_WORMBODY 3
#define SCORE_WORMHEAD 5
#define SCORE_WORMHEAD_PERBODY 2

#define DEBUG_REASON_SHOOT_WORM_BODY_MAXLEN_OVERFLOW   0x11
#define DEBUG_REASON_SHOOT_WORM_BODY_MAXWORM_OVERFLOW  0x22
#define DEBUG_REASON_MOVE_WORM_OLD_HEAD_OFFSCREEN      0x33

#include "../types.h"

// reinvent Mode 5
Tile vram[40*28];
#define TILE_WIDTH 6
#define TILE_HEIGHT 8


#define LEVEL(x,y) *((Tile*)(vram + (x) + (40*(y)) ))

const Tile TILE_FREE          = 0x00;
const Tile TILE_MUSHROOM1     = 0x10;
const Tile TILE_MUSHROOM2     = 0x11;
const Tile TILE_MUSHROOM3     = 0x12;
const Tile TILE_WORMBODY      = 0x20;
const Tile TILE_WORMHEADRIGHT = 0x21;
const Tile TILE_WORMHEADLEFT  = 0x22;
const Tile TILE_PLAYER        = 0x30;
const Tile TILE_SHOT          = 0x31;
const Tile TILE_SPIDER        = 0x40;
const Tile TILE_BEE           = 0x50;

enum { FX_WORMHEAD
};

#include "../globals.h"

#define UNUSED(x) (void)(x)

/*
 *  Uzebox mocks
 */

void SetTile(const Scalar x, const Scalar y, const Tile tile) {
	LEVEL(x,y) = tile;
}

void Fill(const Scalar x1, const Scalar y1, const Scalar x2, const Scalar y2, const Tile tile) {
	Scalar x, y;
	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			SetTile(x, y, tile);
		}
	}
}

/*
 *  Uzepede mocks
 */
void triggerFx3(const unsigned char patch, const unsigned char volume, const bool _retrig) {
	UNUSED(patch);
	UNUSED(volume);
	UNUSED(_retrig);
}

void showDebugDataAndStopExecution(const Scalar val1, const Scalar val2, const Scalar val3, const int tile) {
	UNUSED(val1);
	UNUSED(val2);
	UNUSED(val3);
	UNUSED(tile);

	exit(-1); // hard failure when debug is reached
}

void addScore(const Scalar add) {
	score += add;
}

void gameOver() {
}

