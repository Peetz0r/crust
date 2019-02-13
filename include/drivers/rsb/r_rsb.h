/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_RSB_R_RSB_H
#define DRIVERS_RSB_R_RSB_H

#include <stdbool.h>
#include <stdint.h>

#define RSB_RTADDR(addr) ((addr) << 16)

#if CONFIG_RSB

/**
 * Initialize a PMIC connected to the R_RSB bus.
 *
 * @param dev  PMIC device address.
 * @param addr PMIC-internal register address to write.
 * @param data Data to write to the PMIC register.
 */
void r_rsb_init_pmic(uint32_t dev, uint8_t addr, uint8_t data);

/**
 * Set the R_RSB bus clock rate to approximately the given value.
 */
void r_rsb_set_rate(uint32_t rate);

/**
 * Read a register on a device connected to the R_RSB bus.
 */
uint8_t r_rsb_read(uint8_t dev, uint8_t addr);

/**
 * Write a register on a device connected to the R_RSB bus.
 */
void r_rsb_write(uint8_t dev, uint8_t addr, uint8_t data);

/**
 * Initialize the R_RSB driver.
 */
void r_rsb_init(void);

#else

static inline void
r_rsb_init(void)
{
}

#endif

#endif /* DRIVERS_RSB_R_RSB_H */
