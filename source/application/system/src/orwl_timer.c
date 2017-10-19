/**===========================================================================
 * @file orwl_timer.c
 *
 * @brief This file contains timer related definition used by the application.
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

#include <orwl_timer.h>

/* Global structure to store the used timer id details */
static timerUsedList_t xTimerIdList;

extern xSMAppResources_t xgResource;

/**
 * @brief This function is the Le timeout handler.
 *
 * This handler reset the LE timer and send one event to application task to
 * update the BLE seed.
 *
 * @param None
 *
 * @return void
 */
static void prvLe_Timeout_Handler( void );

/**
 * @brief This function is the 23:59:00 hrs proximity timeout handler.
 *
 * This handler send one event to application task to goto SuC-ST8 for re
 * authentication.
 *
 * @param None
 *
 * @return void
 */
static void prvProximity_Timeout_Handler( void );
/*----------------------------------------------------------------------------*/

static void prvLe_Timeout_Handler( void )
{
	BaseType_t xHigherPriorityTaskWoken, xResult;
	if( lTimerClose( timerLE_TIMER_ID ))
	{
		debugERROR_PRINT(" Failed to close LE TIMER in ISR");
		vDisplayClearScreenNONRTOS(dispWELCOME_IMGLTX,dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
		vDisplaySetTextPosNONRTOS( dispWELCOME_STR_XCOR,
					    dispWELCOME_STR_YCOR);
		vDisplayTextNONRTOS((const int8_t *)uiERROR_MSG);
		while(1);
	}
	xHigherPriorityTaskWoken = pdFALSE;
	xResult = xEventGroupSetBitsFromISR(xgResource.xEventGroupUserModeApp,
		eventsLE_TIMER_OUT, &xHigherPriorityTaskWoken );
	if( xResult == pdPASS )
	{
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
	if ( lTimerLeTmrInit())
	{
		debugERROR_PRINT("Failed to re-init LE timer in Le timeout"
			"handler");
		vDisplayClearScreenNONRTOS(dispWELCOME_IMGLTX,dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
		vDisplaySetTextPosNONRTOS( dispWELCOME_STR_XCOR,
					    dispWELCOME_STR_YCOR);
		vDisplayTextNONRTOS((const int8_t *)uiERROR_MSG);
		while(1);
	}
}
/*----------------------------------------------------------------------------*/

static void prvProximity_Timeout_Handler( void )
{
	BaseType_t xHigherPriorityTaskWoken, xResult;
	if( lTimerClose( timerPROXIMITY_TIMER_ID ))
	{
		debugERROR_PRINT(" Failed to close PROXIMITY TIMER in ISR");
		while(1);
	}
	xHigherPriorityTaskWoken = pdFALSE;
	xResult = xEventGroupSetBitsFromISR(xgResource.xEventGroupUserModeApp,
		eventsPROXIMITY_TIMEOUT, &xHigherPriorityTaskWoken );
	if( xResult == pdPASS )
	{
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}
/*----------------------------------------------------------------------------*/

int32_t lTimerLeTmrInit( void )
{
	mml_tmr_config_t xConfig;

	xConfig.timeout = timerLE_TIMEOUT;
	xConfig.count = 1;
	xConfig.pwm_value = 0;
	xConfig.clock = MML_TMR_PRES_DIV_64;
	xConfig.mode = MML_TMR_MODE_ONE_SHOT;
	xConfig.polarity = MML_TMR_POLARITY_LOW;
	xConfig.handler = prvLe_Timeout_Handler;

	return lTimerInit( &xConfig , timerLE_TIMER_ID );
}
/*----------------------------------------------------------------------------*/

int32_t lTimerProximityTimeoutTmrInit( void )
{
	mml_tmr_config_t xConfig;

	xConfig.timeout = timerPROXIMITY_TIMEOUT;
	xConfig.count = 1;
	xConfig.pwm_value = 0;
	xConfig.clock = MML_TMR_PRES_DIV_4096;
	xConfig.mode = MML_TMR_MODE_ONE_SHOT;
	xConfig.polarity = MML_TMR_POLARITY_LOW;
	xConfig.handler = prvProximity_Timeout_Handler;

	return lTimerInit( &xConfig , timerPROXIMITY_TIMER_ID );
}
/*----------------------------------------------------------------------------*/

int32_t lTimerRead( mml_tmr_id_t eTimer_id , uint32_t * pulCountValue )
{
	return mml_tmr_read(eTimer_id, pulCountValue );
}
/*----------------------------------------------------------------------------*/

int32_t lTimerInit( mml_tmr_config_t *pxConfig, mml_tmr_id_t eTimerId )
{
	configASSERT(pxConfig!=NULL);
	int32_t lRet = NO_ERROR;

	if(eTimerId > MML_TMR_DEV_MAX)
	{
		debugERROR_PRINT(" Invalid parameter received ");
		return COMMON_ERR_INVAL;
	}
	if(( xTimerIdList.ulUsedTimerList )&( 0x01<<eTimerId ))
	{
		debugERROR_PRINT(" Timer ID %d already in use.", eTimerId);
		return COMMON_ERR_UNKNOWN;
	}
	else
	{
		xTimerIdList.ulUsedTimerList |= ( 0x01<<eTimerId );
	}
	/* Initiate Timer */
	lRet = mml_tmr_init( eTimerId, pxConfig );
	if( lRet )
	{
		debugERROR_PRINT("Error while initializing timer ID %d with"
			" error code %d", eTimerId, lRet);
		return lRet;
	}
	/** Once initialized, enable timer's interruption ... */
	lRet = mml_tmr_interrupt_enable( eTimerId );
	if( lRet )
	{
		debugERROR_PRINT("Failed to enable timer interrupt for timer ID"
			" %d with error code %d", eTimerId, lRet);
		return lRet;
	}
	/* Enable timer itself */
	lRet = mml_tmr_enable( eTimerId );
	return lRet;
}
/*----------------------------------------------------------------------------*/

int32_t lTimerClose( mml_tmr_id_t eTimerId )
{
	int32_t lRet = NO_ERROR;
	mml_tmr_interrupt_clear( eTimerId );

	lRet = mml_tmr_disable( eTimerId );
	if(lRet)
	{
		debugERROR_PRINT(" Error while disabling timer ID : %d with"
			" error code %d",eTimerId, lRet);
		return lRet;
	}
	lRet = mml_tmr_close( eTimerId );
	if(lRet)
	{
		debugERROR_PRINT(" Error while closing timer ID : %d with"
			"error code %d", eTimerId, lRet);
		return lRet;
	}
	xTimerIdList.ulUsedTimerList &= (~(0x01<<eTimerId));
	return NO_ERROR;
}
/*----------------------------------------------------------------------------*/
