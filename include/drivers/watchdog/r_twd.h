/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_WATCHDOG_R_TWD_H
#define DRIVERS_WATCHDOG_R_TWD_H

#include <stdint.h>

/**
 * Return the value of a 32-bit 24MHz cycle counter. This is actually a 64-bit
 * counter, but we ignore the high word for simplicity. This introduces a 0.02%
 * timekeeping error due to losing 1032704 cycles when the counter rolls over.
 *
 * The R_TWD counter is used in preference to the R_CPUCFG counter because it
 * does not require latching/synchronization. However, it depends on the R_TWD
 * module clock to be initialized to use OSC24M.
 */
uint32_t r_twd_read_counter(void);

/**
 * Restart the watchdog. This must be called before the interval expires to
 * prevent the system from being reset.
 */
void r_twd_restart(void);

/**
 * Set the watchdog interval, in 24MHz clock cycles.
 */
void r_twd_set_interval(uint32_t cycles);

/**
 * Initialize the R_TWD driver.
 */
void r_twd_init(void);

#endif /* DRIVERS_WATCHDOG_R_TWD_H */
