/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <devices.h>
#include <mmio.h>
#include <platform.h>
#include <stdint.h>
#include <util.h>
#include <drivers/clock/r_prcm.h>
#include <drivers/watchdog/r_twd.h>

#define R_TWD_RESTART_KEY 0x0d140000

enum {
	R_TWD_STAT_REG     = 0x0000,
	R_TWD_CTRL_REG     = 0x0010,
	R_TWD_RESTART_REG  = 0x0014,
	R_TWD_LOW_CNT_REG  = 0x0020,
	R_TWD_HIGH_CNT_REG = 0x0024,
	R_TWD_INTERVAL_REG = 0x0030,
};

uint32_t
r_twd_read_counter(void)
{
	return mmio_read_32(DEV_R_TWD + R_TWD_LOW_CNT_REG);
}

void
r_twd_restart(void)
{
	mmio_write_32(DEV_R_TWD + R_TWD_RESTART_REG,
	              R_TWD_RESTART_KEY | BIT(0));
}

void
r_twd_set_interval(uint32_t cycles)
{
	mmio_write_32(DEV_R_TWD + R_TWD_INTERVAL_REG, cycles);
}

void
r_twd_init(void)
{
	/* Enable R_TWD bus clock. */
	r_prcm_enable_clock(R_PRCM_CLOCK_R_TWD);

	/* Stop the counter; enable system reset. */
	mmio_set_32(DEV_R_TWD + R_TWD_CTRL_REG, BIT(1) | BIT(9));

	/* Program interval until watchdog fires (500 ms). */
	r_twd_set_interval(500 * CLK_KHZ);

	/* Pet the watchdog. */
	r_twd_restart();

	/* Start the counter running at 24MHz. */
	mmio_clrset_32(DEV_R_TWD + R_TWD_CTRL_REG, BIT(1), BIT(31));
}
