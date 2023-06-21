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

#define KEY_STORE_OFFSET_A		(0)
#define KEY_STORE_OFFSET_B		KEY_STORE_OFFSET_A + NUM_KEYS * KEY_SIZE + CRC32_SIZE

static size_t session_refcount;
static TEE_TASessionHandle pta_session = TEE_HANDLE_NULL;

static TEE_Result bkpsram_zeroize_keys(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS]);

// CRC-32 Init Value
#define Crc32Init 	0xffffffffu

//	X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
static uint32_t Crc32Tbl[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

// Calculate CRC-32
static uint32_t Crc32(const void *pMem, uint32_t uiSize)
{
	uint32_t uiCrc = Crc32Init;
	uint8_t *pBuf = (uint8_t *)(pMem);

	while(uiSize--)
		uiCrc = (uiCrc >> 8) ^ Crc32Tbl[(uint8_t)(uiCrc) ^ *(uint8_t *)(pBuf++)];
	return(uiCrc);
}

static bool bkpsram_check_keystore(void)
{
	uint32_t pta;
	TEE_Result res = TEE_SUCCESS;
	TEE_Param params_pta[TEE_NUM_PARAMS] = { };
	uint8_t keystore_a[NUM_KEYS * KEY_SIZE + CRC32_SIZE];
	uint8_t keystore_b[NUM_KEYS * KEY_SIZE + CRC32_SIZE];
	uint32_t crc32a_stor, crc32a_calc, crc32b_stor, crc32b_calc;

	/* Read BKPSRAM PTA */
	pta = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
		  TEE_PARAM_TYPE_MEMREF_OUTPUT,
		  TEE_PARAM_TYPE_NONE,
		  TEE_PARAM_TYPE_NONE);
	params_pta[0].value.a = KEY_STORE_OFFSET_A;
	params_pta[1].memref.buffer = keystore_a;
	params_pta[1].memref.size = NUM_KEYS * KEY_SIZE + CRC32_SIZE;
	res = TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE, PTA_BKPSRAM_READ, pta, params_pta, NULL);
	if(res != TEE_SUCCESS)
		return true;
	/* Read BKPSRAM PTA */
	params_pta[0].value.a = KEY_STORE_OFFSET_B;
	params_pta[1].memref.buffer = keystore_b;
	params_pta[1].memref.size = NUM_KEYS * KEY_SIZE + CRC32_SIZE;
	res = TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE, PTA_BKPSRAM_READ, pta, params_pta, NULL);
	if(res != TEE_SUCCESS)
		return true;

	memcpy(&crc32a_stor, &keystore_a[NUM_KEYS * KEY_SIZE], CRC32_SIZE);
	memcpy(&crc32b_stor, &keystore_b[NUM_KEYS * KEY_SIZE], CRC32_SIZE);
	crc32a_calc = Crc32(keystore_a, NUM_KEYS * KEY_SIZE);
	crc32b_calc = Crc32(keystore_b, NUM_KEYS * KEY_SIZE);


	if(crc32a_stor == crc32a_calc)
	{
		if(crc32b_stor != crc32b_calc)
		{
			// a is good, b is bad, write a over b
			/* Write BKPSRAM PTA */
			pta = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
				TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_NONE,
				TEE_PARAM_TYPE_NONE);
			params_pta[0].value.a = KEY_STORE_OFFSET_B;
			params_pta[1].memref.buffer = keystore_a;
			params_pta[1].memref.size = NUM_KEYS * KEY_SIZE + CRC32_SIZE;
			res = TEE_InvokeTACommand(pta_session,	TEE_TIMEOUT_INFINITE, PTA_BKPSRAM_WRITE, pta, params_pta, NULL);
		}
		return false;
	}
	else if(crc32b_stor == crc32b_calc)
	{
		if(crc32a_stor != crc32a_calc)
		{
			// b is good, a is bad, write b over a
			/* Write BKPSRAM PTA */
			pta = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
				  TEE_PARAM_TYPE_MEMREF_INPUT,
				  TEE_PARAM_TYPE_NONE,
				  TEE_PARAM_TYPE_NONE);
			params_pta[0].value.a = KEY_STORE_OFFSET_A;
			params_pta[1].memref.buffer = keystore_b;
			params_pta[1].memref.size = NUM_KEYS * KEY_SIZE + CRC32_SIZE;
			res = TEE_InvokeTACommand(pta_session,	TEE_TIMEOUT_INFINITE, PTA_BKPSRAM_WRITE, pta, params_pta, NULL);
		}
		return false;
	}
	else 
	{
		// a and b are bad
		/* Zeroize all keys BKPSRAM PTA */
		pta = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
			  TEE_PARAM_TYPE_NONE,
			  TEE_PARAM_TYPE_NONE,
			  TEE_PARAM_TYPE_NONE);
		params_pta[0].value.a = (1 << NUM_KEYS) - 1;
		bkpsram_zeroize_keys(pta, params_pta);
		return true;
	}
}

static TEE_Result bkpsram_update_crc32(uint32_t offset)
{
	uint32_t pta;
	TEE_Result res = TEE_SUCCESS;
	TEE_Param params_pta[TEE_NUM_PARAMS] = { };
	uint8_t keystore[NUM_KEYS * KEY_SIZE];
	uint32_t crc32;

	/* Read BKPSRAM PTA */
 	pta = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
		TEE_PARAM_TYPE_MEMREF_OUTPUT,
		TEE_PARAM_TYPE_NONE,
		TEE_PARAM_TYPE_NONE);
	params_pta[0].value.a = offset;
	params_pta[0].value.b = 0;
	params_pta[1].memref.buffer = keystore;
	params_pta[1].memref.size = NUM_KEYS * KEY_SIZE;
	res = TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE, PTA_BKPSRAM_READ, pta, params_pta, NULL);
	if(res == TEE_SUCCESS)
	{
		/* Write BKPSRAM PTA */
		pta = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
			  TEE_PARAM_TYPE_MEMREF_INPUT,
			  TEE_PARAM_TYPE_NONE,
			  TEE_PARAM_TYPE_NONE);
		crc32 = Crc32(keystore, NUM_KEYS * KEY_SIZE);
		params_pta[0].value.a = NUM_KEYS * KEY_SIZE;
		params_pta[0].value.b = 0;
		params_pta[1].memref.buffer = (uint8_t *)(&crc32);
		params_pta[1].memref.size =  CRC32_SIZE;
		res = TEE_InvokeTACommand(pta_session,	TEE_TIMEOUT_INFINITE, PTA_BKPSRAM_WRITE, pta, params_pta, NULL);
	}
	return res;
}

static TEE_Result bkpsram_read(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_MEMREF_OUTPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);
	uint8_t *out = (uint8_t *)params[1].memref.buffer;
	size_t out_size = params[1].memref.size;
	uint32_t out_start = params[0].value.a;
	TEE_Result res = TEE_SUCCESS;
	TEE_Param params_pta[TEE_NUM_PARAMS] = { };

	if (pt != exp_pt || !out || !out_size)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Read BKPSRAM PTA */
	params_pta[0].value.a = out_start;
	params_pta[1].memref.buffer = out;
	params_pta[1].memref.size = out_size;

	res = TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE, PTA_BKPSRAM_READ, pt, params_pta, NULL);
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

	/* Write BKPSRAM PTA */
	params_pta[0].value.a = in_start;
	params_pta[1].memref.buffer = in;
	params_pta[1].memref.size = in_size;
	res = TEE_InvokeTACommand(pta_session,	TEE_TIMEOUT_INFINITE, PTA_BKPSRAM_WRITE, pt, params_pta, NULL);
	return res;
}

static TEE_Result bkpsram_readkey(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_MEMREF_OUTPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);
	TEE_Result res = TEE_SUCCESS;
	TEE_Param params_pta[TEE_NUM_PARAMS] = { };
	uint8_t *out;
	uint32_t index;

	if(bkpsram_check_keystore())
	{
		res = TEE_ERROR_BAD_STATE;
	}
	else
	{
		out = (uint8_t *)params[1].memref.buffer;
		index = params[0].value.a;

		if (pt != exp_pt || !out || index > (NUM_KEYS - 1))
			return TEE_ERROR_BAD_PARAMETERS;

		/* Read BKPSRAM PTA */
		params_pta[0].value.a = KEY_STORE_OFFSET_A + index * KEY_SIZE;
		params_pta[0].value.b = 0;
		params_pta[1].memref.buffer = out;
		params_pta[1].memref.size = KEY_SIZE;
		res = TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE, PTA_BKPSRAM_READ, pt, params_pta, NULL);
	}
	return res;
}

static TEE_Result bkpsram_writekey(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_MEMREF_INPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);
	TEE_Result res = TEE_SUCCESS;
	TEE_Param params_pta[TEE_NUM_PARAMS] = { };
	size_t in_size = params[1].memref.size;
	uint32_t *in = (uint32_t *)params[1].memref.buffer;
	uint32_t index = params[0].value.a;
	if (pt != exp_pt || !in || in_size != KEY_SIZE || index > (NUM_KEYS - 1))
		return TEE_ERROR_BAD_PARAMETERS;

	/* Write Keys to Keystore A and B*/
	params_pta[0].value.a = KEY_STORE_OFFSET_A + index * KEY_SIZE;
	params_pta[0].value.b = 0;
	params_pta[1].memref.buffer = in;
	params_pta[1].memref.size = KEY_SIZE;
	res = TEE_InvokeTACommand(pta_session,	TEE_TIMEOUT_INFINITE, PTA_BKPSRAM_WRITE, pt, params_pta, NULL);

	params_pta[0].value.a = KEY_STORE_OFFSET_B + index * KEY_SIZE;
	if(res == TEE_SUCCESS)
		res = TEE_InvokeTACommand(pta_session,	TEE_TIMEOUT_INFINITE, PTA_BKPSRAM_WRITE, pt, params_pta, NULL);
	if(res == TEE_SUCCESS)
		res = bkpsram_update_crc32(KEY_STORE_OFFSET_A);
	if(res == TEE_SUCCESS)
		res = bkpsram_update_crc32(KEY_STORE_OFFSET_B);
	return res;
}

static TEE_Result bkpsram_zeroize_keys(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);
	const uint32_t pta   =  TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_MEMREF_INPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);

	TEE_Result res = TEE_SUCCESS;
	TEE_Param params_pta[TEE_NUM_PARAMS] = { };
	uint8_t key_z[KEY_SIZE] = {0};
	uint32_t mask = params[0].value.a;
	uint32_t index = 0;
	
	if (pt != exp_pt || !mask || mask > (1 << NUM_KEYS) - 1)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Write zero keyBKPSRAM PTA */
	params_pta[1].memref.buffer = key_z;
	params_pta[1].memref.size = KEY_SIZE;
	while(mask && res == TEE_SUCCESS)
	{
		if(mask & 1)
		{
			params_pta[0].value.a = index;
			res = bkpsram_writekey(pta, params_pta);
		}
		mask >>= 1;
		index++;
	}
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
	static const TEE_UUID uuid = PTA_BKPSRAM_UUID;
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
	case TA_STM32MP_BKPSRAM_READKEY:
		return bkpsram_readkey(pt, params);
	case TA_STM32MP_BKPSRAM_WRITEKEY:
		return bkpsram_writekey(pt, params);
	case TA_STM32MP_BKPSRAM_ZEROKEYS:
		return bkpsram_zeroize_keys(pt, params);
	default:
		EMSG("Command ID %#"PRIx32" is not supported", cmd);
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
