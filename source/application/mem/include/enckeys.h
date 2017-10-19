 /**===========================================================================
 * @file enckeys.h
 *
 * @brief This file contains all the data structures and macros for ORWL
 * encryption keys
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

#ifndef enckeysINCLUDE_ENC_KEYS_H_
#define enckeysINCLUDE_ENC_KEYS_H_

/*Global includes */
#include <stdint.h>

/** Macros for DSFT encryption keys */
/**
 * Design Shift RSA publick Key length
 */
#define enckeysDSFT_ENC_KEY_LEN		(3072)

/**
 * Reserved region length
 */
#define	enckeysRESERVED_LEN		(28)

/**
 * @brief encryption keys. DesignShift ORWL public RSA key storage
 */
typedef struct
{
    /** Encryption partition magic header */
    uint32_t ulEncKeyMagic;
    /** DesignShift public key */
    uint8_t  ucEncDsftPubKey[enckeysDSFT_ENC_KEY_LEN];
    uint8_t  ucReserved[enckeysRESERVED_LEN];
} enckeysDsftEncKeys_t;

/** @brief Writes encryption key.
 *
 * This function writes encryption key structure to flash.
 *
 * @pxENKey pointer to encryption structure.
 * @return error code..
 *
 */
int32_t lEnckeysWriteEncKey( enckeysDsftEncKeys_t *pxENKey );

/** @brief Reads encryption key.
 *
 * This function Reads encryption key structure from flash..
 *
 * @pxENKey pointer to encryption structure.
 * @return error code..
 *
 */
int32_t lEnckeysReadEncKey( enckeysDsftEncKeys_t *pxENKey );

/** @brief Updates public key.
 *
 * This function updates public key to flash.
 *
 * @pucPublicKey pointer to public key.
 * @return error code..
 *
 */
int32_t lEnckeysUpdatePublicKey( uint8_t *pucPublicKey );

/** @brief Reads public key.
 *
 * This function reads public key from flash.
 *
 * @pucPublicKey pointer to public key.
 * @return error code..
 *
 */
int32_t lEnckeysGetPublicKey( uint8_t *pucPublicKey );
#endif /* enckeysINCLUDE_ENC_KEYS_H_ */
