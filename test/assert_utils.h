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

#include "../thirdparty/greatest.h"

#include "uzepede_mocks.h"

#define MSGLEN 512
static char assertMessage[MSGLEN+1];

#define BREAK_ON_FAILURE(x)						\
	do {								\
		TEST _x_result;						\
		if ((_x_result = (x)) != GREATEST_TEST_RES_PASS) {      \
			return _x_result;				\
		};							\
	} while(0)


TEST assertThatLevelContainsAt(Tile expectedTile, Scalar x, Scalar y) {
	Tile actualTile = LEVEL(x,y);
	snprintf(assertMessage, MSGLEN,
		 "level tile content at (%d, %d) is %d instead of %d",
		 x, y, actualTile, expectedTile);
	ASSERT_EQm(assertMessage, expectedTile, actualTile);
	PASS();
}

TEST assertThatLevelContainsFromUntil(Tile expectedTile, Scalar minX, Scalar minY, Scalar maxX, Scalar maxY) {
	for (Scalar y = minY; y <= maxY; y++) {
		for (Scalar x = minX; x <= maxX; x++) {
			BREAK_ON_FAILURE(assertThatLevelContainsAt(expectedTile, x, y));
		}
	}
	PASS();
}
