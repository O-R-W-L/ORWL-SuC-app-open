/**===========================================================================
 * @file nfc_common.c
 *
 * @brief This file contains all supporting functions used in nfc association
 * and authentication process.
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

#include <nfc_common.h>

/**
 * Buffer used by ucl library for internal operations
 */
uint32_t gulInit_Buffer[nfccommonUCL_USECASE_BUFFER_SIZE_INT];

/**
 * Initializing Vectors used by 3DES_CBC encryption algorithm for encryption
 * and decryption of message.
 */
uint8_t gucIV[commandsINIT_VECTOR_SIZE] = {0x0f,0x1e,0x2d,0x3c,0x4b,0x5a,0x69,0x78};

int32_t lNfcCommonGetKeyByName(keyfobidKeyFobInfo_t *pxKeyInfo,
					    uint8_t *pucKeyName,
					    uint8_t pucKeyLen)
{
    int32_t lStatus = NO_ERROR;
    uint8_t ucCount = 0;

    keyfobidKeyFobInfo_t *pxBackUpKeyfobData = NULL;
    pxBackUpKeyfobData = (keyfobidKeyFobInfo_t *) pvPortMalloc(
	sizeof(keyfobidKeyFobInfo_t));
    if(pxBackUpKeyfobData == NULL)
    {
        debugERROR_PRINT(""
        	"Failed to allocate memory for keyfob entry structure ");
        return COMMON_ERR_NULL_PTR;
    }

    for(ucCount=0;ucCount<keyfobidMAX_KEFOB_ENTRIES;ucCount++)
    {
        lStatus = lKeyfobidReadKey(pxBackUpKeyfobData,ucCount);
        if(lStatus == NO_ERROR)
        {
            if(!memcmp(pxBackUpKeyfobData->ucKeyName,pucKeyName,pucKeyLen))
            {
        	debugPRINT_NFC(" KEY NAME match found        ");
                memcpy((void *)pxKeyInfo, (void *)pxBackUpKeyfobData,
			sizeof(keyfobidKeyFobInfo_t));
                break;
            }
        }
        else
        {
            debugERROR_PRINT(" Failed to read the keyfob info structure ");
        }
    }

    if((lStatus == NO_ERROR) && (ucCount == keyfobidMAX_KEFOB_ENTRIES))
    {
        debugERROR_PRINT(" Key info is not available in flash ");
        lStatus = COMMON_ERR_OUT_OF_RANGE;
    }
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lNfcCommonWriteKeyInfo(uint8_t *pucKeyFOBName, uint8_t ucKeyFOBLen,
				uint32_t ulprivilege, uint8_t *pucSSK,
				uint8_t *pucORWL_SK, uint8_t *pucKeyFob_SK)
{
    int32_t lStatus = NO_ERROR;
    keyfobidKeyFobInfo_t *pxKeyFobInfo = NULL;

    pxKeyFobInfo = (keyfobidKeyFobInfo_t *) pvPortMalloc(
                sizeof(keyfobidKeyFobInfo_t));
    if(pxKeyFobInfo == NULL)
    {
        debugERROR_PRINT("Failed to allocate memory for "
                             "keyfob info structure \r\n");
        return COMMON_ERR_NULL_PTR;
    }

    configASSERT(pucKeyFOBName!=NULL);
    configASSERT(pucSSK!=NULL);
    configASSERT(pucORWL_SK!=NULL);
    configASSERT(pucKeyFob_SK!=NULL);
    configASSERT(ulprivilege!=0);
    configASSERT(ucKeyFOBLen!=0);

    memcpy((void *)pxKeyFobInfo->ucKeyName, (void *)pucKeyFOBName, ucKeyFOBLen);
    pxKeyFobInfo->ulKeyNameLen = ucKeyFOBLen;
    pxKeyFobInfo->ulKeyPrevilage = ulprivilege;
    memcpy((void *)pxKeyFobInfo->xSecKeyZ.ucTDESKey, (void *)pucSSK,
	    cryto3DES_KEY_SIZE);
    memcpy((void *)pxKeyFobInfo->xSecKeyX.ucTDESKey, (void *)pucORWL_SK,
	    cryto3DES_KEY_SIZE);
    memcpy((void *)pxKeyFobInfo->xSecKeyY.ucTDESKey, (void *)pucKeyFob_SK,
	    cryto3DES_KEY_SIZE);
    memcpy((void*)pxKeyFobInfo->ulKeyPIN, gucCVMPIN, sizeof(gucCVMPIN));
    lStatus = lKeyfobidAddKey(pxKeyFobInfo);
    if(lStatus != NO_ERROR)
    {
        debugERROR_PRINT("Failed to add new KeyFob \r\n");
    }

    if(pxKeyFobInfo)
    {
        vPortFree(pxKeyFobInfo);
    }
    return lStatus;
}
/*----------------------------------------------------------------------------*/

void vNfcCommonCryptoInit(void)
{
    if(ucl_init((u32 *)gulInit_Buffer,nfccommonUCL_USECASE_BUFFER_SIZE_INT))
    {
        debugERROR_PRINT(" Failed to initialize the ucl library ");
    }
    return;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCommonVerifyKey(uint8_t *pucMessage1, uint8_t *pucMessage2,
				uint32_t ulMsgLen)
{
    uint16_t usCount = 0;
    for(usCount = 0; usCount<ulMsgLen; usCount++)
    {
	if(pucMessage1[usCount]!=pucMessage2[usCount])
	{
	    debugERROR_PRINT(" Decrypted message and stored SHA1 data "
				    "are not same ");
	    return eventsKEYFOB_NFC_AUTH_FAILURE;
	}
    }
    debugPRINT_NFC(" Keys verification passed ");
    return eventsKEYFOB_NFC_AUTH_SUCCESS;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCommonEncrpytData(uint8_t *pucCipherdata, uint8_t *pucDataBuf,
			uint8_t *pucKey,uint32_t ulDataLen)
{
    uint32_t ulStatus = NO_ERROR;
    uint8_t ucKey[cryto3DES_KEY_SIZE];

    memcpy((void *)ucKey, (void *)pucKey,cryto3DES_KEY_SIZE);
    ulStatus = ucl_3des_cbc(pucCipherdata,pucDataBuf,ucKey,gucIV,ulDataLen,
	    nfccommonUCL_CIPHER_ENCRYPT);
    if( ulStatus )
    {
        debugERROR_PRINT(" Failed to encrypt the message ulStatus = %d",
        			ulStatus);
        return ulStatus;
    }
#if DBG_DATA
    uint16_t usTmp;
    uint8_t ucDecipher[commandsNFC_TAG_SECRET_SIZE];
    for(usTmp = 0;usTmp<=ulDataLen;usTmp++ )
    {
        debugPRINT_NFC(" 0x%02x",pucCipherdata[usTmp]);
    }
#endif
    return ulStatus;
}
/*----------------------------------------------------------------------------*/

uint32_t ulNfcCommonDecrpytData(uint8_t *pucDecipher, uint8_t *pucDataBuf,
	uint8_t *pucKey,uint32_t ulDataLen)
{
    uint32_t ulStatus = NO_ERROR;
    uint8_t ucKey[cryto3DES_KEY_SIZE];

    memcpy((void *)ucKey, (void *)pucKey, cryto3DES_KEY_SIZE);
    ulStatus = ucl_3des_cbc(pucDecipher,pucDataBuf,ucKey,gucIV,ulDataLen,
	    nfccommonUCL_CIPHER_DECRYPT);
    if( ulStatus )
    {
        debugERROR_PRINT(" Failed to decrypt the message ulStatus = %d",
        	ulStatus);
    }
    return ulStatus;
}
/*----------------------------------------------------------------------------*/
