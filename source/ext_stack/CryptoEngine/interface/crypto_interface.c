/**===========================================================================
 * @file crypto_interface.c
 *
 * @brief This file enables ARM mbed tls crypto engine access for the
 * applications
 *
 * @author ravikiran@design-shift.com
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

/* standard headers */
#include <string.h>
#include <stdlib.h>

/* debug includes */
#include <debug.h>

/* ecdh include */
#include <mbedtls/ecdh.h>

#include <crypto_interface.h>

/* Global ecdh context */
mbedtls_ecdh_context gxSrv;

/* Global pseudo random generator info structure */
cryptoRnd_pseudo_info_t gxrnd_info;

uint8_t ucTrue_Rand( void *pvRNGState, uint8_t *pucOutput, size_t xLen )
{
    uint32_t ulStatus;
    if( pvRNGState != NULL )
	pvRNGState  = NULL;

    /* Check if xLen is multiples of 4, if not multiples of 4 return error,
     * random number generater takes input in words.
     */
    if((xLen % trngTRNG_SINGLE_SHOT_READ_LEN) != 0)
    {
	debugERROR_PRINT("xLen not in multiples of four \n");
	return COMMON_ERR_OUT_OF_RANGE;
    }

    /* ulGenerateRandomNumber requires length in word size so dividing it by 4 */
    xLen = xLen/trngTRNG_SINGLE_SHOT_READ_LEN;
    ulStatus=ulGenerateRandomNumber((uint32_t*)pucOutput,xLen);
    xLen = xLen*trngTRNG_SINGLE_SHOT_READ_LEN;
    if(ulStatus)
    {
        debugPRINT(" Failed to generate random number ");
    }
    return ulStatus;
}
/*---------------------------------------------------------------------------*/

uint32_t ulCryptoEcdh_PublicKeyGen(uint8_t ucCurveId, uint8_t *pucDHPubKey, uint32_t *pulLEN)
{
    uint32_t ulStatus;
    uint8_t ucBuf[cryptoMAX_BUF_LEN];
    size_t xlen;

    mbedtls_ecdh_init( &gxSrv );
    memset( &gxrnd_info, 0x00, sizeof( cryptoRnd_pseudo_info_t ) );

    /* Get type of elliptic curve used */
    if((ulStatus=mbedtls_ecp_group_load( &gxSrv.grp, ucCurveId )) != NO_ERROR )
    {
        debugERROR_PRINT(" Failed to load Elliptical Curve Parameter");
        return ulStatus;
    }

    /* Generate public keys and generates the data in ucBuf in Little Endian */
    if((ulStatus=mbedtls_ecdh_make_params( &gxSrv, &xlen, ucBuf, cryptoMAX_BUF_LEN,
                                           &ucTrue_Rand, &gxrnd_info )) != NO_ERROR )
    {
        debugERROR_PRINT(" Failed to generate ecdh parameters");
        return ulStatus;
    }

#if DBG_DATA
    int loop = 0 ;
    while(loop < (xlen))
    {
	debugPRINT("ucBuf[%d] = 0x%x\n",loop,ucBuf[loop]);
	loop++;
    }
#endif
    *pulLEN = xlen-cryptoHEADERLENGTH;
    memcpy(pucDHPubKey,ucBuf+cryptoHEADERLENGTH,*pulLEN);
    return ulStatus;
}
/*---------------------------------------------------------------------------*/

uint32_t ulCryptoEcdh_GenSSK(uint8_t *pucDHPubKey, uint8_t *pucPubKeyLen, uint8_t *pucDataBuf)
{
    uint32_t ulStatus;
    uint8_t ucSHA1_SSK[cryto3DES_KEY_SIZE];
    uint8_t ucBuf[cryptoMAX_BUF_LEN];
    uint8_t ucTmpBuf[cryptoBUF_LEN];
    size_t xlen;

    xlen=*pucPubKeyLen;
    ucTmpBuf[0]=xlen;
    /* Data buffer should contain Length of public key and public key */
    memcpy(&ucTmpBuf[1],pucDHPubKey,xlen);
    xlen+=1;
    if((ulStatus=mbedtls_ecdh_read_public( &gxSrv, ucTmpBuf, xlen )) != NO_ERROR )
    {
        debugERROR_PRINT(" Failed to read Peers Public Key");
        return ulStatus;
    }

    /* Generate the shared secret key and update the parameters accordingly*/
    /* Length of the key will be updated on xlen */
    if((ulStatus=mbedtls_ecdh_calc_secret( &gxSrv, &xlen, ucBuf, cryptoMAX_BUF_LEN,
                                         &ucTrue_Rand, &gxrnd_info))!= NO_ERROR )
    {
        debugERROR_PRINT(" Failed to calculate the secret");
        return ulStatus;
    }

#if DBG_DATA
    int i;
    debugPRINT("\n secret carrying buffer \n" );
    for(i=0;i<xlen;i++)
        debugPRINT(" 0x%02x",ucBuf[i]);
#endif
    if((ulStatus=ucl_sha1(ucSHA1_SSK,ucBuf,xlen))!= NO_ERROR)
    {
        debugERROR_PRINT(" Failed to generate the SHA1 hash");
        return ulStatus;
    }

    memcpy(pucDataBuf,ucSHA1_SSK,cryptoSHA1_LEN);
    memcpy(pucDataBuf+cryptoSHA1_LEN,ucSHA1_SSK,xlen-cryptoSHA1_LEN);
    *pucPubKeyLen = xlen;

    mbedtls_ecdh_free( &gxSrv );
    return ulStatus;
}
/*---------------------------------------------------------------------------*/
