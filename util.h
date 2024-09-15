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

#include <stdlib.h>

#define RAND_RANGE(min_incl, max_excl) ( (rand()%((max_excl)-(min_incl))) + (min_incl) )

#define MAX(x,y) ((x)>(y)?(x):(y))

