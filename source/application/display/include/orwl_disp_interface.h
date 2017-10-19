 /**===========================================================================
 * @file orwl_disp_interface.h
 *
 * @brief This file contains function declaration used for the
 * displaying image and text on OLED.
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
#ifndef INCLUDE_DISP_INTERFACE_H_
#define INCLUDE_DISP_INTERFACE_H_
/* Global includes */
#include <stdint.h>

#define dispWELCOME_IMGLTX		( 0 )
#define dispWELCOME_IMGLTY		( 0 )
#define dispWELCOME_IMGRBX		( 128 )
#define dispWELCOME_IMGRBY		( 128 )
#define dispMAX_VALUE			( 128 )
#define dispMIN_VALUE			( 0 )
#define dispWELCOME_STR_XCOR		( 0 )
#define dispWELCOME_STR_YCOR		( 35 )
#define dispADD_MSG_STR_YCOR		( 18 )
#define dispBACKGROUND_BLACK		( 0xFF )
#define dispBACKGROUND_WHITE		( 0x00 )

/** @brief enum to hold display Image indexes
 */
typedef enum xIMAGE_INDEX
{
    eDipsInt_ACCESS_DENIED = 0,	/**< Access denied Image index */
    eDipsInt_PROXIMITY_PROTECT,	/**<Proximity protect Image index */
    eDipsInt_KEY_ASSOCIATED,	/**<key associated  Image index */
    eDipsInt_NFC_AUTH_REQUEST,	/**<Present Key Image index */
    eDipsInt_FORCE_SHUTDOWN,	/**<power button Long press Image index */
    eDipsInt_ORWL_LOGO,		/**<ORWL LOGO  Image index */
    eDipsInt_UNLOCK,		/**<ORWL unlocked Image index */
    eDipsInt_TAMPER_DETECTION,	/**<Device tampered Image index */
    eDipsInt_PWR_BTN_PRESS,	/**<Power Button press index*/
    eDipsInt_PROXIMITY_LOCK	/**<Proximity Lock index*/
}eImageIndex;

/**@brief for displaying text
*
* This function is used for displaying text on the OLED
*
* @param cpStr is used to pass string to be displayed.
*
* @return void
*/
void vDisplayText( const int8_t *pcStr );

/**@brief for displaying text
*
* This function is used for displaying text on the OLED in the non rtos context.
*
* @param cpStr is used to pass string to be displayed.
*
* @return void
*/
void vDisplayTextNONRTOS( const int8_t *pcStr );

/**@brief for enabling display
*
* This function initialize the hardware, set view port and do alignment
* of text.
*
* @return void
*/
void vDisplayInterfaceEnable( void );

/**@brief for clearing viewport
*
* This function is used for clearing the required portion of the screen
*
* @param lLtx is used to pass top left x coordinate of clearing portion.
* @param lLty is used to pass top left y coordinate of clearing portion.
* @param lRbx is used to pass bottom right x coordinate of clearing portion.
* @param lRby is used to pass bottom right y coordinate of clearing portion.
* @param sBackGroundClr is used to set background color.
*
* @return void
*/
void vDisplayClearScreen(int32_t lLtx, int32_t lLty, int32_t lRbx, int32_t lRby,
	int16_t sBackGroundClr);

/**@brief for clearing viewport
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
void vDisplayClearScreenNONRTOS(int32_t lLtx, int32_t lLty, int32_t lRbx,
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
void vDisplaySetTextPos( uint8_t ucXPos, uint8_t ucYPos );

/**@brief for setting position of text
*
* This function is used for setting position of text by setting its
* x and y coordinate. This function should be called from NON RTOS context.
*
* @param ucXPos is used to pass x coordinate for displaying text
* @param ucYPos is used to pass y coordinate for displaying text.
*
* @return void
*/
void vDisplaySetTextPosNONRTOS( uint8_t ucXPos, uint8_t ucYPos );

/**@brief for displaying char
*
* This function is used for displaying char on OLED.
*
* @param ucChar is used to pass char to be displayed.
*
* @return zero in case of success and non zero in case of failure.
*/
uint8_t ucDisplayChar(uint8_t ucChar);

/**@brief for updating hardware
*
* This function is used for writing buffer on hardware.
*
* @return void
*/
void vDisplayHwUpdate( void );

/**@brief This function provides wrapper from displaying Full screen
 * Images on OLED. Caller can specify the Image to be displayed by specifying
 * the Image index number from the eImageIndex table.
 *
 * @param xImageId Image Index to be displayed
 *
 * @return NO_ERROR on success and error code on failure
 */
int32_t lDisplayImageFullScreen( eImageIndex xImageId );

/**@brief This function provides wrapper from displaying Full screen
 * Images on OLED. Caller can specify the Image to be displayed by specifying
 * the Image index number from the eImageIndex table.
 * This should be called from NON RTOS function.
 *
 * @param xImageId Image Index to be displayed
 *
 * @return NO_ERROR on success and error code on failure
 */
int32_t lDisplayImageFullScreenNONRTOS( eImageIndex xImageId );

/**@brief this function draws rectangle.
*
* @return void.
*/
void vDisplayRectangle( void );

#endif /* INCLUDE_DISP_INTERFACE_H_ */
