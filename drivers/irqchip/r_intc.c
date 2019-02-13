/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <devices.h>
#include <interrupts.h>
#include <irqf.h>
#include <mmio.h>
#include <stdbool.h>
#include <stdint.h>
#include <drivers/gpio/r_pio.h>
#include <drivers/irqchip/r_intc.h>
#include <drivers/msgbox/msgbox.h>
#include <drivers/pmic/pmic.h>

enum {
	R_INTC_VECTOR_REG    = 0x0000,
	R_INTC_BASE_ADDR_REG = 0x0004,
	R_INTC_PROTECT_REG   = 0x0008,
	R_INTC_NMI_CTRL_REG  = 0x000c,
	R_INTC_STAT_REG      = 0x0010,
	R_INTC_EN_REG        = 0x0040,
	R_INTC_MASK_REG      = 0x0050,
	R_INTC_RESP_REG      = 0x0060,
};

void
r_intc_configure_nmi(uint32_t type)
{
	uint32_t mask = GENMASK(1, 0);

	mmio_clrset_32(DEV_R_INTC + R_INTC_NMI_CTRL_REG, mask, type);
}

void
r_intc_disable_irq(uint8_t irq)
{
	mmio_clr_32(DEV_R_INTC + R_INTC_EN_REG, BIT(irq));
}

void
r_intc_enable_irq(uint8_t irq)
{
	mmio_set_32(DEV_R_INTC + R_INTC_EN_REG, BIT(irq));
}

void
r_intc_mask_irq(uint8_t irq)
{
	mmio_set_32(DEV_R_INTC + R_INTC_MASK_REG, BIT(irq));
}

void
r_intc_unmask_irq(uint8_t irq)
{
	mmio_clr_32(DEV_R_INTC + R_INTC_MASK_REG, BIT(irq));
}

/**
 * Handle the next incoming IRQ. Due to the possibility of the IRQ being
 * serviced through the GIC before R_INTC triggers an interrupt exception, this
 * function must handle receiving an IRQ that is no longer pending.
 */
void
r_intc_irq(void)
{
	uint32_t irq = mmio_read_32(DEV_R_INTC + R_INTC_VECTOR_REG) >> 2;
	bool handled;

	/* Handle Linux clearing the module's IRQ before we saw it. */
	if (!mmio_get_32(DEV_R_INTC + R_INTC_STAT_REG, BIT(irq)))
		return;

	switch (irq) {
	case IRQ_NMI:
		handled = pmic_irq();
		break;
	case IRQ_R_PIO_PL:
		handled = r_pio_pl_irq();
		break;
	case IRQ_MSGBOX:
		handled = msgbox_irq();
		break;
	default:
		handled = false;
	}

	if (!handled)
		irqf_forward_irq(irq);

	/* Clear the IRQ pending status. */
	mmio_write_32(DEV_R_INTC + R_INTC_STAT_REG, BIT(irq));
}

void
r_intc_init(void)
{
	/* Clear the table base address (just return IRQ numbers). */
	mmio_write_32(DEV_R_INTC + R_INTC_BASE_ADDR_REG, 0);

	/* Disable, unmask, and clear the status of all IRQs. */
	mmio_write_32(DEV_R_INTC + R_INTC_EN_REG, 0);
	mmio_write_32(DEV_R_INTC + R_INTC_MASK_REG, 0);
	mmio_write_32(DEV_R_INTC + R_INTC_STAT_REG, GENMASK(31, 0));

	/* Enable the CPU external interrupt input. */
	enable_interrupts();
}
