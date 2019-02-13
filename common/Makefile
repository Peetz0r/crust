#
# Copyright © 2017-2019 The Crust Firmware Authors.
# SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
#

obj-y += delay.o
obj-y += exception.o
obj-y += irqf.o
obj-y += main.o
obj-y += reset.o
obj-y += scpi.o
obj-y += system.o

obj-$(CONFIG_DEBUG_LOG) += debug.o

$(call linker_scripts,crust.ld)
