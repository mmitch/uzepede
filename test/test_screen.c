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

TEST initially_the_level_is_empty() {
	clearScreen();

        BREAK_ON_FAILURE(assertThatLevelContainsFromUntil(TILE_FREE, MINX, MINY, MAXX, MAXY));

	PASS();
}

void FIXME_write_additional_tests_for_these_methods_they_are_currently_unused_and_produce_a_warning() {
	drawEmpty(0, 0);
	drawMushroom1(0, 0);
	drawMushroom2(0, 0);
	drawMushroom3(0, 0);
	drawWormBody(0, 0);
	drawWormHead(0, 0, 0);
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
