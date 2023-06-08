/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, STMicroelectronics
 */

#ifndef TA_STM32MP_BKPSRAM_H
#define TA_STM32MP_BKPSRAM_H

#define TA_STM32MP_BKPSRAM_UUID { 0x61148ca7, 0xea00, 0x4294, \
		{ 0x89, 0x71, 0x56, 0x0d, 0x6d, 0x98, 0x12, 0x07, } }

/**
 * Read TAMP BKP registers
 *
 * [in]		value[0].a			BKPSRAM start address offset
 * [in]		value[0].b			BKPSRAM number of bytes to read
 * [out]	memref[1].buffer	Output buffer to store read byte values
 * [out]	memref[1].size		Number of bytes read 
 *
 * Return codes:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 */
#define TA_STM32MP_BKPSRAM_READ			0x915B629D

/**
 * Write TAMP BKP registers
 *
 * [in]		value[0].a			BKPSRAM start address offset
 * [in]		value[0].b			Not used
 * [in]		memref[1].buffer	Input buffer for write data
 * [in]		memref[1].size		Number of bytes to be written
 *
 * Return codes:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 */
#define TA_STM32MP_BKPSRAM_WRITE		0x302122EC

/* 
 * Read 256 bit Encryption Key
 *
 * [in]		value[0].a			Key index (0 to 16)
 * [in]		value[0].b			Unused
 * [out]	memref[1].buffer	Output buffer to store key (256b = 32B)
 * [out]	memref[1].size		Number of bytes in key (32).  If zero then TAMP or error
 *								  and all keys zeroized.
 * Return codes:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 */
#define TA_STM32MP_BKPSRAM_READKEY		0x51143A28

/* 
 * Write 256 bit Encryption Key
 *
 * [in]		value[0].a			Key index (0 to 16)
 * [in]		value[0].b			Unused
 * [in]		memref[1].buffer	Input buffer with key (256b = 32B)
 * [in]		memref[1].size		Number of bytes in key.  Must be 32.
 *
 * Return codes:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 */

#define TA_STM32MP_BKPSRAM_WRITEKEY		0x7FC2C80B

/* 
 * Zeroize Entire Keystore
 *
 * [no parameters]
 * 
 * Return codes:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 */

#define TA_STM32MP_BKPSRAM_ZEROKEYS		0x41A8F9BA 

#endif /* TA_STM32MP_BKPSRAM_H */
