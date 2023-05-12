// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2023, Datum Systems, Inc.
 */

#include <assert.h>
#include <config.h>
#include <drivers/stm32_bkpreg.h>
 #include <io.h>
 #include <kernel/delay.h>
 #include <kernel/dt.h>
 #include <kernel/boot.h>
 #include <kernel/pm.h>
 #include <kernel/spinlock.h>
 #include <libfdt.h>
 #include <limits.h>
 #include <mm/core_memprot.h>
 #include <platform_config.h>
 #include <stm32_util.h>
 #include <string.h>
 #include <tee_api_defines.h>
 #include <types_ext.h>
 #include <util.h>

TEE_Result stm32_read_bkpreg(uint32_t *value, uint32_t bkpreg_id)
{
	*value = io_read32(stm32mp_bkpreg(bkpreg_id));
	return TEE_SUCCESS;
}

TEE_Result stm32_write_bkpreg(uint32_t value, uint32_t bkpreg_id)
{
	io_write32(stm32mp_bkpreg(bkpreg_id), value);
	return TEE_SUCCESS;
}
