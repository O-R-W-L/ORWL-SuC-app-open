/**===========================================================================
 * @file orwl_disp_interface.c
 *
 * @brief This file is used for initialization of hardware and displaying text
 * on OLED.
 *
 * @author priya.gokani@design-shift.com
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
/* Application includes */
#include <orwl_oled.h>
#include <stdint.h>
#include <orwl_disp_interface.h>
#include <errors.h>
#include <delay.h>
#include <oled_ui.h>
#include <display.h>

/* freeRTOS headers */
#include <FreeRTOS.h>
#include <task.h>

/* FreeRTOS included*/
#include <FreeRTOS.h>
#include <task.h>
#include <portable.h>

/*---------------------------------------------------------------------------*/

void vDisplayText( const int8_t *pcStr )
{
	/* Enter critical section and suspend all task
	 * before updating OLED.Entering critical section
	 * will disable interrupt globally and suspending
	 * avoids task switching while updating OLED.This
	 * will avoid corruption of gbuf buffer.Once OLED
	 * is updated resume all the task and exit from
	 * critical section.
	 */
	taskENTER_CRITICAL();
	vTaskSuspendAll();
	/* Displaying text on OLED */
	vDisplayTextOnOLED((const int8_t *)pcStr);
	xTaskResumeAll();
	taskEXIT_CRITICAL();
}
/*---------------------------------------------------------------------------*/

void vDisplayClearScreen(int32_t lLtx, int32_t lLty, int32_t lRbx, int32_t lRby,
	int16_t sBackGroundClr)
{
	/* Enter critical section and suspend all task
	 * before updating OLED.Entering critical section
	 * will disable interrupt globally and suspending
	 * avoids task switching while updating OLED.This
	 * will avoid corruption of gbuf buffer.Once OLED
	 * is updated resume all the task and exit from
	 * critical section.
	 */
	taskENTER_CRITICAL();
	vTaskSuspendAll();
	vDisplayClearScreenOnOLED( lLtx, lLty, lRbx, lRby ,sBackGroundClr );
	vDisplayHwUpdate();
	xTaskResumeAll();
	taskEXIT_CRITICAL();
}
/*---------------------------------------------------------------------------*/

void vDisplaySetTextPos( uint8_t ucXPos, uint8_t ucYPos )
{
	/* Enter critical section and suspend all task
	 * before updating OLED.Entering critical section
	 * will disable interrupt globally and suspending
	 * avoids task switching while updating OLED.This
	 * will avoid corruption of gbuf buffer.Once OLED
	 * is updated resume all the task and exit from
	 * critical section.
	 */
	taskENTER_CRITICAL();
	vTaskSuspendAll();
	vDisplaySetTextPosOnOLED( ucXPos, ucYPos );
	xTaskResumeAll();
	taskEXIT_CRITICAL();
}
/*---------------------------------------------------------------------------*/

uint8_t ucDisplayChar(uint8_t ucChar)
{
	uint8_t ucResult;
	ucResult = cDisplayPutChar( ucChar );
	if(ucResult != 0 )
	{
	    return ucResult;
	}
	/* Enter critical section and suspend all task
	 * before updating OLED.Entering critical section
	 * will disable interrupt globally and suspending
	 * avoids task switching while updating OLED.This
	 * will avoid corruption of gbuf buffer.Once OLED
	 * is updated resume all the task and exit from
	 * critical section.
	 */
	taskENTER_CRITICAL();
	vTaskSuspendAll();
	vDisplayHwUpdate();
	return ucResult;
	xTaskResumeAll();
	taskEXIT_CRITICAL();
}

/*---------------------------------------------------------------------------*/

int32_t lDisplayImageFullScreen( eImageIndex xImageId )
{
    int32_t lResult = NO_ERROR;
    taskENTER_CRITICAL();

    switch(xImageId)
    {
    case eDipsInt_ACCESS_DENIED:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_ACCESS_DENIED);
	break;
    case eDipsInt_PROXIMITY_PROTECT:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_PROXIMITY_PROTECT);
	break;
    case eDipsInt_KEY_ASSOCIATED:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_KEY_ASSOCIATED);
	break;
    case eDipsInt_NFC_AUTH_REQUEST:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_NFC_AUTH_REQUEST);
	break;
    case eDipsInt_FORCE_SHUTDOWN:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_FORCE_SHUTDOWN);
	break;
    case eDipsInt_ORWL_LOGO:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_ORWL_LOGO);
	break;
    case eDipsInt_UNLOCK:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_UNLOCK);
	break;
    case eDipsInt_TAMPER_DETECTION:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_TAMPER_DETECTION);
	break;
    case eDipsInt_PWR_BTN_PRESS:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_PWR_BTN_PRESS);
	break;
    case eDipsInt_PROXIMITY_LOCK:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_PROXIMITY_LOCK);
	break;
    default:
	return COMMON_ERR_INVAL;
	break;
    }

    taskEXIT_CRITICAL();
    return lResult;
}
/*---------------------------------------------------------------------------*/

int32_t lDisplayImageFullScreenNONRTOS( eImageIndex xImageId )
{
    /* This is function will be called from NON RTOS context, so no need to enter
     * critical section and suspend task.
     */
    int32_t lResult = NO_ERROR;
    switch(xImageId)
    {
    case eDipsInt_ACCESS_DENIED:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_ACCESS_DENIED);
	break;
    case eDipsInt_PROXIMITY_PROTECT:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_PROXIMITY_PROTECT);
	break;
    case eDipsInt_KEY_ASSOCIATED:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_KEY_ASSOCIATED);
	break;
    case eDipsInt_NFC_AUTH_REQUEST:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_NFC_AUTH_REQUEST);
	break;
    case eDipsInt_FORCE_SHUTDOWN:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_FORCE_SHUTDOWN);
	break;
    case eDipsInt_ORWL_LOGO:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_ORWL_LOGO);
	break;
    case eDipsInt_UNLOCK:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_UNLOCK);
	break;
    case eDipsInt_TAMPER_DETECTION:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_TAMPER_DETECTION);
	break;
    case eDipsInt_PWR_BTN_PRESS:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_PWR_BTN_PRESS);
	break;
    case eDipsInt_PROXIMITY_LOCK:
	lResult = lDisplayImageOnOLED( dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
		dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, eDipsInt_PROXIMITY_LOCK);
	break;
    default:
	return COMMON_ERR_INVAL;
	break;
    }
    /* Return error code */
    return lResult;
}
/*---------------------------------------------------------------------------*/

void vDisplayRectangle(void)
{
	/* Enter critical section and suspend all task
	 * before updating OLED.Entering critical section
	 * will disable interrupt globally and suspending
	 * avoids task switching while updating OLED.This
	 * will avoid corruption of gbuf buffer.Once OLED
	 * is updated resume all the task and exit from
	 * critical section.
	 */
	taskENTER_CRITICAL();
	vTaskSuspendAll();
	vDisplayRectangleOnOLED();
	xTaskResumeAll();
	taskEXIT_CRITICAL();
}
