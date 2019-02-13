/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_IRQCHIP_R_INTC_H
#define DRIVERS_IRQCHIP_R_INTC_H

#include <stdint.h>

enum {
	IRQ_NMI      = 0x00,
	IRQ_R_TIMER0 = 0x01,
	IRQ_R_TIMER1 = 0x02,
	IRQ_R_WDOG   = 0x04,
	IRQ_R_CIR_RX = 0x05,
	IRQ_R_UART   = 0x06,
#if CONFIG_RSB
	IRQ_R_RSB    = 0x07,
#endif
	IRQ_R_ALARM0 = 0x08,
	IRQ_R_ALARM1 = 0x09,
	IRQ_R_TIMER2 = 0x0a,
	IRQ_R_TIMER3 = 0x0b,
	IRQ_R_I2C    = 0x0c,
	IRQ_R_PIO_PL = 0x0d,
	IRQ_R_TWD    = 0x0e,
	IRQ_MSGBOX   = 0x11,
};

enum {
	NMI_LOW_LEVEL    = 0,
	NMI_FALLING_EDGE = 1,
	NMI_HIGH_LEVEL   = 2,
	NMI_RISING_EDGE  = 3,
};

/**
 * Set the type of the NMI.
 *
 * @param type One of the edge or level trigger types defined in this file.
 */
void r_intc_configure_nmi(uint32_t type);

/**
 * Disable an IRQ.
 *
 * @param irq The IRQ number.
 */
void r_intc_disable_irq(uint8_t irq);

/**
 * Enable an IRQ.
 *
 * @param irq The IRQ number.
 */
void r_intc_enable_irq(uint8_t irq);

/**
 * Mask an IRQ.
 *
 * @param irq The IRQ number.
 */
void r_intc_mask_irq(uint8_t irq);

/**
 * Unmask an IRQ.
 *
 * @param irq The IRQ number.
 */
void r_intc_unmask_irq(uint8_t irq);

/**
 * Handle an IRQ coming from R_INTC.
 */
void r_intc_irq(void);

/**
 * Initialize the R_INTC driver.
 */
void r_intc_init(void);

#endif /* DRIVERS_IRQCHIP_R_INTC_H */
