/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

enum {
	SYSTEM_BOOT    = 0,
	SYSTEM_RUNTIME = 1,
	SYSTEM_SUSPEND = 2,
	SYSTEM_OFF     = 3,
};

/**
 * The current system power state, from the enumeration in this file.
 */
extern volatile uint32_t system_state;

/**
 * Report that the system has finished booting, and the runtime OS is active.
 */
void system_boot_complete(void);

/**
 * Shut down the system. The firmware may continue running if needed to service
 * the power button.
 */
void system_shutdown(void);

/**
 * Reset the system, including this firmware.
 */
noreturn void system_reset(void);

/**
 * Resume the system from a suspended state.
 */
void system_resume(void);

/**
 * Suspend the system. The firmware will continue running to service resume
 * requests.
 */
void system_suspend(void);

#endif /* SYSTEM_H */
