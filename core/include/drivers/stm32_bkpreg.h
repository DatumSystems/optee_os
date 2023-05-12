/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2023, Datum Systems, Inc.
 */

#ifndef __STM32_BKPREG_H
#define __STM32_BKPREG_H

#include <compiler.h>
#include <stdint.h>
#include <tee_api.h>

#define BKPREG_BITS_PER_REG		(8U * sizeof(uint32_t))
#define BKPREG_BYTES_PER_REG 	(sizeof(uint32_t))
#define BKPREG_NUM_REGS 		32


/*
 * Read BKP register
 * @value: Output read value
 * @otp_id: BKP register number
 * Return a TEE_Result compliant return value
 */
TEE_Result stm32_read_bkpreg(uint32_t *value, uint32_t bkpreg_id);

/*
 * Read an BKP register data value
 * @value: Output read value
 * @otp_id: BKP register number
 * Return a TEE_Result compliant return value
 */
TEE_Result stm32_write_bkpreg(uint32_t value, uint32_t bkpreg_id);

#endif /*__STM32_BKPREG_H */
