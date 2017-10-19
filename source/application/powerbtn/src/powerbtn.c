/**===========================================================================
 * @file powerbtn.c
 *
 * @brief This file contains power button management functions.
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ============================================================================
 *
 */

#include <FreeRTOS.h>
#include <event_groups.h>
#include <powerbtn.h>
#include <debug.h>
#include <usermode.h>
#include <events.h>
#include <pinentry.h>

#define powerbtnTASK_SLEEP_TIME		(150) 				/**< Task sleep time */
#define powerbtnLONG_PRESS_TIME		(powerbtnTASK_SLEEP_TIME * 30 + 50) 	/**< Count to indicate >5sec press */
#define powerbtnPRESS_DETECT		(1<<0) 				/**< power button press detect event */

/** internal event handle  */
EventGroupHandle_t prvPowerBtnEventHandleInt;

/**
 * @brief Power button press ISR handler
 *
 * @return void
 */
static void vPowerbtnISRPress( void );

/**
 * @brief This function enables power btn press detection
 *
 * @return void
 */
static void prvPowerbtnEnable( void );

/**
 * @brief This function disables power btn press detection
 *
 * @return void
 */
static void prvPowerbtnDisable( void );

/*---------------------------------------------------------------------------*/

static void vPowerbtnISRPress( void )
{
    uint32_t xResult;
    BaseType_t xHigherPriorityTaskWoken;

    /* xHigherPriorityTaskWoken must be initialized to pdFALSE. */
     xHigherPriorityTaskWoken = pdFALSE;

     /* disable power button IRQ */
     prvPowerbtnDisable();

     /* post an event to POWER BTN handler thread */
     xResult = xEventGroupSetBitsFromISR(prvPowerBtnEventHandleInt,
	     powerbtnPRESS_DETECT,&xHigherPriorityTaskWoken );

     if( xResult != pdFAIL )
       {
   	/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
   	switch should be requested. The macro used is port specific and will
   	be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
   	the documentation page for the port being used. */
   	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
       }
}
/*---------------------------------------------------------------------------*/

void vPowerbtnConfig( void )
{
    mml_gpio_config_t xGpioConfig;
    uint32_t xResult = NO_ERROR;

    prvPowerBtnEventHandleInt = xEventGroupCreate();
    configASSERT(prvPowerBtnEventHandleInt != NULL);

    /* register GPIO and configure for interrupt */
    xGpioConfig.gpio_direction = MML_GPIO_DIR_IN;
    xGpioConfig.gpio_function = MML_GPIO_NORMAL_FUNCTION;
    xGpioConfig.gpio_pad_config = MML_GPIO_PAD_PULLUP;
    xGpioConfig.gpio_intr_mode = MML_GPIO_INT_MODE_LEVEL_TRIGGERED;
    xGpioConfig.gpio_intr_polarity = MML_GPIO_INT_POL_HIGH;

    xResult = mml_gpio_set_config(MML_GPIO_DEV1, powerbtnPWR_BTN_GIO, 1,
	    xGpioConfig);

    /* configure the interrupt*/
    xResult |= lIrqClearInterrupt(MML_GPIO_DEV1, powerbtnPWR_BTN_GIO);
    xResult |= lIrqSetup(MML_GPIO_DEV1, powerbtnPWR_BTN_GIO,
	    MML_GPIO_INT_MODE_LEVEL_TRIGGERED, MML_GPIO_INT_POL_HIGH,
	    vPowerbtnISRPress);

    configASSERT(xResult == NO_ERROR);
}
/*---------------------------------------------------------------------------*/

static void prvPowerbtnEnable( void )
{
    uint32_t xResult = NO_ERROR;

    /* configure the interrupt*/
    xResult = lIrqClearInterrupt(MML_GPIO_DEV1, powerbtnPWR_BTN_GIO);
    xResult |= lIrqEnableInterrupt(MML_GPIO_DEV1, powerbtnPWR_BTN_GIO);

    configASSERT(xResult == NO_ERROR);
}
/*---------------------------------------------------------------------------*/

static void prvPowerbtnDisable( void )
{
    int32_t xResult = NO_ERROR;

    xResult = lIrqDisable(MML_GPIO_DEV1, powerbtnPWR_BTN_GIO);
    xResult |= lIrqClearInterrupt(MML_GPIO_DEV1, powerbtnPWR_BTN_GIO);

    configASSERT(xResult == NO_ERROR);
}
/*---------------------------------------------------------------------------*/

void vPowerbtnTsk( void *pvArgs )
{
    uint32_t ulHoldTime = 0 ;
    uint32_t ulIsBtnReleased = pdFALSE ;
    EventBits_t xBits = 0;
    mml_gpio_pin_data_t xInBuf;
    xSMAppResources_t *xpResHandle;

    configASSERT(pvArgs != NULL);

    xpResHandle = (xSMAppResources_t *)pvArgs;

    do
    {
	/* Configure Interrupt */
	prvPowerbtnEnable();

	/* wait for event */
	xBits = xEventGroupWaitBits(prvPowerBtnEventHandleInt,
		powerbtnPRESS_DETECT, pdTRUE, pdFALSE, portMAX_DELAY);
	if((xBits & powerbtnPRESS_DETECT) != powerbtnPRESS_DETECT)
	{
	    /* should happen only on portMAX_DELAY timeout !*/
	    continue;
	}
	while(1)
	{
	    /* Read GIO
	     * GIO HIGH: Button released
	     * GIO LOW:  Button still pressed
	     */

	    /* button press detected */
	    /* READ GIO & get value */
	    if (mml_gpio_pin_input(powerbtnGIO_GROUP, powerbtnPWR_BTN_GIO,
		    &xInBuf) == NO_ERROR)
	    {
		/* MML_GPIO_OUT_LOGIC_ONE:  BTN released
		 * MML_GPIO_OUT_LOGIC_ZERO: BTN pressed
		 */
		if (xInBuf == MML_GPIO_OUT_LOGIC_ONE)
		{
		    /* check hold time */
		    if(ulHoldTime >= powerbtnLONG_PRESS_TIME)
		    {
			/* post long press event */
			ulHoldTime = 0;
			xEventGroupSetBits( xpResHandle->xEventGroupUserModeApp, eventsKEY_PRESS_LONG );
			break;
		    }
		    else
		    {
			if (ulHoldTime == 0)
			{
			    /*Send event for pin entry Task */
			   xEventGroupSetBits( xpResHandle->xEventGroupPinEntry,
				   pinentryKEY_PRESS );
			}
			/* post short press event */
			ulHoldTime = 0 ;
			xEventGroupSetBits( xpResHandle->xEventGroupUserModeApp, eventsKEY_PRESS_SHORT );
			break;
		    }
		}
		else
		{
		    /* Send event for pin entry Task */
		    xEventGroupSetBits( xpResHandle->xEventGroupPinEntry,
			    pinentryKEY_PRESS );
		    if(ulHoldTime >= powerbtnLONG_PRESS_TIME)
		    {
			ulHoldTime = 0;
			/* still holding post long press event */
			xEventGroupSetBits( xpResHandle->xEventGroupUserModeApp, eventsKEY_PRESS_LONG );
			/* don't break till released */
			ulIsBtnReleased = pdFALSE;
			while(ulIsBtnReleased == pdFALSE)
			{
			    /* Send event for pin entry Task */
			    xEventGroupSetBits( xpResHandle->xEventGroupPinEntry, pinentryKEY_PRESS );
			    if (mml_gpio_pin_input(powerbtnGIO_GROUP,
				    powerbtnPWR_BTN_GIO, &xInBuf) == NO_ERROR)
			    {
				if(xInBuf == MML_GPIO_OUT_LOGIC_ONE)
				{
				    /* button released */
				    ulIsBtnReleased = pdTRUE;
				}
				vTaskDelay(powerbtnTASK_SLEEP_TIME);
			    }
			    else
			    {
				//should never happen. But now assuming key released !
				break;
			    }
			}
			/* Break inner while loop */
			break;
		    }
		    ulHoldTime += powerbtnTASK_SLEEP_TIME;
		    vTaskDelay(powerbtnTASK_SLEEP_TIME);
		}
	    }
	    else
	    {
		/* it should never happen */
		debugERROR_PRINT("Failed to READ GIO !!");
		break;
	    }
	}
	/* continue to block on keypress event */
    }while(1);
}
/*---------------------------------------------------------------------------*/
