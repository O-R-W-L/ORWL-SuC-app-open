 /**===========================================================================
 * @file display.h
 *
 * @brief This file contains the declaration of some common API's needed by ORWL
 * software.
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
#ifndef _INCLUDE_DISPLAY_H_
#define _INCLUDE_DISPLAY_H_

/* global includes */
#include <stdint.h>
#include <orwl_disp_interface.h>

/**@brief for displaying image
*
* This function is used for displaying access denied image.
*
* @param lLtx is used to pass top left x coordinate of img.
* @param lLty is used to pass top left y coordinate of img.
* @param lRbx is used to pass bottom right x coordinate of img.
* @param lRby is used to pass bottom right y coordinate of img.
* @param xImageId Image to be displayed.
*
* @return COMMON_ERR_OUT_OF_RANGE if parameter passed is not correct.
*/
int32_t lDisplayImageOnOLED(int32_t lLtx, int32_t lLty, int32_t lRbx,
	int32_t lRby, eImageIndex xImageId);

/**@brief for displaying text
*
* This function is used for displaying text on the OLED
*
* @param cpStr is used to pass string to be displayed.
*
* @return void
*/
void vDisplayTextOnOLED( const int8_t *pcStr );

/**@brief Put character on OLED.
*
* This function is used for displaying char on OLED
*
* @param cpStr is used to pass string to be displayed.
*
* @return void
*/
int8_t cDisplayPutChar( uint8_t val );

/**@brief for clearing OLED screen.
*
* This function is used for clearing the required portion of the screen. This
* function should be called from NON RTOS context.
*
* @param lLtx is used to pass top left x coordinate of clearing portion.
* @param lLty is used to pass top left y coordinate of clearing portion.
* @param lRbx is used to pass bottom right x coordinate of clearing portion.
* @param lRby is used to pass bottom right y coordinate of clearing portion.
* @param sBackGroundClr is used to set background color.
*
* @return void
*/
void vDisplayClearScreenOnOLED(int32_t lLtx, int32_t lLty, int32_t lRbx,
	int32_t lRby, int16_t sBackGroundClr);

/**@brief for setting position of text
*
* This function is used for setting position of text by setting its
* x and y coordinate
*
* @param ucXPos is used to pass x coordinate for displaying text
* @param ucYPos is used to pass y coordinate for displaying text.
*
* @return void
*/
void vDisplaySetTextPosOnOLED( uint8_t ucXPos, uint8_t ucYPos );

/**@brief this function draws rectangle.
*
* @return void.
*/
void vDisplayRectangleOnOLED( void );
#endif /* _INCLUDE_DISPLAY_H_ */
