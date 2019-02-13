/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_CLOCK_CCU_H
#define DRIVERS_CLOCK_CCU_H

#include <stdint.h>

enum {
	CCU_CLOCK_MSGBOX = 0,
	CCU_CLOCK_COUNT  = 1,
};

/**
 * Enable the gate and deassert the reset for one of the clocks enumerated in
 * this file.
 */
void ccu_enable_clock(uint8_t clock);

/**
 * Initialize the CCU driver.
 */
void ccu_init(void);

#endif /* DRIVERS_CLOCK_CCU_H */
