/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_PMIC_PMIC_H
#define DRIVERS_PMIC_PMIC_H

#include <stdbool.h>

/**
 * Use the PMIC to reset the system.
 */
void pmic_reset(void);

/**
 * Perform actions on the PMIC necessary for resuming the system.
 */
void pmic_resume(void);

/**
 * Use the PMIC to shut down the system.
 */
void pmic_shutdown(void);

/**
 * Perform actions on the PMIC necessary for suspending the system.
 */
void pmic_suspend(void);

/**
 * Handle an IRQ coming from the PMIC.
 *
 * @return True if the IRQ was cleared; false if it was unknown or ignored.
 */
bool pmic_irq(void);

/**
 * Initialize the PMIC driver.
 */
void pmic_init(void);

#endif /* DRIVERS_PMIC_PMIC_H */
