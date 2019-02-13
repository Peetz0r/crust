/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <delay.h>
#include <stdbool.h>
#include <stdint.h>
#include <system.h>
#include <util.h>
#include <drivers/rsb/r_rsb.h>

#include "axp803.h"

#define AXP803_MODE_REG   0x3e
#define AXP803_MODE_VAL   0x7c

#define AXP803_RSB_HWADDR 0x03a3
#define AXP803_RSB_RTADDR 0x2d

enum {
	POWER_SRC_STAT_REG = 0x00,
	POWER_CHG_STAT_REG = 0x01,
	ON_OFF_CTRL_REG1   = 0x10,
	ON_OFF_CTRL_REG2   = 0x12,
	ON_OFF_CTRL_REG3   = 0x13,
	WAKEUP_CTRL_REG    = 0x31,
	POWER_DISABLE_REG  = 0x32,
	POWER_KEY_CTRL_REG = 0x36,
	IRQ_EN_REG1        = 0x40,
	IRQ_EN_REG2        = 0x41,
	IRQ_EN_REG3        = 0x42,
	IRQ_EN_REG4        = 0x43,
	IRQ_EN_REG5        = 0x44,
	IRQ_EN_REG6        = 0x45,
	IRQ_STAT_REG1      = 0x48,
	IRQ_STAT_REG2      = 0x49,
	IRQ_STAT_REG3      = 0x4a,
	IRQ_STAT_REG4      = 0x4b,
	IRQ_STAT_REG5      = 0x4c,
	IRQ_STAT_REG6      = 0x4d,
	FUNCTION_CTRL_REG  = 0x8f,
};

static uint8_t saved_irq_enable[6];

static void
axp803_clr_8(uint8_t addr, uint8_t clr)
{
	uint8_t val = r_rsb_read(AXP803_RSB_RTADDR, addr);

	r_rsb_write(AXP803_RSB_RTADDR, addr, val & ~clr);
}

static void
axp803_set_8(uint8_t addr, uint8_t set)
{
	uint8_t val = r_rsb_read(AXP803_RSB_RTADDR, addr);

	r_rsb_write(AXP803_RSB_RTADDR, addr, val | set);
}

noreturn void
axp803_reset(void)
{
	/* Trigger soft power restart. */
	axp803_set_8(WAKEUP_CTRL_REG, BIT(6));

	/* In case this fails for some reason, reset the firmware. */
	reset();
}

void
axp803_resume(void)
{
	/* Trigger soft power wakeup. */
	axp803_set_8(WAKEUP_CTRL_REG, BIT(5));

	/* Wait an arbitrary 250ms for the the power rails to come back up. */
	udelay(250000);

	/* Restore runtime IRQ settings. */
	for (uint8_t i = 0; i < 6; ++i) {
		uint8_t addr = IRQ_EN_REG1 + i;
		r_rsb_write(AXP803_RSB_RTADDR, addr, saved_irq_enable[i]);
	}
}

noreturn void
axp803_shutdown(void)
{
	/* Trigger soft power off. */
	axp803_set_8(POWER_DISABLE_REG, BIT(7));

	/* In case this fails for some reason, reset the firmware. */
	reset();
}

void
axp803_suspend(void)
{
	/* Save the runtime IRQ settings; disable IRQs except the power key. */
	for (uint8_t i = 0; i < 6; ++i) {
		uint8_t addr = IRQ_EN_REG1 + i;
		uint8_t data = addr == IRQ_EN_REG5 ? BIT(5) : 0;
		saved_irq_enable[i] = r_rsb_read(AXP803_RSB_RTADDR, addr);
		r_rsb_write(AXP803_RSB_RTADDR, addr, data);
	}

	/* Prevent automatic wakeup on IRQ. */
	axp803_clr_8(FUNCTION_CTRL_REG, BIT(7));

	/* Allow IRQs during suspend; enter suspend. */
	axp803_set_8(WAKEUP_CTRL_REG, BIT(4) | BIT(3));

	/* Disable the CPU and main peripheral power rails. */
	axp803_clr_8(ON_OFF_CTRL_REG1, GENMASK(2, 0));
}

bool
axp803_irq(void)
{
	uint8_t status;

	/* Only check IRQs if the system is suspended. */
	if (system_state != SYSTEM_SUSPEND)
		return false;

	/* Power off the system if the PMIC overheats. */
	status = r_rsb_read(AXP803_RSB_RTADDR, IRQ_STAT_REG4);
	if (status & BIT(7))
		axp803_shutdown();

	/* Resume the system if the power button is pressed. */
	status = r_rsb_read(AXP803_RSB_RTADDR, IRQ_STAT_REG5);
	if (status & BIT(5)) {
		r_rsb_write(AXP803_RSB_RTADDR, IRQ_STAT_REG5, BIT(5));
		system_resume();
	}

	return true;
}

void
axp803_init(void)
{
	/* Switch the PMIC to RSB mode. */
	r_rsb_init_pmic(RSB_RTADDR(AXP803_RSB_RTADDR) | AXP803_RSB_HWADDR,
	                AXP803_MODE_REG, AXP803_MODE_VAL);

	/* Set the power key power-off IRQ threshold to 4s. */
	axp803_clr_8(POWER_KEY_CTRL_REG, GENMASK(1, 0));

	/* Enable shutdown on PMIC overheat or >16 seconds button press;
	 * remember previous voltages when waking up from suspend. */
	axp803_set_8(FUNCTION_CTRL_REG, GENMASK(3, 1));
}
