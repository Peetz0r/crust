/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_MSGBOX_MSGBOX_H
#define DRIVERS_MSGBOX_MSGBOX_H

#include <stdbool.h>
#include <stdint.h>

enum {
	MSGBOX_CHAN_SCPI_EL3_RX = 0,
	MSGBOX_CHAN_SCPI_EL3_TX = 1,
	MSGBOX_CHAN_SCPI_EL2_RX = 2,
	MSGBOX_CHAN_SCPI_EL2_TX = 3,
	MSGBOX_CHAN_IRQF_EL2_RX = 6,
	MSGBOX_CHAN_IRQF_EL2_TX = 7,
	MSGBOX_CHAN_COUNT       = 8,
};

/**
 * Enable receiving messages on a MSGBOX channel.
 */
void msgbox_enable_chan(uint8_t chan);

/**
 * Check if the last transmission on the MSGBOX channel was acknowledged by the
 * other user.
 */
bool msgbox_last_tx_done(uint8_t chan);

/**
 * Send a message using the MSGBOX.
 */
void msgbox_send(uint8_t chan, uint32_t msg);

/**
 * Handle an IRQ coming from the MSGBOX.
 *
 * @return True if the IRQ was cleared; false if it was unknown or ignored.
 */
bool msgbox_irq(void);

/**
 * Initialize the MSGBOX driver.
 */
void msgbox_init(void);

#endif /* DRIVERS_MSGBOX_MSGBOX_H */
