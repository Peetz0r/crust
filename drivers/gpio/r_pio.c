/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <devices.h>
#include <mmio.h>
#include <stdbool.h>
#include <system.h>
#include <util.h>
#include <drivers/clock/r_prcm.h>
#include <drivers/gpio/r_pio.h>
#include <drivers/irqchip/r_intc.h>

enum {
	PL_CFG_REG0     = 0x0000,
	PL_CFG_REG1     = 0x0004,
	PL_CFG_REG2     = 0x0008,
	PL_CFG_REG3     = 0x000c,
	PL_DATA_REG     = 0x0010,
	PL_DRIVE_REG0   = 0x0014,
	PL_DRIVE_REG1   = 0x0018,
	PL_PULL_REG0    = 0x001c,
	PL_PULL_REG2    = 0x0020,
	PL_INT_CFG_REG0 = 0x0200,
	PL_INT_CFG_REG1 = 0x0204,
	PL_INT_CFG_REG2 = 0x0208,
	PL_INT_CFG_REG3 = 0x020c,
	PL_INT_CTRL_REG = 0x0210,
	PL_INT_STAT_REG = 0x0214,
	PL_DEBOUNCE_REG = 0x0218,
};

static uint32_t wakeup_data[4];

void
r_pio_disable_wakeup(void)
{
	/* Restore the old values. */
	mmio_write_32(DEV_R_PIO + PL_CFG_REG1, wakeup_data[0]);
	mmio_write_32(DEV_R_PIO + PL_PULL_REG0, wakeup_data[1]);
	mmio_write_32(DEV_R_PIO + PL_INT_CFG_REG1, wakeup_data[2]);
	mmio_write_32(DEV_R_PIO + PL_INT_CTRL_REG, wakeup_data[3]);
}

void
r_pio_enable_wakeup(void)
{
	/* Save the old values. */
	wakeup_data[0] = mmio_read_32(DEV_R_PIO + PL_CFG_REG1);
	wakeup_data[1] = mmio_read_32(DEV_R_PIO + PL_PULL_REG0);
	wakeup_data[2] = mmio_read_32(DEV_R_PIO + PL_INT_CFG_REG1);
	wakeup_data[3] = mmio_read_32(DEV_R_PIO + PL_INT_CTRL_REG);

	/* Set the mode for PL12 to external interrupt (0b110). */
	mmio_clrset_32(DEV_R_PIO + PL_CFG_REG1,
	               GENMASK(18, 16), GENMASK(18, 17));

	/* Disable the internal pull-up/down for PL12. */
	mmio_clr_32(DEV_R_PIO + PL_PULL_REG0, GENMASK(25, 24));

	/* Set the IRQ mode for PL12 to rising edge. */
	mmio_clr_32(DEV_R_PIO + PL_INT_CFG_REG1, GENMASK(19, 16));

	/* Enable the IRQ for PL12. */
	mmio_set_32(DEV_R_PIO + PL_INT_CTRL_REG, BIT(12));
}

void
r_pio_set_pin_function(uint8_t pin, uint8_t function)
{
	/* This happens to be both the bit offset and register offset. */
	uint32_t offset = ((pin) / 8) * 4;
	uint32_t reg    = PL_CFG_REG0 + offset;

	mmio_clrset_32(DEV_R_PIO + reg, 0x7 << offset, function << offset);
}

bool
r_pio_pl_irq(void)
{
	uint32_t status = mmio_read_32(DEV_R_PIO + PL_INT_STAT_REG);

	/* Handle power button or lid switch wakeup. */
	if (status && system_state != SYSTEM_RUNTIME) {
		system_resume();
		mmio_write_32(DEV_R_PIO + PL_INT_STAT_REG, status);
		return true;
	}

	return false;
}

void
r_pio_init(void)
{
	/* Enable R_PIO bus clock. */
	r_prcm_enable_clock(R_PRCM_CLOCK_R_PIO);

	/* Disable and clear the status of all IRQs. */
	mmio_write_32(DEV_R_PIO + PL_INT_CTRL_REG, 0);
	mmio_write_32(DEV_R_PIO + PL_INT_STAT_REG, GENMASK(31, 0));

	/* Enable the IRQ. */
	r_intc_enable_irq(IRQ_R_PIO_PL);
}
