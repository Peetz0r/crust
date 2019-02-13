/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitmap.h>
#include <debug.h>
#include <devices.h>
#include <mmio.h>
#include <stdint.h>
#include <drivers/clock/r_prcm.h>

#include "ccu.h"

enum {
	R_PRCM_CPUS_CLK_REG = 0x0000,
	R_PRCM_APB0_CLK_REG = 0x000c,
};

static const struct ccu_clock r_prcm_clocks[R_PRCM_CLOCK_COUNT] = {
	[R_PRCM_CLOCK_R_PIO] = {
		.gate = BITMAP_INDEX(0x0028, 0),
	},
	[R_PRCM_CLOCK_R_CIR] = {
		.gate  = BITMAP_INDEX(0x0028, 1),
		.reset = BITMAP_INDEX(0x00b0, 1),
	},
	[R_PRCM_CLOCK_R_TIMER] = {
		.gate  = BITMAP_INDEX(0x0028, 2),
		.reset = BITMAP_INDEX(0x00b0, 2),
	},
#if CONFIG_RSB
	[R_PRCM_CLOCK_R_RSB] = {
		.gate  = BITMAP_INDEX(0x0028, 3),
		.reset = BITMAP_INDEX(0x00b0, 3),
	},
#endif
	[R_PRCM_CLOCK_R_UART] = {
		.gate  = BITMAP_INDEX(0x0028, 4),
		.reset = BITMAP_INDEX(0x00b0, 4),
	},
	[R_PRCM_CLOCK_R_I2C] = {
		.gate  = BITMAP_INDEX(0x0028, 6),
		.reset = BITMAP_INDEX(0x00b0, 6),
	},
	[R_PRCM_CLOCK_R_TWD] = {
		.gate = BITMAP_INDEX(0x0028, 7),
	},
};

void
r_prcm_enable_clock(uint8_t clock)
{
	assert(clock < R_PRCM_CLOCK_COUNT);

	ccu_enable_one_clock(DEV_R_PRCM, &r_prcm_clocks[clock]);
}

void
r_prcm_init(void)
{
	/* CPUS and APB0 run at 24MHz (from OSC24M). */
	mmio_write_32(DEV_R_PRCM + R_PRCM_CPUS_CLK_REG, 0x00010000);
	mmio_write_32(DEV_R_PRCM + R_PRCM_APB0_CLK_REG, 0x00000000);
}
