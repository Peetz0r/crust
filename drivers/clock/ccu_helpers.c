/*
 * Copyright © 2019-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitfield.h>
#include <counter.h>
#include <debug.h>
#include <mmio.h>
#include <stddef.h>
#include <stdint.h>
#include <util.h>
#include <watchdog/sunxi-twd.h>
#include <platform/devices.h>
#include <platform/time.h>

#include "ccu.h"

#define PLL_CTRL_REG1_KEY  (0xa7 << 24)
#define PLL_CTRL_REG1_MASK GENMASK(2, 0)

#define THS0_DATA_REG          0x80
#define THS1_DATA_REG          0x84
#define THS2_DATA_REG          0x88

#define THS_CTL_REG0           0x00
#define THS_CTL_REG1           0x04
#define THS_CTL_REG2           0x40

#define THS_FILTER_CONTROL_REG 0x70

static bool once;

void
ccu_helper_calibrate_osc16m(const uint32_t *rate)
{
	uint32_t after, before, end, now;

	if (!once) {
		mmio_set_32(DEV_CCU + 0x0068, BIT(8));
		mmio_set_32(DEV_CCU + 0x02d0, BIT(8));
		mmio_write_32(DEV_CCU + 0x0074, BIT(31));
		mmio_write_32(DEV_THS + THS_CTL_REG1, BIT(17));
#ifndef CONFIG_SOC_H5
		while (mmio_read_32(DEV_THS + THS_CTL_REG1) & BIT(17)) {}
#endif
		mmio_write_32(DEV_THS + THS_CTL_REG0, 0x257);
		mmio_write_32(DEV_THS + THS_CTL_REG2, 0x257 << 16);
		mmio_write_32(DEV_THS + THS_FILTER_CONTROL_REG, 0x06);
		mmio_set_32(DEV_THS + THS_CTL_REG2, BIT(0));
		once = 1;
	}

	/* Cycle until the interval will not span a counter wraparound. */
	do {
		before = counter_read();
		barrier();
		now = r_twd_counter_read();
		end = now + (REFCLK_HZ >> 7);
	} while (end < now);

	/* Cycle until the end of the interval. */
	do {
		after = counter_read();
		/* Ensure the counters are read in a consistent order. */
		barrier();
		now = r_twd_counter_read();
	} while (now < end);

	/*
	 * Convert the number of OSC16M cycles in 1/512 second to Hz. 512 is
	 * chosen because it is the largest power-of-two factor of 24MHz, the
	 * reference clock frequency.
	 *
	 * This writes to a location in .rodata, which is important, because
	 * the value needs to be preserved in case of an exception restart
	 * during SYSTEM_INACTIVE/OFF, where r_ccu_init() does not get called.
	 */
	mmio_write_32((uintptr_t)rate, (after - before) << 7);

	uint32_t temp = mmio_read_32(DEV_THS + THS0_DATA_REG) & GENMASK(11, 0);
	info("temp %08x freq %d", temp, *rate);
}

static void
ccu_helper_update_osc24m(uintptr_t reg, uint32_t val)
{
	uint32_t tmp;

	tmp  = mmio_read_32(reg);
	tmp |= PLL_CTRL_REG1_KEY;
	mmio_write_32(reg, tmp);
	tmp &= ~PLL_CTRL_REG1_MASK;
	tmp |= val;
	mmio_write_32(reg, tmp);
	tmp &= ~PLL_CTRL_REG1_KEY;
	mmio_write_32(reg, tmp);
}

void
ccu_helper_disable_osc24m(uintptr_t reg)
{
	ccu_helper_update_osc24m(reg, 0);
}

void
ccu_helper_enable_osc24m(uintptr_t reg)
{
	ccu_helper_update_osc24m(reg, PLL_CTRL_REG1_MASK);
}

const struct clock_handle *
ccu_helper_get_parent(const struct ccu *self UNUSED,
                      const struct ccu_clock *clk UNUSED)
{
	return NULL;
}

uint32_t
ccu_helper_get_rate(const struct ccu *self UNUSED,
                    const struct ccu_clock *clk UNUSED, uint32_t rate)
{
	return rate;
}

uint32_t
ccu_helper_get_rate_m(const struct ccu *self,
                      const struct ccu_clock *clk, uint32_t rate,
                      uint32_t m_shift, uint32_t m_width)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	rate /= bitfield_get(val, m_shift, m_width) + 1;

	return rate;
}

uint32_t
ccu_helper_get_rate_mp(const struct ccu *self,
                       const struct ccu_clock *clk, uint32_t rate,
                       uint32_t m_shift, uint32_t m_width,
                       uint32_t p_shift, uint32_t p_width)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	rate  /= bitfield_get(val, m_shift, m_width) + 1;
	rate >>= bitfield_get(val, p_shift, p_width);

	return rate;
}

uint32_t
ccu_helper_get_rate_p(const struct ccu *self,
                      const struct ccu_clock *clk, uint32_t rate,
                      uint32_t p_shift, uint32_t p_width)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	rate >>= bitfield_get(val, p_shift, p_width);

	return rate;
}
