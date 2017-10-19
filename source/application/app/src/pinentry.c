/**===========================================================================
 * @file pinentry.c
 *
 * @brief This file is creates task for pin entry and validates
 *        the pin entered by user.
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ============================================================================
 *
 */
/* Global include*/
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* RTOS include */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <event_groups.h>
#include <portable.h>
#include <errors.h>

/* Application includes*/
#include <debug.h>
#include <delay.h>
#include <orwl_disp_interface.h>
#include <pinentry.h>
#include <debug.h>
#include <access_keys.h>
#include <mem_common.h>
#include <hist_devtamper.h>
#include <oob.h>
#include <usermode.h>
#include <oled_ui.h>
#include <delay.h>
#include <trng.h>
#include <irq.h>
#include <powerbtn.h>
#include <init.h>

extern xSMAppResources_t xgResource;

void vPinentryTask(void *pvArg)
{
    uint8_t ucDigit[pinentryNDIGITS]={0};
    uint8_t ucDispCount = 0;
    uint8_t ucPinIndex = 0;
    EventBits_t xBits;
    /* Timeout for TAP event*/
    const TickType_t xTicksToWait = pdMS_TO_TICKS(uiTWO_SEC_DELAY);
    /* Timeout to delete the selected digit using TAP event*/
    const TickType_t xTicksToDel = pdMS_TO_TICKS(uiONE_SEC_DELAY);
    TickType_t xTicksToWaitForButtonPress;
    xSMAppResources_t *pxResHandle;
    TapEventData_t xData;
    uint32_t ulRandom = 0;
    uint8_t ucChangeCusorPos = 0;
    uint8_t ucStartCursorMovment = 0;

    /* Initialize the lStatus to NO_ERROR */
    xData.lStatus = NO_ERROR;
    /*
     * pvArgs must provide an event handler information for the thread created.
     */
    if( pvArg == NULL )
    {
	debugERROR_PRINT(
		"\nFailed to create user mode task, invalid pvArg \n" );
	return;
    }
    pxResHandle = (xSMAppResources_t *)pvArg;

    if (pxResHandle->xEventGroupPinEntry == NULL )
    {
	debugERROR_PRINT("NULL pointer error");
	configASSERT(0);
    }
    while(1)
    {
	xBits = xEventGroupWaitBits(pxResHandle->xEventGroupPinEntry,
		    pinentryGET_PASSWORD | pinentryUSER_GET_PSWD,pdFALSE
		    , pdFALSE, portMAX_DELAY);
	if (xBits & pinentryGET_PASSWORD)
	{
	    debugPRINT_APP("Got pinentryGET_PASSWORD\n");
	    /* In oob mode set xTicksToWaitForButtonPress to portMAX_DELAY */
	    xTicksToWaitForButtonPress = portMAX_DELAY;
#ifdef ENABLE_MPU_GIO_INTERRUPT
	    /* Resume Tap detection task */
	    vTaskResume(xgResource.xMpuFifoTskHandle);
#endif
	    xEventGroupClearBits(pxResHandle->xEventGroupPinEntry, xBits);
	}
	else if(xBits & pinentryUSER_GET_PSWD)
	{
	    debugPRINT_APP("Got pinentryUSER_GET_PSWD\n");
	    xTicksToWaitForButtonPress = pdMS_TO_TICKS(uiONE_MIN_DELAY);
	    xEventGroupClearBits(pxResHandle->xEventGroupPinEntry, xBits);
	}
	else
	{
	    debugERROR_PRINT("Time out occurred");
	    continue;
	}
	/* Display configuration message at the center of OLED*/
	vUiMessageDisplay((const int8_t *)uiTAPMSG,uiXCORDINATE_0,uiYCORDINATE_C - 1);
#ifdef ENABLE_MPU_GIO_INTERRUPT
	/*Wait for user confirmation on message displayed on OLED*/
	xBits = xEventGroupWaitBits(pxResHandle->xEventGroupPinEntry,
	    pinentryGET_TAP_EVENT,pdFALSE, pdFALSE, portMAX_DELAY);
	if (xBits & pinentryGET_TAP_EVENT)
	{
	    xEventGroupClearBits(pxResHandle->xEventGroupPinEntry,
		pinentryGET_TAP_EVENT);

	}
#else
	/* Clear the bits before waiting for events.
	 * Just to make sure previous events are cleared.
	 */
	xEventGroupClearBits(pxResHandle->xEventGroupPinEntry,
		    pinentryKEY_PRESS);
	xBits = xEventGroupWaitBits(pxResHandle->xEventGroupPinEntry,
		pinentryKEY_PRESS ,pdFALSE, pdFALSE, xTicksToWaitForButtonPress);
	if (xBits & pinentryKEY_PRESS)
	{
	    xEventGroupClearBits(pxResHandle->xEventGroupPinEntry,
		    pinentryKEY_PRESS);
	}
	else
	{
	    /* Handle time out error in OOB mode */
	    if (xTicksToWaitForButtonPress == portMAX_DELAY)
	    {
		/* Go back and wait for pinentryGET_PASSWORD or
		 * pinentryUSER_GET_PSWD event.
		 */
		continue;
	    }
	    /* This part of the code should be executed only
	     * when ORWL is in USER mode
	     */
	    debugPRINT(" Time out occurred\n");
	    memset(xData.ucPin, -1,pinentryPIN_LENGTH);
	    /* Time out occurred */
	    xData.lStatus = COMMON_ERR_IN_PROGRESS;
	    /* Send time out status through queue */
	    xQueueSend(pxResHandle->xPinQueue, &xData, portMAX_DELAY);
	    /* Go back and wait for pinentryGET_PASSWORD | pinentryUSER_GET_PSWD event */
	    continue;
	}
#endif
	/* Clear the display and display digits and also draw a rectangle*/
	vDisplayClearScreen(uiXCORDINATE_0,uiYCORDINATE_0
		,uiXCORDINATE_M,uiYCORDINATE_M, uiCLEAR_SCREEN);
	vUiDisplayDigit();
	vUiDrawRectangle();

	ucPinIndex = 0;
	ucChangeCusorPos = 0;

	while(1)
	{
	    if (ucChangeCusorPos == 0)
	    {
		ucChangeCusorPos = 1;
		do
		{
		    /* check if random array values.
		     * if 0 generate the random
		     */
		    if(ulRandom == 0)
		    {
			/* generate 16 byte random number */
			if (ulGenerateRandomNumber(&ulRandom,trngMIN_TRNG_GEN_WORDS))
			{
			    debugERROR_PRINT("Failed to generate random number\n");
			    while(1);
			}
		    }

		    /* As we need random number from 0-9 mask first four bits*/
		    ucDispCount = ulRandom & pinentryMASK;
		    /* Right shift, read 4 bits*/
		    ulRandom = ulRandom >> pinentryRIGHT_SHIFT;
		    if (ucDispCount < pinentryN_ELEMENTS - 1)
		    {
			break;
		    }
		}while(1);
	    }
	    switch (ucDispCount)
	    {
		case ePINENRTY_ZERO:
		    vUiDisplayCusrsor(uiCURSOR_POS_0, uiYCORDINATE_1);
		    memcpy(ucDigit,pinentryTEXT_ZERO,pinentrySTR_LEN);
		    break;
		case ePINENRTY_ONE:
		    vUiDisplayCusrsor(uiCURSOR_POS_1, uiYCORDINATE_1);
		    memcpy(ucDigit,pinentryTEXT_ONE,pinentrySTR_LEN);
		    break;
		case ePINENRTY_TWO:
		    vUiDisplayCusrsor(uiCURSOR_POS_2, uiYCORDINATE_1);
		    memcpy(ucDigit,pinentryTEXT_TWO,pinentrySTR_LEN);
		    break;
		case ePINENRTY_THREE:
		    vUiDisplayCusrsor(uiCURSOR_POS_3, uiYCORDINATE_1);
		    memcpy(ucDigit,pinentryTEXT_THREE,pinentrySTR_LEN);
		    break;
		case ePINENRTY_FOUR:
		    vUiDisplayCusrsor(uiCURSOR_POS_4, uiYCORDINATE_1);
		    memcpy(ucDigit,pinentryTEXT_FOUR,pinentrySTR_LEN);
		    break;
		case ePINENRTY_FIVE:
		    vUiDisplayCusrsor(uiCURSOR_POS_5, uiYCORDINATE_1);
		    memcpy(ucDigit,pinentryTEXT_FIVE,pinentrySTR_LEN);
		    break;
		case ePINENRTY_SIX:
		    vUiDisplayCusrsor(uiCURSOR_POS_6, uiYCORDINATE_1);
		    memcpy(ucDigit,pinentryTEXT_SIX,pinentrySTR_LEN);
		    break;
		case ePINENRTY_SEVEN:
		    vUiDisplayCusrsor(uiCURSOR_POS_7, uiYCORDINATE_1);
		    memcpy(ucDigit,pinentryTEXT_SEVEN,pinentrySTR_LEN);
		    break;
		case ePINENRTY_EIGHT:
		    vUiDisplayCusrsor(uiCURSOR_POS_8, uiYCORDINATE_1);
		    memcpy(ucDigit,pinentryTEXT_EIGHT,pinentrySTR_LEN);
		    break;
		case ePINENRTY_NINE:
		    vUiDisplayCusrsor(uiCURSOR_POS_9, uiYCORDINATE_1);
		    memcpy(ucDigit,pinentryTEXT_NINE,pinentrySTR_LEN);
		    break;
		case ePINENTRY_TEN:
		    vUiDisplayCusrsor(uiCURSOR_POS_10, uiYCORDINATE_1);
		    break;
		default:
		    debugPRINT_APP("Invalid case: Should never happen\n");
		    break;
	    }
#ifdef ENABLE_TAP_FEATURE
	    /* wait for tap tap event to skip and timeout to confirm the digit */
	    xBits = xEventGroupWaitBits(pxResHandle->xEventGroupPinEntry,
		    pinentryGET_TAP_EVENT,pdFALSE, pdFALSE, xTicksToWait);
	    if (xBits & pinentryGET_TAP_EVENT)
	    {
		xEventGroupClearBits(pxResHandle->xEventGroupPinEntry,
			pinentryGET_TAP_EVENT);
		debugPRINT_APP("Got TAP event skipping one digit [%d]\n",ucDispCount);
	    }
#else
	    /* Clear the bits before waiting for events.
	     * Just to make sure previous events are cleared.
	     */
	    xEventGroupClearBits(pxResHandle->xEventGroupPinEntry,
			    pinentryKEY_PRESS);

	    /* Start the cursor movement only after UI displayed
	     * and user has pressed the power button.
	     */
	    if(ucStartCursorMovment == 0)
	    {
		xBits = xEventGroupWaitBits(pxResHandle->xEventGroupPinEntry,
			    pinentryKEY_PRESS ,pdFALSE, pdFALSE, xTicksToWaitForButtonPress);
		xEventGroupClearBits(pxResHandle->xEventGroupPinEntry,
				pinentryKEY_PRESS);
		if((xBits & pinentryKEY_PRESS) != pinentryKEY_PRESS)
		{
		    /* This part of the code should be executed only
		     * when ORWL is in USER mode.
		     */
		    debugPRINT(" Time out occurred\n");
		    memset(xData.ucPin, -1,pinentryPIN_LENGTH);
		    /* Time out occurred */
		    xData.lStatus = COMMON_ERR_IN_PROGRESS;
		    /* Send time out status through queue */
		    xQueueSend(pxResHandle->xPinQueue, &xData, portMAX_DELAY);
		    /* Go back and wait for pinentryGET_PASSWORD | pinentryUSER_GET_PSWD event */
		    break;
		}
		/*Cursor movement started */
		ucStartCursorMovment++;
		/* Clear the cursor*/
		vUiClearCursor(uiCURSOR_POS_0 + ucDispCount, uiYCORDINATE_1);
		/* As user started move the cursor to next position*/
		ucDispCount++;
		/*Go back and start accepting pin*/
		continue;
	    }
	    /* If time out occurs then digit is considered */
	    xBits = xEventGroupWaitBits(pxResHandle->xEventGroupPinEntry,
		    pinentryKEY_PRESS ,pdFALSE, pdFALSE, xTicksToWait);
	    if (xBits & pinentryKEY_PRESS)
	    {
		xEventGroupClearBits(pxResHandle->xEventGroupPinEntry,
			pinentryKEY_PRESS);
		debugPRINT_APP("Got Button press event "
			"skipping one digit [%d]\n",ucDispCount);
	    }
#endif
	    else
	    {
		if(ucDispCount == ePINENTRY_TEN)
		{
		    if(ucPinIndex > 0 && ucPinIndex <= pinentryPIN_LENGTH)
		    {
			ucPinIndex--;
			vUiSetCoordAndDisplay(uiXSTAR_COORD + ucPinIndex,
			    uiYSTAR_COORD, (const int8_t *)" ");
		    }
		}
		else
		{
		    /*Display digit at rectangle box*/
		    vUiSetCoordAndDisplay(uiXSTAR_COORD + ucPinIndex,
			    uiYSTAR_COORD, (const int8_t *)ucDigit);
		    memset(ucDigit, 0, pinentrySTR_LEN);
#ifdef ENABLE_TAP_FEATURE
		    /* wait for Tap event to skip last pin selection */
		    xBits = xEventGroupWaitBits(pxResHandle->xEventGroupPinEntry,
			    pinentryGET_TAP_EVENT,pdFALSE, pdFALSE, xTicksToDel);
		    if (xBits & pinentryGET_TAP_EVENT)
		    {
			xEventGroupClearBits(pxResHandle->xEventGroupPinEntry,
			    pinentryGET_TAP_EVENT);
			debugPRINT_APP(""
			    "Got TAP event deleting previous digit [%d]\n",ucDispCount);
		    }
#else
		    /* Clear the bits before waiting for events.
		     * Just to make sure previous events are cleared.
		     */
		    xEventGroupClearBits(pxResHandle->xEventGroupPinEntry,
				    pinentryKEY_PRESS);
		    /* wait for Button press event to delete last pin selection */
		    xBits = xEventGroupWaitBits(pxResHandle->xEventGroupPinEntry,
			    pinentryKEY_PRESS,pdFALSE,pdFALSE, xTicksToDel);
		    if (xBits & pinentryKEY_PRESS)
		    {
			xEventGroupClearBits(pxResHandle->xEventGroupPinEntry,
				pinentryKEY_PRESS);
			debugPRINT_APP("Got Button press event "
				"deleting [%d]\n",ucDispCount);
			vUiSetCoordAndDisplay(uiXSTAR_COORD + ucPinIndex,
				    uiYSTAR_COORD, (const int8_t *)" ");

		    }
#endif
		    else
		    {
			vUiSetCoordAndDisplay(uiXSTAR_COORD + ucPinIndex,
			    uiYSTAR_COORD, (const int8_t *)" ");
			/* pin selected display "*" in the rectangle */
			vUiSetCoordAndDisplay(uiXSTAR_COORD + ucPinIndex,
			    uiYSTAR_COORD, (const int8_t *)"*");
			xData.ucPin[ucPinIndex] = ucDispCount;
			ucPinIndex++;
			debugPRINT_APP("Timeout event selecting digit [%d]\n",ucDispCount);
			/* Password selected change the cursor position*/
			ucChangeCusorPos = 0;
		    }
		    /* clear selected text */
		    vUiSetCoordAndDisplay(0,uiPINDIS_YCORD,
					(const int8_t *)"         ");
		    /* clear the digit selected.Digit is cleared from the bottom.*/
		    vUiSetCoordAndDisplay(uiPINDIS_XCORD + 2,uiPINDIS_YCORD,
			(const int8_t *)" ");
		}
		/* Reset the cursor movement,
		 * once digit is selected as password*/
		ucStartCursorMovment = 0;
	    }
	    /* clear the cursor */
	    vUiClearCursor(uiCURSOR_POS_0 + ucDispCount, uiYCORDINATE_1);
	    if(ucPinIndex == pinentryPIN_LENGTH)
	    {
#ifdef ENABLE_MPU_GIO_INTERRUPT
		/* collected password. suspend the pin entry read task again */
		vTaskSuspend(xgResource.xMpuFifoTskHandle);
#endif

		debugPRINT_APP("Sending password to the initiator: ");
		xData.lStatus = NO_ERROR;
		xQueueSend(pxResHandle->xPinQueue, &xData, portMAX_DELAY);
		memset(xData.ucPin,0x00,pinentryN_ELEMENTS);
		ucStartCursorMovment = 0;
		break;
	    }
	    if(ucDispCount < pinentryN_ELEMENTS)
	    {
		ucDispCount++;
	    }
	    else
	    {
		ucDispCount = 0;
	    }
	}
    }
}
/* ---------------------------------------------------------------------------*/
