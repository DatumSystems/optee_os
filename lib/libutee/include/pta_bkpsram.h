/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2023, Datum Systems, Inc. - All Rights Reserved
 */

#ifndef __PTA_BKPSRAM_H
#define __PTA_BKPSRAM_H

#define PTA_BKPSRAM_UUID { 0x4c3f4bb0, 0x615d, 0x48d9, \
	{ 0x81, 0xfc, 0x36, 0x34, 0x15, 0xf1, 0x04, 0x40 } }

/**
 * Read BKPSRAM memory
 *
 * [in]		value[0].a			BKPSRAM read offset in bytes
 * [in]		value[0].b			Unused
 * [out]	memref[1].buffer	Output buffer to store read byte values
 * [out]	memref[1].size		BKPSRAM number of bytes to be read
 *
 * Return codes:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 */
#define PTA_BKPSRAM_READ		0x0

/**
 * Write BKPSRAM memory
 *
 * [in]		value[0].a			BKPSRAM write offset in bytes
 * [in]		value[0].b			Unused
 * [in]		memref[1].buffer	Input buffer with byte values to write
 * [in]		memref[1].size		BKPSRAM number of bytes to write
 *
 * Return codes:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 */
#define PTA_BKPSRAM_WRITE		0x1

#endif /* __PTA_BKPSRAM_H */
