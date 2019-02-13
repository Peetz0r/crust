/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef BARRIER_H
#define BARRIER_H

#define barrier() asm volatile ("" : : : "memory")

#endif /* BARRIER_H */
