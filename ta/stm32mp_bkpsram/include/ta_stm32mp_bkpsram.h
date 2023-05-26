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
 * [in]		value[0].b			Not used
 * [out]	memref[1].buffer	Output buffer to store read byte values
 * [out]	memref[1].size		Number of bytes to read 
 *
 * Return codes:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 */
#define TA_STM32MP_BKPSRAM_READ		0x0

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
#define TA_STM32MP_BKPSRAM_WRITE		0x1

#endif /* TA_STM32MP_BKPSRAM_H */
