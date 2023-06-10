/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2023, Datum Systems, Inc. - All Rights Reserved
 */

#ifndef __PTA_BKPREG_H
#define __PTA_BKPREG_H

#define PTA_BKPREG_UUID { 0xa3eea217, 0xfa8d, 0x4541, \
	{ 0x8d, 0xf6, 0x2c, 0x88, 0xae, 0x94, 0x99, 0x20 } }

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
#define PTA_BKPREG_READ_REG		0x0

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
#define PTA_BKPREG_WRITE_REG	0x1

#endif /* __PTA_BKPREG_H */
