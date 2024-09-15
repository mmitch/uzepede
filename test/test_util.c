/*
 * uzepede - a centipede clone for uzebox
 *
 * Copyright (C) 2012-2024 by  Christian Garbs <mitch@cgarbs.de>
 * licensed under the GNU GPL v3 or later
 *
 * see https://github.com/mmitch/uzepede
 *
 */

#include "../thirdparty/greatest.h"

#include "../util.h"

TEST test_max_first() {
	ASSERT_EQ( MAX( 56, 12), 56);
	PASS();
}

TEST test_max_second() {
	ASSERT_EQ( MAX( 21, 65), 65);
	PASS();
}

TEST test_max_both() {
	ASSERT_EQ( MAX( 34, 34), 34);
	PASS();
}

TEST test_rand_range_singular() {
	ASSERT_EQ( RAND_RANGE( 34, 35), 34);
	PASS();
}

TEST test_rand_range_repeated() {
	for (int i = 0; i < 128; i++) {
		int result = RAND_RANGE(16, 32);
		ASSERT_GT( result, 15);
		ASSERT_LT( result, 32);
	}
	PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
	time_t t;
	srand((unsigned) time(&t));

	GREATEST_MAIN_BEGIN();

	SHUFFLE_TESTS(rand(), {
			RUN_TEST(test_max_first);
			RUN_TEST(test_max_second);
			RUN_TEST(test_max_both);

			RUN_TEST(test_rand_range_singular);
			RUN_TEST(test_rand_range_repeated);
		});

	GREATEST_MAIN_END();
}
