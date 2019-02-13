/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_CLOCK_R_PRCM_H
#define DRIVERS_CLOCK_R_PRCM_H

#include <stdint.h>

enum {
	R_PRCM_CLOCK_R_PIO,
	R_PRCM_CLOCK_R_CIR,
	R_PRCM_CLOCK_R_TIMER,
#if CONFIG_RSB
	R_PRCM_CLOCK_R_RSB,
#endif
	R_PRCM_CLOCK_R_UART,
	R_PRCM_CLOCK_R_I2C,
	R_PRCM_CLOCK_R_TWD,
	R_PRCM_CLOCK_COUNT,
};

/**
 * Enable the gate and deassert the reset for one of the clocks enumerated in
 * this file.
 */
void r_prcm_enable_clock(uint8_t clock);

/**
 * Initialize the R_PRCM driver.
 */
void r_prcm_init(void);

#endif /* DRIVERS_CLOCK_R_PRCM_H */
