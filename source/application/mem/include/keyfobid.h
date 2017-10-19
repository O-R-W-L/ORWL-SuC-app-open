 /**===========================================================================
 * @file keyfobid.h
 *
 * @brief This file contains all the data structures and macros for ORWL
 * KeyFob identification and access
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
#ifndef keyfobidINCLUDE_KEYFOB_ID_H_
#define keyfobidINCLUDE_KEYFOB_ID_H_

/*Global includes */
#include <stdint.h>

/** ORWL Keyfob identification macros */

/**
 * length of unique keyFob key.
 */
#define keyfobidSERIAL_NUM_LEN		(32)

/**
 * keyfobid name length.
 */
#define keyfobidNAME_LEN		(32)

/**
 * Actual keyfobid name length. Used for comparison only.
 */
#define keyfobidNAME_LEN_ACT		(19)

/**
 * ISD Key allowed length
 */
#define keyfobidISD_LEN_MAX		(16)

/**
 * SSD Key allowed length
 */
#define keyfobidSSD_LEN_MAX		(16)

/**
 * Dec. Key length
 */
#define keyfobidDEC_KEY_LEN		(64)

/**
 * Enc. Key length
 */
#define keyfobidENC_KEY_LEN		(64)

/**
 * Max allowed TDES Key length
 */
#define keyfobidTDES_LEN_MAX		(24)

/**
 * Reserved
 */
#define keyfobidKEY_INFO_RESERVED	(16)

/**
 * Unique data of Keyfob - Optional filed
 */
#define keyfobidKEY_DATA		(248)

/**
 * Reserved
 */
#define keyfobidKEY_ID_RESERVED		(28)

/**
 * max keyfobss that can be associated with ORWL at any time
 */
#define keyfobidMAX_KEFOB_ENTRIES	(10)

/**
 * keyfob CVM pin
 */
#define keyfobidCVM_PIN_LEN		(8)

/**
 * keyfob size of TDES keys
 */
#define keyfobidTDES_LEN_MAX		(24)

/**
 * keyfob CRC
 */
#define keyfobidCRC_LEN			(8)

/**
 * keyfob reserved bytes
 */
#define keyfobidReserved_LEN		(8)

/**
 * No keyfob entry available
 */
#define keyfobidNO_KEY_ASSOCIATED	(0)

/**
 * key magic to access keyFob info
 */
#define keyfobidKEYFOB_INFO_MAGIC	(0xF0B01DE9U)

/**
 * key magic to access keyfob entry structure
 */
#define keyfobidKEYFOB_ID_MAGIC		(0xF0B3A91CU)

/**
 * Structure contains Secret Key details
 */
typedef struct
{
    /** Secret Key */
    uint8_t	ucTDESKey[keyfobidTDES_LEN_MAX];
    /** CRC checksum */
    uint8_t 	ucCRC[keyfobidCRC_LEN];
    /** Reserved */
    uint8_t 	ucReservedBytes[keyfobidReserved_LEN];
}SecretKey_t;

/**
 * @brief This structure defines KeyFob ID structure.
 */
typedef struct
{
    /** Key magic start field */
    uint32_t	ulKeyMagic;
    /** KeyFob Identification entry */
    uint32_t	ulKeyFobId;
    /** Length of unique key */
    uint32_t	ulKeyLen;
    /** Unique serial number of the KeyFob KeyFob */
    uint8_t	ucUniQueKey[keyfobidSERIAL_NUM_LEN];
    /** Admin and user privilege defined here */
    uint32_t	ulKeyPrevilage;
    /** KeyFob name length */
    uint32_t	ulKeyNameLen;
    /** KeyFob Name */
    uint8_t	ucKeyName[keyfobidNAME_LEN];
    /** ISD domain keys */
    uint8_t	ucISDKey[keyfobidISD_LEN_MAX];
    /** SSD domain keys */
    uint8_t	ucSSDKey[keyfobidSSD_LEN_MAX];
    /** [Z] ECDH Secret key */
    SecretKey_t	xSecKeyZ;
    /** [x] Secret key */
    SecretKey_t	xSecKeyX;
    /** [x] Secret key */
    SecretKey_t	xSecKeyY;
    /** KeyFob unique CVM PIN*/
    uint8_t	ulKeyPIN[keyfobidCVM_PIN_LEN];
    /** Reserved for future */
    uint8_t	ucReserved[keyfobidKEY_INFO_RESERVED];
    /** Optional unique data for the keyfob */
    uint8_t	ucKeyFobData[keyfobidKEY_DATA];
} keyfobidKeyFobInfo_t;

/**
 * @brief This structure defines c* @brief This structure defines
 * flash contents ofKeyFob info structure
 */
typedef struct
{
    /** KeyFob partition Magic */
    uint32_t	 ulKeyFobIdMagic;
    /** Information of each keyfob associated with ORWL */
    keyfobidKeyFobInfo_t xKeyFobInfo[keyfobidMAX_KEFOB_ENTRIES];
    /** Reserved */
    uint8_t	 ucReserved[keyfobidKEY_ID_RESERVED];
} keyfobidKeyFobEntry_t;

/** function declaration */

/** @brief Writes keyfob entry structure.
 *
 * This function writes keyfob entry structure to flash.
 *
 * @param pxKeyFobEntry pointer to keyfobidKeyFobEntry_t structure.
 * @return error code.
 *
 */
int32_t lKeyfobidWriteKeyFobEntry(keyfobidKeyFobEntry_t *pxKeyFobEntry);

/** @brief Reads keyfob entry structure.
 *
 * This function reads keyfob entry structure from flash.
 *
 * @param pxKeyFobEntry pointer to keyfobidKeyFobEntry_t structure.
 * @return error code.
 *
 */
int32_t lKeyfobidReadKeyFobEntry(keyfobidKeyFobEntry_t *pxKeyFobEntry);

/** @brief Reads keyfob key.
 *
 * This function reads keyfob key structure from flash.
 *
 * @param pxKeyInfo pointer to keyfobidKeyFobInfo_t structure.
 * @param ucIndex Index of keyfob key to read.
 * @return error code.
 *
 */
int32_t lKeyfobidReadKey( keyfobidKeyFobInfo_t *pxKeyInfo, uint8_t ucIndex);

/** @brief Adds keyfob key.
 *
 * This function adds keyfob key structure to keyfobidKeyFobEntry_t in flash.
 *
 * @param pxKeyInfo pointer to keyfobidKeyFobInfo_t structure.
 * @return error code.
 *
 */
int32_t lKeyfobidAddKey( keyfobidKeyFobInfo_t *pxKeyInfo);

/** @brief Remove keyfob key.
 *
 * This function removes keyfob key based on keyfobID.
 *
 * @param ulKeyFobId keyfobid value.
 * @return error code.
 *
 */
int32_t lKeyfobidRemoveKeyKeyFobID( uint32_t ulKeyFobId );

/** @brief Remove keyfob key.
 *
 * This function removes keyfob key based on name of key.
 *
 * @param pucKeyName pointer to key name.
 * @return error code.
 *
 */
int32_t lKeyfobidRemoveKeyKeyName( uint8_t *pucKeyName );

/** @brief Update privileges for key.
 *
 * This function update privileges for key based on keyfobID.
 *
 * @param ulKeyFobId keyfobid value.
 * @param ulKeyPrevilage key previlege to update.
 * @return error code.
 *
 */
int32_t lKeyfobidUpdateKeyPrivilages( uint32_t ulKeyFobId, uint32_t ulKeyPrevilage );

/** @brief Check number of key entry present.
 *
 * This function check for the key entry index available in flash.
 *
 * @param pucIndexAdd pointer to get the index.
 *
 * @return error code.
 */
int32_t lKeyfobidCheckFreeIndex( uint8_t *pucIndexAdd);
#endif /* keyfobidINCLUDE_KEYFOB_ID_H_ */
