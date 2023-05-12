// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2023, Datum Systems, Inc. - All Rights Reserved
 */

#include <config.h>
#include <drivers/stm32_bsec.h>
#include <kernel/pseudo_ta.h>
#include <kernel/user_access.h>
#include <kernel/user_mode_ctx.h>
#include <kernel/user_ta.h>
#include <mm/vm.h>
#include <pta_bsec.h>
#include <string.h>
#include <util.h>

#define TA_STM32MP_BKPREG_UUID { 0xdeda93db, 0x8f6b, 0x4f62, \
		{ 0x98, 0x98, 0x39, 0x22, 0xbb, 0xd8, 0x9f, 0xe8, } }

#define PTA_NAME "bkpreg.pta"

#define BKPREG_BYTES_PER_REG (sizeof(uint32_t))
#define BKPREG_NUM_REGS 32

// static TEE_Result bsec_check_access(uint32_t otp_id)
// {
// 	struct ts_session *ts = ts_get_current_session();
// 	struct tee_ta_session *ta_session = to_ta_session(ts);

// 	/* REE kernel is allowed to access non secure OTP */
// 	if (ta_session->clnt_id.login == TEE_LOGIN_REE_KERNEL) {
// 		if (!stm32_bsec_nsec_can_access_otp(otp_id))
// 			return TEE_ERROR_ACCESS_DENIED;

// 		return TEE_SUCCESS;
// 	}

// 	if (stm32_bsec_can_access_otp(otp_id))
// 		return TEE_ERROR_ACCESS_DENIED;

// 	/* All TA have access to fuses */
// 	return TEE_SUCCESS;
// }

static TEE_Result bkpreg_read(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_MEMREF_OUTPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);

	uint32_t *buf = (uint32_t *)params[1].memref.buffer;
	uint32_t bkpreg_start = 0;
	size_t bkpreg_length = 0;
	uint32_t bkpreg_id = 0;
	size_t size = params[1].memref.size;

	if (pt != exp_pt || !buf || !size)
		return TEE_ERROR_BAD_PARAMETERS;

	/* check 32bits alignment and block size*/
	if (params[0].value.a % BKPREG_BYTES_PER_REG || size % BKPREG_BYTES_PER_REG || (params[0].value.a + size) > (BKPREG_NUM_REGS * 4))
		return TEE_ERROR_BAD_PARAMETERS;

	bkpreg_start = params[0].value.a / BKPREG_BYTES_PER_REG;
	bkpreg_length = size / BKPREG_BYTES_PER_REG;

	for (bkpreg_id = bkpreg_start; bkpreg_id < bkpreg_start + bkpreg_length; bkpreg_id++, buf++)
		stm32_read_bkpreg(buf, bkpreg_id);

	return TEE_SUCCESS;
}

static TEE_Result bkpreg_write(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_MEMREF_INPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);

	uint32_t *buf = (uint32_t *)params[1].memref.buffer;
	uint32_t bkpreg_start = 0;
	size_t bkpreg_length = 0;
	uint32_t bkpreg_id = 0;
	size_t size = params[1].memref.size;

	if (pt != exp_pt || !buf || !size)
		return TEE_ERROR_BAD_PARAMETERS;

	/* check 32bits alignment and block size */
	if (params[0].value.a % BSEC_BYTES_PER_WORD || size % BSEC_BYTES_PER_WORD || (params[0].value.a + size) > (BKPREG_NUM_REGS * 4))
		return TEE_ERROR_BAD_PARAMETERS;

	bkpreg_start = params[0].value.a / BKPREG_BYTES_PER_REG;
	bkpreg_length = size / BKPREG_BYTES_PER_REG;


	for (bkpreg_id = bkpreg_start; bkpreg_id < bkpreg_start + bkpreg_length; bkpreg_id++, buf++)
		stm32_write_bkpreg(*buf, bkpreg_id);
	return TEE_SUCCESS;
}

// static TEE_Result bsec_pta_state(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
// {
// 	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT,
// 						TEE_PARAM_TYPE_NONE,
// 						TEE_PARAM_TYPE_NONE,
// 						TEE_PARAM_TYPE_NONE);
// 	TEE_Result res = TEE_ERROR_GENERIC;
// 	uint32_t state = BSEC_STATE_INVALID;

// 	if (pt != exp_pt)
// 		return TEE_ERROR_BAD_PARAMETERS;

// 	res = stm32_bsec_get_state(&state);
// 	if (res)
// 		return res;

// 	params[0].value.a = state;

// 	return TEE_SUCCESS;
// }

static TEE_Result bkpreg_pta_invoke_command(void *pSessionContext __unused,
					  uint32_t cmd_id,
					  uint32_t param_types,
					  TEE_Param params[TEE_NUM_PARAMS])
{
	FMSG(PTA_NAME" command %#"PRIx32" ptypes %#"PRIx32, cmd_id, param_types);

	switch (cmd_id) {
	case PTA_BSEC_READ_MEM:
		return bsec_read_mem(param_types, params);
	case PTA_BSEC_WRITE_MEM:
		return bsec_write_mem(param_types, params);
	case PTA_BSEC_GET_STATE:
		return bsec_pta_state(param_types, params);
	default:
		break;
	}

	return TEE_ERROR_NOT_IMPLEMENTED;
}

static TEE_Result pta_bsec_open_session(uint32_t ptypes __unused,
					TEE_Param par[TEE_NUM_PARAMS] __unused,
					void **session __unused)
{
	uint32_t login = to_ta_session(ts_get_current_session())->clnt_id.login;
	struct ts_session *caller_ts = ts_get_calling_session();
	static const TEE_UUID ta_uuid = TA_STM32MP_NVMEM_UUID;
	uint32_t state = BSEC_STATE_INVALID;
	TEE_Result res = TEE_ERROR_GENERIC;

	if (!IS_ENABLED(CFG_STM32_BSEC))
		return TEE_ERROR_NOT_SUPPORTED;

	if (login == TEE_LOGIN_TRUSTED_APP &&
	    !memcmp((char *)&caller_ts->ctx->uuid, (char *)&ta_uuid,
		    sizeof(TEE_UUID))) {
		assert(is_user_ta_ctx(caller_ts->ctx));

		res = stm32_bsec_get_state(&state);
		if (res || state != BSEC_STATE_SEC_OPEN)
			return TEE_ERROR_ACCESS_DENIED;

		return TEE_SUCCESS;
	}

	if (login == TEE_LOGIN_REE_KERNEL)
		return TEE_SUCCESS;

	return TEE_ERROR_ACCESS_DENIED;
}

pseudo_ta_register(.uuid = PTA_BSEC_UUID, .name = PTA_NAME,
		   .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT |
			    TA_FLAG_DEVICE_ENUM,
		   . open_session_entry_point = pta_bsec_open_session,
		   .invoke_command_entry_point = bsec_pta_invoke_command);
