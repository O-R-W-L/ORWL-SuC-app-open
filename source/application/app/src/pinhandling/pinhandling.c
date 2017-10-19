/**===========================================================================
 * @file pinhandling.c
 *
 * @brief This file contains implementation of PIN handling routines.
 *
 * @author megharaj.ag@design-shift.com
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
#include <rtc.h>


/* Static function declaration */

/**
 * @brief This function send Keyfob Association event to nfc thread and wait for
 * association.
 *
 * @param *pxResHandle user mode resource handle
 *
 * @return SUC_WRITE_STATUS_SUCCESS on success
 *         errors on Failure
 */
static uint32_t prvClaculateDelayForCount( uint16_t usPswdRetryCount );

/* Function definition */

static uint32_t prvClaculateDelayForCount( uint16_t usPswdRetryCount )
{
    /* Index for loop */
    uint16_t usIndex = 0;
    /* RTC delay seconds to wait */
    uint32_t ulRTCDelaySec = 0;

    /* calculate the wait delay for retry count */
    for(usIndex = 1; usIndex <= usPswdRetryCount; usIndex++)
    {
	/* Check if the retry count is below or equal to 3, if so than just increment
	 * the retry count and update RTC delay seconds as zero since we want zero delay
	 * for three attempts.
	 */
	if(usIndex <= pinhandlingNODELAY_RETRYCOUNT)
	{
	    /* RTC delay seconds should be zero */
	    ulRTCDelaySec = 0;
	}
	else
	{
	    /* check if the retry count is 4, if so initialize the seconds to 60 */
	    if(usIndex == pinhandlingFOURTH_PIN_ENTRY)
	    {
		/* this is first time initialization, mutilply by two from now on */
		ulRTCDelaySec = pinhandlingFOURTH_PIN_ENTRY_DELAY;
	    }
	    else
	    {
		/* Mutiply RTC delay seconds by 2 */
		ulRTCDelaySec = ulRTCDelaySec * pinhandlingMULTIPLY_FACTOR_DELAY;
	    }
	}
    }
    /* return the calculated ulRTCDelaySec */
    return ulRTCDelaySec;
}
/*---------------------------------------------------------------------------*/

int32_t lPinhandlingCheckRetryDelay( void )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* PIN entry retry count */
    uint16_t usPswdRetryCount = 0;
    /* RTC delay seconds before next PIN entry */
    uint32_t ulRTCDelaySec = 0;
    /* RTC raw seconds snapshot at time of modifying delay */
    uint32_t ulRTCSnapShot = 0;
    /* RTC current seconds read from hardware */
    uint32_t ulRTCCurrentSeconds = 0;

    /* Read the retry count and RTC delay seconds from NVSRAM */
    lStatus = lNvsramReadPswdRetryCountDelay(&usPswdRetryCount, &ulRTCDelaySec,
	    &ulRTCSnapShot);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to read retry count and RTC delay seconds \n");
	/* This should never fail */
	return lStatus;
    }

    /* Before proceeding further first get the RTC seconds */
    while(lRtcGetRTCSeconds(&ulRTCCurrentSeconds) != NO_ERROR)
    {
	/* if RTC read fails, which should never occur re-try until its success
	 * wait for 1 second before each re-try.
	 */
	vTaskDelay(uiONE_SEC_DELAY);
    }

    /* Display message on OLED to user informing to wait till it is allowed
     * to do PIN entry.
     */
    if((ulRTCSnapShot + ulRTCDelaySec) > (ulRTCCurrentSeconds))
    {
	vUiMessageDisplay((const int8_t *)uiWAIT_TILL_NEXT_TRY,
		uiXCORDINATE_0 ,uiYCORDINATE_C);
	vTaskDelay(uiTWO_SEC_DELAY);
    }

    /* Now check if we have waited enough time till ulRTCDelaySec. This can
     * be achieved by
     */
    while((ulRTCSnapShot + ulRTCDelaySec) > (ulRTCCurrentSeconds))
    {
	/* Wait 2 seconds before reading RTC to check if we have completed
	 * wait time.
	 */
	vTaskDelay(uiTWO_SEC_DELAY);
	/* Read the new RTC time to compare */
	while(lRtcGetRTCSeconds(&ulRTCCurrentSeconds) != NO_ERROR)
	{
	    /* if RTC read fails, which should never occur re-try until its success
	     * wait for 1 second before each re-try.
	     */
	    vTaskDelay(uiONE_SEC_DELAY);
	}
    }
    /* return error code */
    return lStatus;
}
/*---------------------------------------------------------------------------*/

int32_t lPinhandlingAcceptPIN( uint8_t *pucPinBuf, xSMUOobModeResources_t *pxRes,
	uint32_t ulEventToSend)
{
    /* PIN data to read */
    TapEventData_t xdata;
    /* Status to return */
    int32_t lStatus = NO_ERROR;
    EventBits_t uxBits;

    /* validate arguments */
    if (pxRes == NULL || pucPinBuf == NULL)
    {
	debugERROR_PRINT("Invalid args received \n");
	return COMMON_ERR_NULL_PTR;
    }

    /* post event to PinEntry thread */
    do
    {
	uxBits = xEventGroupSetBits(pxRes->xEventGroupPinEntry, ulEventToSend);
    }while (0);

    /* block on queue */
    xQueueReceive(pxRes->xPinQueue, &xdata, portMAX_DELAY);
    /* copy the pin to buffer */
    memcpy(pucPinBuf, xdata.ucPin ,sizeof(TapEventData_t));
    /* update the error code */
    lStatus = xdata.lStatus;
    /* reset the queue */
    xQueueReset(pxRes->xPinQueue);
    /* return the error code */
    return lStatus;
}
/*---------------------------------------------------------------------------*/

int32_t lPinhandlingVerifyPIN( uint8_t *pucPinBuf, ePINTYPE ePIN)
{
    /* Status to return */
    int32_t lStatus = NO_ERROR;
    /* PIN to be verified */
    uint8_t ucRefferencePIN[keysPIN_LEN];

    /* validate the input buffer */
    if(pucPinBuf == NULL)
    {
	/* usually will never occur */
	return COMMON_ERR_NULL_PTR;
    }

    /* PIN verification should be done for user/default pin. */
    if(ePIN == eDEFPINTYPE)
    {
	/* Verify if the given PIN matches default PIN */
	/* Get the default pin*/
	lStatus = lKeysGetDefaultPIN(ucRefferencePIN);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("\n %s failed to get default pin\n", __func__);
	    return lStatus;
	}
    }
    /* Check if the PIN type to be verified is user PIN */
    else if(ePIN == eUSERPIN)
    {
	lStatus = lKeysGetUserPIN(ucRefferencePIN);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("\n %s failed to get user pin\n", __func__);
	    return lStatus;
	}
    }
    else
    {
	/* Not supported as of now */
	return COMMON_ERR_INVAL;
    }

    /* Now we have reference PIN to which the entered PIN to be compared with */
    if (!memcmp(ucRefferencePIN, pucPinBuf, pinentryPIN_LENGTH))
    {
	/* PIN matches with reference PIN */
	return NO_ERROR;
    }
    else
    {
	/* PIN dint match send the error code */
	return COMMON_ERR_NO_MATCH;
    }

    /* return error code */
    return lStatus;
}
/*---------------------------------------------------------------------------*/

int32_t lPinhandlingIncrementRetryCount( void )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* PIN entry retry count */
    uint16_t usPswdRetryCount = 0;
    /* RTC delay seconds before next PIN entry */
    uint32_t ulRTCDelaySec = 0;
    /* RTC raw seconds snapshot at time of modifying delay */
    uint32_t ulRTCSnapShot = 0;

    /* Read the retry count and RTC delay seconds from NVSRAM */
    lStatus = lNvsramReadPswdRetryCountDelay(&usPswdRetryCount, &ulRTCDelaySec,
	    &ulRTCSnapShot);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to read retry count and RTC delay seconds \n");
	/* This should never fail */
	return lStatus;
    }

    /* Increment the retry count */
    usPswdRetryCount++;

    /* calculate the delay needed for this retry count */
    ulRTCDelaySec = prvClaculateDelayForCount(usPswdRetryCount);

    /* Whenever the ulRTCDelaySec has been updated, update the snapshot
     * RTC(current RTC from hardware) so that with this we can calculate
     * the ulRTCDelaySec when RTC time has been updated in user mode.
     */
    while(lRtcGetRTCSeconds(&ulRTCSnapShot) != NO_ERROR)
    {
	/* if RTC read fails, which should never occur re-try until its success
	 * wait for 1 second before each re-try.
	 */
	vTaskDelay(uiONE_SEC_DELAY);
    }

    /* Update retry count and RTC delay seconds */
    lStatus = lNvsramWritePswdRetryCountDelay(&usPswdRetryCount, &ulRTCDelaySec,
	    &ulRTCSnapShot);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to update retry count and RTC delay seconds\n");
	/* updating NVSRAm should never fail */
	return lStatus;
    }
    /* return error code */
    return lStatus;
}
/*---------------------------------------------------------------------------*/
