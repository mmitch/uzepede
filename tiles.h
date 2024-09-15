/*
 * uzepede - a centipede clone for uzebox
 *
 * Copyright (C) 2012-2024 by  Christian Garbs <mitch@cgarbs.de>
 * licensed under the GNU GPL v3 or later
 *
 * see https://github.com/mmitch/uzepede
 *
 */

#pragma once

#include "data/tiles.inc"

// read tile from VRAM
#define LEVEL(x,y) *((Tile*)(vram + (2*(x)) + (80*(y) ) ))

// constants for the actual first tile content
// TODO: borders and characters are missing (except for c_slash only used once)
#define TILE_FREE          t_black[2]
#define TILE_MUSHROOM1     t_mushroom1[2]
#define TILE_MUSHROOM2     t_mushroom2[2]
#define TILE_MUSHROOM3     t_mushroom3[2]
#define TILE_WORMBODY      t_wormbody[2]
#define TILE_WORMHEADLEFT  t_wormheadleft[2]
#define TILE_WORMHEADRIGHT t_wormheadright[2]
#define TILE_PLAYER        t_player[2]
#define TILE_SHOT          t_shot[2]
#define TILE_SPIDER        t_spider[2]
#define TILE_BEE           t_bee[2]

// comparison tile pointers for VRAM
#define T_FREE (Tile)(Tiles + (TILE_FREE          * TILE_WIDTH * TILE_HEIGHT ))
#define T_MSH1 (Tile)(Tiles + (TILE_MUSHROOM1     * TILE_WIDTH * TILE_HEIGHT ))
#define T_MSH2 (Tile)(Tiles + (TILE_MUSHROOM2     * TILE_WIDTH * TILE_HEIGHT ))
#define T_MSH3 (Tile)(Tiles + (TILE_MUSHROOM3     * TILE_WIDTH * TILE_HEIGHT ))
#define T_WORM (Tile)(Tiles + (TILE_WORMBODY      * TILE_WIDTH * TILE_HEIGHT ))
#define T_WMHL (Tile)(Tiles + (TILE_WORMHEADLEFT  * TILE_WIDTH * TILE_HEIGHT ))
#define T_WMHR (Tile)(Tiles + (TILE_WORMHEADRIGHT * TILE_WIDTH * TILE_HEIGHT ))
#define T_PLYR (Tile)(Tiles + (TILE_PLAYER        * TILE_WIDTH * TILE_HEIGHT ))
#define T_SHOT (Tile)(Tiles + (TILE_SHOT          * TILE_WIDTH * TILE_HEIGHT ))
#define T_SPDR (Tile)(Tiles + (TILE_SPIDER        * TILE_WIDTH * TILE_HEIGHT ))
#define T_BEE  (Tile)(Tiles + (TILE_BEE           * TILE_WIDTH * TILE_HEIGHT ))

