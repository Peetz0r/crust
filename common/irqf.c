/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <irqf.h>
#include <stdint.h>
#include <system.h>
#include <util.h>
#include <drivers/irqchip/r_intc.h>
#include <drivers/msgbox/msgbox.h>

enum {
	IRQF_EOI    = 0,
	IRQF_MASK   = 1,
	IRQF_UNMASK = 2,
};

static volatile uint32_t client_masked_irqs;

/**
 * Forward an IRQ over the mailbox.
 */
void
irqf_forward_irq(uint8_t irq)
{
	/* Mask the IRQ on our side. */
	r_intc_mask_irq(irq);

	/* Only forward IRQs while an OS is running. */
	if (system_state != SYSTEM_RUNTIME)
		return;

	msgbox_send(MSGBOX_CHAN_IRQF_EL2_TX, irq);
}

void
irqf_receive_message(unused uint8_t chan, uint32_t msg)
{
	uint8_t irq     = msg >> 0;
	uint8_t request = msg >> 8;

	switch (request) {
	case IRQF_EOI:
		if (!(client_masked_irqs & BIT(irq)))
			r_intc_unmask_irq(irq);
		return;
	case IRQF_MASK:
		client_masked_irqs |= BIT(irq);
		r_intc_mask_irq(irq);
		return;
	case IRQF_UNMASK:
		client_masked_irqs &= ~BIT(irq);
		r_intc_unmask_irq(irq);
		return;
	default:
		debugSX("IRQF: Bad request: ", msg);
		return;
	}
}

void
irqf_init(void)
{
	/* Enable the message box channel. */
	msgbox_enable_chan(MSGBOX_CHAN_IRQF_EL2_RX);
}
