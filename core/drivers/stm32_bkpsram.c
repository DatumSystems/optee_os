// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2023, Datum Systems, Inc.
 */

#include <assert.h>
#include <config.h>
#include <drivers/stm32mp_dt_bindings.h>
#include <drivers/stm32_bkpsram.h>
#include <drivers/stm32mp1_pwr.h>
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

TEE_Result stm32_read_bkpsram_byte(uint8_t *value, vaddr_t offset)
{
	vaddr_t bkpsram_addr = stm32mp_bkpsram_base() + offset;
	*value = io_read8(bkpsram_addr);
	return TEE_SUCCESS;
}

TEE_Result stm32_write_bkpsram_byte(uint8_t value, vaddr_t offset)
{
	vaddr_t bkpsram_addr = stm32mp_bkpsram_base() + offset;
	io_write8(bkpsram_addr, value);
	return TEE_SUCCESS;
}

static TEE_Result init_bkpsram(void)
{
	struct clk *bksram_clk = stm32mp_rcc_clock_id_to_clk(BKPSRAM);
	if(!bksram_clk)
		panic();

	/* Keep backup RAM content in standby */
	io_setbits32(stm32_pwr_base() + PWR_CR2_OFF, PWR_CR2_BREN);

	clk_enable(bksram_clk);
	return TEE_SUCCESS;
}
driver_init(init_bkpsram);
