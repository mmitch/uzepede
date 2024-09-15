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
