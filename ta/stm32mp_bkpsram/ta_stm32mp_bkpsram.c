// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2023, Datum Systems, Inc.
 */

#include <assert.h>
#include <pta_bkpsram.h>
#include <pta_system.h>
#include <stdlib.h>
#include <string.h>
#include <string_ext.h>
#include <ta_stm32mp_bkpsram.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <util.h>

static size_t session_refcount;
static TEE_TASessionHandle pta_session = TEE_HANDLE_NULL;

static TEE_Result bkpsram_read(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_MEMREF_OUTPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);
	uint32_t *out = (uint32_t *)params[1].memref.buffer;
	uint32_t out_start = params[0].value.a;
	size_t out_size = params[1].memref.size;
	TEE_Result res = TEE_SUCCESS;
	TEE_Param params_pta[TEE_NUM_PARAMS] = { };

	if (pt != exp_pt || !out || !out_size)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Read BKP REG, convert from regesters to bytes */
	params_pta[0].value.a = out_start;
	params_pta[0].value.b = 0;
	params_pta[1].memref.buffer = out;
	params_pta[1].memref.size = out_size;

	res = TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					PTA_BKPSRAM_READ,
					pt, params_pta, NULL);
	return res;
}

static TEE_Result bkpsram_write(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_MEMREF_INPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);
	uint32_t *in = (uint32_t *)params[1].memref.buffer;
	uint32_t in_start = params[0].value.a;
	size_t in_size = params[1].memref.size;
	TEE_Result res = TEE_SUCCESS;
	TEE_Param params_pta[TEE_NUM_PARAMS] = { };

	if (pt != exp_pt || !in || !in_size)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Write BKP REG, convert from registers to bytes */
	params_pta[0].value.a = in_start;
	params_pta[0].value.b = 0;
	params_pta[1].memref.buffer = in;
	params_pta[1].memref.size = in_size;

	res = TEE_InvokeTACommand(pta_session,	TEE_TIMEOUT_INFINITE,
						PTA_BKPSRAM_WRITE,
						pt, params_pta, NULL);
	return res;
}

TEE_Result TA_CreateEntryPoint(void)
{
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t pt __unused,
				    TEE_Param params[TEE_NUM_PARAMS] __unused,
				    void **session __unused)
{
	static const TEE_UUID uuid = PTA_BKPREG_UUID;
	TEE_Result res = TEE_ERROR_GENERIC;
	TEE_PropSetHandle h = TEE_HANDLE_NULL;
	TEE_Identity id = { };

	res = TEE_AllocatePropertyEnumerator(&h);
	if (res)
		goto out;

	TEE_StartPropertyEnumerator(h, TEE_PROPSET_CURRENT_CLIENT);

	res = TEE_GetPropertyAsIdentity(h, NULL, &id);
	if (res)
		goto out;

	if (id.login == TEE_LOGIN_REE_KERNEL) {
		res = TEE_ERROR_ACCESS_DENIED;
		goto out;
	}

	if (!session_refcount) {
		res = TEE_OpenTASession(&uuid, TEE_TIMEOUT_INFINITE, 0, NULL,
					&pta_session, NULL);
		if (res)
			goto out;
	}

	session_refcount++;

out:
	if (h)
		TEE_FreePropertyEnumerator(h);

	return res;
}

void TA_CloseSessionEntryPoint(void *sess __unused)
{
	session_refcount--;

	if (!session_refcount)
		TEE_CloseTASession(pta_session);
}

TEE_Result TA_InvokeCommandEntryPoint(void *sess __unused, uint32_t cmd,
				      uint32_t pt,
				      TEE_Param params[TEE_NUM_PARAMS])
{
	switch (cmd) {
	case TA_STM32MP_BKPSRAM_READ:
		return bkpsram_read(pt, params);
	case TA_STM32MP_BKPSRAM_WRITE:
		return bkpsram_write(pt, params);
	default:
		EMSG("Command ID %#"PRIx32" is not supported", cmd);
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
