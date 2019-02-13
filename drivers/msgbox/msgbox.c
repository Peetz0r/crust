/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <devices.h>
#include <irqf.h>
#include <mmio.h>
#include <scpi.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>
#include <drivers/clock/ccu.h>
#include <drivers/irqchip/r_intc.h>
#include <drivers/msgbox/msgbox.h>

#define MSGBOX_CTRL_DRAIN       0x10101010
#define MSGBOX_CTRL_NORMAL      0x01100110
#define MSGBOX_CTRL_MASK        0x11111111

#define MSGBOX_RX_IRQ(n)        BIT(0 + 2 * (n))
#define MSGBOX_RX_IRQ_MASK      0x5555
#define MSGBOX_TX_IRQ(n)        BIT(1 + 2 * (n))
#define MSGBOX_TX_IRQ_MASK      0xaaaa

#define MSGBOX_FIFO_STAT_REG(n) (0x0100 + 0x4 * (n))
#define MSGBOX_MSG_STAT_REG(n)  (0x0140 + 0x4 * (n))
#define MSGBOX_MSG_DATA_REG(n)  (0x0180 + 0x4 * (n))

enum {
	MSGBOX_CTRL_REG0        = 0x0000,
	MSGBOX_CTRL_REG1        = 0x0004,
	MSGBOX_IRQ_EN_REG       = 0x0040,
	MSGBOX_IRQ_STAT_REG     = 0x0050,
	MSGBOX_ARM_IRQ_EN_REG   = 0x0060,
	MSGBOX_ARM_IRQ_STAT_REG = 0x0070,
};

static void
msgbox_handle_msg(uint8_t chan)
{
	uint32_t msg = mmio_read_32(DEV_MSGBOX + MSGBOX_MSG_DATA_REG(chan));

	switch (chan) {
	case MSGBOX_CHAN_SCPI_EL2_RX:
	case MSGBOX_CHAN_SCPI_EL3_RX:
		scpi_receive_message(chan, msg);
		break;
	case MSGBOX_CHAN_IRQF_EL2_RX:
		irqf_receive_message(chan, msg);
		break;
	}
}

static bool
msgbox_peek_data(uint8_t chan)
{
	return mmio_read_32(DEV_MSGBOX + MSGBOX_MSG_STAT_REG(chan));
}

void
msgbox_enable_chan(uint8_t chan)
{
	mmio_set_32(DEV_MSGBOX + MSGBOX_IRQ_EN_REG, MSGBOX_RX_IRQ(chan));
}

bool
msgbox_last_tx_done(uint8_t chan)
{
	uint32_t status = mmio_read_32(DEV_MSGBOX + MSGBOX_ARM_IRQ_STAT_REG);

	return !(status & MSGBOX_RX_IRQ(chan));
}

void
msgbox_send(uint8_t chan, uint32_t msg)
{
	/* Spin while the FIFO is full. */
	while (mmio_read_32(DEV_MSGBOX + MSGBOX_FIFO_STAT_REG(chan)) & BIT(0))
		udelay(10);

	mmio_write_32(DEV_MSGBOX + MSGBOX_MSG_DATA_REG(chan), msg);
}

bool
msgbox_irq(void)
{
	uint32_t status = mmio_read_32(DEV_MSGBOX + MSGBOX_IRQ_STAT_REG);
	bool handled    = false;

	for (uint8_t chan = 0; chan < MSGBOX_CHAN_COUNT; chan += 2) {
		if (status & MSGBOX_RX_IRQ(chan)) {
			handled = true;
			while (msgbox_peek_data(chan))
				msgbox_handle_msg(chan);
			/* Clear the IRQ once the FIFO is empty. */
			mmio_write_32(DEV_MSGBOX + MSGBOX_IRQ_STAT_REG,
			              MSGBOX_RX_IRQ(chan));
		}
	}

	return handled;
}

void
msgbox_init(void)
{
	/* Enable MSGBOX bus clock. */
	ccu_enable_clock(CCU_CLOCK_MSGBOX);

	/* Set even channels ARM -> SCP and odd channels SCP -> ARM. */
	mmio_write_32(DEV_MSGBOX + MSGBOX_CTRL_REG0, MSGBOX_CTRL_NORMAL);
	mmio_write_32(DEV_MSGBOX + MSGBOX_CTRL_REG1, MSGBOX_CTRL_NORMAL);

	/* Drain messages in RX channels (required to clear IRQs). */
	for (uint8_t chan = 0; chan < MSGBOX_CHAN_COUNT; chan += 2) {
		while (msgbox_peek_data(chan))
			mmio_read_32(DEV_MSGBOX + MSGBOX_MSG_DATA_REG(chan));
	}

	/* Disable and clear the status of all IRQs. */
	mmio_write_32(DEV_MSGBOX + MSGBOX_IRQ_EN_REG, 0);
	mmio_write_32(DEV_MSGBOX + MSGBOX_IRQ_STAT_REG, GENMASK(15, 0));

	/* Enable the IRQ. */
	r_intc_enable_irq(IRQ_MSGBOX);
}
