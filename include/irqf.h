/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef IRQF_H
#define IRQF_H

#include <stdint.h>

/**
 * Forward an IRQ using the MSGBOX to the OS running on the AP CPUs.
 */
void irqf_forward_irq(uint8_t irq);

/**
 * Handle a control message sent by the OS running on the AP CPUs.
 */
void irqf_receive_message(uint8_t chan, uint32_t msg);

/**
 * Initialize the IRQ forwarding service.
 */
void irqf_init(void);

#endif /* IRQF_H */
