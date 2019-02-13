/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_GPIO_R_PIO_H
#define DRIVERS_GPIO_R_PIO_H

#include <stdbool.h>
#include <stdint.h>

void r_pio_disable_wakeup(void);
void r_pio_enable_wakeup(void);

/**
 * Set the function of a pin (the values are different for each pin).
 */
void r_pio_set_pin_function(uint8_t pin, uint8_t function);

/**
 * Handle an IRQ coming from R_PIO.
 *
 * @return True if the IRQ was cleared; false if it was unknown or ignored.
 */
bool r_pio_pl_irq(void);

/**
 * Initialize the R_PIO driver.
 */
void r_pio_init(void);

#endif /* DRIVERS_GPIO_R_PIO_H */
