/**===========================================================================
 * @file rot.c
 *
 * @brief This file contains routines and definition for ORWL ROT mode
 * This mode is also referenced as non tampered production mode
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

#include <systemRes.h>
#include <rot.h>
#include <usermode.h>
#include <orwl_disp_interface.h>
#include <Init.h>
#include <errors.h>
#include <portable.h>
#include <debug.h>
#include <trng.h>
#include <access_keys.h>
#include <mfgdata.h>
#include <pinentry.h>
#include <nvsram.h>
#include <rtc.h>
#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>
#include <events.h>
#include <user_config.h>
#include <intel_suc_comm.h>
#include <orwl_err.h>
#include <oled_ui.h>
#include <orwl_gpio.h>
#include <mfgdata.h>

/** Modulus for encrypting data */
#define rotRSA_N "df5bb8a343906d96c7a8fff6a6fffffba1937245c39a05f9adcb756ba59d" \
		 "81249508719b91e0aa2f2ef3e4ee2026c08c4f2107bc0a7965419aa3dab2" \
		 "0ac80d60727b3d86dc736d64b49b8898ff1d4d3a90b6b840e4018b88cd34" \
		 "7f890ed40c000e60bc00d5bbc3def6af48d2870b3da0b2327dbc0ece41db" \
		 "af4e72e608a0a5e00f5b2127cd28737af69a247146f1bd7d30d69d517b7d" \
		 "a67a696fc180767dd391f8aa397120845a2504761f8a168a7fc880dfbc53" \
		 "23ca074bb5e92fb8a26952899ccb7203ad2973625c67131baebf292831df" \
		 "9bf54b19541b423fd1c1c3d13ab5af53753fc5eb8aa06c76bd10ce821a8b" \
		 "197e562702a57e463943c5b12cb6fb2f"

/** Public key exponent for encryption */
#define rotRSA_E "10001"

#define rotBASEVALUE_FOR_HEX		(16)	/** Base value for hexadecimal */
#define rotMAX_DATA_SIZE		(40)	/** Data size to be encrypted */
#define	rotDEFAULT_PIN_LEN		(6)	/** Valid data in default pin */
#define rotENCRYPTED_MSG_SIZE		(256)	/** Size of Encrypted Message */
#define rotDISPLAY_MESSAGE_DURATION	(3000)	/** OLED message display duration in rot Mode */
#define rotBYTES_To_WORD_FACT		(4)	/** factor to convert Bytes to words 4Bytes=1Word*/
#define rotOS_INSTALLED			(1)	/** macro to indicate OS installed */
#define rotOS_NOT_INSTALLED		(2)	/** macro to indicate OS not installed  */
#define rotBIT_SHIFT			(4)	/** bit shift for extracting lower 4 bits of number */
#define rotBYTE_MASK			(0x0F)	/** mask for extracting only a Byte from number */

/** global resource handler */
extern xSMAppResources_t xgResource;
/**
 * @brief function pointer table holds various function pointers to
 * to handle different states in OOB mode.
 */
static void (*prvROTModeStates[eSTATE_ROT_MAX])(eSuCRotStates *xpDevState,
	    xSMAppResources_t *pxResHandle);

/**
 * @brief This function performs the Intel CPU power on and then moves to next
 * state automatically.
 *
 * @param *xpDevState  device state
 * @param *pxResHandle application resource handle
 *
 * @return void
 */
static void prvRotCpuPowerOn(eSuCRotStates *xpDevState,
	xSMAppResources_t *pxResHandle);

/**
 * @brief Generate and store ORWL secret keys
 *
 * Generates and stores below secrets
 * 1) SSD Encryption Key
 * 2) HASH
 * 3) Default ORWL password
 *
 * @param *xpDevState  device state
 * @param *pxResHandle application resource handle
 *
 * @return void
 */
static void prvRotSetSecrets(eSuCRotStates *xpDevState,
	xSMAppResources_t *pxResHandle);

/**
 * @brief Wait for the OS installation success command from BIOS.
 * On receiving the success command, It prints the *ENCRYPTED* HASH
 * and *ENCRYPTED* default password on console along with the serial number
 * in below format.
 *
 *##########################################################
 *<Device>
 *	<SUCSerial-Num>8568132338</SUCSerial-Num>
 *	<ORWLSerial-Num>0876356379</ORWLSerial-Num>
 *	<Secret>a936af92b0ae20b1ff6c3347a72e5fbe</Secret>
 *	<Date>2017-04-29</Date>
 *##########################################################
 * @param *pxDevState  device state
 * @param *pxResHandle application resource handle
 *
 * @return void
 */
static void prvRotComplete(eSuCRotStates *pxDevState,
	xSMAppResources_t *pxResHandle);

/**
 * @brief This function performs ROT error state
 *
 * @param *pxDevState  device state
 * @param *pxResHandle application resource handle
 *
 * @return void
 */
static void prvRotError(eSuCRotStates *pxDevState,
	xSMAppResources_t *pxResHandle);

/**
 * @brief This function generates SSD password and store on NVSRAM
 *
 * @return NO_ERROR on success
 *	   error code on failure
 */
static uint32_t prvRotGenerateSSDPassword( uint8_t *pucBuf, uint32_t ulSSDKeyLen );

/**
 * @brief This function erases the password and store on NVSRAM
 *
 * This function provides a simple interface to erase the password.
 *
 * @return NO_ERROR on success
 *	   error code on failure
 */
static uint32_t prvRotEraseSSDPassword( void );

/**
 * @brief This function generates HASH and store on NVSRAM
 *
 * @param pucBuf Buffer to store Hash
 * @param ulHashLen Length of hash requested
 *
 * @return NO_ERROR on success
 *	   error code on failure
 */
static uint32_t prvRotGenerateHash( uint8_t *pucBuf, uint32_t ulHashLen );

/**
 * @brief This function generates Default PIN and store on flash
 *
 * @return NO_ERROR on success
 *	   error code on failure
 */
static uint32_t prvRotGenerateDefaultPIN( void );

/**
 * @brief This function checks for SSD admin password
 *
 * @return void
 */
static uint32_t prvRotCheckSSDAdminPassword( void );

/**
 * @brief This function generates Default user configuration and store on flash
 *
 * @return NO_ERROR on success
 *	   error code on failure
 */
static uint32_t prvSetUserConfiguration ( void );

/**
 * @brief This function writes the SSD serial number to flash and updates the
 * OS installation flag.
 *
 * @param xIntelUserData Pointer to Intel User data structure.
 * @param ulIsOsInstalled Pointer to OS installation flag.
 *
 * @return Intel error code.
 */
static uint8_t prvWriteSSDSerialNUM( IntelUserData_t *xIntelUserData, uint32_t *ulIsOsInstalled );

/*---------------------------------------------------------------------------*/

static void (*prvROTModeStates[eSTATE_ROT_MAX])(eSuCRotStates *pxDevState,
	xSMAppResources_t *pxResHandle) =
{
	prvRotSetSecrets,	/* eSTATE_ROT_SET_SECRETS */
	prvRotCpuPowerOn,	/* eSTATE_ROT_POWER_ON */
	prvRotComplete,		/* eSTATE_ROT_COMPLETE */
	prvRotError,		/* eSTATE_ROT_ERR */
};
/*---------------------------------------------------------------------------*/

static void prvRotCpuPowerOn(eSuCRotStates *pxDevState,
	xSMAppResources_t *pxResHandle)
{
    configASSERT( pxDevState != NULL );
    configASSERT( pxResHandle != NULL );

    vSystemResPowerOn_IntelCpu();

    vInitDisplayWelcomeScreen();
    vTaskDelay(rotDISPLAY_MESSAGE_DURATION);

    *pxDevState = eSTATE_ROT_COMPLETE;
}
/*---------------------------------------------------------------------------*/

static void prvRotSetSecrets(eSuCRotStates *pxDevState,
	xSMAppResources_t *pxResHandle)
{
    NvsramData_t *pxDatPtr;
    uint32_t ulResult = 0;
    int32_t lRet = 0;
    /* RTC structure to read time from RTC hardware */
    rtcDateTime_t xRTCDateTime;
    int32_t lRTCReadStatus = NO_ERROR;

    /* Check if the RTC is initialized. If RTC is not initialized we cannot
     * proceed further. Display the same on OLED and update the product cycle
     * to PCBA.
     */
    lRTCReadStatus = lRtcGetDateTime(&xRTCDateTime);
    if(lRTCReadStatus != NO_ERROR)
    {
	/* Check the error code for RTC not initialized */
	if(lRTCReadStatus == COMMON_ERR_NOT_INITIALIZED)
	{
	    uint32_t ulUpdateProdCycle = eBOOTMODE_PRODUCTION_PCBA;
	    /* RTC is not initialized, display on OLED */
	    vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
	    vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR);
	    vDisplayText(
		    (const int8_t *) "RTC not Initialized, moving to PCBA. Flash production Image");
	    /* Display on console the same message */
	    debugERROR_PRINT(
		    "RTC not Initialized, moving to PCBA. Flash production Image");

	    /* Update the product cycle to PCBA */
	    lRTCReadStatus = lMfgdataUpdateProductCycle(&ulUpdateProdCycle);
	    if(lRTCReadStatus != NO_ERROR)
            {
		debugERROR_PRINT("Failed to update product cycle\n");
		/*This is an error case. Ideally we need to move ORWL to ERROR state.*/
		goto error;
            }
	    /* After updating the product cycle to PCBA, enter while(1) so that
	     * this thread does not proceed further.
	     */
	    while(1);
	}
	/* If RTC is initialized and still read fails, may be because the second
	 * register was read zero. This does not occur normally. In this case
	 * move to rot error, since RTC read can be retired in next boot.
	 */
	else
	{
	    debugERROR_PRINT("Failed to read Date..\n");
	    /*This is an error case. Ideally we need to move ORWL to ERROR state.*/
	    goto error;
	}
    }
    configASSERT( pxDevState != NULL );
    configASSERT( pxResHandle != NULL );

    pxDatPtr = (NvsramData_t *)pvPortMalloc(sizeof(NvsramData_t));
    configASSERT( pxDatPtr != NULL );

    memset(pxDatPtr, 0x00, sizeof(NvsramData_t));

    /* check for SSD administrator password */
    if( prvRotCheckSSDAdminPassword() != NO_ERROR)
    {
	/* Admin password not set.
	 * Expected to set in Production
	 * Lets stop
	 */
	goto error;
    }

    /* generate and store default PIN in FLASH */
    if( prvRotGenerateDefaultPIN() != NO_ERROR )
    {
	goto error;
    }

    /* check if keys already stored. If stored skip generation.
     * If not, generate and store.
     */
    lRet = lNvsramCheckHeader();
    if( lRet == eORWL_ERROR_NVSRAM_INVALID_HEADER )
    {
	/* Set the NVSRAM Aes encryption */
	vNvsramAESKeySet();

	debugPRINT_APP("Secret key Gen started ");

	lEraseNvsramComplete();

	/* generate HASH  */
	ulResult |= prvRotGenerateHash(pxDatPtr->ucHASH,
		nvsramSNVSRAM_HASH_LEN_MAX);

	/* Generate SSD Enc Key */
	ulResult |= ulRotGenerateUserSSDPassword(pxDatPtr->ulSSDEncKey,
		nvsramSNVSRAM_ENC_KEY_LEN_MAX);

	/* Init. password retry count to 0 */
	pxDatPtr->usPswdRetryCount = 0 ;

	/* store keys */
	ulResult |= lNvsramWriteData(pxDatPtr);

	if(ulResult != NO_ERROR)
	{
	    /* Something went wrong. Lets reboot */
	    debugERROR_PRINT("ROT Key Generation: Something went wrong !");
	    goto error;
	}
    }
    else if ( lRet == eORWL_ERROR_NVSRAM_NOT_ACCESSIBLE ||
	    lRet == COMMON_ERR_NULL_PTR )
    {
	goto error;
    }

    /* Write user default configuration values to flash */
    if ( prvSetUserConfiguration() != NO_ERROR)
    {
	debugERROR_PRINT("Failed to set user default configuration\n");
	goto error;
    }

    /* All keys generated & stored */
    *pxDevState = eSTATE_ROT_POWER_ON;
    if(pxDatPtr)
    {
	vPortFree(pxDatPtr);
    }
    return;

error:
    if(pxDatPtr)
    {
	vPortFree(pxDatPtr);
    }
    /* All keys generated & stored */
    *pxDevState = eSTATE_ROT_ERR;
}
/*---------------------------------------------------------------------------*/

static void prvRotComplete(eSuCRotStates *pxDevState,
	xSMAppResources_t *pxResHandle)
{
    uint32_t ulIsOsInstalled = rotOS_NOT_INSTALLED;
    uint32_t ulProductCycle;
    EventBits_t xBits;
    int32_t lResult;
    uint32_t ulLoop;
    uint8_t *pucHash;
    keysDSFT_MASTER_KEYS_t *pxMKeys;
    uint8_t *pucORWLSerial;
    rtcDateTime_t *xRTCDateTime;
    uint8_t * pucEncryptedMsg;
    uint8_t * pucMessage;
    uint8_t *pucSUCSerial;
    uint8_t pucDate ;
    uint8_t pucMonth ;
    uint16_t pusYear ;
    /* User data from Intel */
    IntelUserData_t xIntelUserData;
    /* Ack to Intel communication task */
    uint8_t ucACK = 0;
    const TickType_t xTicksToWait = 1000;

    configASSERT( pxDevState != NULL );
    configASSERT( pxResHandle != NULL );

    /* Allocate memory for reading hash */
    pucHash = (uint8_t *)pvPortMalloc(nvsramSNVSRAM_HASH_LEN_MAX);
    if( pucHash == NULL )
    {
	debugERROR_PRINT( "Failed to allocate memory for Hash \r\n");
	goto rotCompErr;
    }

    /* Allocate memory for reading mater keys */
    pxMKeys = (keysDSFT_MASTER_KEYS_t *)pvPortMalloc(sizeof(keysDSFT_MASTER_KEYS_t));
    if(pxMKeys == NULL )
    {
	debugERROR_PRINT("Failed to allocate Memory for key storage\r\n");
	goto rotCompErr;
    }

    /* Allocate memory for reading serial number */
    pucORWLSerial = (uint8_t *)pvPortMalloc(mfgdataSERIAL_NUM_SIZE);
    if(pucORWLSerial == NULL )
    {
	debugERROR_PRINT("Failed to allocate Memory for ORWL serial number\r\n");
	goto rotCompErr;
    }

    /* Allocate memory for reading  Suc serial number*/
    pucSUCSerial = (uint8_t *)pvPortMalloc(mfgdataSERIAL_NUM_SIZE);
    if(pucSUCSerial == NULL )
    {
	debugERROR_PRINT("Failed to allocate Memory for SUC serial number\r\n");
	goto rotCompErr;
    }

    /* Allocate memory for reading date and time */
    xRTCDateTime = (rtcDateTime_t *)pvPortMalloc(sizeof(rtcDateTime_t));
    if(xRTCDateTime == NULL )
    {
	debugERROR_PRINT("Failed to allocate Memory for Date&Time structure\r\n");
	goto rotCompErr;
    }

    /* Allocate memory for populating encrypted message */
    pucEncryptedMsg = (uint8_t *)pvPortMalloc(rotENCRYPTED_MSG_SIZE);
    if(pucEncryptedMsg == NULL )
    {
	debugERROR_PRINT("Failed to allocate Memory for encrypted msg\r\n");
	goto rotCompErr;
    }

    /* Allocate memory for storing msg to be encrypted */
    pucMessage = (uint8_t *)pvPortMalloc(keysPIN_LEN + nvsramSNVSRAM_HASH_LEN_MAX);
    if(pucMessage == NULL )
    {
	debugERROR_PRINT("Failed to allocate Memory for message\r\n");
	goto rotCompErr;
    }

    debugPRINT_APP(" Poling on SuC-BIOS UART communication ");

    /* Indicate user to install OS */
    vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
    vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR );
    vDisplayText((const int8_t *)"Please install OS \n");
    vTaskDelay(rotDISPLAY_MESSAGE_DURATION);

    while(ulIsOsInstalled != rotOS_INSTALLED)
    {

	/* Wait for OS installation status break only on Success
	 * Block on Confirmation from BIOS
	 */
	do
	{
	    /* Wait for event from Intel communication task regarding OS installation */
	    /* Check for Intel Command */
	    if(xQueueReceive(xgResource.xSucBiosSendQueue, &xIntelUserData, xTicksToWait) == pdTRUE)
	    {
		/* Check the command we need to address */
		switch(xIntelUserData.ucCommand)
		{
		case INTEL_HLOS_STATUS:
		    ucACK = prvWriteSSDSerialNUM(&xIntelUserData, &ulIsOsInstalled);
		    break;
		default:
		    /* This should not occur in proper working condition, if Intel
		     * Communication task sends any other event please report it
		     * as a bug with BIOS team.
		     */
		    debugERROR_PRINT(
			    "Received event %u from Intel which is not expected\n",
			    xIntelUserData.ucCommand);
		    /* Just send success to BIOS to avoid BIOS hangs */
		    ucACK = SUC_WRITE_STATUS_SUCCESS;
		    break;
		}

		/* We have received the event, send ACK for the same */
		xQueueSend(xgResource.xSucBiosReceiveQueue, &ucACK, xTicksToWait);

		/* If OS installation is failed ucACK will not be success, in that
		 * case send the ACK as write fail. Power off the Intel and wait
		 * for 30 seconds until Intel shutdown and then restart the ORWL.
		 */
		if(ucACK != SUC_WRITE_STATUS_SUCCESS)
		{
		    /* send SHUTDOWN signal using long press. This is safe shutdown */
		    vSystemResIntel_LongPress();

		    /* 30 sec wait. Allow Intel to shutdown smoothly */
		    vTaskDelay(SystemResINTEL_SHUTDOWN_WAIT_PERIOD*SystemResDELAY_INC_1S_30S_COUNT) ;

		    /* reboot ORWL */
		    resetSYSTEM_RESET;
		}
	    } /* xQueueReceive loop end */
	}while(ulIsOsInstalled != rotOS_INSTALLED);

	debugPRINT_APP("eventsROT_OS_INSTALLATION_COMPLETE");
    };

    /* Now OS installed: Enable UART for logging
     * and allow user to collect the logs with enough delay
     */
    vDisableUSBToIntel();
    vTaskDelay(rotDISPLAY_MESSAGE_DURATION);

    vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
    vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR );
    vDisplayText((const int8_t *)"OS Installation  Done !\n");
    vTaskDelay(rotDISPLAY_MESSAGE_DURATION);

    /* Read suc Serial number */
    lResult =lMfgdataReadSUCSerial(pucSUCSerial);
    if( lResult != NO_ERROR )
    {
	debugERROR_PRINT("Failed to read SUC serial number..\n");
	/*This is an error case. Ideally we need to move ORWL to ERROR state */
	goto rotCompErr;
    }
    else
    {
	debugPLAIN_PRINT("\t<SUCSerial-Num> %s </SUCSerial-Num>\n\r",pucSUCSerial);
    }

    /* Read ORWL Serial Number */
    lResult =lMfgdataReadORWLSerial(pucORWLSerial);
    if( lResult != NO_ERROR )
    {
	debugERROR_PRINT("Failed to read ORWL serial number..\n");
  	/*This is an error case. Ideally we need to move ORWL to ERROR state */
  	goto rotCompErr;
    }
    else
    {
	debugPLAIN_PRINT("\t<ORWLSerial-Num> %s </ORWLSerial-Num>\n\r",pucORWLSerial);
    }

    /* Read default PIN */
    lResult = lKeysReadMasterKeys( pxMKeys );
    if( lResult != NO_ERROR )
    {
  	debugERROR_PRINT("Failed to read default Pin..\n");
  	/*This is an error case. Ideally we need to move ORWL to ERROR state */
  	goto rotCompErr;
    }
    else
    {
	memcpy((void *)pucMessage ,(const void *)pxMKeys -> ucDefaultPIN,
		rotDEFAULT_PIN_LEN);
    }

#ifdef ROT_PRINT_DEF_PIN
    /* let print the default PIN to help debugging */
    debugPLAIN_PRINT("Default PIN: %d %d %d %d %d %d",
			pucMessage[0],
			pucMessage[1],
			pucMessage[2],
			pucMessage[3],
			pucMessage[4],
			pucMessage[5]);
#endif
    /* Display ORWL unique data on console. To be collected without fail*/
    /* Read Hash */
    lResult = lNvsramReadHash(pucHash);
    if( lResult != NO_ERROR )
    {
  	debugERROR_PRINT("Failed to read Hash..\n");
  	/*This is an error case. Ideally we need to move ORWL to ERROR state */
  	goto rotCompErr;
    }
    else
    {
	memcpy((void *)(pucMessage + rotDEFAULT_PIN_LEN),(const void *)pucHash,
		nvsramSNVSRAM_HASH_LEN_MAX);
    }
    /* Last byte must be zero for RSA encryption */
    pucMessage[rotDEFAULT_PIN_LEN + nvsramSNVSRAM_HASH_LEN_MAX + 1] = 0;

    /* Encrypt Default pin and hash..*/
    vRotRsaEncryption(pucMessage , pucEncryptedMsg);

    /* print encrypted secret data */
    debugPLAIN_PRINT("\t<Secret>");
    for( ulLoop = 0 ; ulLoop < rotENCRYPTED_MSG_SIZE; ulLoop++ )
	debugPLAIN_PRINT("%02x",pucEncryptedMsg[ulLoop]);
    debugPLAIN_PRINT("</Secret>\n\r");

    /* Reading of current date */
    lResult = lRtcGetDateTime(xRTCDateTime);
    if( lResult != NO_ERROR )
    {
  	debugERROR_PRINT("Failed to read Date..\n");
  	/*This is an error case. Ideally we need to move ORWL to ERROR state.*/
  	goto rotCompErr;
    }
    else
    {
	pucDate = xRTCDateTime -> ucDate;
	pucMonth = xRTCDateTime -> ucMonth;
	pusYear = xRTCDateTime -> usYear;
	debugPLAIN_PRINT("\t<Date> %d / %d / %d </Date>\n",pucDate,pucMonth,pusYear);
    }

    debugPLAIN_PRINT("</Device>\n\r");

   /* display console log & OLED message to Power off Intel CPU &
    * reboot ORWL to proceed further.
    */

    /* Ask User to press power button to move to oob mode */
    vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
    vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR );
    vDisplayText((const int8_t *)"Press power button to complete ROT \n");
    vDisplayText((const int8_t *)"...............! \n");

    /* clear event */
    xEventGroupClearBits(pxResHandle->xEventGroupUserModeApp,
	    eventsKEY_PRESS_SHORT);

    /* first clear & then wait for power button press event */
    do
    {
	/* Wait with max delay */
	xBits = xEventGroupWaitBits(pxResHandle->xEventGroupUserModeApp,
		eventsKEY_PRESS_SHORT,pdTRUE, pdFALSE, rotSECRET_PRINT_PREIOD);

	/* Display the XML Log so that we don't miss to capture while switching
	 * USB port by user */
	debugPLAIN_PRINT("<Device>\n\r");
	debugPLAIN_PRINT("\t<SUCSerial-Num> %s </SUCSerial-Num>\n\r",pucSUCSerial);
	debugPLAIN_PRINT("\t<ORWLSerial-Num> %s </ORWLSerial-Num>\n\r",pucORWLSerial);
	/* print encrypted secret data */
	debugPLAIN_PRINT("\t<Secret>");
	for( ulLoop = 0 ; ulLoop < rotENCRYPTED_MSG_SIZE; ulLoop++ )
	    debugPLAIN_PRINT("%02x",pucEncryptedMsg[ulLoop]);
	debugPLAIN_PRINT("</Secret>\n\r");
	debugPLAIN_PRINT("\t<Date> %d / %d / %d </Date>\n\r",pucDate,pucMonth,pusYear);
	    debugPLAIN_PRINT("</Device>\n\r");

    }while(xBits != eventsKEY_PRESS_SHORT);

    debugPRINT_APP("Power button pressed. Changing boot mode");

    /* Change Product Cycle to OOB */
    ulProductCycle = eBOOTMODE_DEV_OOB;
    lMfgdataUpdateProductCycle( &ulProductCycle );

    /*send shutdown command to BIOS [UART]
     * Alternate is to force shutdown using GIO.
     * This is not recommended.
     */

    /* send SHUTDOWN signal using long press */
    vSystemResIntel_LongPress();

    /* 30 sec wait. Allow Intel to shutdown smoothly */
    vTaskDelay(SystemResINTEL_SHUTDOWN_WAIT_PERIOD*SystemResDELAY_INC_1S_30S_COUNT) ;

    /* cutoff power supply */
    vSystemResForcePowerOff_IntelCpu();

    vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
    vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR );
    vDisplayText((const int8_t *)"ROT Completed   Successfully !\n");
    vTaskDelay(rotDISPLAY_MESSAGE_DURATION);

    rotCompErr:

    /* This step is just a place holder to say that USB is the defualt
     * setting and not UART. Though we are going to reboot sooner,
     * placing ports as USB as default.
     */
    vEnableUSBToIntel();

    /* Change system state to "eSTATE_ROT_ERR" */
    *pxDevState = eSTATE_ROT_ERR;

    /* release allocated memory */
    if(pucORWLSerial)
    {
	vPortFree(pucORWLSerial);
    }

    if(xRTCDateTime)
    {
	vPortFree(xRTCDateTime);
    }

    if(pucHash)
    {
	vPortFree(pucHash);
    }

    if(pxMKeys)
    {
	vPortFree(pxMKeys);
    }

    if(pucEncryptedMsg)
    {
	vPortFree(pucEncryptedMsg);
    }

    if(pucMessage)
    {
	vPortFree(pucMessage);
    }
    if(pucSUCSerial)
    {
	vPortFree(pucSUCSerial);
    }

    /** reboot ORWL */
    resetSYSTEM_RESET;

}
/*---------------------------------------------------------------------------*/

static void prvRotError(eSuCRotStates *pxDevState,
	xSMAppResources_t *pxResHandle)
{
    configASSERT( pxDevState != NULL );
    configASSERT( pxResHandle != NULL );

    vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
    vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR );
    vDisplayText((const int8_t *)"ORWL ROT ERROR: !!\n");

    while(1);
}
/*---------------------------------------------------------------------------*/

static uint32_t prvRotGenerateDefaultPIN( void )
{
    keysDSFT_MASTER_KEYS_t *pxMKeys;
    uint16_t usCrc;
    uint32_t ucDefPin = 0;
    uint32_t ulIndex = 0;
    uint32_t lRetVal = NO_ERROR;

    pxMKeys = (keysDSFT_MASTER_KEYS_t *)pvPortMalloc(sizeof(keysDSFT_MASTER_KEYS_t));

    if(pxMKeys == NULL )
    {
	debugERROR_PRINT("Failed to allocate Memory");
	return COMMON_ERR_FATAL_ERROR;
    }

    memset(pxMKeys, 0x00, sizeof(keysDSFT_MASTER_KEYS_t));

    /* Read keys and check if stored already */
    if(lKeysReadMasterKeys( pxMKeys ))
    {
	/* No Keys Present - Virgin Flash ? */
	debugPRINT_APP(" It is a Virgin Flash !");

	vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
	vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR );
	vDisplayText((const int8_t *)"No SSD admin password");
	return COMMON_ERR_FATAL_ERROR;
    }
    else
    {
	/* read key successful */
	debugPRINT_APP(" Read Default PIN from Flash !");

	/* validate Keys [CRC] */
	usCrc = usInitCommonCalcCrc( pxMKeys->ucDefaultPIN, pinentryPIN_LENGTH);

	/* compare CRC */
	if(!memcmp((void *)pxMKeys->ucDefaultPIN+pinentryPIN_LENGTH, (void *)&usCrc, (keysPIN_LEN - pinentryPIN_LENGTH)))
	{
	    /* CRC matched: Lets return */
	    debugPRINT_APP(" Default PIN Verified !");
	    if(pxMKeys != NULL)
	    {
		vPortFree(pxMKeys);
	    }
	    return NO_ERROR;
	}
	else
	{
	    /* CRC match failed. Probably power failure last time?
	     * Lets store keys again
	     * TODO: This code will be never executed in default PIN case
	     * However, This logic must be used for user keys.
	     */
	    debugPRINT_APP(" Default PIN Verification Failed: \n"
			    " Regenerating Pin again !");
		do
		{
		    uint8_t ucTemp;

		    if(ucDefPin == 0)
		    {
			if(ulGenerateRandomNumber(&ucDefPin, trngMIN_TRNG_GEN_WORDS))
			{
			    /* failed to Generate Key */
			    if(pxMKeys != NULL)
			    {
				vPortFree(pxMKeys);
			    }
			    return COMMON_ERR_FATAL_ERROR;
			}
		    }

		    /* now extract 6 digits from the random number */
		    ucTemp = ((ucDefPin >> rotBIT_SHIFT) & rotBYTE_MASK);
		    ucDefPin = (ucDefPin >> rotBIT_SHIFT);

		    if( ucTemp < 9 )
		    {
			if(ulIndex == 0 && ucTemp == 0)
			{
			    //first digit of password should not be 0
			    continue;
			}
			else
			{
			    pxMKeys->ucDefaultPIN[ulIndex] = ucTemp;
			    debugPRINT_APP("Selected Digit %d ",pxMKeys->ucDefaultPIN[ulIndex]);
			    ulIndex++;
			}
		    }
		}while(ulIndex < pinentryPIN_LENGTH);

	    /* Calculate CRC */
	    usCrc = usInitCommonCalcCrc( pxMKeys->ucDefaultPIN, pinentryPIN_LENGTH);

	    /* write CRC to last 2 bytes of Default PIN */
	    memcpy(pxMKeys->ucDefaultPIN + pinentryPIN_LENGTH, &usCrc,
		(keysPIN_LEN - pinentryPIN_LENGTH));

	    lRetVal = lKeysWriteMasterKeys(pxMKeys);
	    if (lRetVal  != NO_ERROR)
	    {
		debugERROR_PRINT("Failed to read master key\n");
	    }
	}
    }

    debugPRINT_APP(" Pin Generation Complete !");

    if(pxMKeys != NULL)
    {
	vPortFree(pxMKeys);
    }

    return lRetVal;
}
/*---------------------------------------------------------------------------*/

static uint32_t prvRotGenerateHash( uint8_t *pucBuf, uint32_t ulHashLen )
{
    uint32_t ulIndex = 0;

    if(pucBuf == NULL)
    {
	return COMMON_ERR_NULL_PTR;
    }

    if(ulHashLen == 0 || (ulHashLen % rotBYTES_To_WORD_FACT))
    {
	return COMMON_ERR_OUT_OF_RANGE;
    }

    /* TRNG */
    while(ulHashLen > 0)
    {
	if(ulHashLen < trngTRNG_SINGLE_SHOT_READ_LEN)
	{
	    if(ulGenerateRandomNumber((uint32_t *)(pucBuf+ulIndex), (ulHashLen/rotBYTES_To_WORD_FACT)))
	    {
		/* failed to generate Key */
		return COMMON_ERR_FATAL_ERROR;
	    }
	    /* Key Generated */
	    ulHashLen = 0;
	}
	else
	{
	    if(ulGenerateRandomNumber((uint32_t *)(pucBuf+ulIndex), trngTRNG_SINGLE_SHOT_READ_LEN))
	    {
		/* failed to generate Key */
		return COMMON_ERR_FATAL_ERROR;
	    }

	    ulHashLen -= (trngTRNG_SINGLE_SHOT_READ_LEN*rotBYTES_To_WORD_FACT);
	    ulIndex += (trngTRNG_SINGLE_SHOT_READ_LEN*rotBYTES_To_WORD_FACT);

	}
    }
    debugPRINT_APP(" HASH Generation completed !");
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static uint32_t prvRotCheckSSDAdminPassword( void )
{
    keysDSFT_MASTER_KEYS_t *pxMKeys;
    uint32_t ulProductCycle;

    pxMKeys = (keysDSFT_MASTER_KEYS_t *)pvPortMalloc(sizeof(keysDSFT_MASTER_KEYS_t));
    if(pxMKeys == NULL )
    {
	debugERROR_PRINT("Failed to allocate Memory");
	return COMMON_ERR_NULL_PTR;
    }

    memset(pxMKeys, 0xff, sizeof(keysDSFT_MASTER_KEYS_t));

    /* TODO:
     * Here we are checking only formasterKey
     * Ideally we need to examine the SSD password field of the
     * structure here !
     */
    /* Read keys and check if stored already */
    if(lKeysReadMasterKeys( pxMKeys ) != NO_ERROR )
    {
	/* No Keys Present - Virgin Flash ? */
	debugPRINT_APP(" It is a Virgin Flash !");
	ulProductCycle = eBOOTMODE_DEV_ERROR;
	if(lMfgdataUpdateProductCycle( &ulProductCycle ) != NO_ERROR )
	{
	    /* Failed to set product cycle */
	    debugERROR_PRINT("Failed to set product cycle");
	    if (pxMKeys != NULL)
	    {
		vPortFree(pxMKeys);
	    }
	    return COMMON_ERR_FATAL_ERROR;
	}

	if(pxMKeys != NULL)
	{
	    vPortFree(pxMKeys);
	}
	return COMMON_ERR_NOT_INITIALIZED;
    }

    if(pxMKeys != NULL)
    {
	vPortFree(pxMKeys);
    }

    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

uint32_t ulRotGenerateUserSSDPassword( uint32_t *pulBuf, uint32_t ulSSDKeyLen )
{

    uint32_t ulIndex = 0;

    if(pulBuf == NULL || ulSSDKeyLen == 0)
    {
	return COMMON_ERR_NULL_PTR;
    }

    /* TRNG */
    while(ulSSDKeyLen != 0)
    {
	if(ulSSDKeyLen < trngTRNG_SINGLE_SHOT_READ_LEN)
	{
	    if(ulGenerateRandomNumber((uint32_t *)(pulBuf+ulIndex), ulSSDKeyLen))
	    {
		/* failed to generate Key */
		return COMMON_ERR_FATAL_ERROR;
	    }
	    /* Key Generated */
	    ulSSDKeyLen = 0;
	}
	else
	{
	    if(ulGenerateRandomNumber((uint32_t *)(pulBuf+ulIndex), trngTRNG_SINGLE_SHOT_READ_LEN))
	    {
		/* failed to generate Key */
		return COMMON_ERR_FATAL_ERROR;
	    }
	    ulSSDKeyLen -= (trngTRNG_SINGLE_SHOT_READ_LEN);
	    ulIndex += (trngTRNG_SINGLE_SHOT_READ_LEN);

	}
    }
    debugPRINT_APP(" SSD User Password Generation completed !");
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

void vRotModeTask(void *pvArg)
{
    xSMAppResources_t *pxResHandle;
    volatile eSuCRotStates xDevState;

    /*
     * pvArgs must provide an event handler information for the thread created.
     */
    if (pvArg == NULL)
    {
	debugERROR_PRINT("\nFailed to create ROT mode task, invalid pvArg \n");
	return;
    }

    pxResHandle = (xSMAppResources_t *) pvArg;

    xDevState = eSTATE_ROT_SET_SECRETS ;

    do
     {
	if( xDevState < eSTATE_ROT_MAX)
	{
	    prvROTModeStates[xDevState]( &xDevState , pxResHandle );
	}
	else
	{
	    /* device has reached a bad state. Probably stack corruption?
	     * It should never happen
	     *
	     * Note: 'xDevState' is a local variable which will be accessed and modified
	     * by this thread alone. So, we should be able to manage it as required.
	     */
	    debugERROR_PRINT( "Invalid xDevState %d set, aborting..." ,
		    xDevState );
	    break;
	}
     } while ( 1 );

    vTaskDelete( NULL );
}
/*---------------------------------------------------------------------------*/

void vRotRsaEncryption(uint8_t *pucMessage ,uint8_t *pucEncryptedMsg)
{
    int32_t lRet;
    configASSERT(pucMessage != NULL);
    configASSERT(pucEncryptedMsg != NULL);

    mbedtls_rsa_context xRsa;
    /* Initialize the RSA context */
    mbedtls_rsa_init( &xRsa, MBEDTLS_RSA_PKCS_V15, 0 );

    xRsa.len = rotENCRYPTED_MSG_SIZE;
    /* Convert data in hex & assign to RSA Structure */
    lRet = mbedtls_mpi_read_string( &xRsa.N, rotBASEVALUE_FOR_HEX, rotRSA_N );
    lRet |= mbedtls_mpi_read_string( &xRsa.E, rotBASEVALUE_FOR_HEX, rotRSA_E );
    if( lRet )
    {
	debugPRINT("Failed to convert in hex format.. \n");
	while(1);
    }

    /* Check for the validation of public key */
    lRet = mbedtls_rsa_check_pubkey( &xRsa );
    if( lRet )
    {
	debugPRINT("Invalid public key..\n");
	while(1);
    }

    /* Encryption of msg */
    lRet = mbedtls_rsa_pkcs1_encrypt( &xRsa, ucTrue_Rand, NULL,
	    MBEDTLS_RSA_PUBLIC, rotMAX_DATA_SIZE, pucMessage, pucEncryptedMsg);
    if( lRet )
    {
	debugPRINT("Failed to encrypt data.. \n");
	while(1);
    }
}
/*---------------------------------------------------------------------------*/

static uint32_t prvSetUserConfiguration ( void )
{
    xUserConfig_t *pxUserConfig;
    uint32_t ulRetVal = NO_ERROR;

    pxUserConfig = pvPortMalloc(sizeof(xUserConfig_t));
    if ( pxUserConfig == NULL )
    {
	debugERROR_PRINT("Failed to allocate memory\n");
	return COMMON_ERR_NULL_PTR;
    }

    memset(pxUserConfig, 0xFF, sizeof(xUserConfig_t));
    /* default User configuration values */
    pxUserConfig -> ucBLERange    = userconfigBLE_RANGE;
    pxUserConfig -> ucIOEnDis     = userconfigDISABLE;
    pxUserConfig -> ucAdverRotInt = userconfigADVR_ROT_INT;
    pxUserConfig -> ucMode        = userconfigMODE;
    pxUserConfig -> ulSuCAction   = userconfigPROXPROTACTION;
    pxUserConfig -> ucAskPinOnBoot = (userconfigASKPINONBOOT);

    ulRetVal = lUserWriteUserConfig(pxUserConfig);
    if( ulRetVal != NO_ERROR )
    {
	debugERROR_PRINT(" Failed to write default user configuration \n");
	goto cleanup;
    }

cleanup:
    if (pxUserConfig)
    {
	 vPortFree(pxUserConfig);
    }
    return ulRetVal;
}
/*---------------------------------------------------------------------------*/

static uint8_t prvWriteSSDSerialNUM( IntelUserData_t *xIntelUserData, uint32_t *ulIsOsInstalled )
{
    /* Error code to check */
    int32_t lStatus = NO_ERROR;
    /* Pointer to Access key structure */
    keysDSFT_MASTER_KEYS_t *pxAccessKeys = NULL;
    /* ACK to Intel communication task */
    uint8_t ucACK = SUC_WRITE_STATUS_SUCCESS;
    /* OS installation structure */
    HlosInstallStat_t xOSInstallStatus;
    /* String length of SSD serial number */
    uint8_t ucSTRLength = 0;
    /* Index to for loop */
    uint8_t ucIndex = 0;
    /* Temporary variable to swap the string */
    uint8_t ucTemp = 0;
    /* Memset to zero to avoid invalid data */
    memset(&xOSInstallStatus, 0 ,sizeof(HlosInstallStat_t));

    /* Copy the OS Installation data passed from BIOS to our local structure */
    memcpy(&xOSInstallStatus, xIntelUserData->pvData, sizeof(HlosInstallStat_t));

    /* Update OS installation flag */
    *ulIsOsInstalled = xOSInstallStatus.statusUpdate;

    /* If OS installation failed, please display the same on OLED and ask the
     * user to re-install the OS.
     */
    if(*ulIsOsInstalled != rotOS_INSTALLED)
    {
	/* Indicate user failed to install OS */
	vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
	vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR);
	vDisplayText((const int8_t *) "Installation Failed !!\n");
	vTaskDelay(rotDISPLAY_MESSAGE_DURATION);

	/* Indicate user that ORWL will shutdown */
	vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
	vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR);
	vDisplayText((const int8_t *) "ORWL will shutdown \n");
	vTaskDelay(rotDISPLAY_MESSAGE_DURATION);

	/* return write fail */
	return INTEL_WRITE_STATUS_FAIL;
    }

    /* Get the string length of SSD serial number.
     * NOTE : We are expecting SSD serial number as string with NULL terminating,
     * BIOS as of now is sending as expected. If BIOS does not send NULL terminating
     * string in future then our logic goes on toss.
     */
    ucSTRLength = strlen((const char *)xOSInstallStatus.serNum);

    /* SSD serial number will be in BIG endian, convert to small endian */
    for(ucIndex = 0; ucIndex < ucSTRLength; ucIndex = ucIndex + 2)
    {
	ucTemp = xOSInstallStatus.serNum[ucIndex];
	xOSInstallStatus.serNum[ucIndex] = xOSInstallStatus.serNum[ucIndex + 1];
	xOSInstallStatus.serNum[ucIndex + 1] = ucTemp;
    }

    /* Allocate memory to read access keys */
    pxAccessKeys = (keysDSFT_MASTER_KEYS_t *)pvPortMalloc(sizeof(keysDSFT_MASTER_KEYS_t));
    if(pxAccessKeys == NULL )
    {
	debugERROR_PRINT("Failed to allocate Memory for key storage\r\n");
	/* Return Intel error code */
	return SUC_WRITE_STATUS_FAIL_MEM;
    }
    /* Memset the pxAccessKeys to 0xff */
    memset(pxAccessKeys, 0xff, sizeof(keysDSFT_MASTER_KEYS_t));

    /* Read the Access keys from flash */
    lStatus = lKeysReadMasterKeys(pxAccessKeys);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to read access keys \n");
	/* Update ucACK with correct error code */
	ucACK = SUC_WRITE_STATUS_FAIL_UNKOWN;
	/* Got to ERROR */
	goto ERROR;
    }

    /* Now update the SSD serial number to flash */
    memcpy(pxAccessKeys->ucSSDSerialNum, xOSInstallStatus.serNum, sizeof(HlosInstallStat_t));

    /* After updating the SSD serial number to acces key structure write back the
     * same to flash.
     */
    lStatus = lKeysWriteMasterKeys(pxAccessKeys);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to write access keys \n");
	/* Update ucACK with correct error code */
	ucACK = SUC_WRITE_STATUS_FAIL_UNKOWN;
	/* Got to ERROR */
	goto ERROR;
    }

    ERROR:
    /* After write is success, free access key memory */
    if(pxAccessKeys)
    {
	vPortFree(pxAccessKeys);
    }

    /* We are done return ACK */
    return ucACK;
}
/*---------------------------------------------------------------------------*/
