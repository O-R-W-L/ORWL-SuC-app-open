 /**===========================================================================
 * @file access_keys.h
 *
 * @brief This file contains all the data structures and macros for ORWL PINS and
 * access keys.
 *
 * @author ravikiran.hv@design-shift.com
 *
 ============================================================================
 *
 * Copyright � Design SHIFT, 2017-2018
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

#ifndef keysINCLUDE_ACCESS_KEYS_H_
#define keysINCLUDE_ACCESS_KEYS_H_

/*Global includes */
#include <stdint.h>

/** Macros for DSFT encryption keys */

/**
 * Design Shift RSA publick Key length
 */
#define keysPIN_LEN		(8)

/**
 * Length of ISD keys
 */
#define keysISD_LEN		(8)

/**
 * Length of SSD keys
 */
#define keysSSD_LEN		(8)

/**
 * Length of SSD Serial number. BIOS to give string length of 30. Allocating 32
 * to be 4 byte address aligned.
 */
#define keysSSD_SERIAL_NUM_LEN  (32)

/**
 * Reserved region length
 */
#define	keyRESERVED_LEN		(16)

/**
 * ORWL unique Admin password.We need 32 Characters+1null terminated.
 * So defining 36 for 4byte boundary
 */
#define keysADMIN_PASSWD_LEN	(36)

/**
 * @brief encryption keys. DesignShift ORWL public RSA key storage
 */
typedef struct
{
    /** Access Keys & PIN partition magic */
    uint32_t ulAccessKeyMagic;
    /** Defualt PIN generated by ORWL */
    uint8_t  ucDefaultPIN[keysPIN_LEN];
    /** Default PIN set by user */
    uint8_t  ucUserPIN[keysPIN_LEN];
    /** Default ISD keys */
    uint8_t  ucDefaultISDKey[keysISD_LEN];
    /** Default SSD keys */
    uint8_t  ucDefaultSSDKey[keysSSD_LEN];
    /** Reserved */
    uint8_t  ucReserved[keyRESERVED_LEN];
    /** Place holder for Admin password */
    uint8_t  ucAdminPassword[keysADMIN_PASSWD_LEN];
   /** SSD Serial Number of the ORWL for recovery*/
    uint8_t ucSSDSerialNum[keysSSD_SERIAL_NUM_LEN] ;
} keysDSFT_MASTER_KEYS_t;

/* function declaration */

/** @brief Writes access keys.
 *
 * This function write the access keys to flash.
 *
 * @param pxACKeys pointer to access key structure..
 * @return error code.
 *
 */
int32_t lKeysWriteMasterKeys( keysDSFT_MASTER_KEYS_t *pxACKeys );

/** @brief Writes access keys, NON RTOS.
 *
 * This function write the access keys to flash. This function should be called
 * from NON RTOS context.
 *
 * @param pxACKeys pointer to access key structure..
 * @return error code.
 *
 */
int32_t lKeysWriteMasterKeysNONRTOS( keysDSFT_MASTER_KEYS_t *pxACKeys );

/** @brief Reads access keys.
 *
 * This function reads the access keys from flash.
 *
 * @param pxMKeys pointer to access key structure..
 * @return error code.
 *
 */
int32_t lKeysReadMasterKeys( keysDSFT_MASTER_KEYS_t *pxMKeys );

/** @brief Reads default pin.
 *
 * This function reads the default pin from flash.
 *
 * @param pucDefaultPin pointer to default pin.
 * @return error code.
 *
 */
int32_t lKeysGetDefaultPIN( uint8_t *pucDefaultPin );

/** @brief Reads user pin.
 *
 * This function reads the user pin from flash.
 *
 * @param pucUserPin pointer to user pin.
 * @return error code.
 *
 */
int32_t lKeysGetUserPIN( uint8_t *pucUserPin );

/** @brief Updates user pin.
 *
 * This function updates the user pin to flash.
 *
 * @param pucUserPin pointer to user pin.
 * @return error code.
 *
 */
int32_t lKeysUpdateUserPIN( uint8_t *pucUserPin );

/** @brief Reads default ISD key.
 *
 * This function reads the default ISD key from flash.
 *
 * @param pucISDKey pointer to default ISD key.
 * @return error code.
 *
 */
int32_t lKeysGetDefISDKey( uint8_t *pucISDKey );

/** @brief Reads default SSD key.
 *
 * This function reads the default SSD key from flash.
 *
 * @param pucSSDKey pointer to default SSD key.
 * @return error code.
 *
 */
int32_t lKeysGetDefSSDKey( uint8_t *pucSSDKey );

/** @brief Reads admin password.
 *
 * This function reads the Reads admin password from flash.
 *
 * @param pupPasswd pointer to admin password.
 * @return error code.
 *
 */
int32_t lKeysGetDefAdminPasswd( uint8_t *pucPasswd );

/** @brief Reads SSD serial number.
 *
 * This function reads the SSD serial number from flash.
 *
 * @param pucSSDSerial pointer to SSD serial number.
 * @return error code.
 *
 */
int32_t lKeysGetSSDserialNum( uint8_t *pucSSDSerial );

/** @brief Writes SSD serial number.
 *
 * This function writes the SSD serial number to flash.
 *
 * @param pucSSDSerial pointer to SSD serial number.
 * @return error code.
 *
 */
int32_t lKeysUpdateSSDserialNum( uint8_t *pucSSDSerial );

#endif /* keysINCLUDE_ACCESS_KEYS_H_ */
