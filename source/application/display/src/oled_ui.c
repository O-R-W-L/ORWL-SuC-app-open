/**===========================================================================
 * @file oled_ui.c
 *
 * @brief This file contains implementation of UI api's.
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
/* Application includes*/
#include <orwl_oled.h>
#include <orwl_disp_interface.h>
#include <pinentry.h>
#include <oled_ui.h>

/* RTOS includes*/
#include <FreeRTOS.h>
#include <task.h>
#include <portable.h>

void vUiDrawRectangle(void)
{
    vDisplayRectangle();
}
/* ---------------------------------------------------------------------------*/

void vUiDisplayCusrsor(uint8_t ucXPos, uint8_t ucYPos)
{
    vDisplaySetTextPos(ucXPos,ucYPos);
    vDisplayText((const int8_t *)"^");
}
/* ---------------------------------------------------------------------------*/

void vUiDisplayDigit(void)
{
    /* Set the x-coordinate to 3 and y-coordinate to 0
     * to display the number at top center.
     */
    vDisplaySetTextPos(uiXCORDINATE,uiYCORDINATE_0);
    vDisplayText((const int8_t *)uiDIGITS);
}
/* ---------------------------------------------------------------------------*/

void vUiClearCursor(uint8_t ucXPos, uint8_t ucYPos)
{
    vDisplaySetTextPos(ucXPos,ucYPos);
    vDisplayText((const int8_t *)" ");
}
/* ---------------------------------------------------------------------------*/

void vUiSetCoordAndDisplay(uint8_t ucXPos, uint8_t ucYPos, const int8_t *pcMsg)
{
    vDisplaySetTextPos(ucXPos,ucYPos);
    vDisplayText(pcMsg);
}
/* ---------------------------------------------------------------------------*/

void vUiMessageDisplay(const int8_t *pcMsg,uint8_t ucXcord, uint8_t ucYcord)
{
    vDisplayClearScreen(uiXCORDINATE_0,uiYCORDINATE_0
		,uiXCORDINATE_M,uiYCORDINATE_M, uiCLEAR_SCREEN);
    vUiSetCoordAndDisplay(ucXcord, ucYcord, pcMsg);
}
/* ---------------------------------------------------------------------------*/
