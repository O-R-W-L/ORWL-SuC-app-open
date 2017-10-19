/**===========================================================================
 * @file mem_common.h
 *
 * @brief This file contains macros and function declaration for
 * mem_common.c
 *
 * @author megharaj.ag@design-shift.com
 *
 ============================================================================
 *
 * Copyright © Design SHIFT, 2017-2018
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright.
 *     * Neither the name of the [ORWL] nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY DESIGN SHIFT ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL DESIGN SHIFT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ============================================================================
 *
 */
#ifndef commonINCLUDE_MEM_COMMON_H_
#define commonINCLUDE_MEM_COMMON_H_

/* Global includes */
#include <stdint.h>

/* Common macros */
/**
 * Partition one
 */
#define commonPARTITION1	(1)

/**
 * Partition two
 */
#define commonPARTITION2	(2)

/**
 * None of the partition.
 */
#define commonPARTITIONNONE	(3)

/**
 * Initialize memory with oxff.
 */
#define commonDEFAULT_VALUE	(0xff)

/* function declaration */

/** @brief Choose valid partition.
 *
 * This function checks both the partition and informs which partition
 * contains valid magic header. If both the partition does not contains
 * valid header than return partition 1.
 *
 * @param ulPar1Address Partition 1 address.
 * @param ulPar2Address Partition 2 address.
 * @param ulMagicNum Valid magic number that need to be checked in flash.
 * @param ulSize Size of magic header.
 * @return valid partition.
 *
 */
int32_t lCommonChoosePartition(uint32_t ulPar1Address, uint32_t ulPar2Address,
	uint32_t ulMagicNum, uint32_t ulSize);

/** @brief Update partition.
 *
 * This function updates the partition with valid data. Takes care of poweroff
 * condition by updating magic header last.
 * NOTE : Before calling this function critical section should be enabled can
 * be disabled after executing this function, since flash erase and write
 * needs critical section.
 * This function needs scheduler to be suspended before calling, once this
 * function returns then scheduler can be resumed.
 *
 * @param ulAddress Partition address..
 * @param ulDataSize Data size.
 * @param pucDataBuffer Pointer to data buffer.
 * @param ulMagicNum Magic number/header.
 * @param ulMagicSize Size of magic number..
 * @return error code.
 *
 */
int32_t lCommonUpdatePartition(uint32_t ulAddress, uint32_t ulDataSize,
	uint8_t *pucDataBuffer, uint32_t ulMagicNum, uint32_t ulMagicSize);
/** @brief Erase Flash, RTOS API
 *
 * This function erase flash according to address and size. Use this function
 * only in freeRTOS context, since it uses disable/enable interrupt/scheduler
 * RTOS API's.
 *
 * @param ulAddress is the starting address from where erase should start.
 * @param ulLenth is the size of flash to be erase.
 *
 * @return error code.
 *
 */
int32_t lCommonEraseFlashRTOS (uint32_t ulAddress, uint32_t ulLength);

/** @brief Erase Flash
 *
 * This function erase flash according to address and size. Use this function
 * only in Non freeRTOS context.
 *
 * @param ulAddress is the starting address of part to be erase.
 * @param ulLen is the size of flash to be erase.
 *
 * @return error code.
 *
 */
int32_t lCommonEraseFlash (uint32_t ulAddress, uint32_t ulLen);

/** @brief Erase of Flash partition
 *
 * This function erase encryption key partition of the flash.
 *
 * @return error code.
 */
int32_t lcommonEraseEncyKey ( void );

/** @brief Erase of Flash partition
 *
 * This function erase access key partition of the flash.
 *
 * @return error code.
 */
int32_t lcommonEraseAccessKey ( void );

/** @brief Erase of Flash partition
 *
 * This function erase keyfob Id partition of the flash.
 *
 * @return error code.
 */
int32_t lcommonEraseKeyFobId( void );

/** @brief Erase of Flash partition
 *
 * This function erase user config data partition of the flash.
 *
 * @return error code.
 */
int32_t lcommonEraseUserConfigData( void );

#endif /*commonINCLUDE_MEM_COMMON_H_ */
