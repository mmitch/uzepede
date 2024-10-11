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

#include "types.h"

// #optimization:
// - generally, IS_SHOT_AT() is faster and shorter
// - but if we already are in a comparison using LEVEL(x,y),
//   IS_SHOT_AT_TILED() is better, because the registers are already set up
#define IS_SHOT_AT(x,y) (shot_x == (x) && shot_y == (y))
#define IS_SHOT_AT_TILED(x,y) (LEVEL((x),(y)) == TILE_SHOT)

Worm worms[MAXWORMCOUNT];
Scalar wormx[MAXWORMLEN];
Scalar wormy[MAXWORMLEN];
Scalar wormmax;
Scalar wormcount;

Scalar shot_x, shot_y;
Boolean shooting;

BigScalar score;

Scalar wormkills_spider;
Scalar wormkills_bee;
