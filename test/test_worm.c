/*
 * uzepede - a centipede clone for uzebox
 *
 * Copyright (C) 2012-2024 by  Christian Garbs <mitch@cgarbs.de>
 * licensed under the GNU GPL v3 or later
 *
 * see https://github.com/mmitch/uzepede
 *
 */

#include "assert_utils.h"

#include "../screen.c"
#include "../worm.c"

const Boolean looking_right = true;
const Boolean looking_left  = false;

TEST init_worm_draws_only_the_head_initially() {
	// GIVEN
	Scalar initx = 17;
	Scalar inity = 6;
	Scalar length = 5;
	clearScreen();

	// WHEN
	initWorm(initx, inity, length, looking_right);

	// THEN

	// rows above
	BREAK_ON_FAILURE(assertThatLevelContainsFromUntil(TILE_FREE, MINX, MINY, MAXX, inity-1));

	// rows below
	BREAK_ON_FAILURE(assertThatLevelContainsFromUntil(TILE_FREE, MINX, inity+1, MAXX, MAXY));

	// left of worm
	BREAK_ON_FAILURE(assertThatLevelContainsFromUntil(TILE_FREE, MINX, inity, initx-1, inity));

	// right of worm
	BREAK_ON_FAILURE(assertThatLevelContainsFromUntil(TILE_FREE, initx+1, inity, MAXX, inity));

	// head at initial position
	BREAK_ON_FAILURE(assertThatLevelContainsAt(TILE_WORMHEADRIGHT, initx, inity));

	PASS();
}

// FIXME: check internal worms[], wormx[], wormy[] structures after init

TEST moving_the_worm_once_moves_the_head_and_puts_a_body_part_where_the_head_was_initially() {
	// GIVEN
	Scalar initx = 17;
	Scalar inity = 6;
	Scalar length = 5;
	clearScreen();
	initWorm(initx, inity, length, looking_right);

	// WHEN
	moveWorm(0);

	// THEN

	// rows above
	BREAK_ON_FAILURE(assertThatLevelContainsFromUntil(TILE_FREE, MINX, MINY, MAXX, inity-1));

	// rows below
	BREAK_ON_FAILURE(assertThatLevelContainsFromUntil(TILE_FREE, MINX, inity+1, MAXX, MAXY));

	// left of worm
	BREAK_ON_FAILURE(assertThatLevelContainsFromUntil(TILE_FREE, MINX, inity, initx-1, inity));

	// right of worm + 1
	BREAK_ON_FAILURE(assertThatLevelContainsFromUntil(TILE_FREE, initx+2, inity, MAXX, inity));

	// body at initial position
	BREAK_ON_FAILURE(assertThatLevelContainsAt(TILE_WORMBODY, initx, inity));

	// head right of initial position
	BREAK_ON_FAILURE(assertThatLevelContainsAt(TILE_WORMHEADRIGHT, initx+1, inity));

	PASS();
}

// FIXME: check internal worms[], wormx[], wormy[] structures after init

TEST shoot_at_wormbody_with_other_parts_of_wormbody_still_offscreen() {
	// GIVEN
	Scalar initx = 17;
	Scalar inity = 6;
	Scalar length = 5;

	shot_x = initx;
	shot_y = inity;

	clearScreen();
	initWorm(initx, inity, length, looking_right);
	moveWorm(0);
	moveWorm(0);
	moveWorm(0);

	// WHEN
	shootWormBody();

// THIS SEGFAULTS, YAY! CONTINUE HERE

	// THEN

	// FIXME: ADD TEST CONDITIONS

	PASS();
}

// FIXME: check internal worms[], wormx[], wormy[] structures after shot

void FIXME_find_a_way_around_unused_warnings_for_these() {
	drawMushroom2(0, 0);
	drawMushroom3(0, 0);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
	time_t t;
	srand((unsigned) time(&t));

	GREATEST_MAIN_BEGIN();

	SHUFFLE_TESTS(rand(), {
			RUN_TEST(init_worm_draws_only_the_head_initially);
			RUN_TEST(moving_the_worm_once_moves_the_head_and_puts_a_body_part_where_the_head_was_initially);
			RUN_TEST(shoot_at_wormbody_with_other_parts_of_wormbody_still_offscreen);
		});

	GREATEST_MAIN_END();
}
