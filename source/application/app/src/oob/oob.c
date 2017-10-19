/**===========================================================================
 * @file oob.c
 *
 * @brief This file contains implementation of ORWL oob mode.
 *
 * @author vikram.k@design-shift.com
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

/*---------------------------------------------------------------------------*/
/* Global includes */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <errors.h>
#include <mml_sflc.h>

/* FreeRTOS includes */
#include <FreeRTOS.h>
#include <task.h>
#include <portable.h>
#include <queue.h>
#include <event_groups.h>

/* Application includes*/
#include <debug.h>
#include <access_keys.h>
#include <mem_common.h>
#include <hist_devtamper.h>
#include <oob.h>
#include <pinentry.h>
#include <orwl_disp_interface.h>
#include <oled_ui.h>
#include <nvsram.h>
#include <events.h>
#include <init.h>
#include <reset.h>
#include <mfgdata.h>
#include <powerbtn.h>
#include <irq.h>
#include <init.h>
#include <usermode.h>
#include <nfc_common.h>
#include <systemRes.h>
#include <pinhandling.h>

/* local to this file */
#define oobKEYFOBNAME	"KEYFOB_NAME_ID_" /**<Fixed 15 char KEYFOB NAME */
/*---------------------------------------------------------------------------*/
/**
 * @brief This function checks the PIN status on ORWL
 *
 * Reads default PIN and checks if it matches the PIN guidelines
 * Reads User PIN and validates it
 * Based on above verification, it changes the state automatically.
 *
 * @return void
 */
static void prvOobCheckPinStat( eSuCOobStates *xpDevState, xSMUOobModeResources_t *pxResHandle );

/**
 * @brief This function accepts the PIN from user and verifies it against
 * default PIN.
 *
 * @return void
 */
static void prvOobVerifyDefaultPIN( eSuCOobStates *xpDevState, xSMUOobModeResources_t *pxResHandle );

/**
 * @brief This function accepts the PIN from user validates it and stores on flash
 *
 * @return void
 */
static void prvOobSetUserPIN( eSuCOobStates *xpDevState, xSMUOobModeResources_t *pxResHandle );

/**
 * @brief This function accepts the PIN from user and verifies it against
 * User PIN.
 *
 * @return void
 */
static void prvOobVerifyUserPIN( eSuCOobStates *xpDevState, xSMUOobModeResources_t *pxResHandle );

/**
 * @brief This function implements the oob error state.
 * This is the un expected state of the device and device is no more functional
 * It requires service from the manufacturer. If you enter here, probably your
 * flash has gone bad?
 *
 * @return void
 */
static void prvOobErrorState( eSuCOobStates *xpDevState, xSMUOobModeResources_t *pxResHandle );

/**
 * @brief This function implements the oob unlocked state
 *
 * ORWL displays the Hash. If user accepts the HASH,
 * ORWL moves to KeyFob association process !
 *
 * @return void
 */
static void prvOobUnlockedState( eSuCOobStates *xpDevState, xSMUOobModeResources_t *pxResHandle );

/**
 * @brief function pointer table holds various function pointers to
 * to handle different states in OOB mode.
 */
static void (*vOOBModeStates[eSTATE_OOB_MAX])(eSuCOobStates *xpDevState,
		xSMUOobModeResources_t *pxResHandle);

/**
 * @brief this function converts interger to ascii.
 *
 * @param pucIn input hash buffer which as integer.
 * @param pucOut output hash buffer with ascii value.
 * @param ucLength of the input hash buffer.
 *
 * @return void.
 */
static int32_t lIntegetToAscii(uint8_t *pucIn, uint8_t *pucOut , uint8_t ucLength);
/*---------------------------------------------------------------------------*/

static int32_t lIntegetToAscii(uint8_t *pucIn, uint8_t *pucOut , uint8_t ucLength)
{
    int8_t cLsb;
    int8_t cMsb;
    uint8_t ucLoopCount;
    uint8_t ucIndex = 0;

    configASSERT( pucIn != NULL );
    configASSERT( pucOut != NULL );
    for ( ucLoopCount = 0; ucLoopCount < ucLength; ucLoopCount++)
    {
	cLsb = pucIn[ucLoopCount] & oobMASK;
	cMsb = (pucIn[ucLoopCount] >> oobROT_NIBBLE_SHIFT) & oobMASK;

	if ( cMsb >= oobINTEGER_MIN_RANGE && cMsb <= oobINTEGER_MAX_RANGE )
	{
	    pucOut[ucIndex] = oobASCII_0 + cMsb;
	}
	else if(cMsb >= oobALPABHET_MIN_RANGE && cMsb <= oobALPHABET_MAX_RANGE)
	{
	    pucOut[ucIndex] = oobASCII_A + cMsb;
	}
	else
	{
	    debugERROR_PRINT("MSB out of range\n");
	    return COMMON_ERR_INVAL;
	}
	ucIndex++;

	if ( cLsb >= oobINTEGER_MIN_RANGE && cLsb <= oobINTEGER_MAX_RANGE )
	{
	    pucOut[ucIndex] = oobASCII_0 + cLsb;
	}
	else if(cLsb >= oobALPABHET_MIN_RANGE && cLsb <= oobALPHABET_MAX_RANGE)
	{
	    pucOut[ucIndex] = oobASCII_A + cLsb;
	}
	else
	{
	    debugERROR_PRINT("LSB out of range\n");
	    return COMMON_ERR_INVAL;
	}
	ucIndex++;
    }
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

/*--------------------------- HELPER API's ----------------------------------*/
uint32_t ulOobIsUserPINSet(void)
{
    uint8_t ucUserPin[keysPIN_LEN];
    uint8_t temp_0[keysPIN_LEN];
    uint8_t temp_1[keysPIN_LEN];

    /* set temp_0 and temp_1 buffer to 0 and -1.
     * These to pins are invalid pin.
     */
    memset(temp_0, 0, keysPIN_LEN);
    memset(temp_1, -1,keysPIN_LEN);

    /*Check whether user has set the pin*/
    if (lKeysGetUserPIN(ucUserPin))
    {
	debugERROR_PRINT("Failed to read user pin\n");
	return COMMON_ERR_INVAL;
    }

    /* Compare 6 digit pin*/
    if (!memcmp(ucUserPin, temp_0, pinentryPIN_LENGTH)
	    || !memcmp(ucUserPin, temp_1, pinentryPIN_LENGTH))
    {
	debugERROR_PRINT(" User PIN not set !!");
	return COMMON_ERR_NO_MATCH;
    }

    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static void prvOobUnlockedState(eSuCOobStates *xpDevState,
		xSMUOobModeResources_t *pxResHandle)
{
    uint8_t *pucHash;
    uint8_t *pucHashOut;
    /* Buffer pointer for HASH to printed on OLED */
    uint8_t *pucHashOLED;
    /* Count to move through pucHashOut buffer */
    uint8_t ucHashCount = 0;
    /* Count to move through pucHashOLED buffer */
    uint8_t ucHashOLEDCount = 0;
    EventBits_t xBits;
    uint32_t ulEventToSend = 0, ulEventToWait = 0;
    uint32_t ulProductCycle;
    uint32_t ulKeyfobName;
    NfcEventData_t xNfcData;

    configASSERT(pxResHandle !=NULL );
    configASSERT(xpDevState != NULL );

    pucHash = pvPortMalloc(sizeof(uint8_t) * nvsramSNVSRAM_HASH_LEN_MAX);
    configASSERT( pucHash != NULL );
    pucHashOut = pvPortMalloc(sizeof(uint8_t) * nvsramSNVSRAM_HASH_LEN_MAX * 2 + 1);
    configASSERT( pucHashOut != NULL );
    pucHashOLED = pvPortMalloc(oobHASH_LEN_OLED_FORMAT);
    configASSERT( pucHashOLED != NULL );
    /* memset pucHashOLED to NULL */
    memset(pucHashOLED, '\0', oobHASH_LEN_OLED_FORMAT);

    if (lNvsramReadHash(pucHash))
    {
	debugERROR_PRINT("Failed to read HASH from nvsram\n");
	while(1);
    }

    if ( lIntegetToAscii(pucHash,pucHashOut,nvsramSNVSRAM_HASH_LEN_MAX)
	    != NO_ERROR)
    {
	debugERROR_PRINT("Failed to convert from integer to ascii\n");
	configASSERT(0);
    }
    pucHashOut[nvsramSNVSRAM_HASH_LEN_MAX * 2] = '\0';

    /* Now we need to format the HASH into 4 digit printable on OLED */
    while(pucHashOut[ucHashCount] != '\0')
    {
	/* There should be a space before every 4 hash character, because in a
	 * OLED line we can have three 4 bytes HASH, with 2 spaces between them
	 * forms 14 so there are 2 space left which can be added start and end of
	 * each OLED line(16).
	 */
	pucHashOLED[ucHashOLEDCount] = ' ';
	ucHashOLEDCount++;
	/* Now copy four bytes from pucHashOut to pucHashOLED */
	strncpy((char *) &pucHashOLED[ucHashOLEDCount],
		(const char *) &pucHashOut[ucHashCount], oob4DIGIT_GROUP);
	/* Now increment both the buffers by size of oob4DIGIT_GROUP */
	ucHashOLEDCount = ucHashOLEDCount + oob4DIGIT_GROUP;
	ucHashCount = ucHashCount + oob4DIGIT_GROUP;
	/* After adding three 4 byte hash to end a OLED line add one more space
	 * so that end of line will be a space. So there will one space at start
	 * and one at end of OLED line.
	 */
	if((ucHashCount % 12 ) == 0)
	{
	    pucHashOLED[ucHashOLEDCount] = ' ';
	    ucHashOLEDCount++;
	}
    }

    vUiMessageDisplay((const int8_t *)uiHASH_DISP,
		uiXCORDINATE_0 ,uiYCORDINATE_C - 1);
    vTaskDelay(uiTHREE_SEC_DELAY);
    vUiMessageDisplay((const int8_t *)pucHashOLED,
		uiXCORDINATE_0 ,uiYCORDINATE_0);
    /* ORWL unlocked state implementation to follow here */
    debugPRINT_APP("ORWL Unlocked state");
    if(pucHash)
    {
	vPortFree(pucHash);
    }
    if(pucHashOut)
    {
	vPortFree(pucHashOut);
    }
    if(pucHashOLED)
    {
	vPortFree(pucHashOLED);
    }

    /* Enable TAP detection thread */
#ifdef ENABLE_MPU_GIO_INTERRUPT
    /* Resume Tap detection task */
    vTaskResume(pxResHandle->xMpuFifoTskHandle);
#endif
    /* Wait for TAP/Timeout */
#ifdef ENABLE_TAP_FEATURE
    xBits = xEventGroupWaitBits(pxResHandle->xEventGroupPinEntry,
	    pinentryGET_TAP_EVENT,pdFALSE, pdFALSE, oobTAP_WAIT_TIMEOUT_SECS);
    if (xBits & pinentryGET_TAP_EVENT)
    {
	xEventGroupClearBits(pxResHandle->xEventGroupPinEntry,
		    pinentryGET_TAP_EVENT);
	vUiMessageDisplay((const int8_t *)uiHASH_CONFIRM_MSG,
				uiXCORDINATE_0 ,uiYCORDINATE_C - 1);
	vTaskDelay(uiTHREE_SEC_DELAY);
    }
#else
    /* Before waiting for short/long key press event
     * clear the bits first.Just to clear events arrived
     * because of previous button press.
     */
    xEventGroupClearBits(pxResHandle->xEventGroupUserModeApp,
		eventsKEY_PRESS_SHORT | eventsKEY_PRESS_LONG );
    /*Wait for Button Press/Timeout*/
    xBits = xEventGroupWaitBits(pxResHandle->xEventGroupUserModeApp,
	    eventsKEY_PRESS_SHORT | eventsKEY_PRESS_LONG ,pdFALSE,pdFALSE,
	    oobTAP_WAIT_TIMEOUT_SECS);
    if ((xBits & eventsKEY_PRESS_SHORT) || (xBits & eventsKEY_PRESS_SHORT))
    {
	xEventGroupClearBits(pxResHandle->xEventGroupUserModeApp,
		eventsKEY_PRESS_SHORT | eventsKEY_PRESS_SHORT);
	vUiMessageDisplay((const int8_t *)uiHASH_CONFIRM_MSG,
				uiXCORDINATE_0 ,uiYCORDINATE_C);
	vTaskDelay(uiTHREE_SEC_DELAY);

    }

#endif
    else
    {
	/* Wake on timeout */
	vUiMessageDisplay((const int8_t *)uiHASH_REJECT_MSG,
		uiXCORDINATE_0 ,uiYCORDINATE_C);
	vTaskDelay(uiTHREE_SEC_DELAY);
	resetSYSTEM_RESET;
    }

    /* Disable TAP detection thread */
#ifdef ENABLE_MPU_GIO_INTERRUPT
    vTaskSuspend(pxResHandle->xMpuFifoTskHandle);
#endif
    vUiMessageDisplay((const int8_t *)uiUSER_KEY_MSG,
		    uiXCORDINATE_0 ,uiYCORDINATE_C);
    vTaskDelay(uiTHREE_SEC_DELAY);

    /* Generate 4-byte random number */
    if (ulGenerateRandomNumber(&ulKeyfobName,trngMIN_TRNG_GEN_WORDS))
    {
	debugERROR_PRINT("Failed to generate random number\n");
	while(1);
    }
    /* Append the 4-byte random number to fixed keyfob name */
    lite_snprintf((char *)xNfcData.ucKeyFobName,sizeof(xNfcData.ucKeyFobName),
	    oobKEYFOBNAME"%u", ulKeyfobName);
    xNfcData.ucKeyFobName[commandsKEYFOB_NAME_LENGTH] = '\0';
    debugPRINT("KEYFOBNAME = %s\n",xNfcData.ucKeyFobName);
    /* Give administrator permission */
    xNfcData.ulKeyPrivilege = nfccommonKEY_PRIVILEGE_ADMIN;

    do
    {
	/* Send the keyfob name and permission to nfc thread */
	if(xQueueSend(pxResHandle->xNfcQueue, &xNfcData, portMAX_DELAY)
		!= pdTRUE)
	{
	    debugERROR_PRINT(" Failed to send data into the queue ");
	    continue;
	}
	break;
    }while(1);
    /* wait for KeyFob association */
    /* post association commands & validate */
    do
    {
	xBits = 0;
	ulEventToSend = eventsNFC_ASSOCIATE_KEYFOB;
	xBits = xEventGroupSetBits(pxResHandle->xEventGroupNFC, ulEventToSend);

	ulEventToWait = eventsKEYFOB_NFC_ASSOCIATION_SUCCESS |
		eventsKEYFOB_NFC_ASSOCIATION_FAILURE |
		eventsKEYFOB_DETECT_TIMEOUT;
	xBits = xEventGroupWaitBits(pxResHandle->xEventGroupUserModeApp, /* The event group being used */
				ulEventToWait, /* The bits within the event group to wait for. */
				pdFALSE, /* event bits should NOT be cleared before returning. */
				pdFALSE, /* Don't wait for both bits, either bit will do. */
				oobKEYFOB_ASSOC_TIMEOUT);

	xBits = xEventGroupClearBits(pxResHandle->xEventGroupUserModeApp,
		ulEventToWait);
	if(xBits & eventsKEYFOB_NFC_ASSOCIATION_SUCCESS)
	{
	    /* change product cycle to USER */
	    ulProductCycle = eBOOTMODE_DEV_USER;
	    if(lMfgdataUpdateProductCycle( &ulProductCycle ) != NO_ERROR )
	    {
		/* Failed to set product cycle */
		debugERROR_PRINT("Failed to set product cycle");
		while(1);
	    }
	    if ( lDisplayImageFullScreen(eDipsInt_KEY_ASSOCIATED))
	    {
		debugERROR_PRINT("Failed to display image\n");
	    }
	    vTaskDelay(uiEIGHT_SEC_DELAY);
	    vUiMessageDisplay((const int8_t *)uiCONFIG_SUC,
			    uiXCORDINATE_0 ,uiYCORDINATE_C);
	    vTaskDelay(uiEIGHT_SEC_DELAY);
	    resetSYSTEM_RESET;
	}
	else if(xBits & eventsKEYFOB_DETECT_TIMEOUT)
	{
	    vUiMessageDisplay((const int8_t *)uiASSOCIATION_FAIL,
			    uiXCORDINATE_0 ,uiYCORDINATE_C);
	    vTaskDelay(uiTHREE_SEC_DELAY);
	    /* retry again */
	    continue;
	}
	else if(xBits & eventsKEYFOB_NFC_ASSOCIATION_FAILURE)
	{
	    vUiMessageDisplay((const int8_t *)uiASSOCIATION_FAIL,
			    uiXCORDINATE_0 ,uiYCORDINATE_C);
	    vTaskDelay(uiTHREE_SEC_DELAY);
	    /* retry again */
	    continue;
	}
	else
	{
	    vUiMessageDisplay((const int8_t *)uiTIMEOUT_MSG,
			    uiXCORDINATE_0 ,uiYCORDINATE_C);
	    vTaskDelay(uiTHREE_SEC_DELAY);
	    resetSYSTEM_RESET;
	    /* association failed: Let retry association */
	    debugERROR_PRINT("association failed: Please retry...");
	}
    }while (1);
}
/*---------------------------------------------------------------------------*/

static void prvOobErrorState(eSuCOobStates *xpDevState,
		xSMUOobModeResources_t *pxResHandle)
{
    configASSERT(xpDevState != NULL);
    configASSERT(pxResHandle != NULL);

    /* ORWL Error state Never returns
     */
    debugERROR_PRINT("ORWL OOB ERROR state");

    vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
    vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR );
    vDisplayText((const int8_t *)"ORWL OOB ERROR: !!\n");
    while (1);
}
/*--------------------------------END of HELPER API's------------------------*/

static void (*vOOBModeStates[eSTATE_OOB_MAX])(eSuCOobStates *xpDevState,
		xSMUOobModeResources_t *pxResHandle) =
{
	prvOobCheckPinStat,
	prvOobVerifyDefaultPIN,
	prvOobSetUserPIN,
	prvOobVerifyUserPIN,
	prvOobUnlockedState,
	prvOobErrorState
};
/*---------------------------------------------------------------------------*/

static void prvOobCheckPinStat(eSuCOobStates *xpDevState,
		xSMUOobModeResources_t *pxResHandle)
{
    configASSERT( xpDevState != NULL );
    configASSERT( pxResHandle != NULL );
    configASSERT( pxResHandle -> xPinQueue != NULL );

    /* Display welcome screen on OLED */
    vInitDisplayWelcomeScreen();

    /* check if user PIN exists */
    if(ulOobIsUserPINSet())
    {
	/* User PIN not set */
	*xpDevState = eSTATE_VERIFY_DEFAULT_PIN;
    }
    else
    {
	/* valid user PIN exists */
	*xpDevState = eSTATE_VERIFY_USER_PIN;
    }
}
/*---------------------------------------------------------------------------*/

static void prvOobVerifyDefaultPIN(eSuCOobStates *xpDevState,
		xSMUOobModeResources_t *pxResHandle)
{
    /* error code to return */
    int32_t lStatus = NO_ERROR;
    /* Event to send to PIN entry task, for getting password */
    uint32_t ulEventToSend = pinentryGET_PASSWORD;
    /* PIN entry type default password */
    ePINTYPE ePIN = eDEFPINTYPE;
    TapEventData_t ucPinBuf;
    /* validate the arguments */
    configASSERT( xpDevState != NULL );
    configASSERT( pxResHandle != NULL );

    /* Before allowing user to enter the PIN check the retry count and take
     * corresponding actions.
     */
    lStatus = lPinhandlingCheckRetryDelay();
    if(lStatus != NO_ERROR)
    {
	/* should never occur */
	debugERROR_PRINT("Failed to check retry delay\n");
	/* update state to error mode, and return */
	*xpDevState = eSTATE_OOB_ERROR;
	return;
    }

    /* ask user to enter default shipping pin */
    vUiMessageDisplay((const int8_t *)uiPREDEF_PIN,
		uiXCORDINATE_0 ,uiYCORDINATE_C);
    vTaskDelay(uiTWO_SEC_DELAY);

    /* Now we have permission to get default PIN from user */
    lStatus = lPinhandlingAcceptPIN((uint8_t *)&ucPinBuf, pxResHandle,ulEventToSend);
    if(lStatus != NO_ERROR)
    {
	/* error should not occur, because timeout from PIN entry task should occur
	 * only for pinentryUSER_GET_PSWD in user mode.
	 */
	debugERROR_PRINT("\nFailed get a valid PIN from user \n");
	/* update state to error mode, and return */
	*xpDevState = eSTATE_OOB_ERROR;
	return;
    }
    /* print on console for debugging */
    debugPRINT("Received Default PIN");
    /* verify default pin */
    lStatus = lPinhandlingVerifyPIN((uint8_t *)&ucPinBuf, ePIN);
    if(lStatus != NO_ERROR)
    {
	/* Default PIN verification failed, increment retry count */
	lStatus = lPinhandlingIncrementRetryCount();
	if(lStatus != NO_ERROR)
	{
	    /* should never occur */
	    debugERROR_PRINT("Failed to increment retry count and delay\n");
	    /* update state to error mode, and return */
	    *xpDevState = eSTATE_OOB_ERROR;
	    return;
	}
	debugERROR_PRINT("Predefined PIN match failed ");
	/* PIN entered by user dint match with the default PIN display the same
	 * to user on OLED. Continue in the same state.
	 */
	vUiSetCoordAndDisplay(uiXSTAR_COORD + 3, uiYSTAR_COORD,
		(const int8_t *) uiCLEAR_STAR);
	vUiMessageDisplay((const int8_t *) uiPREDEF_PIN_FAIL, uiXCORDINATE_0,
		uiYCORDINATE_C);
	vTaskDelay(uiTWO_SEC_DELAY);
	return;
    }
    else
    {
	/* PIN verified, reset the password retry count and RTC wait delay */
	lStatus = lNvsramResetPswdRetryCountDelay();
	if(lStatus != NO_ERROR)
	{
	    /* should never occur */
	    debugERROR_PRINT("Failed to reset retry count and delay\n");
	    /* update state to error mode, and return */
	    *xpDevState = eSTATE_OOB_ERROR;
	    return;
	}
	/* PIN entered by user verified with default PIN, ready to move to next level */
	vUiMessageDisplay((const int8_t *)uiPREDEF_PIN_MATCH,
		uiXCORDINATE_0,uiYCORDINATE_C);
	vTaskDelay(uiTWO_SEC_DELAY);
	*xpDevState = eSTATE_SET_USER_PIN;
	return;
    }
}
/*---------------------------------------------------------------------------*/

static void prvOobVerifyUserPIN(eSuCOobStates *xpDevState,
		xSMUOobModeResources_t *pxResHandle)
{
    /* error code to return */
    int32_t lStatus = NO_ERROR;
    /* Event to send to PIN entry task, for getting password */
    uint32_t ulEventToSend = pinentryGET_PASSWORD;
    /* PIN entry type user password */
    ePINTYPE ePIN = eUSERPIN;
    TapEventData_t ucPinBuf;
    /* validate the arguments */
    configASSERT( xpDevState != NULL );
    configASSERT( pxResHandle != NULL );

    /* Before allowing user to enter the PIN check the retry count and take
     * corresponding actions.
     */
    lStatus = lPinhandlingCheckRetryDelay();
    if(lStatus != NO_ERROR)
    {
	/* should never occur */
	debugERROR_PRINT("Failed to check retry delay\n");
	/* update state to error mode, and return */
	*xpDevState = eSTATE_OOB_ERROR;
	return;
    }

    /* Display on OLED to enter user PIN */
    vUiMessageDisplay((const int8_t *)uiUSER_SET_PIN,
		uiXCORDINATE_0 ,uiYCORDINATE_C);
    vTaskDelay(uiTWO_SEC_DELAY);

    /* Now we have permission to get default PIN from user */
    lStatus = lPinhandlingAcceptPIN((uint8_t *)&ucPinBuf, pxResHandle,ulEventToSend);
    if(lStatus != NO_ERROR)
    {
	/* error should not occur, because timeout from PIN entry task should occur
	 * only for pinentryUSER_GET_PSWD in user mode.
	 */
	debugERROR_PRINT("\nFailed get a valid PIN from user \n");
	/* update state to error mode, and return */
	*xpDevState = eSTATE_OOB_ERROR;
	return;
    }
    /* print on console for debugging */
    debugPRINT("Received user PIN");
    /* verify user pin */
    lStatus = lPinhandlingVerifyPIN((uint8_t *)&ucPinBuf, ePIN);
    if(lStatus != NO_ERROR)
    {
	/* user PIN verification failed, increment retry count */
	lStatus = lPinhandlingIncrementRetryCount();
	if(lStatus != NO_ERROR)
	{
	    /* should never occur */
	    debugERROR_PRINT("Failed to increment retry count and delay\n");
	    /* update state to error mode, and return */
	    *xpDevState = eSTATE_OOB_ERROR;
	    return;
	}
	debugERROR_PRINT("user PIN match failed ");
	/* PIN entered by user dint match with the default PIN display the same
	 * to user on OLED. Continue in the same state.
	 */
	vUiSetCoordAndDisplay(uiXSTAR_COORD + 3, uiYSTAR_COORD,
		(const int8_t *) uiCLEAR_STAR);
	vUiMessageDisplay((const int8_t *) uiLOGIN_FAIL, uiXCORDINATE_0,
		uiYCORDINATE_C);
	vTaskDelay(uiTWO_SEC_DELAY);
	return;
    }
    else
    {
	/* PIN verified, reset the password retry count and RTC wait delay */
	lStatus = lNvsramResetPswdRetryCountDelay();
	if(lStatus != NO_ERROR)
	{
	    /* should never occur */
	    debugERROR_PRINT("Failed to reset retry counf and delay\n");
	    /* update state to error mode, and return */
	    *xpDevState = eSTATE_OOB_ERROR;
	    return;
	}
	/* PIN entered by user verified with user PIN, ready to move to next level */
	vUiMessageDisplay((const int8_t *)uiLOGIN_SUCCESS,
		uiXCORDINATE_0,uiYCORDINATE_C);
	vTaskDelay(uiTWO_SEC_DELAY);
	*xpDevState = eSTATE_OOB_UNLOCKED;
	return;
    }
}
/*---------------------------------------------------------------------------*/

static void prvOobSetUserPIN(eSuCOobStates *xpDevState,
		xSMUOobModeResources_t *pxResHandle)
{
    /* error code to return */
    int32_t lStatus = NO_ERROR;
    /* Event to send to PIN entry task, for getting password */
    uint32_t ulEventToSend = pinentryGET_PASSWORD;
    TapEventData_t xNewPin;
    TapEventData_t xConfirmPin;
    /* These are temporary buffer to compare if the user pin is '0' or '0xff'
     * because these user pins are not supported.
     */
    uint8_t ucCheckForZero[keysPIN_LEN];
    uint8_t ucCheckForFF[keysPIN_LEN];

    /* memset the temporary buffers to '0' and '0xff' to compare */
    memset(ucCheckForZero, 0, keysPIN_LEN);
    memset(ucCheckForFF, -1,keysPIN_LEN);

    /* validate the arguments */
    configASSERT( xpDevState != NULL );
    configASSERT( pxResHandle != NULL );

    /* Ask user to enter user custom pin */
    vUiMessageDisplay((const int8_t *)uiCUSTOM_PIN_MSG,
	    uiXCORDINATE_0,uiYCORDINATE_C);
    vTaskDelay(uiTHREE_SEC_DELAY);

    /* Get the New PIN from user */
    lStatus = lPinhandlingAcceptPIN((uint8_t *)&xNewPin, pxResHandle,ulEventToSend);
    if(lStatus != NO_ERROR)
    {
	/* error should not occur, because timeout from PIN entry task should occur
	 * only for pinentryUSER_GET_PSWD in user mode.
	 */
	debugERROR_PRINT("\nFailed get a valid PIN from user \n");
	/* update state to error mode, and return */
	*xpDevState = eSTATE_OOB_ERROR;
	return;
    }

    /* Ask user to re-enter the PIN to validate */
    vUiMessageDisplay((const int8_t *)uiREENTER_PIN,
	    uiXCORDINATE_0,uiYCORDINATE_C);
    vTaskDelay(uiTWO_SEC_DELAY);

    /* Get the confirm PIN from user to check if it was same as new PIN */
    lStatus = lPinhandlingAcceptPIN((uint8_t *)&xConfirmPin, pxResHandle,ulEventToSend);
    if(lStatus != NO_ERROR)
    {
	/* error should not occur, because timeout from PIN entry task should occur
	 * only for pinentryUSER_GET_PSWD in user mode.
	 */
	debugERROR_PRINT("\nFailed get a valid PIN from user \n");
	/* update state to error mode, and return */
	*xpDevState = eSTATE_OOB_ERROR;
	return;
    }

    /* Now we have New pin and confirm pin check if the new PIN is valid or not */
    if (!memcmp(xNewPin.ucPin, ucCheckForZero, pinentryPIN_LENGTH) ||
	    !memcmp(xNewPin.ucPin, ucCheckForFF, pinentryPIN_LENGTH))
    {
	/* Inform user on OLED this password is not valid and ask to enter valid
	 * password.
	 */
	vUiMessageDisplay((const int8_t *)uiINVALID_PSWD,
		uiXCORDINATE_0,uiYCORDINATE_C);
	vTaskDelay(uiTHREE_SEC_DELAY);
	debugERROR_PRINT("PIN not valid Please retry\n");
	return;
    }

    /* check if the new PIN is same as confirm PIN */
    if (!memcmp(xNewPin.ucPin, xConfirmPin.ucPin, pinentryPIN_LENGTH ))
    {
	debugPRINT_APP(" User PIN valid & matched now !!!!!!");
	/* New pin and confirm pin are valid, Store the PIN on flash */
	lStatus = lKeysUpdateUserPIN(xNewPin.ucPin);
	if(lStatus != NO_ERROR)
	{
	    /* storing pin on flash should never fail, update state and return */
	    debugERROR_PRINT("Failed to store custom pin");
	    *xpDevState = eSTATE_OOB_ERROR;
	    return;
	}
	/* ready to go to next level */
	*xpDevState = eSTATE_OOB_UNLOCKED;
	/* update on OLED custom pin is set */
	vUiMessageDisplay((const int8_t *)uiUSER_PIN_MATCH,
		uiXCORDINATE_0,uiYCORDINATE_C);
	vTaskDelay(uiTWO_SEC_DELAY);
	return;
    }
    /* if new and confirm pin are not same ask user to retry */
    else
    {
	vUiMessageDisplay((const int8_t *)uiUSER_INCORRECT_PIN,
		uiXCORDINATE_0,uiYCORDINATE_C);
	vTaskDelay(uiTWO_SEC_DELAY);
	return;
    }
}
/*---------------------------------------------------------------------------*/

void vOobModeTask(void *pvArg)
{
    xSMUOobModeResources_t *pxResHandle;
    volatile eSuCOobStates xDevState;

    /*
     * pvArgs must provide an event handler information for the thread created.
     */
    if (pvArg == NULL)
    {
	debugERROR_PRINT("\nFailed to create user mode task, invalid pvArg \n");
	return;
    }
    pxResHandle = (xSMUOobModeResources_t *) pvArg;

    /* set default state */
    xDevState = eSTATE_CHECK_PIN_STAT;

    do
     {
	if( xDevState < eSTATE_OOB_MAX) {
	    vOOBModeStates[xDevState]( &xDevState , pxResHandle );
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
