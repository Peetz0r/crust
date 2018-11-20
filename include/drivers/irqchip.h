/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_IRQCHIP_H
#define DRIVERS_IRQCHIP_H

#include <dm.h>
#include <intrusive.h>
#include <stdint.h>

#define IRQCHIP_OPS(dev) \
	(&container_of((dev)->drv, struct irqchip_driver, drv)->ops)

#define IRQ_HANDLE &(struct irq_handle)

typedef int irq_callback_t (void *);

struct irq_handle {
	struct irq_handle *next;
	struct device     *dev;
	irq_callback_t    *fn;
	const uint8_t      irq;
	const uint8_t      mode;
};

struct irqchip_driver_ops {
	int (*enable)(struct device *dev, struct irq_handle *handle);
	int (*mask)(struct device *dev, uint8_t id);
	int (*unmask)(struct device *dev, uint8_t id);
};

struct irqchip_driver {
	const struct driver             drv;
	const struct irqchip_driver_ops ops;
};

static inline int
irqchip_enable(struct device *dev, struct irq_handle *handle)
{
	return IRQCHIP_OPS(dev)->enable(dev, handle);
}

static inline int
irqchip_mask(struct device *dev, uint8_t id)
{
	return IRQCHIP_OPS(dev)->mask(dev, id);
}

static inline int
irqchip_unmask(struct device *dev, uint8_t id)
{
	return IRQCHIP_OPS(dev)->unmask(dev, id);
}

#endif /* DRIVERS_IRQCHIP_H */
