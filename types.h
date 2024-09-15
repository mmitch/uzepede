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

typedef u8   Scalar;
typedef u8   Boolean;
typedef char Tile;
typedef u16  Joypad;
typedef unsigned int BigScalar;

typedef struct {
  Boolean direction_right;
  // worm has a ringbuffer of body elements
  // buffer "head" is the tail element
  Scalar tailidx;
  Scalar startidx;
  Scalar length;  // length == 0 -> worm is dead
  // @TODO: optimize: add endidx to cache (startidx + length - 1) ?
} Worm;
