/**===========================================================================
 * @file nfc_common.h
 *
 * @brief This file contains the macros and function prototypes used in
 * nfc_common.c
 *
 * @author viplav.roy@design-shift.com
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

#ifndef NFC_COMMON_H
#define NFC_COMMON_H

#include <stdint.h>
#include <errors.h>
#include <events.h>
#include <debug.h>
#include <trng.h>
#include <keyfobid.h>
#include <nfc_commands.h>
#include <phNfcLib.h>

#include <ucl/ucl_types.h>
#include <ucl/ucl_sys.h>
#include <ucl/ucl_3des_cbc.h>
#include <ucl/ucl_defs.h>
#include <crypto_interface.h>

#include <portable.h>

#define nfccommonUCL_USECASE_BUFFER_SIZE_INT	(2048)	/**< UCL Init Buffer Size */
#define nfccommonUCL_CIPHER_ENCRYPT		(UCL_CIPHER_ENCRYPT)	/**< Encryption Mode */
#define nfccommonUCL_CIPHER_DECRYPT		(UCL_CIPHER_DECRYPT)	/**< Decryption Mode */
#define nfccommonKEYFOBNAME			"KEYFOB_ID_2017_ABCD"	/**< KeyFob name */
#define nfccommonKEYFOBNAME_FULL_LEN		(32)	/**< MAX KeyFob name */
#define nfccommonKEYFOBNAME_RND_LEN		(4)	/**< Length of random number in KeyFob name */
#define nfccommonKEYFOBNAME_RND_WLEN		(1)	/**< Word Length of random number */
#define nfccommonKEYFOB_SEED_WLEN		(6)	/**< Length of Challenge in words*/
#define nfccommonKEYFOB_NAME_PAD_LEN		(5)	/**< 5 bytes are added to make 8 byte align */
#define nfccommonROT_INTERVAL			(15)	/**< Broadcast data rotation interval */
#define nfccommonSEED_INC_WLEN			(1)	/**< Seed Increment factor word length */
#define nfccommonSEED_SIZE			(32)	/**< Seed length */
#define nfccommonROT_INTERVAL_SIZE		(2)	/**< 2 bytes of rotation interval */
#define nfccommonSEED_INC_SIZE			(4)	/**< Seed Increment factor size */
#define nfccommonKEY_PRIVILEGE_ADMIN		(0xAD4D494E)	/**< Admin privilege */
#define nfccommonKEY_PRIVILEGE_USER		(0x5553E52)	/**< User privilege */

extern uint8_t gucPlain[commandsCONFIRMSSK_SIZE];

/** @brief Structure to hold Seed Information
 *
 * This seed will be updated by Suc. Suc will send this seed to its ble
 * controller and to the keyfob/peer.
 */
typedef struct
{
    uint8_t ucSeed[nfccommonSEED_SIZE];			/**< random number seed */
    uint8_t ucSeedIncrFactor[nfccommonSEED_INC_SIZE];	/**< Seed Increment factor */
    uint8_t ucRotInterval[nfccommonROT_INTERVAL_SIZE];	/**< Rotational Interval */
    uint8_t ucBleRange;					/**< Reserved field */
    uint8_t ucReserved;					/**< Reserved field */
}NfcCommonSeed_Info_t;

/**
 * @brief Initializes the crypto library
 *
 * @param  void
 *
 * @return void
 */
void vNfcCommonCryptoInit(void);

/**
 * @brief Verify the key received is same as key stored in flash

 * @param pucMessage1  Pointer to data buffer having decrypted message
 * @param pucMessage2  Pointer to data buffer having SHA1 message
 * @param ulMsgLen     Message Length
 *
 * @return uint32_t
 * returns eventsKEYFOB_NFC_AUTH_SUCCESS on success
 * returns eventsKEYFOB_NFC_AUTH_FAILURE on error
 */
uint32_t ulNfcCommonVerifyKey( uint8_t *pucMessage1, uint8_t *pucMessage2,
				uint32_t ulMsgLen );

/**
 * @brief Encrypt the data

 * @param pucCipherdata Pointer to encrypted data buffer
 * @param pucDataBuf    Pointer to data buffer having plain message
 * @param pucKey        Pointer to shared secret key
 * @param ulDataLen     Data Length
 *
 * @return uint32_t
 * returns NO_ERROR on success
 * returns error code on error
 */
uint32_t ulNfcCommonEncrpytData(uint8_t *pucCipherdata, uint8_t *pucDataBuf,
			uint8_t *pucKey, uint32_t ulDataLen);

/**
 * @brief Decrypt the data

 * @param pucDecipher   Pointer to decrypted data buffer
 * @param pucDataBuf    Pointer to data buffer having encrypted message
 * @param pucKey        Pointer to shared secret key
 * @param ulDataLen     Data Length
 *
 * @return uint32_t
 * returns NO_ERROR on success
 * returns error code on error
 */
uint32_t ulNfcCommonDecrpytData(uint8_t *pucDecipher, uint8_t *pucDataBuf,
	uint8_t *pucKey,uint32_t ulDataLen);
/**
 * @brief This function is used to get the KeyFob information.
 *
 * Function takes key name as input and search for the key entry in the flash.
 * If key is present, it will update the key info structure from flash.
 *
 * @param pxKeyInfo     Pointer to keyfobidKeyFobInfo_t buffer
 * @param pucKeyName    Key name
 * @param pucKeyLen     Length of key
 *
 * @return int32_t
 * returns NO_ERROR on success
 * returns error codes on error
 */
int32_t lNfcCommonGetKeyByName(keyfobidKeyFobInfo_t *pxKeyInfo,
					    uint8_t *pucKeyName,
					    uint8_t pucKeyLen);

/**
 * @brief This function is used to write the KeyFob information to flash.
 *
 * @param pucKeyFOBName Pointer to Key Fob name
 * @param ucKeyFOBLen   Length of the KeyFob name
 * @param ulprivilege   Privilege of the KeyFob
 * @param pucSSK        Pointer to shared secret key
 * @param pucORWL_SK    Pointer to ORWL seed/challenge
 * @param pucKeyFob_SK  Pointer to key fob seed/challenge
 *
 * @return int32_t
 * returns NO_ERROR on success
 * returns error codes on error
 */
int32_t lNfcCommonWriteKeyInfo(uint8_t *pucKeyFOBName, uint8_t ucKeyFOBLen,
				uint32_t ulprivilege, uint8_t *pucSSK,
				uint8_t *pucORWL_SK, uint8_t *pucKeyFob_SK);
#endif /* NFC_COMMON_H */
