 /**===========================================================================
 * @file nvsram.h
 *
 * @brief This file contains all the data structures and macros for NVSRAM
 * access
 *
 * @author ravikiran.hv@design-shift.com
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

#ifndef nvsramSNVRAM_INCLUDE_H_
#define nvsramSNVRAM_INCLUDE_H_

/** include all header files required for module build */
#include <debug.h>
#include <stdint.h>

/**
 * Max. SSD Encryption key length
 */
#define nvsramSNVSRAM_ENC_KEY_LEN_MAX	( 4 )

/**
 * Max. Hash length
 */
#define nvsramSNVSRAM_HASH_LEN_MAX	( 32 )

/**
 * Reserved field
 */
#define nvsramSNVSRAM_RESERVED_LEN	( 20 )

/**
 * @brief structure to hold all NVSRAM data.
 */
typedef struct xNVSRAM_DATA
{
    uint32_t ulMagic;					/** Magic number */
    uint32_t ulEncKeyLen;				/** Encryption key length */
    uint32_t ulSSDEncKey[nvsramSNVSRAM_ENC_KEY_LEN_MAX];/** SSD Encryption Key */
    uint8_t  ucHASH[nvsramSNVSRAM_HASH_LEN_MAX];	/** Unique HASH */
    uint16_t usPswdRetryCount;				/** Password retry count*/
    uint32_t ulRTCDelaySec;				/** Delay seconds before next try*/
    uint32_t ulRTCSnapShot;				/** RTC time at time of calculating delay */
    uint8_t  ucRTCSnapshotDirtyBit;			/** ulRTCSnapShot dirty bit for validation */
    uint8_t  ucReserved[nvsramSNVSRAM_RESERVED_LEN];	/** Reserved for future use */
} NvsramData_t;

/** @brief Write NVSRAM partitions
 *
 * This function checks valid partition, Invalidates header before writing,
 * take backup in other partition, writes on NVSRAM & validate header.
 *
 * @param pxData is pointer to the structure.
 *
 * @return error code..
 */
int32_t lNvsramWriteData( NvsramData_t *pxData );

/** @brief Read Encryption key from NVSRAM
 *
 * This function reads encryption key from NVSRAM and populates pointer with it.
 *
 * @param ulEncKeyLen is size of Encryption key to be read.
 * @param pucSSDEncKey is pointer to be populated with enckey.
 *
 * @return error code..
 */
int32_t lNvsramReadSSDEncKey ( uint32_t ulEncKeyLen, uint32_t *pulSSDEncKey );

/** @brief Read hash from NVSRAM
 *
 * This function reads hash from NVSRAM and populates pointer with it.
 *
 * @param pucHashBuf is pointer to be populated with unique hash.
 *
 * @return error code..
 */
int32_t lNvsramReadHash ( uint8_t *pucHashBuf );

/** @brief Read the password retry count from NVSRAM.
 *
 *  This function reads password retry count from NVSRAM and updates the pointer
 *  passed by the application. This also reads the RTC delay seconds to wait before
 *  next PIN entry.
 *
 *  @param pusPswdRetryCount pointer variable used by application.
 *  @param pulRTCDelaySec RTC delay time to wait before next PIN entry.
 *  @param pulRTCSnapShot Snapshot of RTC raw seconds at time of delay set.
 *
 *  @return NO_ERROR on success or error code on failure.
 */
int32_t lNvsramReadPswdRetryCountDelay(uint16_t *pusPswdRetryCount,
	uint32_t *pulRTCDelaySec, uint32_t *pulRTCSnapShot);

/** @brief Read the password retry count from NVSRAM.
 *
 *  This function reads password retry count from NVSRAM and updates the pointer
 *  passed by the application. This also reads the RTC delay seconds to wait before
 *  next PIN entry.
 *
 *  @param pusPswdRetryCount pointer variable used by application.
 *  @param pulRTCDelaySec RTC delay time to wait before next PIN entry.
 *  @param pulRTCSnapShot Snapshot of RTC raw seconds at time of delay set.
 *
 *  @return NO_ERROR on success or error code on failure.
 */
int32_t lNvsramWritePswdRetryCountDelay(uint16_t *pusPswdRetryCount,
	uint32_t *pulRTCDelaySec, uint32_t *pulRTCSnapShot);

/** @brief Reset's password retry count.
 *
 *  This function reset's password retry count to 0, if valid password is entered.
 *  Also resets the RTC delay seconds.
 *
 *  @return NO_ERROR on success or error code on failure
 */
int32_t lNvsramResetPswdRetryCountDelay ( void );

/** @brief Set rtc snapshot dirty bit.
 *
 *  This function sets RTC snapshot dirty bit. This is will be helpful in validating
 *  if wait delay and RTC snapshot was updated properly when RTC was set by user.
 *
 *  @return NO_ERROR on success or error code on failure
 */
int32_t lNvsramSetRTCSnapshotDirtyBit ( void );

/** @brief Reset rtc snapshot dirty bit.
 *
 *  This function resets RTC snapshot dirty bit. This is will be helpful in validating
 *  if wait delay and RTC snapshot was updated properly when RTC was set by user.
 *
 *  @return NO_ERROR on success or error code on failure
 */
int32_t lNvsramResetRTCSnapshotDirtyBit ( void );

/** @brief reads rtc snapshot dirty bit.
 *
 *  This function reads RTC snapshot dirty bit. This is will be helpful in validating
 *  if wait delay and RTC snapshot was updated properly when RTC was set by user.
 *
 *  @param pucRTCSnapshotDirtyBit Pointer to RTC snapshot dirty bit.
 *
 *  @return NO_ERROR on success or error code on failure
 */
int32_t lNvsramReadRTCSnapshotDirtyBit ( uint8_t *pucRTCSnapshotDirtyBit );

/** @brief function to enable AES encryption keys generation.
 *
 * This function sets the trng control register to generate new AES keys.
 *
 */
void vNvsramAESKeySet( void );

/** @brief This function check for valid header in the NVSRAM.
 *
 * @return NO_ERROR on valid header found
 *         eORWL_ERROR_NVSRAM_INVALID_HEADER on no valid header found.
 *         eORWL_ERROR_NVSRAM_NOT_ACCESSIBLE on error
 */
int32_t lNvsramCheckHeader( void );

/** @brief This function erase SSD password of NVSRAM
 *
 *  return error code
 */
int32_t lNvsSSDErasePassword( void );

/** @brief This function erases complete 8KB of NVSRAM
 *
 *  return error code
 */
int32_t lEraseNvsramComplete( void );

#endif /* nvsramSNVRAM_INCLUDE_H_ */
