/*
 * uzepede - a centipede clone for uzebox
 *
 * Copyright (C) 2012-2024 by  Christian Garbs <mitch@cgarbs.de>
 * licensed under the GNU GPL v3 or later
 *
 * see https://github.com/mmitch/uzepede
 *
 */

#include "uzepede-mocks.h"

#include "../worm.c"

#include "../thirdparty/greatest.h"

#define TOLERANCE 0.00000000001

TEST test_canary() {
	ASSERT_EQ(11, 11);
	ASSERT_EQ(0x01, 0x01);

	PASS();
}

void fix_unused_warnings_until_tests_are_written() {
	moveWorm(0);
	shootWormBody();
	initWorm(0, 0, 1, true);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
	time_t t;
	srand((unsigned) time(&t));

	GREATEST_MAIN_BEGIN();

	SHUFFLE_TESTS(rand(), {
			RUN_TEST(test_canary);
		});

	GREATEST_MAIN_END();
}
