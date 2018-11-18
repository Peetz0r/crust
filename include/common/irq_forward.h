/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef COMMON_IRQ_FORWARD_H
#define COMMON_IRQ_FORWARD_H

#include <stdint.h>

/**
 */
void forward_irq(uint8_t irq);

/**
 */
void irq_forward_init(void);

#endif /* COMMON_IRQ_FORWARD_H */
