/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, STMicroelectronics
 */

#ifndef TA_STM32MP_BKPREG_H
#define TA_STM32MP_BKPREG_H

#define TA_STM32MP_BKPREG_UUID { 0xdeda93db, 0x8f6b, 0x4f62, \
		{ 0x98, 0x98, 0x39, 0x22, 0xbb, 0xd8, 0x9f, 0xe8, } }

/**
 * Read TAMP BKP registers
 *
 * [in]		value[0].a			BKP REG start offset in byte
 * [in]		value[0].b			Not used
 * [out]	memref[1].buffer	Output buffer to store read byte values
 * [out]	memref[1].size		Bumber of bytes to be read (4 byte steps)
 *
 * Return codes:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 */
#define TA_STM32MP_BKPREG_READ		0x0

/**
 * Write TAMP BKP registers
 *
 * [in]		value[0].a			BKP REG offset in byte
 * [in]		value[0].b			Not used
 * [in]		memref[1].buffer	Input buffer to read values
 * [in]		memref[1].size		Number of bytes to be written (4 byte steps)
 *
 * Return codes:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 */
#define TA_STM32MP_BKPREG_WRITE		0x1

#endif /* TA_STM32MP_BKPREG_H */
