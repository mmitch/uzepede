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

TEST initially_the_level_is_empty() {
	clearScreen();

        BREAK_ON_FAILURE(assertThatLevelContainsFromUntil(TILE_FREE, MINX, MINY, MAXX, MAXY));

	PASS();
}

void FIXME_write_additional_tests_for_these_methods_they_are_currently_unused_and_produce_a_warning() {
	moveWorm(0);
	shootWormBody();
	initWorm(0, 0, 1, true);
}

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
			RUN_TEST(initially_the_level_is_empty);
		});

	GREATEST_MAIN_END();
}
