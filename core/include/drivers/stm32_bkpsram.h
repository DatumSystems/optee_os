/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2023, Datum Systems, Inc.
 */

#ifndef __STM32_BKPSRAM_H
#define __STM32_BKPSRAM_H

#include <compiler.h>
#include <stdint.h>
#include <tee_api.h>

/*
 * Read BSKPSRAM byte
 * @value: Output read value
 * @otp_id: BKPSRAM address offset
 * Return a TEE_Result compliant return value
 */
TEE_Result stm32_read_bkpsram_byte(uint8_t *value, vaddr_t offset);

/*
 * Write an BKPSRAM byte
 * @value: Value to write
 * @otp_id: BKPSRAM address offset
 * Return a TEE_Result compliant return value
 */
TEE_Result stm32_write_bkpreg(uint8_t value, vaddr_t offset);

#endif /*__STM32_BKPSRAM_H */
