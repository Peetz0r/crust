/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitmap.h>
#include <debug.h>
#include <devices.h>
#include <mmio.h>
#include <stdint.h>
#include <drivers/clock/ccu.h>

#include "ccu.h"

static const struct ccu_clock ccu_clocks[CCU_CLOCK_COUNT] = {
	[CCU_CLOCK_MSGBOX] = {
		.gate  = BITMAP_INDEX(0x0064, 21),
		.reset = BITMAP_INDEX(0x02c4, 21),
	},
};

void
ccu_enable_clock(uint8_t clock)
{
	assert(clock < CCU_CLOCK_COUNT);

	ccu_enable_one_clock(DEV_CCU, &ccu_clocks[clock]);
}

void
ccu_init(void)
{
}
