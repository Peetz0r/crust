/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <limits.h>
#include <platform.h>
#include <stdbool.h>
#include <stdint.h>
#include <drivers/watchdog/r_twd.h>

void
udelay(uint32_t micros)
{
	uint32_t start = r_twd_read_counter();
	uint32_t msb   = start & 0x80000000U;
	uint32_t end   = (start ^ msb) + CLK_MHZ * micros;

	while ((r_twd_read_counter() ^ msb) < end) {
		/* Do nothing. */
	}
}
