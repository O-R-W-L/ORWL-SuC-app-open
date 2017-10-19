/**===========================================================================
 * @file nfc_interface_app.c
 *
 * @brief This file contains the all the NFC stack initialization and
 * application start routines.
 *
 *
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

/* nfc interface header */
#include <nfc_interface.h>

/* NFC application header */
#include <nfc_app.h>

/* NFC command header */
#include <nfc_commands.h>

/* task includes */
#include <FreeRTOS.h>
#include <task.h>

/* local includes */
#include <usermode.h>
#include <events.h>
#include <trng.h>
#include <crypto_interface.h>
#include <nfc_common.h>

extern uint8_t gucPlain[commandsCONFIRMSSK_SIZE];

/* Global NFC peer device information */
phNfcLib_PeerInfo_t PeerInfo = {0};

#ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
uint32_t ulNfcAppTaskBuffer[SIMPLIFIED_ISO_STACK];
#else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
#define ulNfcAppTaskBuffer       NULL	/**< place holder for NFC application task stack */
#endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */

#define nfcappCLEAR_EVENT		(0)	/**< Clear the event */
#define nfcapp100MSEC_SEC_FACTOR	(10)	/**< usermodeDELAY_MSEC_100 * 10 = 1 sec */
#define nfcappSEC_MIN_FACTOR		(60)	/**< number of seconds in one minute */
#define nfcappDELAY_IN_MINS		(15)	/**< 15 mins delay */
#define nfcappDELAY_IN_SECS		(10)	/**< 10 secs delay */
/**
 *  10 secs timeout for card detection
 */
#define nfcappTIMEOUT			( nfcapp100MSEC_SEC_FACTOR * \
					nfcappDELAY_IN_SECS )

/** KeyFob serial last four bytes */
#define nfcappKEYFOB_SERIAL		(4)
/**
 * Global Structure to store keyFob details.
 * Structure get updated on first boot and used for further authentication
 */
static keyfobidKeyFobInfo_t gxKeyFobInfo;

/**
 * @brief This is a dedicated freeRTOS task for NFC detection and read data
 * from the NFC cards or tags. Polls for the NFC tags and initiates the
 * communication if the tag is DESFIRE type.
 * All other card types are ignored.
 *
 * This task uses NFC custom command interface required for ISO7816
 * communication with peer
 */
static void prvNfcInterfaceTask(void* pvParams);

uint32_t ulNfcInterafceInit(void* pvParams)
{
    /* NFC thread object */
    phOsal_ThreadObj_t NfcInterfaceObj;

    configASSERT(pvParams != NULL);

    /* Perform OSAL Initialization. */
    phOsal_Init();

    NfcInterfaceObj.pTaskName = (uint8_t *) "NfcInterfaceApp";
    NfcInterfaceObj.pStackBuffer = ulNfcAppTaskBuffer;
    NfcInterfaceObj.priority = NFC_APP_PRIO;
    NfcInterfaceObj.stackSizeInNum = NFC_APP_STACK;
    return (phOsal_ThreadCreate(&NfcInterfaceObj.ThreadHandle, &NfcInterfaceObj,
	    &prvNfcInterfaceTask, pvParams));
}
/*----------------------------------------------------------------------------*/

static void prvNfcInterfaceTask(void* pvParams)
{
    uint32_t ulStatus = 0;
    uint32_t ulLen = 0;
    uint32_t ulPLen = 0;
    uint32_t ulRLen = 0;
    uint32_t ulKeyFobLen = 0;
    uint32_t ulTimeOutCounter = 0;
    uint8_t ucKeyEntryNum = 0;
    uint8_t ucKeyFOBName[nfccommonKEYFOBNAME_FULL_LEN]={0};
    uint8_t ucDataBuf[commandsRAW_ACCESS_BUF_LEN] = {0};
    uint8_t ucTmpBuf[commandsRAW_ACCESS_BUF_LEN] = {0};
    uint8_t ucSSKey[commandsRAW_ACCESS_BUF_LEN] = {0};
    uint8_t ucORWLKey[cryto3DES_KEY_SIZE] = {0};
    uint8_t ucKeyFobKey[cryto3DES_KEY_SIZE] = {0};
    keyfobidKeyFobInfo_t *pxKeyInfo = NULL;
    NfcCommonSeed_Info_t xChallenge;
    EventBits_t xBits, xEventRecieved, xStopEventRecieved;
    const TickType_t xTicksToWait = portMAX_DELAY;
    uint32_t ulEventToSend = pdFALSE, ulWaitForEvent = pdTRUE;
    xSMAppResources_t *pxResHandle;
    static uint8_t ucFirstBoot = pdTRUE;
    NfcEventData_t xNfcData;

    configASSERT(pvParams != NULL);

    pxResHandle = pvParams;

    debugPRINT_NFC(" \n Starting NFC Application: \n");

    while (1)
    {
	/* Initialize the NfcRdLib */
	if (phNfcLib_Init() != PH_NFCLIB_STATUS_SUCCESS)
	{
	    break;
	}
	do
	{
	    if (ulWaitForEvent)
	    {
		xEventRecieved = xEventGroupWaitBits(
			pxResHandle->xEventGroupNFC, /* The event group being tested. */
			eventsNFC_DETECT_KEYFOB |
			eventsNFC_ASSOCIATE_KEYFOB |
			eventsNFC_DELETE_KEYFOB_ENTRY |
			eventsNFC_GET_KEYFOB_SERIAL |
			eventsNFC_STOP_EXEC |
			eventsNFC_FIRST_DETECT,
			pdFALSE, /* Wake up but DON'T clear the events */
			pdFALSE,
			xTicksToWait);

		xEventRecieved = xEventGroupClearBits(
			pxResHandle->xEventGroupNFC,
			eventsNFC_DETECT_KEYFOB |
			eventsNFC_ASSOCIATE_KEYFOB |
			eventsNFC_DELETE_KEYFOB_ENTRY |
			eventsNFC_GET_KEYFOB_SERIAL |
			eventsNFC_STOP_EXEC |
			eventsNFC_FIRST_DETECT);

		/* Received event. Lets clear the flag */
		ulWaitForEvent = pdFALSE;
	    }
	    /* Stop nfc execution event */
	    if ((xEventRecieved & eventsNFC_STOP_EXEC) == eventsNFC_STOP_EXEC)
	    {
		ulEventToSend = eventsKEYFOB_STOP_NFC_SUCCESS;
		goto END;
	    }
	    /* Event to detect Keyfob as on boot first keyfob detection */
	    else if((xEventRecieved & eventsNFC_FIRST_DETECT) ==
							eventsNFC_FIRST_DETECT)
	    {
		ucFirstBoot = pdTRUE;
		ulEventToSend = eventsKEYFOB_FIRST_DETECT_SUCCESS;
		goto END;
	    }
	    /* Event to delete the Keyfob Entry */
	    else if ((xEventRecieved & eventsNFC_DELETE_KEYFOB_ENTRY)
	    			== eventsNFC_DELETE_KEYFOB_ENTRY)
	    {
		/* Delay is required for proper logs on console */
		vTaskDelay(usermodeDELAY_MSEC_100);
		if(gxKeyFobInfo.ulKeyPrevilage != nfccommonKEY_PRIVILEGE_ADMIN)
		{
		    debugERROR_PRINT(" Please use ADMIN privilege Keyfob"
			    " to delete the keyfob Entry");
		    ulEventToSend = eventsKEYFOB_NOT_ADMIN;
		    goto END;
		}
		xQueueReceive(pxResHandle->xNfcQueue, &xNfcData, portMAX_DELAY);
		if(lKeyfobidRemoveKeyKeyName(xNfcData.ucKeyFobName)
			!= NO_ERROR)
		{
		    debugERROR_PRINT(" Failed to delete the key Fob entry ");
		    ulEventToSend = eventsKEYFOB_ENTRY_DELETE_FAILURE;
		    goto END;
		}
		else
		{
		    debugPRINT_NFC(" Key Fob entry is succesfully deleted ");
		    ulEventToSend = eventsKEYFOB_ENTRY_DELETE_SUCCESS;
		    goto END;
		}
	    }

	    /* Enable NFC BOOSTER */
	    phNfcLibEnableNfcBooster();

	    /* Configuring the activation profile as ISO */
	    phNfcLib_SetConfig_Value(PH_NFCLIB_CONFIG_ACTIVATION_PROFILE,
		    PH_NFCLIB_ACTIVATION_PROFILE_ISO);
	    if (ulStatus != PH_NFCLIB_STATUS_SUCCESS)
	    {
		break;
	    }
	    /* Activating the card with non blocking activation mode.
	     * Polling every 100ms for detecting card for 15 mins.
	     * So nfcappTIMEOUT_15MINS = (((100msdelay * 10)sec * 60)min * 15)
	     */
	    ulTimeOutCounter = 0;
	    do
	    {
		if(phNfcLib_Activate(appTECHNOLOGY_MASK, &PeerInfo,
				    NULL) != PH_NFCLIB_STATUS_PEER_ACTIVATION_DONE)
		{
		    if(ulTimeOutCounter<nfcappTIMEOUT)
		    {
			/* Checking for nfc stop execution event */
			xStopEventRecieved = xEventGroupWaitBits(
				pxResHandle->xEventGroupNFC,
				eventsNFC_STOP_EXEC,
				pdFALSE,
				pdFALSE,
				usermodeDELAY_MSEC_100);
			xStopEventRecieved = xEventGroupClearBits(
				pxResHandle->xEventGroupNFC,
				eventsNFC_STOP_EXEC);
			if((xStopEventRecieved & eventsNFC_STOP_EXEC) ==
				eventsNFC_STOP_EXEC)
			{
				ulEventToSend = eventsKEYFOB_STOP_NFC_SUCCESS;
				xEventRecieved = xStopEventRecieved;
				break;
			}
			ulTimeOutCounter++;
		    }
		    else
		    {
			ulEventToSend = eventsKEYFOB_DETECT_TIMEOUT;
			break;
		    }
		}
		else
		{
		    break;
		}
	    }while(1);
	    switch (PeerInfo.dwActivatedType)
	    {
	    case E_PH_NFCLIB_MIFARE_DESFIRE:
		debugPRINT_NFC("Mifare Desfire/type-4 detected...\n");
		if (((xEventRecieved & eventsNFC_DETECT_KEYFOB)
			== eventsNFC_DETECT_KEYFOB)
			&& ((xEventRecieved & eventsNFC_ASSOCIATE_KEYFOB)
				== eventsNFC_ASSOCIATE_KEYFOB))
		{
		    debugPRINT_NFC(
			" eventsNFC_DETECT_KEYFOB & eventsNFC_ASSOCIATE_KEYFOB"
			" are mutually exclusive. It can't occur simultaneously");

		    ulEventToSend = eventsKEYFOB_NFC_UNKNOWN_COMMAND;
		    break;
		}
		else if((xEventRecieved & eventsNFC_GET_KEYFOB_SERIAL)
			== eventsNFC_GET_KEYFOB_SERIAL)
		{
		    if (ulNfcCmdSelectMasterApplet() != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Failed to select Master applet \n");
			break;
		    }
		    if (ulNfcCmdSelectUserApplet() != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Failed to select User applet \n");
			break;
		    }
		    ulRLen = commandsRAW_ACCESS_BUF_LEN;
		    if (ulNfcCmdGetSerial(ucDataBuf, &ulRLen)
			    != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Failed to get peer's serial number\n");
			ulEventToSend = eventsKEYFOB_GET_SERIAL_FAILURE;
			break;
		    }
		    memset((void*)xNfcData.ucKeyFobSerial, 0,
					usermodeKEYFOB_SERIAL_LEN);
		    memcpy((void*)xNfcData.ucKeyFobSerial, (void *)ucDataBuf,
			    ulRLen);
		    if(xQueueReset(pxResHandle->xNfcQueue) != pdTRUE)
		    {
			debugERROR_PRINT(" Failed to reset queue ");
			ulEventToSend = eventsKEYFOB_GET_SERIAL_FAILURE;
			break;
		    }
		    if(xQueueSend(pxResHandle->xNfcQueue, &xNfcData,
			    portMAX_DELAY) != pdTRUE)
		    {
			debugPRINT_NFC(" Failed to send data into the queue ");
			ulEventToSend = eventsKEYFOB_GET_SERIAL_FAILURE;
			break;
		    }
		    else
		    {
			debugPRINT_NFC("eventsKEYFOB_GET_SERIAL_SUCCESS\n");
			ulEventToSend = eventsKEYFOB_GET_SERIAL_SUCCESS;
		    }
		}
		else if ((xEventRecieved & eventsNFC_DETECT_KEYFOB)
			== eventsNFC_DETECT_KEYFOB)
		{
		    debugPRINT_NFC("Running keyFob authentication procedure \n");
		    if (ulNfcCmdSelectMasterApplet() != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Failed to select Master applet \n");
			break;
		    }
		    if (ulNfcCmdSelectUserApplet() != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Failed to select User applet \n");
			break;
		    }
		    /* Get Key fob name */
		    ulRLen = commandsRAW_ACCESS_BUF_LEN;
		    if (ulNfcCmdGetName(ucDataBuf, &ulRLen)
			    != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Failed to read name of peer \n");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
		    pxKeyInfo = (keyfobidKeyFobInfo_t *) pvPortMalloc(
		                    sizeof(keyfobidKeyFobInfo_t));
		    if(pxKeyInfo == NULL)
		    {
			debugERROR_PRINT("Failed to allocate memory for "
					     "keyfob info structure");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
		    if(pdTRUE == ucFirstBoot)
		    {
			/* Get key fob details from flash */
			if(lNfcCommonGetKeyByName(pxKeyInfo,ucDataBuf,ulRLen)
				!= NO_ERROR)
			{
			    debugERROR_PRINT(
				    " Error in reading the key from flash ");
			    ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			    break;
			}
		    }
		    else
		    {
			memcpy((void*)pxKeyInfo, (void*)&gxKeyFobInfo,
				sizeof(keyfobidKeyFobInfo_t));
		    }
		    /* Verify CVM pins */
		    /* TODO: enable CVM support */
		    /*
		    if(ulNfcCmdVerifyCVM(pxKeyInfo->xSecKeyZ.ucTDESKey)
			    != NO_ERROR)
		    {
			debugERROR_PRINT(" Error in CVM Verification ");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
		    */
		    /* generate random number */
		    if (ulGenerateRandomNumber((uint32_t*) xChallenge.ucSeed,
			    commandsTRNG_NUM_OF_WORDS))
		    {
			debugERROR_PRINT("Failed to generate random number \n");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
		    if (ulGenerateRandomNumber(
			    (uint32_t*) xChallenge.ucSeedIncrFactor,
			    nfccommonSEED_INC_WLEN))
		    {
			debugERROR_PRINT("Failed to generate random number \n");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
		    xChallenge.ucRotInterval[0] = nfccommonROT_INTERVAL;
		    xChallenge.ucBleRange = pxResHandle->ucBLERange;
		    /* Generate the SHA1 hash */
		    ulStatus=ucl_sha1(ucSSKey, xChallenge.ucSeed,
			    commandsMAX_DATA_SIZE);
		    if(ulStatus!= NO_ERROR)
		    {
			debugERROR_PRINT(" Failed to generate the SHA1 hash");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
		    /* Encrypt using x and then z*/
		    ulStatus = ulNfcCommonEncrpytData(ucTmpBuf,
			    (uint8_t *)&xChallenge,
			    pxKeyInfo->xSecKeyX.ucTDESKey,
			    sizeof(NfcCommonSeed_Info_t));
		    if(ulStatus != NO_ERROR )
		    {
			debugERROR_PRINT("Failed to encrypt data \n");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
		    ulStatus = ulNfcCommonEncrpytData(ucDataBuf,
			    ucTmpBuf,
			    pxKeyInfo->xSecKeyZ.ucTDESKey,
			    sizeof(NfcCommonSeed_Info_t));
		    if(ulStatus != NO_ERROR )
		    {
			debugERROR_PRINT("Failed to encrypt data \n");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
		    /* Send the encrypted data to keyFob */
		    if(ulNfcCmdWriteRawData(ucDataBuf,
			    sizeof(NfcCommonSeed_Info_t))
			    != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Failed to write data to peer \n");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
		    /* Read data */
		    ulRLen = commandsRAW_ACCESS_BUF_LEN;
		    if (ulNfcCmdReadRawData(ucDataBuf, &ulRLen)
			    != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Failed to read data from peer \n");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
		    /* Decrypt using z then y */
		    ulStatus = ulNfcCommonDecrpytData(ucTmpBuf, ucDataBuf,
			    pxKeyInfo->xSecKeyZ.ucTDESKey, ulRLen);
		    if (ulStatus != NO_ERROR)
		    {
			debugERROR_PRINT("Failed to decrypt data \n");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
		    ulStatus = ulNfcCommonDecrpytData(ucKeyFobKey, ucTmpBuf,
			    pxKeyInfo->xSecKeyY.ucTDESKey, ulRLen);
		    if (ulStatus != NO_ERROR)
		    {
			debugERROR_PRINT("Failed to decrypt data \n");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
		    if(ulNfcCommonVerifyKey(ucKeyFobKey, ucSSKey,
			    commandsSHA_HASH_LENGTH)
			    != eventsKEYFOB_NFC_AUTH_SUCCESS)
		    {
			debugERROR_PRINT("Invalid Data Received \n");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
#ifdef BLE_SEED_KEYFOB_SERIAL
		    memset((void*)ucTmpBuf, 0, commandsRAW_ACCESS_BUF_LEN );
		    ulPLen = commandsRAW_ACCESS_BUF_LEN;
		    if(ulNfcCmdGetSerial( ucTmpBuf, &ulPLen ))
		    {
			debugERROR_PRINT(" Failed to get the KeyFob serial ");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			break;
		    }
#endif
		    else
		    {
#ifdef BLE_SEED_KEYFOB_SERIAL
			/* Sending the BLE seed with 28 bytes of KeyFob serial
			 * number and last 4 bytes as zeros.
			 */
			memset((void*)xNfcData.ucBleSeed, 0, nfccommonSEED_SIZE);
			memcpy((void*)xNfcData.ucBleSeed,
				(void*)ucTmpBuf, ulPLen );
			/*And updating BLE seed increment factor with KeyFob
			 * serial number last 4 bytes.
			 */
			memcpy((void*)xNfcData.ucLeIncrFactor,
				(void*) (xNfcData.ucBleSeed + ulPLen
					- nfcappKEYFOB_SERIAL),
				nfccommonSEED_INC_SIZE);
			memset((void*)ucTmpBuf, 0, commandsRAW_ACCESS_BUF_LEN );
#else
			/* Sending seed to user for generating BLE broadcast data */
			memcpy((void*)xNfcData.ucBleSeed,
				(void*)xChallenge.ucSeed, nfccommonSEED_SIZE);
			memcpy((void*)xNfcData.ucLeIncrFactor,
				(void*) xChallenge.ucSeedIncrFactor,
				nfccommonSEED_INC_SIZE);
#endif
			if(xQueueSend(pxResHandle->xNfcQueue, &xNfcData,
				portMAX_DELAY) != pdTRUE)
			{
			    debugERROR_PRINT("Failed to send data into queue\n");
			    ulEventToSend = eventsKEYFOB_NFC_AUTH_FAILURE;
			    break;
			}
			/* Storing the key to global buffer for further
			 * processing
			 */
			memcpy((void*)&gxKeyFobInfo, (void*)pxKeyInfo,
				sizeof(keyfobidKeyFobInfo_t));
			ucFirstBoot = pdFALSE;
			debugPRINT_NFC("KEYFOB AUTHENTICATION SUCCESS\n");
			ulEventToSend = eventsKEYFOB_NFC_AUTH_SUCCESS;
			if(gxKeyFobInfo.ulKeyPrevilage !=
				nfccommonKEY_PRIVILEGE_ADMIN)
			{
			    ulEventToSend |= eventsKEYFOB_NOT_ADMIN;
			}
		    }
		}
		else if ((xEventRecieved & eventsNFC_ASSOCIATE_KEYFOB)
			== eventsNFC_ASSOCIATE_KEYFOB)
		{
		    /* Check for first keyfob association */
		    if(lKeyfobidCheckFreeIndex(&ucKeyEntryNum) != NO_ERROR)
		    {
			debugERROR_PRINT("Error in checking the free index\n");
		    	ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
		    	break;
		    }
		    /* Checking for key entry full */
		    if(keyfobidMAX_KEFOB_ENTRIES == ucKeyEntryNum)
		    {
			debugPRINT_NFC(" Max number of KeyFobs are already"
					"associated ");
			ulEventToSend = eventsKEYFOB_NFC_ENTRY_FULL;
			break;
		    }
		    else if(keyfobidNO_KEY_ASSOCIATED == ucKeyEntryNum)
		    {
			debugPRINT_NFC(" First KeyFob Associating ");
		    }
		    else
		    {
			/* Check KeyFob privilege */
			if(gxKeyFobInfo.ulKeyPrevilage
				!= nfccommonKEY_PRIVILEGE_ADMIN)
			{
			    debugERROR_PRINT(" Please use ADMIN privilege Keyfob"
				    " to associate new keyfob ");
			    ulEventToSend = eventsKEYFOB_NOT_ADMIN;
			    break;
			}
		    }

		    debugPRINT_NFC("Running keyFob association procedure \n");
		    /* Generating the Public Key */
		    if (ulCryptoEcdh_PublicKeyGen(cryptoCURVE_ID, ucSSKey,
			    &ulLen) != NO_ERROR)
		    {
			debugERROR_PRINT("Public Key Generation failed \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }

		    if (ulNfcCmdSelectMasterApplet() != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Failed to select Master applet \n");
			break;
		    }
		    if (ulNfcCmdSelectUserApplet() != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Failed to select User applet \n");
			break;
		    }
		    /*Update OLED display */
		    vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
		    vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR);
		    vDisplayText((const int8_t *)uiASSOCIATION_PROGRESS);
		    if (ulNfcCmdKeyAssociate() != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Key association request failed \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    /* Read peers public key */
		    ulPLen = commandsRAW_ACCESS_BUF_LEN;
		    if (ulNfcCmdReadPublicKey(ucDataBuf, &ulPLen)
			    != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Failed to read Public Key from peer\n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    /* Send the Public Key to Keyfob */
		    if (ulNfcCmdWritePublicKey(ucSSKey, ulLen)
			    != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Failed to write Public Key to peer\n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    /* Generate Shared Secret Key */
		    if (ulCryptoEcdh_GenSSK(ucDataBuf, (uint8_t *)&ulPLen,
			    ucSSKey) != NO_ERROR)
		    {
			debugERROR_PRINT(
				"Shared Secret Key Generation failed \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
#if DBG_DATA
		    uint32_t ulTmp;
		    for(ulTmp=0;ulTmp<ulPLen;ulTmp++)
		    {
			debugPRINT_NFC(" ucSSKey 0x%02x",ucSSKey[ulTmp]);
		    }
#endif
		    /* Confirming the shared secret keys */
		    ulStatus = ulNfcCommonEncrpytData(ucDataBuf,gucPlain,ucSSKey,
							sizeof(gucPlain));
		    if(ulStatus != NO_ERROR )
		    {
			debugERROR_PRINT("Failed to encrypt data \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    if(ulNfcCmdConfirmSSK(ucDataBuf, sizeof(gucPlain))
			    != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("SSK Verification failed \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    /* Generate ORWL challenge */
		    if (ulGenerateRandomNumber((uint32_t*) ucORWLKey,
			    nfccommonKEYFOB_SEED_WLEN))
		    {
			debugERROR_PRINT("Failed to generate random number \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    ulStatus = ulNfcCommonEncrpytData(ucTmpBuf, ucORWLKey,
					    ucSSKey, cryto3DES_KEY_SIZE);
		    if (ulStatus != NO_ERROR)
		    {
			debugERROR_PRINT("Failed to encrypt data \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    /* Share encrypted ORWL Challenge */
		    if(ulNfcCmdSendORWLChallenge(ucTmpBuf, cryto3DES_KEY_SIZE)
					    != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Sending ORWL challenge failed \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    /* Get Keyfob Challenge */
		    ulLen = commandsRAW_ACCESS_BUF_LEN;
		    if(ulNfcCmdGetKeyFobChallenge(ucDataBuf, &ulLen)
		    			    != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Get KeyFob challenge failed \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    /* Decrypt the KeyFob Challenge */
		    ulStatus = ulNfcCommonDecrpytData(ucKeyFobKey, ucDataBuf,
					    ucSSKey, cryto3DES_KEY_SIZE);
		    if (ulStatus != NO_ERROR)
		    {
			debugERROR_PRINT("Failed to decrypt data \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    /* Verify CVM pins */
		    /* TODO: enable CVM support */
		    /*
		    if(ulNfcCmdVerifyCVM(ucSSKey) != NO_ERROR)
		    {
			debugERROR_PRINT(" Error in CVM Verification ");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    */
		    /* Blocking to get Nfc Key name and privilege */
		    xQueueReceive(pxResHandle->xNfcQueue, &xNfcData, portMAX_DELAY);
		    ulKeyFobLen = commandsKEYFOB_NAME_LENGTH;
		    memcpy((void *)ucKeyFOBName, (void *)xNfcData.ucKeyFobName,
			    ulKeyFobLen);
		    if(strlen((char *)ucKeyFOBName)!= commandsKEYFOB_NAME_LENGTH)
		    {
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    /* Encrypt Key Fob name */
		    /* Taking 5 bytes extra at the end to make multiple of 8 */
		    ulStatus = ulNfcCommonEncrpytData(ucDataBuf, ucKeyFOBName,
			    ucSSKey, ulKeyFobLen+nfccommonKEYFOB_NAME_PAD_LEN);
		    if (ulStatus != NO_ERROR)
		    {
			debugERROR_PRINT("Failed to encrypt data \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    /* Store the key Fob name in Key Fob */
		    if(ulNfcCmdSaveKeyFobName(ucDataBuf, ulKeyFobLen+
			    nfccommonKEYFOB_NAME_PAD_LEN)
			    != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Sending KeyFob name failed \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    /* Sending save key info */
		    if(ulNfcCmdSaveKey() != PH_NFCLIB_STATUS_SUCCESS)
		    {
			debugERROR_PRINT("Saving KeyFob info failed \n");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    /* Confirming first keyFob is stored with ADMIN privilege */
		    if(keyfobidNO_KEY_ASSOCIATED == ucKeyEntryNum)
		    {
			xNfcData.ulKeyPrivilege = nfccommonKEY_PRIVILEGE_ADMIN;
		    }
		    /* Storing the KeyFob info in the flash*/
		    if(lNfcCommonWriteKeyInfo(ucKeyFOBName, ulKeyFobLen,
			    xNfcData.ulKeyPrivilege, ucSSKey, ucORWLKey,
			    ucKeyFobKey)!= NO_ERROR)
		    {
			debugERROR_PRINT(" Error in writing the key to flash ");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_FAILURE;
			break;
		    }
		    else
		    {
			debugPRINT_NFC(" KEYFOB_NFC_ASSOCIATION_SUCCESS ");
			ulEventToSend = eventsKEYFOB_NFC_ASSOCIATION_SUCCESS;
		    }
		}
		break;

	    default:
		if( ulEventToSend != eventsKEYFOB_DETECT_TIMEOUT)
		{
		    debugERROR_PRINT(
			    " Failed to identify a valid KeyFob ActivatedType = %d\n",
			    PeerInfo.dwActivatedType);
		}
		break;
	    }
	    if(pxKeyInfo != NULL)
	    {
		vPortFree(pxKeyInfo);
		pxKeyInfo = NULL;
	    }
	    if( ulEventToSend != eventsKEYFOB_DETECT_TIMEOUT)
	    {
		ulStatus = phNfcLib_Deactivate(PH_NFCLIB_DEACTIVATION_MODE_RELEASE,
			&PeerInfo);
		debugPRINT_NFC(" Please remove the card \n");
		if (ulStatus != PH_NFCLIB_STATUS_SUCCESS)
		{
		    debugPRINT_NFC("Deactivate with Release Mode failed, "
				    "card was removed while deactivating... \n");
		    debugPRINT_NFC("Thus Performing Deactivate with RF OFF mode... \n");
		    ulStatus = phNfcLib_Deactivate(
				    PH_NFCLIB_DEACTIVATION_MODE_RF_OFF, &PeerInfo);
		}
	    }
	    /* Disable NFC BOOSTER */
	    phNfcLibDisableNfcBooster();
	    /* Disable the RF field so that field should not detect in ideal condition */
	    phNfcLibDisable_RF_field();

	    END:
	    if (ulEventToSend)
	    {
		do
		{
		    xBits = xEventGroupSetBits(
			    pxResHandle->xEventGroupUserModeApp, /* The event group being updated. */
			    ulEventToSend); /* The bits being set. */

		    if ((xBits & ulEventToSend) != ulEventToSend)
		    {
			debugPRINT_NFC("KeyFob event send failed,"
					"retrying..\n");
			/* TODO: Multiple task priority is not supported so
			 * sending event only once.
			 */
			ulEventToSend = nfcappCLEAR_EVENT;
			vTaskDelay( eventsEVENT_SET_DELAY);
		    }
		    else
		    {
			ulEventToSend = nfcappCLEAR_EVENT; /* clearing the flag */
			debugPRINT_NFC("keyfob event sent to supervisor"
				" successfully ");
			break; /* set config. successful */
		    }
		}
		while (ulEventToSend != nfcappCLEAR_EVENT);

		/* We need to wait for indication for supervisor after
		 * sending a message to it
		 */
		ulWaitForEvent = pdTRUE;
	    }
	}
	while (1);
	(void) phNfcLib_DeInit();
	debugERROR_PRINT(" \n ERR: Should never reach here \n");
    }
}
/*----------------------------------------------------------------------------*/
