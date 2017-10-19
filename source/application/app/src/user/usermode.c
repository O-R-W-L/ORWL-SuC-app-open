/**===========================================================================
 * @file usermode.c
 *
 * @brief This file contains implementation of ORWL user mode.
 * It defines various states of ORWL and implements the complete use case
 * and state machine.
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

/* RTOS includes */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

/* local include */
#include <usermode.h>
#include <events.h>
#include <systemRes.h>
#include <oob.h>
#include <rot.h>
#include <pinentry.h>
#include <rtc.h>
#include <oled_ui.h>
#include <pinhandling.h>

/* gpio def include */
#include <orwl_gpio.h>

/* Static Le data used for updating the seed every 15 mins */
static usermodeLeData_t xLeData;

/**
 * @brief This function send Keyfob Association event to nfc thread and wait for
 * association.
 *
 * @param *pxResHandle user mode resource handle
 *
 * @return SUC_WRITE_STATUS_SUCCESS on success
 *         errors on Failure
 */
static uint8_t prvAddKeyFob( xSMAppResources_t *pxResHandle );

/**
 * @brief This function send Keyfob delete entry event to nfc thread and wait
 * for the response events.
 *
 * @param *pxResHandle user mode resource handle
 *
 * @return SUC_WRITE_STATUS_SUCCESS on success
 *         errors on Failure
 */
static uint8_t prvDeleteNFCKeyEntry( xSMAppResources_t *pxResHandle );

/**
 * @brief This function check for nfc authentication, ble proximity and pin
 * entry check.
 *
 * @param *pxResHandle user mode resource handle
 * @param ucCheckPinEntry set to 1 for pin entry check.
 * @param ucCheckAdmin	set for checking admin privilege
 *
 * @return SUC_WRITE_STATUS_SUCCESS on success
 *         errors on Failure
 */
static uint8_t prvCheckNfcBlePin( xSMAppResources_t *pxResHandle ,
				uint8_t ucCheckPinEntry, uint8_t ucCheckAdmin );

/**
 * @brief This function check for short power button press.
 *
 * @param *pxResHandle user mode resource handle
 *
 * @return pdFALSE on short button press otherwise pdTRUE.
 */
static BaseType_t prvCheckShortBtnPress( xSMAppResources_t *pxResHandle);

/**
 * @brief This function check for Long power button press. If Long press event
 * occurs then send long press event to intel and reset the SuC.
 *
 * @param *pxResHandle user mode resource handle
 *
 * @return void
 */
static void prvCheckLongBtnPress( xSMAppResources_t *pxResHandle);

/**
 * @brief This function set new AES keys, erase NVSRAM and re generate new SSD
 * password.
 *
 * @param None
 *
 * @return SUC_WRITE_STATUS_SUCCESS on success
 *         SUC_WRITE_STATUS_FAIL_MEM on Failure
 */
static uint8_t prvUserRotReGen( void );

/**
 * @brief This function handle the Intel device state change command.
 *
 * @param pxResHandle user mode resource handle
 * @param pxIntelUserData Buffer for data transfer between Intel SuC task and
 *                        user app.
 *
 * @return Intel error code.
 */
static uint8_t prvHandleIntelDevState( xSMAppResources_t *pxResHandle,
	IntelUserData_t *pxIntelUserData );

/**
 * @brief This function handle the Intel device state change command in proximity
 * lock state.
 *
 * @param pxResHandle user mode resource handle.
 * @param ucWaitSec seconds to wait to receive Intel device state.
 *
 * @return Intel error code.
 */
static uint8_t prvHandleIntelDevStateProxi(xSMAppResources_t *pxResHandle, uint8_t ucWaitSec);

/**
 * @brief This function reads the KeyFOb data and gives to BIOS.
 *
 * @param pxIntelUserData Buffer for data transfer between Intel SuC task and
 *                        user app.
 *
 * @return Intel error code.
 */
static uint8_t prvReadKeyFobData(IntelUserData_t *pxIntelUserData);

/**
 * @brief This function get RTC date and time from RTC hardware.
 *
 * @param pxIntelUserData Buffer for data transfer between Intel SuC task and
 *                        user app.
 *
 * @return Intel error code.
 */
static uint8_t prvGetRTCDateTime(IntelUserData_t *pxIntelUserData);

/**
 * @brief This function set RTC date and time to RTC hardware.
 *
 * @param pxIntelUserData Buffer for data transfer between Intel SuC task and
 *                        user app.
 *
 * @return Intel error code.
 */
static uint8_t prvSetRTCDateTime(IntelUserData_t *pxIntelUserData);

/**
 * @brief This function add the seed with increment factor, generate new
 * broadcast data and write the broadcast data to ble.
 *
 * @param None
 *
 * @return NO_ERROR on success and usermodeERROR_STATE on error
 */
static uint8_t prvUpdateLeSeed( void );

/**
 * @brief This function send event to the nfc thread to stop its current
 * execution.
 *
 * @param pxResHandle user mode resource handle
 *
 * @return NO_ERROR on success and usermodeERROR_STATE on error
 */
static int32_t prvStopNfcExecution( xSMAppResources_t *pxResHandle );

/**
 * @brief This function implements SuC state-1.
 *
 * This is the first state of SuC in user boot mode.
 *
 * @param *pxDevState device state details
 * @param *pxResHandle user mode resource handle
 *
 * @return void
 */
static void prvSuCSM_LockedState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle );

/**
 * @brief This function implements SuC state-2.
 *
 * This function waits for short power button press. On getting short button
 * press event, it will send an event to NFC event handler to poll for KeyFob &
 * authenticate the Keyfob. It blocks on few events and then takes appropriate
 * action based on event type.
 * --> NFC authenticated event
 * --> NFC authentication failed event
 * --> short key press event
 *
 * @param *pxDevState device state details
 * @param *pxResHandle user mode resource handle
 *
 * @return void
 */
static void prvSuCSM_NfcAuthState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle );

/**
 * @brief This function implements SuC state-4
 *
 * This function will be called only when NFC authentication or communication
 * fails.
 *
 * @param *pxDevState device state details
 * @param *pxResHandle user mode resource handle
 *
 * @return void
 */
static void prvSuCSM_AccessDenialState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle );

/**
 * @brief This function implements SuC state-3
 *
 * It looks for keyfob LE in proximity.
 *
 * @param *pxDevState device state details
 * @param *pxResHandle user mode resource handle
 *
 * @return void
 */
static void prvSuCSM_LeAliveCheckState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle );

/**
 * @brief This function implements SuC state-5
 *
 * It powers the Intel CPU
 *
 * @param *pxDevState device state details
 * @param *pxResHandle user mode resource handle
 *
 * @return void
 */
static void prvSuCSM_IntelCPUPowerON( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle );

/**
 * @brief This function implements SuC state-6
 *
 * Execution comes to this state when there is proximity failure.
 *
 * @param *pxDevState device state details
 * @param *pxResHandle user mode resource handle
 *
 * @return void
 */
static void prvSuCSM_ProximityError( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle );

/**
 * @brief This function implements SuC state-7
 *
 * It constantly monitors the KeyFob proximity. Additionally, it checks for
 * NFC association and deletion, 2 timeouts, power button press events, shutdown
 * pin status and the intel SuC commands.
 *
 * @param *pxDevState device state details
 * @param *pxResHandle user mode resource handle
 *
 * @return void
 */
static void prvSuCSM_UnlockedState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle );

/**
 * @brief This function implements SuC state-8
 *
 * It check for short power button press and motion. If power button is pressed
 * it will go for nfc authentication.
 *
 * @param *pxDevState device state details
 * @param *pxResHandle user mode resource handle
 *
 * @return void
 */
static void prvSuCSM_ProximityLock( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle );

/**
 * @brief This function implements SuC state-9
 *
 * It monitors the device motion.
 *
 * @param *pxDevState device state details
 * @param *pxResHandle user mode resource handle
 *
 * @return void
 */
static void prvSuCSM_DeviceMotion( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle );

/**
 * @brief This function implements SuC state-10
 *
 * It monitors the KeyFob LE alive status.
 *
 * @param *pxDevState device state details
 * @param *pxResHandle user mode resource handle
 *
 * @return void
 */
static void prvSuCSM_ReLeAliveCheckState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle );

/**
 * @brief This function implements SuC state-11
 *
 * If NFC authentication fails in state 8, this state takes control back to
 * state-8 for re authentication.
 *
 * @param *pxDevState device state details
 * @param *pxResHandle user mode resource handle
 *
 * @return void
 */
static void prvSuCSM_ReNfcAuthState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle );

/**
 * @brief This is the usermode error state.
 *
 * @param *pxDevState  device state
 * @param *pxResHandle application resource handle
 *
 * @return void
 */
static void prvSuCSM_ErrorState( eSuCRotStates *pxDevState,
	xSMAppResources_t *pxResHandle );

/*----------------------------------------------------------------------------*/

static uint8_t prvAddKeyFob( xSMAppResources_t *pxResHandle )
{
	uint8_t ucRet;
	xSMAppResources_t *pxRes = pxResHandle;
	EventBits_t xBits = 0;
	uint32_t ulEventToSend = 0;
	uint32_t ulEventToWait = 0;
	configASSERT(pxResHandle != NULL);
	/* Update the oled display to present keyfob */
	vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
	vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispADD_MSG_STR_YCOR);
	vDisplayText((const int8_t *)uiPRESENT_KEY);
	/* Block on event handler:
	 * NFC association failed
	 * NFC association successful
	 * NFC card detection timeout
	 * Admin KeyFob
	 */
	ulEventToWait = eventsKEYFOB_NFC_ASSOCIATION_SUCCESS |
			eventsKEYFOB_NFC_ASSOCIATION_FAILURE |
			eventsKEYFOB_DETECT_TIMEOUT |
			eventsKEYFOB_NOT_ADMIN;
	/* Clearing the events before sending associate events */
	xBits = xEventGroupClearBits( pxRes->xEventGroupUserModeApp ,
			ulEventToWait );
	/* Post associate KeyFob event to nfc task */
	do
	{
		ulEventToSend = eventsNFC_ASSOCIATE_KEYFOB;
		xBits = xEventGroupSetBits( pxRes->xEventGroupNFC ,
				ulEventToSend );
		if( (xBits & eventsNFC_ASSOCIATE_KEYFOB)
			!= eventsNFC_ASSOCIATE_KEYFOB )
		{
			debugERROR_PRINT("post event: "
				"eventsNFC_ASSOCIATE_KEYFOB Failed, retry... ");
			vTaskDelay( eventsEVENT_SET_DELAY );
		}
		else
		{
			debugPRINT_APP("post event:"
				" eventsNFC_ASSOCIATE_KEYFOB Done ");
			break;
		}
	} while ( 0 );

	/* Block here for the event from nfc task */
	xBits = xEventGroupWaitBits(
				pxRes->xEventGroupUserModeApp ,
				ulEventToWait,
				pdFALSE ,
				pdFALSE ,
				portMAX_DELAY );
	xBits = xEventGroupClearBits( pxRes->xEventGroupUserModeApp ,
		ulEventToWait );
	/* To make debug print properly, delay is required */
	vTaskDelay(usermodeDELAY_MSEC_100);

	if( (xBits & eventsKEYFOB_NFC_ASSOCIATION_SUCCESS)
		== eventsKEYFOB_NFC_ASSOCIATION_SUCCESS )
	{
		if( lDisplayImageFullScreen(eDipsInt_KEY_ASSOCIATED))
		{
			debugERROR_PRINT(" Failed to display image Nfc Key"
					" associated on OLED ");
			return usermodeERROR_STATE;
		}
		debugPRINT_APP("Receive event:"
			" eventsKEYFOB_NFC_ASSOCIATION_SUCCESS ");
		vTaskDelay(usermodeMSG_DISPLAY_TIME);
		ucRet = SUC_WRITE_STATUS_SUCCESS;
	}
	else if( (xBits & eventsKEYFOB_NFC_ASSOCIATION_FAILURE)
		== eventsKEYFOB_NFC_ASSOCIATION_FAILURE )
	{
		vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
		vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR);
		vDisplayText((const int8_t *)uiNFC_NOT_ASSOCIATED);
		debugERROR_PRINT("Receive event:"
			" eventsKEYFOB_NFC_ASSOCIATION_FAILURE ");
		vTaskDelay(usermodeMSG_DISPLAY_TIME);
		ucRet = SUC_WRITE_STATUS_FAIL_UNKOWN;
	}
	else if( (xBits & eventsKEYFOB_NOT_ADMIN)
			== eventsKEYFOB_NOT_ADMIN )
	{
		vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
		vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_IMGLTY);
		vDisplayText((const int8_t *)uiUSE_ADMIN_KEYFOB);
		debugERROR_PRINT("Receive event:"
			" eventsKEYFOB_NOT_ADMIN ");
		vTaskDelay(usermodeMSG_DISPLAY_TIME);
		ucRet = SUC_WRITE_STATUS_FAIL_AUTH;
	}
	else
	{
		debugERROR_PRINT(" Receive event: eventsKEYFOB_DETECT_TIMEOUT "
			"in new KeyFob association");
		ucRet = SUC_WRITE_STATUS_TIMEOUT;
	}
	if( ucRet != SUC_WRITE_STATUS_SUCCESS )
	{
		/* Reseting the Queue in case of association errors */
		if(xQueueReset(pxRes->xNfcQueue) != pdTRUE)
		{
			debugERROR_PRINT(" Failed to reset queue ");
		}
	}
	return ucRet;
}
/*----------------------------------------------------------------------------*/

static uint8_t prvDeleteNFCKeyEntry( xSMAppResources_t *pxResHandle )
{
	xSMAppResources_t *pxRes = pxResHandle;
	EventBits_t xBits = 0;
	uint8_t ucRet;
	configASSERT(pxResHandle != NULL);

	/* Clearing the events before sending associate events */
	xBits = xEventGroupClearBits( pxRes->xEventGroupUserModeApp ,
			eventsKEYFOB_ENTRY_DELETE_SUCCESS |
			eventsKEYFOB_ENTRY_DELETE_FAILURE |
			eventsKEYFOB_NOT_ADMIN );

	do
	{
		xBits = xEventGroupSetBits( pxRes->xEventGroupNFC ,
			eventsNFC_DELETE_KEYFOB_ENTRY );

		if((xBits & eventsNFC_DELETE_KEYFOB_ENTRY)
			!= eventsNFC_DELETE_KEYFOB_ENTRY)
		{
			debugERROR_PRINT("post event: "
				"eventsNFC_DELETE_KEYFOB_ENTRY Failed, retry...");
			vTaskDelay( eventsEVENT_SET_DELAY );
		}
		else
		{
			debugPRINT_APP("post event:"
				" eventsNFC_DELETE_KEYFOB_ENTRY Done");
			break;
		}
	} while ( 0 );
	xBits = xEventGroupWaitBits(
				pxRes->xEventGroupUserModeApp ,
				eventsKEYFOB_ENTRY_DELETE_SUCCESS |
				eventsKEYFOB_ENTRY_DELETE_FAILURE |
				eventsKEYFOB_NOT_ADMIN,
				pdFALSE ,
				pdFALSE ,
				portMAX_DELAY );
	xBits = xEventGroupClearBits( pxRes->xEventGroupUserModeApp ,
				eventsKEYFOB_ENTRY_DELETE_SUCCESS |
				eventsKEYFOB_ENTRY_DELETE_FAILURE |
				eventsKEYFOB_NOT_ADMIN );

	vTaskDelay(usermodeDELAY_MSEC_100);
	/* change state based on event type */
	if( (xBits & eventsKEYFOB_ENTRY_DELETE_SUCCESS)
		== eventsKEYFOB_ENTRY_DELETE_SUCCESS )
	{
		vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
		vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR);
		vDisplayText((const int8_t *)uiKEYFOB_DELETE_SUCCESS);
		debugPRINT_APP("Receive event: "
			"eventsKEYFOB_ENTRY_DELETE_SUCCESS ");
		vTaskDelay( usermodeMSG_DISPLAY_TIME );
		ucRet = SUC_WRITE_STATUS_SUCCESS;
	}
	else if( (xBits & eventsKEYFOB_ENTRY_DELETE_FAILURE)
		== eventsKEYFOB_ENTRY_DELETE_FAILURE )
	{
		vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
		vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR);
		vDisplayText((const int8_t *)uiKEYFOB_DELETE_FAILURE);
		debugERROR_PRINT("Receive event: "
			"eventsKEYFOB_ENTRY_DELETE_FAILURE ");
		vTaskDelay( usermodeMSG_DISPLAY_TIME );
		ucRet = SUC_WRITE_STATUS_FAIL_UNKOWN;
	}
	else if( (xBits & eventsKEYFOB_NOT_ADMIN)
			== eventsKEYFOB_NOT_ADMIN )
	{
		vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
		vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_IMGLTY);
		vDisplayText((const int8_t *)uiUSE_ADMIN_KEYFOB);
		debugERROR_PRINT("Receive event:"
			" eventsKEYFOB_NOT_ADMIN ");
		vTaskDelay(usermodeMSG_DISPLAY_TIME);
		ucRet = SUC_WRITE_STATUS_FAIL_AUTH;
	}
	else
	{
		debugERROR_PRINT("Keyfob entry delete, Timeout occurred \n");
		return usermodeERROR_STATE;
	}
	if( ucRet != SUC_WRITE_STATUS_SUCCESS )
	{
		/* Reset Queue in case of delete error */
		if(xQueueReset(pxRes->xNfcQueue) != pdTRUE)
		{
			debugERROR_PRINT("Failed to reset the queue \n");
		}
	}
	return ucRet;
}
/*----------------------------------------------------------------------------*/

static uint8_t prvCheckNfcBlePin( xSMAppResources_t *pxResHandle,
				uint8_t ucCheckPinEntry, uint8_t ucCheckAdmin)
{
	xSMAppResources_t *pxRes = pxResHandle;
	uint8_t ucRet;
	EventBits_t xBits = 0;
	uint32_t ulLeStatusCount = 0;
	uint8_t ucPinStatus = -1;
	uint8_t ucBLeBroadCastData[leMAX_BUF_LENGTH]={0};
	NfcEventData_t xNfcData;
	TapEventData_t xPinData;
	uint32_t ulPinentryRet = 0;
	/* PIN entry type user password */
	ePINTYPE ePIN = eUSERPIN;

	configASSERT(pxResHandle != NULL);

	/* Update OLED dislay to present keyfob */
	if(lDisplayImageFullScreen(eDipsInt_NFC_AUTH_REQUEST) != NO_ERROR)
	{
		debugERROR_PRINT("Failed to display present key image on OLED");
		return usermodeERROR_STATE;
	}
	/* Clearing the events before posting detect keyFob */
	xBits = xEventGroupClearBits( pxRes->xEventGroupUserModeApp ,
			eventsKEYFOB_NFC_AUTH_SUCCESS |
			eventsKEYFOB_NFC_AUTH_FAILURE |
			eventsKEYFOB_DETECT_TIMEOUT |
			eventsKEYFOB_NOT_ADMIN);
	/* Post detect KeyFob event */
	do
	{
		xBits = xEventGroupSetBits( pxRes->xEventGroupNFC ,
			eventsNFC_DETECT_KEYFOB );
		if((xBits & eventsNFC_DETECT_KEYFOB) != eventsNFC_DETECT_KEYFOB)
		{
			debugERROR_PRINT("post event: "
				"eventsNFC_DETECT_KEYFOB Failed, retry... ");
			vTaskDelay( eventsEVENT_SET_DELAY );
		}
		else
		{
			debugPRINT_APP("post event: "
				"eventsNFC_DETECT_KEYFOB Done");
			break;
		}
	} while ( 0 );

	xBits = xEventGroupWaitBits(
				pxRes->xEventGroupUserModeApp ,
				eventsKEYFOB_NFC_AUTH_SUCCESS |
				eventsKEYFOB_NFC_AUTH_FAILURE |
				eventsKEYFOB_DETECT_TIMEOUT,
				pdFALSE ,
				pdFALSE ,
				portMAX_DELAY );
	xBits = xEventGroupClearBits( pxRes->xEventGroupUserModeApp ,
		eventsKEYFOB_NFC_AUTH_SUCCESS |
		eventsKEYFOB_NFC_AUTH_FAILURE |
		eventsKEYFOB_DETECT_TIMEOUT |
		eventsKEYFOB_NOT_ADMIN);
	/* Delay is required to sync the console prints */
	vTaskDelay(usermodeDELAY_MSEC_100);
	/* change state based on event type */
	if( (xBits & eventsKEYFOB_NFC_AUTH_SUCCESS)
		== eventsKEYFOB_NFC_AUTH_SUCCESS )
	{
		ucRet = SUC_WRITE_STATUS_SUCCESS;
		debugPRINT_APP("Receive event: events KEYFOB_NFC_AUTH_SUCESS ");
	}
	else if( (xBits & eventsKEYFOB_NFC_AUTH_FAILURE)
		== eventsKEYFOB_NFC_AUTH_FAILURE )
	{
		debugERROR_PRINT("Receive event: eventsKEYFOB_NFC_AUTH_FAILURE");
		ucRet = SUC_WRITE_STATUS_FAIL_AUTH;
	}
	else
	{
		debugERROR_PRINT("Receive event: eventsKEYFOB_DETECT_TIMEOUT in"
			" nfc authentication check ");
		ucRet = SUC_WRITE_STATUS_TIMEOUT;
	}
	if( ucRet != SUC_WRITE_STATUS_SUCCESS )
	{
		return ucRet;
	}
	if( ucCheckAdmin == pdTRUE )
	{
		if(( xBits & eventsKEYFOB_NOT_ADMIN ) == eventsKEYFOB_NOT_ADMIN )
		{
		    vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
		    vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_IMGLTY);
		    vDisplayText((const int8_t *)uiUSE_ADMIN_KEYFOB);
		    debugERROR_PRINT("Receive event: eventsKEYFOB_NOT_ADMIN");
		    /* Reseting the Queue in this case */
		    if(xQueueReset(pxRes->xNfcQueue) != pdTRUE)
		    {
			debugERROR_PRINT(" Failed to reset queue ");
		    }
		    vTaskDelay(usermodeMSG_DISPLAY_TIME);
		    return SUC_WRITE_STATUS_FAIL_AUTH;
		}
	}
	debugPRINT_APP(" NFC Check success");
	/* Update Oled */
	vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
	vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispADD_MSG_STR_YCOR);
	vDisplayText((const int8_t *)uiNFC_AUTHENTICATED);
	vTaskDelay( usermodeMSG_DISPLAY_TIME );

	ucRet = SUC_WRITE_STATUS_FAIL_AUTH;
	do
	{
		ucRet = xQueueReceive(pxResHandle->xNfcQueue, &xNfcData,
					portMAX_DELAY);
		if(ucRet == pdFALSE)
		{
			debugERROR_PRINT(" Failed to receive data from queue ");
			break;
		}
		ucRet = SUC_WRITE_STATUS_FAIL_AUTH;
		/* Generate BLE Broadcast data */
		ucBLeBroadCastData[usermodeINDEX_0] = leDATA_LENGTH;
		ucBLeBroadCastData[usermodeINDEX_1] = leMANF_SPEC_DATA;
		if(xNfcData.ucBleSeed == NULL)
		{
			debugERROR_PRINT(" Invalid parameters received ");
			break;
		}
		/* Generate PRNG using Nfc Seed */
		if(lPrngReadSeed(xNfcData.ucBleSeed, nfccommonSEED_SIZE))
		{
			debugERROR_PRINT(" Failed to read seed for pseudo"
					" random number");
			break;
		}
		if(lPrngGenerateRandom(&ucBLeBroadCastData[usermodeINDEX_2],
			lePRND_LENGTH))
		{
			debugERROR_PRINT(" Failed to generate pseudo random"
					" number");
			break;
		}
#if usermodeENABLE_GPIO_READ
		/* Before writing the expected broadcast data to LE controller,
		 * Let us check if LE has reported the proximity lock for last
		 * session or not.
		 * Since we have changed the LE SEED on KeyFob during authentication,
		 * KeyFob will stop broadcasting and start 'new' data broadcast
		 * within specified time (30 sec as of now)
		 */

		/* Wait for proximity lock to happen */
		for(ulLeStatusCount = 0; ulLeStatusCount < usermodeLE_MAX_RETRY;
						    ulLeStatusCount++)
		{
			vTaskDelay(usermodeDELAY_MSEC_100);
			vSystemResBT_GPIO_Status(&ucPinStatus);
			if ( ucPinStatus == usermodeLE_GPIO_HIGH )
			{
			    /* proximity lock occurred :) */
				break;
			}
		}

		/* Check if we break due to max retry count or really proximity lock occurred !
		 * If we break due to max retry count, there must be something wrong on LE controller
		 * Note: Ideally this should never Happen on a stable ORWL.
		 * */
		if ( ucPinStatus != usermodeLE_GPIO_HIGH )
		{
		    /* This should never happen */
		    return SUC_WRITE_STATUS_FAIL_AUTH;
		}

		/* Now LE controller has indicated the SuC that proximity lock occurred
		 * Lets update LE controller with NEW SEED
		 * */
		if( lLeInterfaceLe_WriteData(ucBLeBroadCastData,
						leMAX_BUF_LENGTH))
		{
			debugERROR_PRINT(" Failed to write data to ble eeprom");
			break;
		}

		/* Provided delay in loop as BLE central toggle GPIO after few seconds */
		for(ulLeStatusCount = 0; ulLeStatusCount < usermodeLE_MAX_RETRY;
						    ulLeStatusCount++)
		{
			vTaskDelay(usermodeDELAY_MSEC_100);
			vSystemResBT_GPIO_Status(&ucPinStatus);
			if ( ucPinStatus == usermodeLE_GPIO_LOW  )
			{
				ucRet = SUC_WRITE_STATUS_SUCCESS;
				break;
			}
		}
		if(ulLeStatusCount == usermodeLE_MAX_RETRY)
		{
			debugERROR_PRINT(" Peer is out of proximity ");
			break;
		}
#else
		/* Check LE status using GPIO pin in EVT3 board */
		/* In EVT2 reading the eeprom value to get the GPIO status */
		for(ulLeStatusCount = 0; ulLeStatusCount < usermodeLE_MAX_RETRY;
			ulLeStatusCount++)
		{
			vTaskDelay(usermodeDELAY_MSEC_100);
			if( lI2cRead((uint8_t)leNORDIC_I2C_SLAVE_ADDR,
				(uint8_t)leGPIO_REG_ADDR,
				usermodeLE_NUM_OF_BYTES,
				ucLeGpioReg))
			{
				debugERROR_PRINT(" Failed to read data from LE "
					"EEPROM ");
				break;
			}
			else if(ucLeGpioReg[usermodeINDEX_0] ||
				ucLeGpioReg[usermodeINDEX_1] ||
				ucLeGpioReg[usermodeINDEX_2])
			{
				ucRet = SUC_WRITE_STATUS_SUCCESS;
				break;
			}
			else
			{
				continue;
			}
		}
		if(ulLeStatusCount == usermodeLE_MAX_RETRY)
		{
			debugERROR_PRINT(" Peer is out of proximity ");
			break;
		}
#endif
	}while(0);
	if( ucRet != SUC_WRITE_STATUS_SUCCESS )
	{
		return ucRet;
	}
	/* Updating the LE Seed structure */
	memcpy((void*)xLeData.ucSeed, (void*)xNfcData.ucBleSeed,
				nfccommonSEED_SIZE);
	memcpy((void*)xLeData.ucSeedIncrFactor, (void*)xNfcData.ucLeIncrFactor,
			nfccommonSEED_INC_SIZE);
	/* reset Le seed updating timer */
	if ( lTimerClose( timerLE_TIMER_ID ))
	{
		debugERROR_PRINT(" Failed to close Le timer ");
		return usermodeERROR_STATE;
	}
	if ( lTimerLeTmrInit() )
	{
		debugERROR_PRINT(" Failed to init Le timer ");
		return usermodeERROR_STATE;
	}
	debugPRINT_APP(" LE Check success");
	if(ucCheckPinEntry)
	{
	    /* Before allowing user to enter the PIN check the retry count and take
	     * corresponding actions.
	     */
	    ulPinentryRet = lPinhandlingCheckRetryDelay();
	    if(ulPinentryRet != NO_ERROR)
	    {
		/* should never occur, writing/reading from NVSRAM has failed */
		debugERROR_PRINT("Failed to check retry delay\n");
		/* update user error mode */
		return usermodeERROR_STATE;
	    }

	    /* Display on OLED to enter user PIN */
	    vUiMessageDisplay((const int8_t *)uiUSER_SET_PIN,
			uiXCORDINATE_0 ,uiYCORDINATE_C);
	    vTaskDelay(uiTWO_SEC_DELAY);

	    /* Now we have permission to get default PIN from user */
	    ulPinentryRet = lPinhandlingAcceptPIN((uint8_t *)&xPinData,
		(xSMUOobModeResources_t *) pxRes, pinentryUSER_GET_PSWD);
	    if(ulPinentryRet != NO_ERROR)
	    {
		/* timeout has occurred while entering PIN */
		debugERROR_PRINT("Timeout occurred while entering PIN \n");
		/* update state to error mode, and return */
		ucRet = SUC_WRITE_STATUS_TIMEOUT;
		/* Clearing the events which are not used in pin entry check*/
		xEventGroupClearBits(pxResHandle->xEventGroupUserModeApp,
		eventsKEY_PRESS_SHORT | eventsKEY_PRESS_LONG);
		/* return error code */
		return ucRet;
	    }

	    /* verify user pin */
	    ulPinentryRet = lPinhandlingVerifyPIN((uint8_t *)&xPinData, ePIN);
	    if( ulPinentryRet != NO_ERROR )
	    {
		/* user has entered wrong pin. Authentication failed */
		ucRet = SUC_WRITE_STATUS_FAIL_AUTH;
		/* user has entered wrong password, increment retry count */
		ulPinentryRet = lPinhandlingIncrementRetryCount();
		if(ulPinentryRet != NO_ERROR)
		{
		    /* should never occur */
		    debugERROR_PRINT("Failed to increment retry count\n");
		    /* update user error mode */
		    return usermodeERROR_STATE;
		}
	    }
	    else
	    {
		/* user has entered right pin. Authentication success */
		ucRet = SUC_WRITE_STATUS_SUCCESS;
		/* User has entered right password, reset the retry count */
		ulPinentryRet = lNvsramResetPswdRetryCountDelay();
		if(ulPinentryRet != NO_ERROR)
		{
		    /* should never occur */
		    debugERROR_PRINT("Failed to reset retry count and delay\n");
		    /* update user error mode */
		    return usermodeERROR_STATE;
		}
	    }

	    /* Clearing the events which are not used in pin entry check*/
	    xEventGroupClearBits(pxResHandle->xEventGroupUserModeApp,
	    eventsKEY_PRESS_SHORT | eventsKEY_PRESS_LONG);
	}
	debugPRINT_APP(" Pin entry Checked ");
	return ucRet;
}
/*----------------------------------------------------------------------------*/

static BaseType_t prvCheckShortBtnPress( xSMAppResources_t *pxResHandle)
{
	EventBits_t xBits = 0;
	configASSERT(pxResHandle != NULL);
	/* Wait for short press button */
	xBits = xEventGroupWaitBits(
				pxResHandle->xEventGroupUserModeApp ,
				eventsKEY_PRESS_SHORT,
				pdFALSE ,
				pdFALSE ,
				usermodeMIN_WAIT_DELAY );
	xBits = xEventGroupClearBits( pxResHandle->xEventGroupUserModeApp ,
		eventsKEY_PRESS_SHORT );
	/* To make debug print properly, delay is required */
	vTaskDelay(usermodeDELAY_MSEC_100);
	if((xBits & eventsKEY_PRESS_SHORT) == eventsKEY_PRESS_SHORT)
	{
		debugPRINT_APP(" Receive event: eventsKEY_PRESS_SHORT ");
		return pdFALSE;
	}
	else
	{
		/* No event occurred */
		return pdTRUE;
	}
}
/*----------------------------------------------------------------------------*/

static void prvCheckLongBtnPress( xSMAppResources_t *pxResHandle)
{
	EventBits_t xBits = 0;
	configASSERT(pxResHandle != NULL);
	/* Wait for Long press button */
	xBits = xEventGroupWaitBits(
				pxResHandle->xEventGroupUserModeApp ,
				eventsKEY_PRESS_LONG,
				pdFALSE ,
				pdFALSE ,
				usermodeMIN_WAIT_DELAY );
	xBits = xEventGroupClearBits( pxResHandle->xEventGroupUserModeApp ,
		eventsKEY_PRESS_LONG );
	/* To make debug print properly, delay is required */
	vTaskDelay(usermodeDELAY_MSEC_100);
	if((xBits & eventsKEY_PRESS_LONG) == eventsKEY_PRESS_LONG)
	{
		debugPRINT_APP(" Receive event: eventsKEY_PRESS_LONG ");
		if(lDisplayImageFullScreen(eDipsInt_FORCE_SHUTDOWN)!=NO_ERROR)
		{
			debugERROR_PRINT("Failed to display present key image"
						" on OLED");
		}
		/* Send Long press event to Intel */
		vSystemResIntel_LongPress();
		/* Reset the system for long button press */
		resetSYSTEM_RESET
	}
	else
	{
		/* No event occurred */
		return;
	}
}
/*----------------------------------------------------------------------------*/

static uint8_t prvUserRotReGen( void )
{
	NvsramData_t xNvsData;
	/* Set New AES keys */
	vNvsramAESKeySet();
	/* Erase NVSRAM */
	if( lEraseNvsramComplete() != NO_ERROR)
	{
		debugERROR_PRINT(" Failed to erase the SSD password\n");
		return SUC_WRITE_STATUS_FAIL_MEM;
	}
	/* Reset password retry count */
	xNvsData.usPswdRetryCount = 0;
	/* Generate new SSD password */
	if( ulGenerateRandomNumber(xNvsData.ulSSDEncKey,
	    nvsramSNVSRAM_ENC_KEY_LEN_MAX) != NO_ERROR )
	{
		debugERROR_PRINT(" Failed to generate the SSD password\n");
		return SUC_WRITE_STATUS_FAIL_UNKOWN;
	}
	/* Write new password and retry count to NVSRAM */
	if( lNvsramWriteData(&xNvsData) != NO_ERROR)
	{
		debugERROR_PRINT("Failed update NVSRAM with new SSD password");
		return SUC_WRITE_STATUS_FAIL_MEM;
	}
	return SUC_WRITE_STATUS_SUCCESS;
}
/*----------------------------------------------------------------------------*/

static uint8_t prvHandleIntelDevState(xSMAppResources_t *pxResHandle,
					IntelUserData_t *pxIntelUserData)
{
	/* 30 seconds Delay counter */
	uint8_t ucDelayCount = 0;
	/* Shutdown GPIO status */
	uint8_t ucGPIOVal ;
	/* Return error code to Intel */
	uint8_t ucACK = SUC_WRITE_STATUS_SUCCESS;

	/* Check if the Intel is working and all security attributes are
	 * satisfied*/
	if(pxIntelUserData->ucSubCommand == INTEL_DEV_STATE_S0)
	{
		/* In this case we dont't need to worry, we can send the Intel
		 * ACK saying write was successful.
		 */
		return SUC_WRITE_STATUS_SUCCESS;
	}
	/* Check if the Intel is about to enter standby mode. Do nothing, just
	 * send success status.
	 */
	else if(pxIntelUserData->ucSubCommand == INTEL_DEV_STATE_STD_BY)
	{
		/* return success code */
		return SUC_WRITE_STATUS_SUCCESS;
	}
	/* Check if Intel is about to enter hibernate/shutdown  mode, in this
	 * case probe the Intel shutdown and reset the ORWL.
	 */
	else if ((pxIntelUserData->ucSubCommand == INTEL_DEV_STATE_HIBER)
		|| (pxIntelUserData->ucSubCommand == INTEL_DEV_STATE_SHT_DWN))
	{
		/* We have received hibernate/shutdown from Intel ACK the same.
		 * We are writing ACK to the queue here only since
		 * resetSYSTEM_RESET will be done and this function might not
		 * return.
		 */
		if(xQueueSend(pxResHandle->xSucBiosReceiveQueue, &ucACK,
				portMAX_DELAY) != pdTRUE)
		{
			debugERROR_PRINT(" ERROR in Queue data send");
			return usermodeERROR_STATE;
		}
		/* Intel has send shutdown command, poll GPIO to check if Intel
		 * has shutdown.
		 */
		/* Poll for the shutdown status for 10 times every 3 second */
		do
		{
			/* wait for 3 seconds for system to shutdown*/
			vTaskDelay(SystemResINTEL_SHUTDOWN_WAIT_PERIOD) ;
			ucDelayCount++ ;
			ucGPIOVal = ulSystemResPowerState_Intel() ;
		}while ((ucDelayCount<SystemResDELAY_INC_1S_30S_COUNT) &&
			ucGPIOVal) ;

		/* Restart the ORWL device */
		resetSYSTEM_RESET;
	}
	/* Check if Intel will restart, in this case perform NFC authentication,
	 */
	else if(pxIntelUserData->ucSubCommand == INTEL_DEV_STATE_RST)
	{
		return prvCheckNfcBlePin( pxResHandle, pdFALSE, pdFALSE);
	}
	/* else Intel might have sent an unknown device state return correct
	 * error code */
	else
	{
		return SUC_WRITE_STATUS_FAIL_INVALID;
	}
	/* return error code */
	return SUC_WRITE_STATUS_SUCCESS;
}
/*----------------------------------------------------------------------------*/

static uint8_t prvHandleIntelDevStateProxi(xSMAppResources_t *pxResHandle, uint8_t ucWaitSec)
{
    /* Intel user data */
    IntelUserData_t xIntelUserData;
    /* ACK to Intel */
    uint8_t ucACK = SUC_WRITE_STATUS_SUCCESS;
    /* Max count in seconds to wait for INTEL_DEV_ACT */
    uint8_t ucCount = 0;

    /* Wait for INTEL_DEV_ACT for 10 seconds */
    while(ucCount < ucWaitSec)
    {
	/* Wait for queue event from Intel Task */
	if(xQueueReceive(pxResHandle->xSucBiosSendQueue, &xIntelUserData, usermodeDELAY_SEC_1) == pdTRUE)
        {
	    if(xIntelUserData.ucCommand == INTEL_DEV_ACT)
	    {
		xQueueSend(pxResHandle->xSucBiosReceiveQueue, &ucACK, portMAX_DELAY);
		return SUC_WRITE_STATUS_SUCCESS;
	    }
	    else
	    {
		/* Invalid command received */
		ucACK = SUC_WRITE_STATUS_FAIL_INVALID;
		xQueueSend(pxResHandle->xSucBiosReceiveQueue, &ucACK, portMAX_DELAY);
		return SUC_WRITE_STATUS_FAIL_INVALID;
	    }
	}
	/* increment the counter */
	ucCount++;
    }
    /* return error code */
    return ucACK;
}
/*----------------------------------------------------------------------------*/

static uint8_t prvReadKeyFobData(IntelUserData_t *pxIntelUserData)
{
    /* KeyFOB data to send to BIOS */
    OrwlKeyData_t xDataToSend;
    uint8_t ucIndex = 0,ucIndexUpdate = 0,ucIndexUsed = 0;
    int32_t lResult = NO_ERROR;
    /* Variable for reading keyfob Info */
    keyfobidKeyFobEntry_t *pxKeyfobInfo;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);
    memset(&xDataToSend,0,sizeof(xDataToSend));

    /* Get number of KeyFob associated */
    lResult = lKeyfobidCheckFreeIndex( &ucIndexUsed );
    if(lResult != NO_ERROR)
    {
	debugERROR_PRINT("Failed to read keyfob index entries..\n");
	return SUC_WRITE_STATUS_FAIL_UNKOWN;
    }

    /* Allocate memory to read KeyFOb entry structure */
    pxKeyfobInfo = pvPortMalloc(sizeof(keyfobidKeyFobEntry_t));
    if(pxKeyfobInfo == NULL)
    {
	debugERROR_PRINT("Failed to allocate memory.\n");
	return SUC_WRITE_STATUS_FAIL_MEM;
    }

    /* Reading of keyfob entry structure */
    lResult = lKeyfobidReadKeyFobEntry(pxKeyfobInfo);
    if(lResult != NO_ERROR)
    {
	debugERROR_PRINT("Failed to read keyfob data..\n");
	if(pxKeyfobInfo)
	{
	    vPortFree(pxKeyfobInfo);
	}
	return SUC_WRITE_STATUS_FAIL_UNKOWN;
    }

    /* Initialize the update index for having sequential data */
    ucIndexUpdate = 0;
    /* We should start reading from Index 1 because we are not sending the
     * admin keyfob details to BIOS. Index zero has admin Keyfob details.
     */
    /* TODO : As of now we are sending admin keyfob also because of BIOS limitation
     * this should be assumed at BIOS also.
     */
    for(ucIndex = 0; ucIndex < ORWL_KEY_FOB_MAX_COUNT; ucIndex++)
    {
	if(pxKeyfobInfo->xKeyFobInfo[ucIndex].ulKeyMagic ==
		keyfobidKEYFOB_INFO_MAGIC)
	    {
		memcpy(xDataToSend.keyFob[ucIndexUpdate].keyFobNam,
			pxKeyfobInfo->xKeyFobInfo
			[ucIndex].ucKeyName,intelMAX_KEYFOB_NAME_LEN);
		xDataToSend.keyFob[ucIndexUpdate].preAttr = pxKeyfobInfo->xKeyFobInfo
			[ucIndex].ulKeyPrevilage;

		/*Increment the linear update index to have sequential KeyFOB names*/
		ucIndexUpdate++ ;
	    }
    }

    /* Update the KeyFob count, excluding admin keyfob */
    xDataToSend.keyFobCount = ucIndexUpdate ;

    /* Now copy the contents of xDataToSend to pxIntelUserData->pvData so
     * that it can be used at Intel task.
     */
    memcpy(pxIntelUserData->pvData, &xDataToSend, sizeof(OrwlKeyData_t));
    /* Free the Keyfob entry structre allocated memory */
    if(pxKeyfobInfo)
    {
	vPortFree(pxKeyfobInfo);
    }

    /* Return success */
    return SUC_WRITE_STATUS_SUCCESS;
}
/*----------------------------------------------------------------------------*/

static uint8_t prvGetRTCDateTime(IntelUserData_t *pxIntelUserData)
{
    /* RTC data time structure, to communicate between SUC and BIOS */
    OrwlRTCTimeStat_t xRTCTimeComData;
    /* Error code to check RTC read */
    int32_t lStatus = NO_ERROR;
    /* RTC structure to read from RTC hardware */
    rtcDateTime_t xReadRTCDateTime;
    /* Error code to return to BIOS */
    uint8_t ucACK = ORWL_RTC_RD_STAT_SUCC;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);
    memset(&xRTCTimeComData,0,sizeof(xRTCTimeComData));

    /* Read the RTC time from RTC hardware */
    lStatus = lRtcGetDateTime(&xReadRTCDateTime);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed read RTC time.\n");
	/* This should usually not occur if RTC is initialized, we don't need to
	 * retry multiple times since it being done in lRtcGetDateTime.
	 */
	if(lStatus == COMMON_ERR_NOT_INITIALIZED)
	{
	    /* ORWL RTC not initialized */
	    ucACK = ORWL_RTC_RD_STAT_ERR;
	    /* Handle error */
	    goto ERROR;
	}
	/* ORWL RTC read second register was zero, must not occur but still BIOS
	 * can retry again. This is in else because these are the only two possibilities
	 * of error that can occur.
	 */
	else
	{
	    /* ORWL RTC second register read wrong data, but can be retried */
	    ucACK = ORWL_RTC_RD_STAT_BUSY;
	    goto ERROR;
	}
    }

    /* Now we have read the RTC, update the same to xRTCTimeComData */
    xRTCTimeComData.currTime.ucDate = xReadRTCDateTime.ucDate;
    xRTCTimeComData.currTime.ucHour = xReadRTCDateTime.ucHour;
    xRTCTimeComData.currTime.ucMinute = xReadRTCDateTime.ucMinute;
    xRTCTimeComData.currTime.ucMonth = xReadRTCDateTime.ucMonth;
    xRTCTimeComData.currTime.ucSecond = xReadRTCDateTime.ucSecond;
    xRTCTimeComData.currTime.ucWeekday = xReadRTCDateTime.ucWeekday;
    /* Year should be updated in terms of offset to 2000. Example 2015 offset is
     * 15 for 2000.
     */
    xRTCTimeComData.currTime.ucYearOffset2000 = xReadRTCDateTime.usYear - 2000;

    ERROR:
    /* Update the error code */
    xRTCTimeComData.ucReadRTCStat = ucACK;

    /* Now memcpy the xRTCTimeComData to pxIntelUserData so that it can be used
     * at Intel communication task.
     */
    memcpy(pxIntelUserData->pvData, &xRTCTimeComData, sizeof(OrwlRTCTimeStat_t));

    /* Return success */
    return ucACK;
}
/*----------------------------------------------------------------------------*/

static uint8_t prvSetRTCDateTime(IntelUserData_t *pxIntelUserData)
{
    /* RTC data time structure, to communicate between SUC and BIOS */
    OrwlRTCTime_t xRTCTimeComData;
    /* Error code to check RTC read */
    int32_t lStatus = NO_ERROR;
    /* RTC structure to read from RTC hardware */
    rtcDateTime_t xReadRTCDateTime;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);
    memset(&xRTCTimeComData,0,sizeof(xRTCTimeComData));

    /* memcpy BIOS data to xRTCTimeComData */
    memcpy(&xRTCTimeComData, pxIntelUserData->pvData, sizeof(OrwlRTCTime_t));

    /* update rtcDateTime_t to write to RTC hardware */
    xReadRTCDateTime.ucDate = xRTCTimeComData.ucDate;
    xReadRTCDateTime.ucHour = xRTCTimeComData.ucHour;
    xReadRTCDateTime.ucMinute = xRTCTimeComData.ucMinute;
    xReadRTCDateTime.ucMonth = xRTCTimeComData.ucMonth;
    xReadRTCDateTime.ucSecond = xRTCTimeComData.ucSecond;
    /* Year sent by BIOS is offset to 2000, so add the same to 2000 to calculate
     * actual year.
     */
    xReadRTCDateTime.usYear = xRTCTimeComData.ucYearOffset2000 + 2000;
    /* WeekDay is not necessary to set RTC data and time */
    /* xReadRTCDateTime.ucWeekday = xRTCTimeComData.ucWeekday; */

    /* Now we have updated xReadRTCDateTime write the same to RTC hardware */
    lStatus = lRtcSetDateTime(&xReadRTCDateTime);
    if(lStatus != NO_ERROR)
    {
	/* Check if RTC values are out of range */
	if(lStatus == COMMON_ERR_OUT_OF_RANGE)
	{
	    debugERROR_PRINT("RTC set values out of range.\n");
	    return SUC_WRITE_STATUS_RTC_INVALID;
	}
	else
	{
	    debugERROR_PRINT("Failed to set RTC time.\n");
	    return SUC_WRITE_STATUS_RTC_FAIL;
	}
    }

    /* RTC set success. Return success */
    return SUC_WRITE_STATUS_SUCCESS;
}
/*----------------------------------------------------------------------------*/

static uint8_t prvUpdateLeSeed( void )
{
	uint8_t ucCarry = 0;
	uint8_t ucTemp = 0;
	uint8_t ucTempSeed[nfccommonSEED_SIZE] = {0};
	uint8_t ucBLeBroadCastData[leMAX_BUF_LENGTH] = {0};
	uint32_t ulCount = 0;

	/* add increment factor to seed */
	for(ulCount = 0; ulCount < nfccommonSEED_SIZE; ulCount++)
	{
		ucTemp = (((ulCount) < nfccommonSEED_INC_SIZE)?
			xLeData.ucSeedIncrFactor[nfccommonSEED_INC_SIZE -1 - ulCount] : 0);
		ucTempSeed[nfccommonSEED_SIZE -1 - ulCount] =
			xLeData.ucSeed[nfccommonSEED_SIZE -1 - ulCount] +
			ucTemp + ucCarry;
		if((ucTempSeed[nfccommonSEED_SIZE -1 - ulCount] <=
			xLeData.ucSeed[nfccommonSEED_SIZE -1 - ulCount]) &&
			(ucTempSeed[nfccommonSEED_SIZE -1 - ulCount] <= ucTemp))
		{
			ucCarry = pdTRUE;
		}
		else
		{
			ucCarry = pdFALSE;
		}
		if((0 == xLeData.ucSeed[nfccommonSEED_SIZE -1 - ulCount]) &&
			(0 == ucTemp))
		{
			ucCarry = pdFALSE;
		}
	}
	memcpy(xLeData.ucSeed, ucTempSeed, nfccommonSEED_SIZE);

	/* Generate BLE Broadcast data */
	ucBLeBroadCastData[usermodeINDEX_0] = leDATA_LENGTH;
	ucBLeBroadCastData[usermodeINDEX_1] = leMANF_SPEC_DATA;
	/* Generate PRNG using Nfc Seed */
	if(lPrngReadSeed(xLeData.ucSeed, nfccommonSEED_SIZE))
	{
		debugERROR_PRINT(" Failed to read seed for pseudo"
				" random number");
		return usermodeERROR_STATE;
	}
	if(lPrngGenerateRandom(&ucBLeBroadCastData[usermodeINDEX_2],
		lePRND_LENGTH))
	{
		debugERROR_PRINT(" Failed to generate pseudo random"
				" number");
		return usermodeERROR_STATE;
	}
	/* Write Data to BLE Central */
	if( lLeInterfaceLe_WriteData(ucBLeBroadCastData,
					leMAX_BUF_LENGTH))
	{
		debugERROR_PRINT(" Failed to write data to ble eeprom");
		return usermodeERROR_STATE;
	}
	return NO_ERROR;
}
/*----------------------------------------------------------------------------*/

static int32_t prvStopNfcExecution( xSMAppResources_t *pxResHandle )
{
	EventBits_t xBits = 0;
	configASSERT(pxResHandle != NULL);
	xBits = xEventGroupSetBits( pxResHandle->xEventGroupNFC ,
					eventsNFC_STOP_EXEC );
	xBits = xEventGroupWaitBits(pxResHandle->xEventGroupUserModeApp,
				eventsKEYFOB_STOP_NFC_SUCCESS,
				pdFALSE,
				pdFALSE,
				portMAX_DELAY );
	/* Clearing all other events also */
	xBits = xEventGroupClearBits(pxResHandle->xEventGroupUserModeApp,
				eventsKEYFOB_NFC_AUTH_SUCCESS |
				eventsKEYFOB_NFC_AUTH_FAILURE |
				eventsKEYFOB_NFC_ASSOCIATION_SUCCESS |
				eventsKEYFOB_NFC_ASSOCIATION_FAILURE |
				eventsKEYFOB_NFC_ENTRY_FULL |
				eventsKEYFOB_STOP_NFC_SUCCESS |
				eventsKEYFOB_NOT_ADMIN);
	if( (xBits & eventsKEYFOB_STOP_NFC_SUCCESS)
			== eventsKEYFOB_STOP_NFC_SUCCESS )
	{
		debugPRINT_APP(" Receive event: eventsKEYFOB_STOP_NFC_SUCCESS");
		if(xQueueReset(pxResHandle->xNfcQueue) != pdTRUE)
		{
			debugERROR_PRINT("Failed to reset queue");
			return usermodeERROR_STATE;
		}
	}
	else
	{
		debugERROR_PRINT("Receive event: Invalid Event in sending Nfc"
			"stop execution event");
		return usermodeERROR_STATE;
	}
	return NO_ERROR;
}
/*----------------------------------------------------------------------------*/

/**
 * @brief function pointer table holds various function pointers to
 * to handle different states.
 */
static void (*vUserModeStates[eSTATE_SUC_UST_MAX])( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle ) =
{
		NULL, /* place holder for ST0 */
		prvSuCSM_LockedState,         /* ST1 */
		prvSuCSM_NfcAuthState,        /* ST2 */
		prvSuCSM_LeAliveCheckState,   /* ST3 */
		prvSuCSM_AccessDenialState,   /* ST4 */
		prvSuCSM_IntelCPUPowerON,     /* ST5 */
		prvSuCSM_ProximityError,      /* ST6 */
		prvSuCSM_UnlockedState,       /* ST7 */
		prvSuCSM_ProximityLock,       /* ST8 */
		prvSuCSM_DeviceMotion,        /* ST9 */
		prvSuCSM_ReLeAliveCheckState, /* ST10 */
		prvSuCSM_ReNfcAuthState,      /* ST11 */
		prvSuCSM_ErrorState           /* ST12 */
};
/*----------------------------------------------------------------------------*/

static void prvSuCSM_LockedState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxRes )
{
	int32_t lErr = NO_ERROR;
	/* Read RTC snapshot dirty bit */
	uint8_t ucRTCSnapshotDirtyBit = 0xff;
	/* PIN entry retry count */
	uint16_t usPswdRetryCount = 0;
	/* RTC delay seconds before next PIN entry */
	uint32_t ulRTCDelaySec = 0;
	/* RTC raw seconds snapshot at time of modifying delay */
	uint32_t ulRTCSnapShot = 0;
	/* RTC current seconds read from hardware */
	uint32_t ulRTCCurrentSeconds = 0;
	xUserConfig_t *pxUserConfig = NULL;
	configASSERT(pxDevState != NULL);
	configASSERT(pxRes != NULL);

	debugPRINT_APP("Accessing Event handlers: "
		"xEventGroupNFC = %p xEventGroupUserModeApp = %p" ,
		pxRes->xEventGroupNFC , pxRes->xEventGroupUserModeApp );

	/* power off Intel CPU */
	vSystemResPowerOff_IntelCpu();

	/* Configuring GPIO LE status pin */
	vSystemResBT_GPIO_Config();

	/* updated OLED display with Locked symbol.
	 * Indicate user to present keyFob to ORWL
	 */
	lErr = lDisplayImageFullScreen(eDipsInt_PWR_BTN_PRESS);
	if(lErr)
	{
		debugERROR_PRINT(" Failed to display image on OLED ");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}

	/* Check for RTC snapshot dirty bit. This bit is set when RTC delay is
	 * calculated and will be cleared after setting user RTC time and writing
	 * delay and snapshot to NVSRAM. In case this bit is not cleared then after
	 * calculating the delay we have not updated NVSRAM. In this case take the
	 * Current RTC time as RTC snapshot.
	 */
	lErr = lNvsramReadRTCSnapshotDirtyBit(&ucRTCSnapshotDirtyBit);
	if(lErr != NO_ERROR)
	{
	    debugERROR_PRINT("Failed to read RTCSnapshot\n");
	    /* reading NVSRAM should never fail, go to error state */
	    *pxDevState = eSTATE_SUC_UST_12;
	    goto ERR_STATE;
	}
	/* check if the RTC snapshot dirty bit is set */
	if(ucRTCSnapshotDirtyBit == pdTRUE)
	{
	    /* read the NVSRAM structure */
	    lErr = lNvsramReadPswdRetryCountDelay(&usPswdRetryCount, &ulRTCDelaySec,
		&ulRTCSnapShot);
	    if(lErr != NO_ERROR)
	    {
		debugERROR_PRINT("Failed to read retry count and RTC delay seconds \n");
		/* This should never fail */
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	    }

	    /* read the current RTC time */
	    while(lRtcGetRTCSeconds(&ulRTCCurrentSeconds) != NO_ERROR)
	    {
		/* if RTC read fails, which should never occur re-try until its success
		 * wait for 1 second before each re-try.
		 */
		vTaskDelay(uiONE_SEC_DELAY);
	    }
	    /* update RTC snapshot time with current RTC time */
	    ulRTCSnapShot = ulRTCCurrentSeconds;

	    /* Update the latest RTC snapshot to NVSRAM */
	    /* update the wait delay and snapshot rtc to NVSRAM */
	    lErr = lNvsramWritePswdRetryCountDelay(&usPswdRetryCount,
		&ulRTCDelaySec, &ulRTCSnapShot);
	    if(lErr != NO_ERROR)
	    {
		debugERROR_PRINT("Failed to update retry count and RTC delay seconds\n");
		/* updating NVSRAm should never fail, if failed report RTC fail */
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	    }
	    /* After updating the NVSRAM now, reset the dirty bit */
	    lErr = lNvsramResetRTCSnapshotDirtyBit();
	    if(lErr != NO_ERROR)
	    {
		debugERROR_PRINT("Failed to read RTCSnapshot\n");
		/* reading NVSRAM should never fail, go to error state */
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	    }
 	}

	pxUserConfig = (xUserConfig_t *)pvPortMalloc(sizeof(xUserConfig_t));
	if (pxUserConfig == NULL)
	{
		debugERROR_PRINT(" Failed to allocate memory for user"
			" configuration");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}
	/* Read the user configuration and update the global resource */
	lErr = lUserReadUserConfig( pxUserConfig );
	if ( lErr == NO_ERROR )
	{
		pxRes->ucAskPinOnBoot = pxUserConfig->ucAskPinOnBoot;
		pxRes->ucBLERange = pxUserConfig->ucBLERange;
		pxRes->ucIOEnDis = pxUserConfig->ucIOEnDis;
		pxRes->ucMode = pxUserConfig->ucMode;
		pxRes->ulSuCAction = pxUserConfig->ulSuCAction;
	}
	else
	{
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}

	/* Done with initial setup:
	 * change the system state to KeyFob NFC authentication */
	*pxDevState = eSTATE_SUC_UST_2;
	ERR_STATE:
	/* Return from function */
	return;
}
/*----------------------------------------------------------------------------*/

static void prvSuCSM_NfcAuthState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle )
{
	int32_t lErr = NO_ERROR;
	uint32_t ulEventToSend = 0;
	uint32_t ulEventToWait = 0;
	EventBits_t xBits = 0;
	xSMAppResources_t *pxRes = pxResHandle;
	configASSERT(pxDevState != NULL);
	configASSERT(pxResHandle != NULL);

	/* Clearing the Power button interrupt occurred before this point */
	xEventGroupClearBits( pxResHandle->xEventGroupUserModeApp ,
			eventsKEY_PRESS_SHORT | eventsKEY_PRESS_LONG );

	/* Update OLED dislay to press power button */
	lErr = lDisplayImageFullScreen(eDipsInt_PWR_BTN_PRESS);
	if(lErr)
	{
		debugERROR_PRINT("Failed to display Press button image on OLED");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}

	/* Wait here for the short button press event */
	while(pdFALSE != prvCheckShortBtnPress( pxRes ));

	/* Send event to allow authentication using new KeyFob */
	xBits = xEventGroupSetBits( pxRes->xEventGroupNFC ,
					eventsNFC_FIRST_DETECT);
	xBits = xEventGroupWaitBits(
					pxRes->xEventGroupUserModeApp ,
					eventsKEYFOB_FIRST_DETECT_SUCCESS,
					pdFALSE ,
					pdFALSE ,
					portMAX_DELAY );
	xBits = xEventGroupClearBits( pxRes->xEventGroupUserModeApp ,
					eventsKEYFOB_FIRST_DETECT_SUCCESS );
	if( (xBits & eventsKEYFOB_FIRST_DETECT_SUCCESS)
			!= eventsKEYFOB_FIRST_DETECT_SUCCESS )
	{
		debugPRINT_APP("Receive event: Invalid Event received ");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}
	/* Update OLED dislay to present keyfob */
	lErr = lDisplayImageFullScreen(eDipsInt_NFC_AUTH_REQUEST);
	if(lErr)
	{
		debugERROR_PRINT("Failed to display present key image on OLED");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}
	/* Block on event handler:
	 * NFC auth. failed
	 * NFC auth. successful
	 * NFC detect timeout
	 */
	ulEventToWait = eventsKEYFOB_NFC_AUTH_SUCCESS |
			eventsKEYFOB_NFC_AUTH_FAILURE |
			eventsKEYFOB_DETECT_TIMEOUT;
	/* Clearing events before posting authentication event */
	xBits = xEventGroupClearBits( pxRes->xEventGroupUserModeApp ,
			ulEventToWait | eventsKEYFOB_NOT_ADMIN );
	/* Post event for authentication */
	do
	{
		ulEventToSend = eventsNFC_DETECT_KEYFOB;
		xBits = xEventGroupSetBits( pxRes->xEventGroupNFC , /* The event group being updated. */
				ulEventToSend ); /* The bits being set. */
		if((xBits & eventsNFC_DETECT_KEYFOB) != eventsNFC_DETECT_KEYFOB)
		{
			debugERROR_PRINT("post event: "
				"eventsNFC_DETECT_KEYFOB Failed, retry... ");
			vTaskDelay( eventsEVENT_SET_DELAY );
		}
		else
		{
			debugPRINT_APP("post event: "
				"eventsNFC_DETECT_KEYFOB Done");
			break; /* set config. successful */
		}
	} while ( 0 );

	xBits = xEventGroupWaitBits(
				pxRes->xEventGroupUserModeApp , /* The event group being tested. */
				ulEventToWait, 			/* The bits within the event group to wait for. */
				pdFALSE , 			/* event bits should NOT be cleared before returning. */
				pdFALSE , 			/* Don't wait for both bits, either bit will do. */
				portMAX_DELAY ); 	/* ticks to wait */

	/* We received some events, lets read them and clear
	 * In this way, we are clearing the bits only when we are in "run" state.
	 * In this way bits set will not be lost !
	 */
	xBits = xEventGroupClearBits( pxRes->xEventGroupUserModeApp ,
		ulEventToWait | eventsKEYFOB_NOT_ADMIN );
	/* Delay is required to sync the console prints */
	vTaskDelay(usermodeDELAY_MSEC_100);
	/* change state based on event type */
	if( ((xBits & eventsKEYFOB_NFC_AUTH_SUCCESS)
		== eventsKEYFOB_NFC_AUTH_SUCCESS)
		&& ((xBits & eventsKEYFOB_NFC_AUTH_FAILURE)
			== eventsKEYFOB_NFC_AUTH_FAILURE) )
	{
		*pxDevState = eSTATE_SUC_UST_2;
		debugERROR_PRINT(" Both NFC AUTH FAIL/SUCCESS Received together."
			"This is an error case. Lets Rerun the detection "
			"and authentication process");
	}
	else if( (xBits & eventsKEYFOB_NFC_AUTH_SUCCESS)
		== eventsKEYFOB_NFC_AUTH_SUCCESS )
	{
		vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
		vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispADD_MSG_STR_YCOR);
		vDisplayText((const int8_t *)uiNFC_AUTHENTICATED);
		vTaskDelay( usermodeMSG_DISPLAY_TIME );
		*pxDevState = eSTATE_SUC_UST_3;
		debugPRINT_APP("Receive event: events KEYFOB_NFC_AUTH_SUCESS ");
	}
	else if( (xBits & eventsKEYFOB_NFC_AUTH_FAILURE)
		== eventsKEYFOB_NFC_AUTH_FAILURE )
	{
		*pxDevState = eSTATE_SUC_UST_4;
		debugERROR_PRINT("Receive event: eventsKEYFOB_NFC_AUTH_FAILURE ");
	}
	else
	{
		*pxDevState = eSTATE_SUC_UST_2;
		debugERROR_PRINT(" Receive event: Nfc Authentication timeout ");
	}
	ERR_STATE:
	/* Return from the function */
	return;
}
/*----------------------------------------------------------------------------*/

static void prvSuCSM_LeAliveCheckState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle )
{
	int32_t lErr = NO_ERROR;
	uint8_t ucPinStatus = -1;
	uint32_t ulLeStatusCount = 0;
	uint8_t ucLeGpioReg[usermodeLE_NUM_OF_BYTES]={0};
	uint8_t ucBLeBroadCastData[leMAX_BUF_LENGTH]={0};
	uint32_t ulPinentryRet = 0;
	NfcEventData_t xNfcData;
	TapEventData_t xPinData;
	configASSERT(pxDevState!= NULL);
	configASSERT(pxResHandle != NULL);
	ePINTYPE ePIN = eUSERPIN;

	/*Update OLED display */
	vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
	vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR);
	vDisplayText((const int8_t *)uiBT_PROXIMITY_CHECK);
	vTaskDelay( usermodeMSG_DISPLAY_TIME );

	do
	{
		lErr = xQueueReceive(pxResHandle->xNfcQueue, &xNfcData,
					portMAX_DELAY);
		if(lErr == pdFALSE)
		{
			lErr = COMMON_ERR_MAX;
			debugERROR_PRINT(" Failed to receive data from queue ");
			break;
		}
		lErr = NO_ERROR;
		/* Generate BLE Broadcast data */
		ucBLeBroadCastData[usermodeINDEX_0] = leDATA_LENGTH;
		ucBLeBroadCastData[usermodeINDEX_1] = leMANF_SPEC_DATA;
		if(xNfcData.ucBleSeed == NULL)
		{
			lErr = COMMON_ERR_INVAL;
			debugERROR_PRINT(" Invalid parameters received ");
			break;
		}
		/* Generate PRNG using Nfc Seed */
		lErr = lPrngReadSeed(xNfcData.ucBleSeed, nfccommonSEED_SIZE);
		if(lErr)
		{
			debugERROR_PRINT(" Failed to read seed for pseudo"
					" random number");
			break;
		}
		lErr = lPrngGenerateRandom(&ucBLeBroadCastData[usermodeINDEX_2],
			lePRND_LENGTH);
		if(lErr)
		{
			debugERROR_PRINT(" Failed to generate pseudo random"
					" number");
			break;
		}
		/* Write Data to BLE Central */
		lErr = lLeInterfaceLe_WriteData(ucBLeBroadCastData,
						leMAX_BUF_LENGTH);
		if(lErr)
		{
			debugERROR_PRINT(" Failed to write data to ble eeprom");
			break;
		}
#if usermodeENABLE_GPIO_READ
		/* Providing delay in loop as BLE central toggle GPIO after few seconds */
		for(ulLeStatusCount = 0; ulLeStatusCount < usermodeLE_MAX_RETRY;
					ulLeStatusCount++)
		{
			vTaskDelay(usermodeDELAY_MSEC_100);
			vSystemResBT_GPIO_Status(&ucPinStatus);
			if ( ucPinStatus == usermodeLE_GPIO_LOW )
			{
				*pxDevState = eSTATE_SUC_UST_5;
				break;
			}
		}
		if(ulLeStatusCount == usermodeLE_MAX_RETRY)
		{
			debugERROR_PRINT(" Peer is out of proximity ");
			lErr = COMMON_ERR_OUT_OF_RANGE;
			break;
		}
#else
		/* Check LE status using GPIO pin in EVT3 board */
		/* In EVT2 reading the eeprom value to get the GPIO status */
		for(ulLeStatusCount = 0; ulLeStatusCount < usermodeLE_MAX_RETRY;
			ulLeStatusCount++)
		{
			vTaskDelay(usermodeDELAY_MSEC_100);
			lErr = lI2cRead((uint8_t)leNORDIC_I2C_SLAVE_ADDR,
				(uint8_t)leGPIO_REG_ADDR,
				usermodeLE_NUM_OF_BYTES,
				ucLeGpioReg);
			if(lErr)
			{
				debugERROR_PRINT(" Failed to read data from LE "
					"EEPROM ");
				break;
			}
			else if(ucLeGpioReg[usermodeINDEX_0] ||
				ucLeGpioReg[usermodeINDEX_1] ||
				ucLeGpioReg[usermodeINDEX_2])
			{
				*pxDevState = eSTATE_SUC_UST_5;
				break;
			}
		}
		if(ulLeStatusCount == usermodeLE_MAX_RETRY)
		{
			debugERROR_PRINT(" Peer is out of proximity ");
			lErr = COMMON_ERR_OUT_OF_RANGE;
			break;
		}
#endif
	}while(0);
	if(lErr != NO_ERROR)
	{
		*pxDevState = eSTATE_SUC_UST_6;
	}
	else
	{
		/* Updating the LE Seed structure */
		memcpy((void*)xLeData.ucSeed, (void*)xNfcData.ucBleSeed,
					nfccommonSEED_SIZE);
		memcpy((void*)xLeData.ucSeedIncrFactor,
			(void*)xNfcData.ucLeIncrFactor,
			nfccommonSEED_INC_SIZE);
		/* Initialize Le seed updating timer */
		if(lTimerLeTmrInit() != NO_ERROR)
		{
			debugERROR_PRINT(" Failed to initiate the Le timer ");
			*pxDevState = eSTATE_SUC_UST_12;
			goto ERR_STATE;
		}
		if( (pxResHandle->ucAskPinOnBoot & PROX_PROT_BOOTPIN_MASK) == PROX_PROT_BOOTPIN_EN )
		{
			/* Clearing the events before pin entry check*/
			xEventGroupClearBits(pxResHandle->xEventGroupUserModeApp,
			eventsKEY_PRESS_SHORT | eventsKEY_PRESS_LONG);

			/* Before allowing user to enter the PIN check the retry count and take
			 * corresponding actions.
			 */
			ulPinentryRet = lPinhandlingCheckRetryDelay();
			if(ulPinentryRet != NO_ERROR)
			{
			    /* should never occur, writing/reading from NVSRAM has failed */
			    debugERROR_PRINT("Failed to check retry delay\n");
			    /* update user error mode 12, this should not happen */
			    *pxDevState = eSTATE_SUC_UST_12;
			    /* Disable the timer in case of NVRAM read/write error */
			    if( lTimerClose( timerLE_TIMER_ID ))
			    {
				debugERROR_PRINT("Failed to close the LE timer");
				*pxDevState = eSTATE_SUC_UST_12;
				goto ERR_STATE;
			    }
			    goto ERR_STATE;
			}

			/* Display on OLED to enter user PIN */
			vUiMessageDisplay((const int8_t *)uiUSER_SET_PIN,
				    uiXCORDINATE_0 ,uiYCORDINATE_C);
			vTaskDelay(uiTWO_SEC_DELAY);

			/* Now we have permission to get default PIN from user */
			ulPinentryRet = lPinhandlingAcceptPIN((uint8_t *)&xPinData,
			    (xSMUOobModeResources_t *) pxResHandle, pinentryUSER_GET_PSWD);
			if(ulPinentryRet != NO_ERROR)
			{
			    /* Timeout has occurred while entering PIN */
			    debugERROR_PRINT("Timeout occurred while entering PIN \n");
			    /* update error state 1 to retry pin entry */
			    *pxDevState = eSTATE_SUC_UST_1;
			    /* Disable the timer in case of NVRAM read/write error */
			    if( lTimerClose( timerLE_TIMER_ID ))
			    {
				debugERROR_PRINT("Failed to close the LE timer");
				*pxDevState = eSTATE_SUC_UST_12;
				goto ERR_STATE;
			    }
			    goto ERR_STATE;
			}

			/* verify user pin */
			ulPinentryRet = lPinhandlingVerifyPIN((uint8_t *)&xPinData, ePIN);
			if( ulPinentryRet != NO_ERROR )
			{
			    /* update error mode to 1, to retry pin entry */
			    *pxDevState = eSTATE_SUC_UST_1;
			    /* increment retry count */
			    ulPinentryRet = lPinhandlingIncrementRetryCount();
			    if(ulPinentryRet != NO_ERROR)
			    {
				/* should never occur */
				debugERROR_PRINT("Failed to increment retry count\n");
				/* update user error mode */
				*pxDevState = eSTATE_SUC_UST_12;
			    }
			    /* Disable the timer in case of NVRAM read/write error */
			    if( lTimerClose( timerLE_TIMER_ID ))
			    {
				debugERROR_PRINT("Failed to close the LE timer");
				*pxDevState = eSTATE_SUC_UST_12;
				goto ERR_STATE;
			    }
			    /* user has entered wrong pin. Authentication failed */
			    vDisplayClearScreen(dispWELCOME_IMGLTX,
				dispWELCOME_IMGLTY, dispWELCOME_IMGRBX,
				dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
			    vDisplaySetTextPos( dispWELCOME_STR_XCOR,
				    dispADD_MSG_STR_YCOR);
			    vDisplayText((const int8_t *)uiUSER_INCORRECT_PIN);
			    vTaskDelay( usermodeMSG_DISPLAY_TIME );
			}
			else
			{
			    ulPinentryRet = lNvsramResetPswdRetryCountDelay();
			    if(ulPinentryRet != NO_ERROR)
			    {
				/* should never occur */
				debugERROR_PRINT("Failed to reset retry count and delay\n");
				/* update user error mode */
				*pxDevState = eSTATE_SUC_UST_12;
			    }
			}
			/* Clearing the events which are not used in pin entry check*/
			xEventGroupClearBits(pxResHandle->xEventGroupUserModeApp,
			eventsKEY_PRESS_SHORT | eventsKEY_PRESS_LONG);
		}
	}
	ERR_STATE:
	/* Return from the function */
	return;
}
/*----------------------------------------------------------------------------*/

static void prvSuCSM_AccessDenialState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle )
{
	int32_t lErr = NO_ERROR;
	configASSERT(pxDevState!= NULL);
	configASSERT(pxResHandle != NULL);
	/* updated OLED display with Access denied message & image */
	debugERROR_PRINT("Access Denied ....! ");
	/* Update OLED Display */
	lErr = lDisplayImageFullScreen(eDipsInt_ACCESS_DENIED);
	if(lErr)
	{
		debugERROR_PRINT("Failed to display Access Denied image on "
				"OLED");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}
	/* 30 second timeout */
	vTaskDelay( usermodeDELAY_SEC_10 );
	/* change the state */
	*pxDevState = eSTATE_SUC_UST_2;
	ERR_STATE:
	/* Return from the function */
	return;
}
/*----------------------------------------------------------------------------*/

static void prvSuCSM_IntelCPUPowerON( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle )
{
	configASSERT(pxDevState!= NULL);
	configASSERT(pxResHandle!= NULL);

	/* power on Intel CPU */
	vSystemResPowerOn_IntelCpu();

	/* change the state */
	*pxDevState = eSTATE_SUC_UST_7;
}
/*----------------------------------------------------------------------------*/

static void prvSuCSM_ProximityError( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle )
{
	configASSERT(pxDevState!= NULL);
	configASSERT(pxResHandle!= NULL);

	int32_t lErr = NO_ERROR;
	/* update OLED with proximity protect logo & message */
	lErr = lDisplayImageFullScreen(eDipsInt_PROXIMITY_PROTECT);
	if(lErr)
	{
		debugERROR_PRINT(" Failed to display Proximity Protect image on"
					" OLED ");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}
	/* Wait for 10 secs*/
	vTaskDelay( usermodeDELAY_SEC_10 );
	/* change the state */
	*pxDevState = eSTATE_SUC_UST_1;
	ERR_STATE:
	/* Return from the function */
	return;
}
/*----------------------------------------------------------------------------*/

static void prvSuCSM_UnlockedState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle )
{
	int32_t lErr = NO_ERROR;
	uint8_t ucIntelRet = -1;
	uint8_t ucPinstatus = -1;
	uint8_t ucDelayCount = 0;
	uint8_t ucLeGpioReg[usermodeLE_NUM_OF_BYTES]={0};
	uint32_t ulCountValue;
	uint32_t ulGPIOVal ;
	uint32_t ulLeStatusCount = 0;
	xSMAppResources_t *pxRes = pxResHandle;
	NfcEventData_t xNfcData;
	IntelUserData_t xIntelUserData;
	EventBits_t xBits = 0;
	configASSERT(pxDevState!= NULL);
	configASSERT(pxResHandle!= NULL);
	configASSERT(pxResHandle->xSucBiosSendQueue!=NULL);

	/* Update OLED */
	if( lDisplayImageFullScreen(eDipsInt_UNLOCK))
	{
		debugERROR_PRINT(" Failed to display Unlock image on OLED ");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}
	/* Clearing the Power button interrupt occurred before this point */
	xEventGroupClearBits( pxResHandle->xEventGroupUserModeApp ,
			eventsKEY_PRESS_SHORT | eventsKEY_PRESS_LONG );
	/* Initialize 23:59:00 proximity timer */
	if(lTimerProximityTimeoutTmrInit() != NO_ERROR)
	{
		debugERROR_PRINT(" Failed to initiate the 24 timer ");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}
	/* This while poll for 4 events */
	/* Check for Power button Press */
	/* Check for Proximity for infinite/23:59:00 hrs timeout */
	/* Check for Intel Command */
	/* Check for Le Seed update timeout */
	while(1)
	{
		/* Check for Short Power button Press */
		if( prvCheckShortBtnPress( pxRes ) != pdTRUE)
		{
			vSystemResIntel_ShortPress();
		}
		/* Check for Long Power button Press */
		prvCheckLongBtnPress( pxRes );
		/* Check for Proximity for infinite/24 hrs time */
#if usermodeENABLE_GPIO_READ
		vSystemResBT_GPIO_Status(&ucPinstatus);
		if ( ucPinstatus == usermodeLE_GPIO_HIGH )
		{
			ulLeStatusCount++;
			vTaskDelay(usermodeDELAY_MSEC_100);
		}
		else
		{
			ulLeStatusCount = 0;
		}
#else
		if(lI2cRead((uint8_t)leNORDIC_I2C_SLAVE_ADDR,
					(uint8_t)leGPIO_REG_ADDR,
					usermodeLE_NUM_OF_BYTES,
					ucLeGpioReg))
		{
			*pxDevState = eSTATE_SUC_UST_8;
			debugERROR_PRINT(" Failed to read data from LE "
				"EEPROM ");
			break;
		}
		else if(ucLeGpioReg[usermodeINDEX_0] ||
			ucLeGpioReg[usermodeINDEX_1] ||
			ucLeGpioReg[usermodeINDEX_2])
		{
			/* continue with further checking */
		ulLeStatusCount = 0;
		}
		else
		{
		ulLeStatusCount++;
		vTaskDelay(usermodeDELAY_MSEC_100);
		}
#endif
		/* Check for Le retry count */
		if(ulLeStatusCount == usermodeLE_MAX_RETRY_UNLOCK_STATE )
		{
		    debugERROR_PRINT(" Peer is out of proximity ");
		    *pxDevState = eSTATE_SUC_UST_8;
		    break;
		}
		/* Check for Intel Command only if BLE Advertise is proper */
		if (ulLeStatusCount ==0 )
		{
		if(xQueueReceive(pxRes->xSucBiosSendQueue, &xIntelUserData,
			usermodeDELAY_SEC_1) == pdTRUE)
		{
			switch(xIntelUserData.ucCommand)
			{
			case ORWL_KEY_MGT:
			    memcpy(xNfcData.ucKeyFobName,
			     ((OrwlKeyMgt_t *)xIntelUserData.pvData)->keyFobNam,
			     keyfobidNAME_LEN);
			    if((((OrwlKeyMgt_t *)xIntelUserData.pvData)->preAttr)
				== ORWL_KEY_FOB_ATTR_ADMIN)
			    {
			        xNfcData.ulKeyPrivilege = nfccommonKEY_PRIVILEGE_ADMIN;
			    }
			    else if((((OrwlKeyMgt_t *)xIntelUserData.pvData)->preAttr)
			    	== ORWL_KEY_FOB_ATTR_USER)
			    {
				xNfcData.ulKeyPrivilege = nfccommonKEY_PRIVILEGE_USER;
			    }
			    if(xQueueSend(pxRes->xNfcQueue, &xNfcData,
				    portMAX_DELAY) != pdTRUE)
			    {
				debugERROR_PRINT(" Failed to send data"
					    " into the queue ");
			    }
			    if( xIntelUserData.ucSubCommand ==
							ORWL_KEY_FOB_MGT_ADD)
			    {
				ucIntelRet = prvAddKeyFob( pxRes );
			    }
			    else if( xIntelUserData.ucSubCommand ==
							ORWL_KEY_FOB_MGT_DEL)
			    {
				ucIntelRet = prvDeleteNFCKeyEntry(pxRes);
			    }
			    break;
			case ORWL_SEC_CHECK_LEVEL0:
			    ucIntelRet = prvCheckNfcBlePin( pxRes, pdFALSE,
				    pdFALSE);
			    switch(ucIntelRet)
			    {
				case SUC_WRITE_STATUS_SUCCESS:
					    break;
				case SUC_WRITE_STATUS_TIMEOUT:
					    vDisplayClearScreen(
						    dispWELCOME_IMGLTX,
						    dispWELCOME_IMGLTY,
						    dispWELCOME_IMGRBX,
						    dispWELCOME_IMGRBY,
						    dispBACKGROUND_BLACK);
					    vDisplaySetTextPos(
						    dispWELCOME_STR_XCOR,
						    dispWELCOME_STR_YCOR);
					    vDisplayText(
						    (const int8_t *)
						    uiTIME_OUT);
					    break;

				case SUC_WRITE_STATUS_FAIL_AUTH:
					    vDisplayClearScreen(
						    dispWELCOME_IMGLTX,
						    dispWELCOME_IMGLTY,
						    dispWELCOME_IMGRBX,
						    dispWELCOME_IMGRBY,
						    dispBACKGROUND_BLACK);
					    vDisplaySetTextPos(
						    dispWELCOME_STR_XCOR,
						    dispWELCOME_STR_YCOR);
					    vDisplayText(
						    (const int8_t *)
						    uiSECURITY_CHECK_FAIL);
						break;
				default:
					debugERROR_PRINT("Unknown error\n");
					while(1);
			    }
			    break;
			case ORWL_SEC_CHECK_LEVEL1:
			    ucIntelRet = prvCheckNfcBlePin( pxRes, pdTRUE,
				    pdTRUE);
			    switch(ucIntelRet)
			    {
				case SUC_WRITE_STATUS_SUCCESS:
					    break;
				case SUC_WRITE_STATUS_TIMEOUT:
					    vDisplayClearScreen(
						    dispWELCOME_IMGLTX,
						    dispWELCOME_IMGLTY,
						    dispWELCOME_IMGRBX,
						    dispWELCOME_IMGRBY,
						    dispBACKGROUND_BLACK);
					    vDisplaySetTextPos(
						    dispWELCOME_STR_XCOR,
						    dispWELCOME_STR_YCOR);
					    vDisplayText(
						    (const int8_t *)
						    uiTIME_OUT);
					    break;

				case SUC_WRITE_STATUS_FAIL_AUTH:
					    vDisplayClearScreen(
						    dispWELCOME_IMGLTX,
						    dispWELCOME_IMGLTY,
						    dispWELCOME_IMGRBX,
						    dispWELCOME_IMGRBY,
						    dispBACKGROUND_BLACK);
					    vDisplaySetTextPos(
						    dispWELCOME_STR_XCOR,
						    dispWELCOME_STR_YCOR);
					    vDisplayText(
						    (const int8_t *)
						    uiSECURITY_CHECK_FAIL);
						break;
				default:
					debugERROR_PRINT("Unknown error\n");
					while(1);
			    }
			    break;
			case SUC_ROT_REGEN:
			    ucIntelRet = prvCheckNfcBlePin( pxRes, pdTRUE,
				    pdTRUE );
			    if(ucIntelRet == SUC_WRITE_STATUS_SUCCESS)
			    {
				ucIntelRet = prvUserRotReGen();
			    }
			    break;
			case INTEL_DEV_ACT:
			    ucIntelRet = prvHandleIntelDevState( pxRes, &xIntelUserData );
			    break;
			case ORWL_KEY_FOR_DAT:
			    ucIntelRet = prvReadKeyFobData( &xIntelUserData );
			    break;
			case GET_RTC_TIME:
			    ucIntelRet = prvGetRTCDateTime( &xIntelUserData );
			    break;
			case SET_RTC_TIME:
			    ucIntelRet = prvSetRTCDateTime( &xIntelUserData );
			    break;
			default:
			    debugERROR_PRINT(" Invalid case ");
			    ucIntelRet = SUC_WRITE_STATUS_FAIL_UNKOWN;
			    break;
			}
			if (ucIntelRet == usermodeERROR_STATE)
			{
				/* Send the error to Intel */
				ucIntelRet = SUC_WRITE_STATUS_FAIL_UNKOWN;
				xQueueSend(pxRes->xSucBiosReceiveQueue,
					&ucIntelRet, portMAX_DELAY);
				*pxDevState = eSTATE_SUC_UST_12;
				goto ERR_STATE;
			}
			/* Send the return status to Intel */
			if(xQueueSend(pxRes->xSucBiosReceiveQueue, &ucIntelRet,
				portMAX_DELAY) != pdTRUE)
			{
				debugERROR_PRINT(" ERROR in Queue data send");
				*pxDevState = eSTATE_SUC_UST_12;
				goto ERR_STATE;
			}
			/* Update OLED */
			if( lDisplayImageFullScreen(eDipsInt_UNLOCK))
			{
				debugERROR_PRINT(" Failed to display image on"
						" OLED ");
				*pxDevState = eSTATE_SUC_UST_12;
				goto ERR_STATE;
			}
		}
		}
		/* Check for Le Seed update event and 23:59:00 hrs timeout */
		xBits = xEventGroupWaitBits(pxRes->xEventGroupUserModeApp ,
						eventsLE_TIMER_OUT |
						eventsPROXIMITY_TIMEOUT,
						pdFALSE ,
						pdFALSE ,
						usermodeMIN_WAIT_DELAY );
		if(( xBits & eventsPROXIMITY_TIMEOUT) ==
			eventsPROXIMITY_TIMEOUT)
		{
			xBits = xEventGroupClearBits(
				pxRes->xEventGroupUserModeApp,
				eventsPROXIMITY_TIMEOUT );
			/* 23:59:00 hrs proximity timeout occurred */
			debugPRINT_APP(" 23:59:00 hrs proximity timeout"
				" occurred. Please re authenticate");
			*pxDevState = eSTATE_SUC_UST_8;
			break;
		}
		if( (xBits & eventsLE_TIMER_OUT) == eventsLE_TIMER_OUT )
		{
			xBits = xEventGroupClearBits(
				pxRes->xEventGroupUserModeApp,
				eventsLE_TIMER_OUT );
			/* Update LE seed */
			if( prvUpdateLeSeed() == usermodeERROR_STATE )
			{
				*pxDevState = eSTATE_SUC_UST_12;
				goto ERR_STATE;
			}
			debugPRINT_APP(" LE seed updated ");
		}
	}
	/* Display proximity lock on OLED */
	lErr = lDisplayImageFullScreen(eDipsInt_PROXIMITY_PROTECT);
	if(lErr)
	{
		debugERROR_PRINT("Failed to display Proximity lock image on "
			"OLED");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}
	/* Disable IO Interface */
	vSystemResProxi_On();
	/* If proximity fails then stop the Le update timer */
	if( lTimerClose( timerLE_TIMER_ID ))
	{
		debugERROR_PRINT(" Failed to close the LE timer ");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}
	/* If while loop fails not due to proximity fails then stop the 23:59:00
	 * hrs timer. If lTimerRead function returns NO_ERROR means timer has
	 * not been closed. */
	if( lTimerRead( timerPROXIMITY_TIMER_ID , &ulCountValue)
		== NO_ERROR)
	{
		if( lTimerClose( timerPROXIMITY_TIMER_ID ))
		{
			debugERROR_PRINT("Failed to close the PROXIMITY timer");
			*pxDevState = eSTATE_SUC_UST_12;
			goto ERR_STATE;
		}
	}
	/* Intel display off and send signal to intel according to user
	 * configuration */
	/* Check for the Mode of Intel subsytem */
	switch(pxRes->ucMode)
	{
	case PROX_PROT_MOD_NO_CG:
		break;
	case PROX_PROT_MOD_STD_BY:
		/* Check for standby/shutdown pin. If Intel is not in standby
		 * mode and user config is standby then send the system to
		 * standby */
		if ( usermodePWRBTN_HIGH == ulSystemResPowerState_Intel())
		{
			vSystemResIntel_ShortPress();
		}
		/* handle BIOS communication for INTEL_DEV_ACT */
		lErr = prvHandleIntelDevStateProxi( pxRes, usermodeINTEL_DEV_WAIT_STDBY );
		if(lErr != SUC_WRITE_STATUS_SUCCESS)
		{
		    debugERROR_PRINT("Failed to handle prvHandleIntelDevStateProxi\n");
		}
		break;
	case PROX_PROT_MOD_SHT_DWN:
		/* Assert low bat for shutdown to initiate */
		vSystemResPowerOff_IntelCpu();
		/* handle BIOS communication for INTEL_DEV_ACT, if any */
		lErr = prvHandleIntelDevStateProxi( pxRes, usermodeINTEL_DEV_WAIT_SHTDWN );
		if(lErr != SUC_WRITE_STATUS_SUCCESS)
		{
		    debugERROR_PRINT("Failed to handle prvHandleIntelDevStateProxi\n");
		}
		do
		{
			/* wait for 3 seconds for system to shutdown*/
			vTaskDelay(SystemResINTEL_SHUTDOWN_WAIT_PERIOD) ;
			ucDelayCount++ ;
			ulGPIOVal = ulSystemResPowerState_Intel() ;
		}while ((ucDelayCount<SystemResDELAY_INC_1S_30S_COUNT) &&
			ulGPIOVal) ;
		/* Shutdown the Intel Power */
		vSystemResForcePowerOff_IntelCpu();
		break;
	default:
		debugERROR_PRINT(" Invalid case of user configuration mode ");
		/* Assert low bat for shutdown to initiate */
		vSystemResPowerOff_IntelCpu();
		do
		{
			/* wait for 3 seconds for system to shutdown*/
			vTaskDelay(SystemResINTEL_SHUTDOWN_WAIT_PERIOD) ;
			ucDelayCount++ ;
			ulGPIOVal = ulSystemResPowerState_Intel() ;
		}while ((ucDelayCount<SystemResDELAY_INC_1S_30S_COUNT) &&
			ulGPIOVal) ;
		resetSYSTEM_RESET
		break;
	}

	ERR_STATE:
	/* Return from the function */
	return;
	/* If proximity check fail goto state 8 */

	/* update OLED: "Welcome to ORWL", "www.orwl.org", unlock Image */

	/* Run below actions in infinite loop until stated */

	/* LE engine alive status check */

	/* Reset LE engine, if alive status check fails wait for 3 seconds &
	 * retry alive packet check (LE engine reset time expected 2 seconds) */

	/* quit application if alive check fails
	 * Note: This should never happen. This condition indicates device
	 * level issue or LE software stack issue */

	/* Monitor proximity [GIO sample] */

	/* Poll for UART1 message queue for BIOS communication (Intel to SuC)
	 * Few of the important commands listed below
	 * --> SSD auth. password
	 * --> associate new KeyFob
	 * --> de-associate a already associated KeyFob
	 * --> define/change LE proximity range
	 */

	/* Send message to BIOS, if any (SuC to Intel) */

	/* Sleep for PROX_QUE_LEN*PROXIMITY_CHECK_TIME seconds. Yield processor
	 * to others. If no one waiting probably Idle loop will take over here.
	 *
	 * Note:
	 * PROX_QUE_LEN*PROXIMITY_CHECK_TIME  second is ideal time here because
	 * Proxmity check engine (BLE) will change its state only for 3 seconds.
	 * i.e. it receives broadcast packet for every PROXIMITY_CHECK_TIME second.
	 * If packet not received for more than PROX_QUE_LEN*PROXIMITY_CHECK_TIME
	 * sec then only it will toggle the GIO.
	 * [where	PROXIMITY_CHECK_TIME is the duration for which LE
	 * 				advertisement is configured
	 * 		PROX_QUE_LEN is the length of the circular buffer
	 * 				implemented on LE receiver ]
	 * There is no chance of GIO level change before this time period
	 * TODO:
	 * Please note that PROX_QUE_LEN & PROXIMITY_CHECK_TIME values to be
	 * tuned for optimal user experience.
	 * Lesser the value, device locks frequently based on 2.4G band congestion
	 * Higher the value, device takes too long to lock after
	 * proximity check failure.
	 */

	/* end of infinite loop: continue in loop if device in proximity */

	/* Turn OFF GIO to indicate BIOS proximity lost */

	/* change state if KeyFob not in proximity */
}
/*----------------------------------------------------------------------------*/

static void prvSuCSM_ProximityLock( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle )
{
	int32_t lErr = NO_ERROR;
	uint32_t ulEventToSend = 0;
	uint32_t ulEventToWait = 0;
	EventBits_t xBits = 0;
	BaseType_t xMotionStatus = pdFALSE;
	xSMAppResources_t *pxRes = pxResHandle;
	configASSERT(pxDevState!= NULL);
	configASSERT(pxResHandle!= NULL);

	/* Clearing the Power button interrupt occurred before this point */
	xEventGroupClearBits( pxResHandle->xEventGroupUserModeApp ,
			eventsKEY_PRESS_SHORT | eventsKEY_PRESS_LONG );

	/* Update OLED dislay to proximity lock and press power button */
	lErr = lDisplayImageFullScreen(eDipsInt_PROXIMITY_LOCK);
	if(lErr)
	{
		debugERROR_PRINT("Failed to display Proximity lock image on "
			"OLED");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}

	/* Clearing all motion detected before enabling device motion */
	lErr = xMpuinterfaceIsMotionDetected();
	lErr = NO_ERROR;
	/* Enable Device motion detection */
	lErr = lMpuinterfaceDisableDmp();
	lErr |= lMpuinterfaceEnableMotionDetection();
	if(lErr != NO_ERROR)
	{
		debugERROR_PRINT("Failed to initialize motion detection");
		*pxDevState = eSTATE_SUC_UST_12;
		goto ERR_STATE;
	}
	do
	{
		/* Check for long button press */
		prvCheckLongBtnPress( pxRes );
		/* Check for short button press */
		if(prvCheckShortBtnPress( pxRes ) == pdFALSE )
		{
			break;
		}
		else
		{
			/* Checking for motion detection */
			if( pxResHandle->ulSuCAction == PROX_PROT_MOT_EN )
			{
				xMotionStatus = xMpuinterfaceIsMotionDetected();
			}
			if( pdTRUE == xMotionStatus )
			{
				/* Disable device motion detection here */
				lErr = lMpuinterfaceDisableMotionDetection();
				lErr |= lMpuinterfaceEnableDmp();
				if(lErr != NO_ERROR)
				{
					debugERROR_PRINT("Failed to "
					"de-initialize motion detection");
					*pxDevState = eSTATE_SUC_UST_12;
					goto ERR_STATE;
				}
				*pxDevState = eSTATE_SUC_UST_9;
				break;
			}
		}
	}while(1);

	if( pdFALSE == xMotionStatus )
	{
		/* Update OLED dislay to present keyfob */
		lErr = lDisplayImageFullScreen(eDipsInt_NFC_AUTH_REQUEST);
		if(lErr)
		{
			debugERROR_PRINT("Failed to display present key image "
						"on OLED");
			*pxDevState = eSTATE_SUC_UST_12;
			goto ERR_STATE;
		}
		/* Events to wait */
		ulEventToWait = eventsKEYFOB_NFC_AUTH_SUCCESS |
				eventsKEYFOB_NFC_AUTH_FAILURE |
				eventsKEYFOB_DETECT_TIMEOUT;
		xBits = xEventGroupClearBits( pxRes->xEventGroupUserModeApp ,
					ulEventToWait );
		/* Posting event for keyfob authentication */
		do
		{
			ulEventToSend = eventsNFC_DETECT_KEYFOB;
			xBits = xEventGroupSetBits( pxRes->xEventGroupNFC ,
					ulEventToSend );
			if((xBits & eventsNFC_DETECT_KEYFOB) !=
				eventsNFC_DETECT_KEYFOB)
			{
				debugERROR_PRINT("post event: "
				"eventsNFC_DETECT_KEYFOB Failed, retry... ");
				vTaskDelay( eventsEVENT_SET_DELAY );
			}
			else
			{
				debugPRINT_APP("post event:"
					" eventsNFC_DETECT_KEYFOB Done");
				break;
			}
		} while ( 0 );
		/* Wait for events from nfc */
		do
		{
			xBits = xEventGroupWaitBits(
						pxRes->xEventGroupUserModeApp ,
						ulEventToWait,
						pdFALSE ,
						pdFALSE ,
						usermodeMIN_WAIT_DELAY );
			if( pxResHandle->ulSuCAction == PROX_PROT_MOT_EN )
			{
				xMotionStatus = xMpuinterfaceIsMotionDetected();
			}
			if( ( xBits & ulEventToWait ) == 0 )
			{
				xBits = 0;
			}
		}while( (!xBits) && (!xMotionStatus) );
		xBits = xEventGroupClearBits( pxRes->xEventGroupUserModeApp ,
			ulEventToWait | eventsKEYFOB_NOT_ADMIN );
		vTaskDelay(usermodeDELAY_MSEC_100);
		/* change state based on event type */
		if( pdTRUE == xMotionStatus )
		{
			/* Disable device motion detection here */
			lErr = lMpuinterfaceDisableMotionDetection();
			lErr |= lMpuinterfaceEnableDmp();
			if(lErr != NO_ERROR)
			{
				debugERROR_PRINT("Failed to "
				"de-initialize motion detection");
				*pxDevState = eSTATE_SUC_UST_12;
				goto ERR_STATE;
			}
			*pxDevState = eSTATE_SUC_UST_9;
		}
		else if( ((xBits & eventsKEYFOB_NFC_AUTH_SUCCESS)
			== eventsKEYFOB_NFC_AUTH_SUCCESS)
			&& ((xBits & eventsKEYFOB_NFC_AUTH_FAILURE)
				== eventsKEYFOB_NFC_AUTH_FAILURE) )
		{
			debugERROR_PRINT(" Both NFC AUTH FAIL/SUCCESS Received"
				" together. This is an error case. "
				"Lets Rerun the detection "
				"and authentication process");
			*pxDevState = eSTATE_SUC_UST_11;
		}
		else if( (xBits & eventsKEYFOB_NFC_AUTH_SUCCESS)
			== eventsKEYFOB_NFC_AUTH_SUCCESS )
		{
			vDisplayClearScreen(dispWELCOME_IMGLTX,
				dispWELCOME_IMGLTY, dispWELCOME_IMGRBX,
				dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
			vDisplaySetTextPos( dispWELCOME_STR_XCOR,
				dispADD_MSG_STR_YCOR);
			vDisplayText((const int8_t *)uiNFC_AUTHENTICATED);
			debugPRINT_APP("Receive event: events "
				"KEYFOB_NFC_AUTH_SUCCESS");
			vTaskDelay( usermodeMSG_DISPLAY_TIME );
			*pxDevState = eSTATE_SUC_UST_10;
		}
		else if( (xBits & eventsKEYFOB_NFC_AUTH_FAILURE)
			== eventsKEYFOB_NFC_AUTH_FAILURE )
		{
			debugERROR_PRINT("Receive event: "
				"eventsKEYFOB_NFC_AUTH_FAILURE ");
			*pxDevState = eSTATE_SUC_UST_11;
		}
		else
		{
			debugERROR_PRINT(" Receive event: "
				"Timeout in nfc authentication ");
			*pxDevState = eSTATE_SUC_UST_8;
		}
	}
	ERR_STATE:
	/* Return from the function */
	return;
}
/*----------------------------------------------------------------------------*/

static void prvSuCSM_DeviceMotion( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle )
{
	configASSERT(pxDevState!= NULL);
	configASSERT(pxResHandle!= NULL);

	/* Update Display */
	vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
	vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR);
	vDisplayText((const int8_t *)uiDEVICE_IN_MOTION);
	vTaskDelay( usermodeMSG_DISPLAY_TIME );

	/* Shutdown the Intel and reset ORWL */
	resetSYSTEM_RESET
	return;
}
/*----------------------------------------------------------------------------*/

static void prvSuCSM_ReLeAliveCheckState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle )
{
	int32_t lErr = NO_ERROR;
	uint8_t ucPinStatus = -1;
	uint32_t ulLeStatusCount = 0;
	uint8_t ucLeGpioReg[usermodeLE_NUM_OF_BYTES]={0};
	uint8_t ucBLeBroadCastData[leMAX_BUF_LENGTH]={0};
	NfcEventData_t xNfcData;
	configASSERT(pxDevState!= NULL);
	configASSERT(pxResHandle!= NULL);

	/* Update Display */
	vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
	vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR);
	vDisplayText((const int8_t *)uiBT_PROXIMITY_CHECK);

	do
	{
		lErr = xQueueReceive(pxResHandle->xNfcQueue, &xNfcData,
					portMAX_DELAY);
		if(lErr == pdFALSE)
		{
			lErr = COMMON_ERR_MAX;
			debugERROR_PRINT(" Failed to receive data from queue ");
			break;
		}
		lErr = NO_ERROR;
		/* Generate BLE Broadcast data */
		ucBLeBroadCastData[usermodeINDEX_0] = leDATA_LENGTH;
		ucBLeBroadCastData[usermodeINDEX_1] = leMANF_SPEC_DATA;
		if(xNfcData.ucBleSeed == NULL)
		{
			lErr = COMMON_ERR_INVAL;
			debugERROR_PRINT(" Invalid parameters received ");
			break;
		}
		/* Generate PRNG using Nfc Seed */
		lErr = lPrngReadSeed(xNfcData.ucBleSeed, nfccommonSEED_SIZE);
		if(lErr)
		{
			debugERROR_PRINT(" Failed to read seed for pseudo"
					" random number");
			break;
		}
		lErr = lPrngGenerateRandom(&ucBLeBroadCastData[usermodeINDEX_2],
			lePRND_LENGTH);
		if(lErr)
		{
			debugERROR_PRINT(" Failed to generate pseudo random"
					" number");
			break;
		}
		/* Write Data to BLE Central */
		lErr = lLeInterfaceLe_WriteData(ucBLeBroadCastData,
						leMAX_BUF_LENGTH);
		if(lErr)
		{
			debugERROR_PRINT(" Failed to write data to ble eeprom");
			break;
		}
#if usermodeENABLE_GPIO_READ
		/* Delay is required as BLE central toggle GPIO after few seconds */
		for(ulLeStatusCount = 0; ulLeStatusCount < usermodeLE_MAX_RETRY;
			ulLeStatusCount++)
		{
			vTaskDelay(usermodeDELAY_MSEC_100);
			vSystemResBT_GPIO_Status(&ucPinStatus);
			if ( ucPinStatus == usermodeLE_GPIO_LOW  )
			{
				*pxDevState = eSTATE_SUC_UST_7;
				break;
			}
		}
		if(ulLeStatusCount == usermodeLE_MAX_RETRY)
		{
			lErr = COMMON_ERR_OUT_OF_RANGE;
			break;
		}
#else
		/* Delay is required as BLE central toggle GPIO after few seconds */
		/* Check LE status using GPIO pin in EVT3 board */
		/* In EVT2 reading the eeprom value to get the GPIO status */
		for(ulLeStatusCount = 0; ulLeStatusCount < usermodeLE_MAX_RETRY;
			ulLeStatusCount++)
		{
			vTaskDelay(usermodeDELAY_MSEC_100);
			lErr = lI2cRead((uint8_t)leNORDIC_I2C_SLAVE_ADDR,
				(uint8_t)leGPIO_REG_ADDR,
				usermodeLE_NUM_OF_BYTES,
				ucLeGpioReg);
			if(lErr)
			{
				debugERROR_PRINT(" Failed to read data from LE "
					"EEPROM ");
				break;
			}
			else if(ucLeGpioReg[usermodeINDEX_0] ||
				ucLeGpioReg[usermodeINDEX_1] ||
				ucLeGpioReg[usermodeINDEX_2])
			{
				*pxDevState = eSTATE_SUC_UST_7;
				break;
			}
		}
		if(ulLeStatusCount == usermodeLE_MAX_RETRY)
		{
			lErr = COMMON_ERR_OUT_OF_RANGE;
			break;
		}
#endif
	}while(0);
	if(lErr != NO_ERROR)
	{
		*pxDevState = eSTATE_SUC_UST_8;
	}
	else
	{
		/* Updating the LE Seed structure */
		memcpy((void*)xLeData.ucSeed, (void*)xNfcData.ucBleSeed,
			nfccommonSEED_SIZE);
		memcpy((void*)xLeData.ucSeedIncrFactor,
			(void*)xNfcData.ucLeIncrFactor,
			nfccommonSEED_INC_SIZE);
		/* Initialize Le seed updating timer */
		if(lTimerLeTmrInit() != NO_ERROR)
		{
			debugERROR_PRINT(" Failed to initiate the Le timer ");
			*pxDevState = eSTATE_SUC_UST_12;
			goto ERR_STATE;
		}
		/* Send signal to Intel according to user configuration */
		switch(pxResHandle->ucMode)
		{
		case PROX_PROT_MOD_NO_CG:
			break;
		case PROX_PROT_MOD_STD_BY:
			vSystemResIntel_ShortPress();
			break;
		case PROX_PROT_MOD_SHT_DWN:
			vSystemResPowerOn_IntelCpu();
			break;
		default:
			debugERROR_PRINT(" Invalid case of user SuCAction ");
			break;
		}
		/* Enable IO Interface */
		vSystemResProxi_Off();
	}
	ERR_STATE:
	/* Return from the function */
	return;
}
/*----------------------------------------------------------------------------*/

static void prvSuCSM_ReNfcAuthState( eUserSuCUserStates *pxDevState,
	xSMAppResources_t *pxResHandle )
{
	int32_t lErr = NO_ERROR;
	configASSERT(pxDevState!= NULL);
	configASSERT(pxResHandle!= NULL);
	/* Update the SuC state */
	*pxDevState = eSTATE_SUC_UST_8;
	/* updated OLED display with Access denied message & image */
	debugERROR_PRINT("Access Denied ....! ");
	/* Update OLED Display */
	lErr = lDisplayImageFullScreen(eDipsInt_ACCESS_DENIED);
	vTaskDelay( usermodeMSG_DISPLAY_TIME );
	if(lErr)
	{
		debugERROR_PRINT("Failed to display Access Denied image on "
				"OLED");
		*pxDevState = eSTATE_SUC_UST_12;
	}
}
/*----------------------------------------------------------------------------*/

static void prvSuCSM_ErrorState(eSuCRotStates *pxDevState,
	xSMAppResources_t *pxResHandle)
{
	configASSERT(pxDevState!= NULL);
	configASSERT(pxResHandle!= NULL);

	/* Update Display */
	vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
	vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR);
	vDisplayText((const int8_t *)uiERROR_MSG);
	/* Hang here */
	while(1);
}
/*----------------------------------------------------------------------------*/

uint32_t ulUserModeInitGlobalApplicationResources( xSMAppResources_t *pxRes )
{
	/* Initialize the Event handlers */
	if( pxRes == NULL )
	{
	    return COMMON_ERR_NULL_PTR;
	}

	pxRes->xEventGroupUserModeApp = xEventGroupCreate( );
	if( pxRes->xEventGroupUserModeApp == NULL )
	{
	    debugERROR_PRINT(
		"Insufficient Heap: Failed to create UserModeApp event group" );
	    return COMMON_ERR_FATAL_ERROR;
	}

	pxRes->xEventGroupNFC = xEventGroupCreate( );
	if( pxRes->xEventGroupNFC == NULL )
	{
	    debugERROR_PRINT(
		"Insufficient Heap: Failed to create NFC event group" );
	    return COMMON_ERR_FATAL_ERROR;
	}

	pxRes->xEventGroupPinEntry = xEventGroupCreate();
	if (pxRes->xEventGroupPinEntry == NULL)
	{
	    debugERROR_PRINT(
		"Insufficient Heap: Failed to create oobModeApp event group");
	    return COMMON_ERR_FATAL_ERROR;
	}

	pxRes->xPowerBtnEventHandle = xEventGroupCreate();
	if (pxRes->xPowerBtnEventHandle == NULL)
	{
	    debugERROR_PRINT(
		"Insufficient Heap: Failed to create xPowerBtnEventHandle ");
	    return COMMON_ERR_FATAL_ERROR;
	}

	pxRes->xPinQueue = xQueueCreate(1, sizeof(TapEventData_t));
	if (pxRes->xPinQueue == NULL)
	{
	    debugERROR_PRINT("Insufficient Heap: Failed to create Queue");
	    return COMMON_ERR_FATAL_ERROR;
	}

	pxRes->xNfcQueue = xQueueCreate(1, sizeof(NfcEventData_t));
	if (pxRes->xNfcQueue == NULL)
	{
	    debugERROR_PRINT("Insufficient Heap: Failed to create User NFC Queue");
	    return COMMON_ERR_FATAL_ERROR;
	}

	pxRes->xSucBiosSendQueue = xQueueCreate(1, sizeof(IntelUserData_t));
	if (pxRes->xSucBiosSendQueue == NULL)
	{
	    debugERROR_PRINT("Insufficient Heap: Failed to create User Send BIOSSuC Queue");
	    return COMMON_ERR_FATAL_ERROR;
	}

	pxRes->xSucBiosReceiveQueue = xQueueCreate(1, sizeof(uint8_t));
	if (pxRes->xSucBiosReceiveQueue == NULL)
	{
	    debugERROR_PRINT("Insufficient Heap: Failed to create User Rx SuCBIOS Queue");
	    return COMMON_ERR_FATAL_ERROR;
	}

	pxRes->prvI2CSemaphore = xSemaphoreCreateBinary();
	if (pxRes->prvI2CSemaphore == NULL)
	{
	    debugERROR_PRINT("Failed to create prvI2CSemaphore");
	    return COMMON_ERR_FATAL_ERROR;
	}
	else
	{
	    xSemaphoreGive(pxRes->prvI2CSemaphore);
	}
	return NO_ERROR;
}
/*----------------------------------------------------------------------------*/

/* user mode application thread */
void vUserModeUserModeTask( void *pvArg )
{
	/* usermode device states handler */
	eUserSuCUserStates xDevState;

	/* thread resources */
	xSMAppResources_t *pxResHandle;

	if( pvArg == NULL )
	{
		debugERROR_PRINT(
			"\nFailed to create user mode task, invalid pvArg \n" );
		return;
	}

	/*
	 * pvArgs must provide an event handler information for the thread
	 * created.
	 */
	pxResHandle = (xSMAppResources_t *)pvArg;

	/* default state on boot */
	xDevState = eSTATE_SUC_UST_1;

	do
	{
		if( xDevState > eSTATE_SUC_UST_INVALID
			&& xDevState < eSTATE_SUC_UST_MAX )
		{
			vUserModeStates[xDevState]( &xDevState , pxResHandle );
		}
		else
		{
			/* device has reached a bad state.
			 * Probably stack corruption?
			 * It should never happen
			 *
			 * Note: 'xDevState' is a local variable which will be
			 * accessed and modified by this thread alone.
			 * So, we should be able to manage it as required.
			 */
			debugERROR_PRINT("Invalid xDevState %d set, aborting...",
				xDevState );
			break;
		}
	} while ( 1 );

	/* release all the resources */
	vTaskDelete( NULL );
}
/*----------------------------------------------------------------------------*/
