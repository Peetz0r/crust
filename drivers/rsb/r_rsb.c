/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <devices.h>
#include <mmio.h>
#include <platform.h>
#include <util.h>
#include <drivers/clock/r_prcm.h>
#include <drivers/gpio/r_pio.h>
#include <drivers/rsb/r_rsb.h>

#define I2C_BCAST_ADDR 0

enum {
	RSB_SRTA = 0xe8,
	RSB_RD8  = 0x8b,
	RSB_RD16 = 0x9c,
	RSB_RD32 = 0xa6,
	RSB_WR8  = 0x4e,
	RSB_WR16 = 0x59,
	RSB_WR32 = 0x63,
};

enum {
	RSB_CTRL_REG   = 0x00,
	RSB_CCR_REG    = 0x04,
	RSB_INT_EN_REG = 0x08,
	RSB_STAT_REG   = 0x0c,
	RSB_ADDR_REG   = 0x10,
	RSB_DLEN_REG   = 0x18,
	RSB_DATA_REG   = 0x1c,
	RSB_LCR_REG    = 0x24,
	RSB_PMCR_REG   = 0x28,
	RSB_CMD_REG    = 0x2c,
	RSB_SADDR_REG  = 0x30,
};

static void
r_rsb_do_command(uint32_t dev, uint32_t cmd)
{
	mmio_write_32(DEV_R_RSB + RSB_CMD_REG, cmd);
	mmio_write_32(DEV_R_RSB + RSB_SADDR_REG, dev);
	mmio_write_32(DEV_R_RSB + RSB_CTRL_REG, BIT(7));
	mmio_pollz_32(DEV_R_RSB + RSB_CTRL_REG, BIT(7));

	/* Handle errors by restarting the firmware. */
	if (mmio_read_32(DEV_R_RSB + RSB_STAT_REG) != BIT(0)) {
		debugSX("RSB transaction failed: ",
		        mmio_read_32(DEV_R_RSB + RSB_STAT_REG));
		reset();
	}
}

void
r_rsb_init_pmic(uint32_t dev, uint8_t addr, uint8_t data)
{
	/* Switch the PMIC to RSB mode. */
	mmio_write_32(DEV_R_RSB + RSB_PMCR_REG,
	              BIT(31) | data << 16 | addr << 8 | I2C_BCAST_ADDR);
	mmio_pollz_32(DEV_R_RSB + RSB_PMCR_REG, BIT(31));

	/* Raise the clock to 3MHz. */
	r_rsb_set_rate(3000000);

	/* Set the PMIC's runtime address. */
	r_rsb_do_command(dev, RSB_SRTA);
}

void
r_rsb_set_rate(uint32_t rate)
{
	uint32_t divider = CLK_HZ / 2 / rate - 1;

	mmio_write_32(DEV_R_RSB + RSB_CCR_REG, 1U << 8 | divider);
}

uint8_t
r_rsb_read(uint8_t dev, uint8_t addr)
{
	mmio_write_32(DEV_R_RSB + RSB_ADDR_REG, addr);
	r_rsb_do_command(RSB_RTADDR(dev), RSB_RD8);
	return mmio_read_32(DEV_R_RSB + RSB_DATA_REG);
}

void
r_rsb_write(uint8_t dev, uint8_t addr, uint8_t data)
{
	mmio_write_32(DEV_R_RSB + RSB_ADDR_REG, addr);
	mmio_write_32(DEV_R_RSB + RSB_DATA_REG, data);
	r_rsb_do_command(RSB_RTADDR(dev), RSB_WR8);
}

void
r_rsb_init(void)
{
	/* Enable R_RSB bus clock and reset. */
	r_prcm_enable_clock(R_PRCM_CLOCK_R_RSB);

	/* Set the data and clock pinmuxes. */
	r_pio_set_pin_function(0, 2);
	r_pio_set_pin_function(1, 2);

	/* Soft reset the controller. */
	mmio_write_32(DEV_R_RSB + RSB_CTRL_REG, BIT(0));
	mmio_pollz_32(DEV_R_RSB + RSB_CTRL_REG, BIT(0));

	/* Set the bus clock to a rate also compatible with I²C. */
	r_rsb_set_rate(400000);
}
