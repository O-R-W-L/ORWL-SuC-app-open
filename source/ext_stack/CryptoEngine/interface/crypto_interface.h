/**===========================================================================
 * @file crypto_interface.h
 *
 * @brief This file contains the macro, structures and function declarations
 * used in crypto_interface
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

#ifndef CRYPTO_INTERFACE_H
#define CRYPTO_INTERFACE_H

#define __API__
#include <trng.h>
#include <ucl/ucl_types.h>
#include <ucl/ucl_sha1.h>

#define cryptoCURVE_ID          (1)         /**< curve id for 192-bits NIST curve */
#define cryptoMAX_BUF_LEN       (1000)      /**< Max Buffer Length */
#define cryptoBUF_LEN           (64)        /**< Buffer length */
#define cryto3DES_KEY_SIZE      (24)        /**< Key length used in 3DES */
#define cryptoHEADERLENGTH      (4)         /**< Tls server header length */
#define cryptoSHA1_LEN          (20)        /**< Length of SHA1 Output */

/**
 * Info structure for the pseudo random function
 *
 * Key should be set at the start to a test-unique value.
 * Do not forget endianness!
 * State( v0, v1 ) should be set to zero.
 */
typedef struct
{
    uint32_t key[16];
    uint32_t v0, v1;
} cryptoRnd_pseudo_info_t;

extern unsigned char g[50];

/**
 * @brief This function used to generate the public key.
 *
 * It takes elliptical curve parameter as input and set all ecp parameters.
 * Generates public key and the private key
 *
 * @param ucCurveId   Curve ID
 * @param pucDHPubKey Pointer to ECDH public key Buffer
 * @param pulLEN      Pointer to Length of the Public key
 *
 * @return uint32_t status of public key generation
 */
uint32_t ulCryptoEcdh_PublicKeyGen(uint8_t ucCurveId, uint8_t *pucDHPubKey,
					uint32_t *pulLEN);

/**
 * @brief This function used to generate the shared secret key.
 *
 * This function generates the shared secret key from the peer's public key.
 * And Encrypt the SSK using SHA1.
 *
 * @param pucDHPubKey  Pointer to ECDH public key Buffer
 * @param pucPubKeyLen Used as ECDH public key Length input and Shared Secret
 *                     Key Length Output
 * @param pucDataBuf   Pointer to output Shared Secret Key buffer
 *
 * @return uint32_t status of public key generation
 */
uint32_t ulCryptoEcdh_GenSSK(uint8_t *pucDHPubKey, uint8_t *pucPubKeyLen, uint8_t *pucDataBuf);

/**
 * @brief This function generates random number using TRNG.
 *
 * pvrng_state shall be a pointer to a rnd_pseudo_info structure.
 *
 * @param pvRNGState Random number generator state
 * @param pucOutput   Pointer to output data
 * @param xLen        Length of the random number
 *
 * @return uint8_t
 * returns error codes on error in random number generation
 */
uint8_t ucTrue_Rand( void *pvRNGState, uint8_t *pucOutput, size_t xLen );
#endif /* CRYPTO_INTERFACE_H */
