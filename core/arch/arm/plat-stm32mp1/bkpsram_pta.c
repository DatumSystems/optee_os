// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2023, Datum Systems, Inc. - All Rights Reserved
 */

#include <config.h>
#include <drivers/stm32_bkpsram.h>
#include <kernel/pseudo_ta.h>
#include <kernel/user_access.h>
#include <kernel/user_mode_ctx.h>
#include <kernel/user_ta.h>
#include <mm/vm.h>
#include <pta_bkpsram.h>
#include <string.h>
#include <util.h>

// mcarlin@MARK-DATUM-PC-PRECISION-5820:~/dev/brick/br/github/datum/optee_os$ uuidgen
// 61148ca7-ea00-4294-8971-560d6d981207
#define TA_STM32MP_BKPSRAM_UUID { 0x61148ca7, 0xea00, 0x4294, \
		{ 0x89, 0x71, 0x56, 0x0d, 0x6d, 0x98, 0x12, 0x07, } }

#define PTA_NAME "bkpsram.pta"

static TEE_Result bkpsram_read(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_MEMREF_OUTPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);

	uint32_t *buf = (uint32_t *)params[1].memref.buffer;
	uint32_t start = 0;
	uint32_t addr = 0;
	size_t size = params[1].memref.size;

	if (pt != exp_pt || !buf || !size)
		return TEE_ERROR_BAD_PARAMETERS;

	/* check block size does not overflow BKPSRAM */
	if( (BKPSRAM_BASE + BKPSRAM_SIZE) > params[0].value.a || (BKPSRAM_BASE + BKPSRAM_SIZE) > (params[0].value.a + size))
		return TEE_ERROR_BAD_PARAMETERS;

	start = params[0].value.a;

	for(addr = start; addr < start + size; addr++, buf++)
		stm32_read_bkpsram_byte(buf, addr);

	return TEE_SUCCESS;
}

static TEE_Result bkpsram_write(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_MEMREF_INPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);

	uint32_t *buf = (uint32_t *)params[1].memref.buffer;
	uint32_t start = 0;
	size_t length = 0;
	uint32_t addr = 0;
	size_t size = params[1].memref.size;

	if (pt != exp_pt || !buf || !size)
		return TEE_ERROR_BAD_PARAMETERS;

	/* check block size does not overflow BKPSRAM */
	if( (BKPSRAM_BASE + BKPSRAM_SIZE) > params[0].value.a || (BKPSRAM_BASE + BKPSRAM_SIZE) > (params[0].value.a + size))
		return TEE_ERROR_BAD_PARAMETERS;

	start = params[0].value.a;


	for(addr = start; addr < start + size; addr++, buf++)
		stm32_write_bkpsram_byte(*buf, addr);
	return TEE_SUCCESS;
}

static TEE_Result pta_bkpreg_invoke_command(void *pSessionContext __unused,
					  uint32_t cmd_id,
					  uint32_t param_types,
					  TEE_Param params[TEE_NUM_PARAMS])
{
	FMSG(PTA_NAME" command %#"PRIx32" ptypes %#"PRIx32, cmd_id, param_types);

	switch (cmd_id) {
	case PTA_BKPSRAM_READ:
		return bkpsram_read(param_types, params);
	case PTA_BKPREG_WRITE:
		return bkpsram_write(param_types, params);
	default:
		break;
	}

	return TEE_ERROR_NOT_IMPLEMENTED;
}

static TEE_Result pta_bkpreg_open_session(uint32_t ptypes __unused,
					TEE_Param par[TEE_NUM_PARAMS] __unused,
					void **session __unused)
{
	uint32_t login = to_ta_session(ts_get_current_session())->clnt_id.login;
	struct ts_session *caller_ts = ts_get_calling_session();
	static const TEE_UUID ta_uuid = TA_STM32MP_BKPSRAM_UUID;

	if (!IS_ENABLED(CFG_STM32_TAMP))
		return TEE_ERROR_NOT_SUPPORTED;

	if (login == TEE_LOGIN_TRUSTED_APP &&
	    !memcmp((char *)&caller_ts->ctx->uuid, (char *)&ta_uuid,
		    sizeof(TEE_UUID))) {
		assert(is_user_ta_ctx(caller_ts->ctx));

		return TEE_SUCCESS;
	}

	if (login == TEE_LOGIN_REE_KERNEL)
		return TEE_SUCCESS;

	return TEE_ERROR_ACCESS_DENIED;
}

pseudo_ta_register(.uuid = PTA_BKPREG_UUID, .name = PTA_NAME,
		   .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT |
			    TA_FLAG_DEVICE_ENUM,
		   . open_session_entry_point = pta_bkpreg_open_session,
		   .invoke_command_entry_point = pta_bkpreg_invoke_command);
