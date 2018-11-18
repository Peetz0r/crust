/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <delay.h>
#include <devices.h>
#include <error.h>
#include <msgbox.h>
#include <irqchip/sun4i-intc.h>
#include <platform/irq.h>

#define IRQF_CHANNEL 3

enum {
	IRQF_EOI    = 0,
	IRQF_MASK   = 1,
	IRQF_UNMASK = 2,
};

static uint32_t client_masked_irqs;
static struct device *msgbox;

void
forward_irq(uint8_t irq)
{
	int err;

	irqchip_mask(&r_intc, irq);

	if (msgbox == NULL)
		return;

	if ((err = msgbox_send(msgbox, IRQF_CHANNEL, irq)) && err != EBUSY)
		panic("IRQF: Unable to forward IRQ %d", irq);
}

static void
irq_forward_handle_request(struct device *dev __unused,
                           uint8_t channel __unused, uint32_t request)
{
	uint8_t irq = request >> 0;
	uint8_t msg = request >> 8;

	switch (msg) {
	case IRQF_EOI:
		if (!(client_masked_irqs & BIT(irq)))
			irqchip_unmask(&r_intc, irq);
		return;
	case IRQF_MASK:
		client_masked_irqs |= BIT(irq);
		irqchip_mask(&r_intc, irq);
		return;
	case IRQF_UNMASK:
		client_masked_irqs &= ~BIT(irq);
		irqchip_unmask(&r_intc, irq);
		return;
	default:
		warn("IRQF: Unknown request 0x%08x", request);
		return;
	}
}

void
irq_forward_init(void)
{
	int err;

	if ((msgbox = dm_first_dev_by_class(DM_CLASS_MSGBOX)) == NULL) {
		warn("IRQF: No message box device");
		return;
	}
	if ((err = msgbox_enable(msgbox, IRQF_CHANNEL,
	                         irq_forward_handle_request)))
		panic("IRQF: Error registering handler: %d", err);

	info("IRQF: Initialization complete");
}
