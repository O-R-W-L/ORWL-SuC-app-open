/**===========================================================================
 * @file pinentry.h
 *
 * @brief This file contains function prototype for pinentry.
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ============================================================================
 *
 */

#ifndef PINENTRY_H
#define PINENTRY_H

#define pinentryN_ELEMENTS	(10)	/**<Number of digits*/
#define pinentryPIN_LENGTH	(6)	/**<Pin length*/
#define pinentryNDIGITS		(2)	/**<Elements in display buffer*/
#define pinentrySTR_LEN		(2)	/**<Max length of digit*/
#define pinentryRAND_ELEM	(4)	/**<Random array elements*/
#define pinentryCOMP_SUC	(0)	/**<Pin compare success macro*/
#define pinentryCOMP_FAIL	(-1)    /**<Pin compare failure macro*/
/*Events to wait/set for pin-entry */
#define pinentryGET_TAP_EVENT	(1 << 0)/**<Event bit for Tap*/
#define pinentryGET_PASSWORD	(1 << 1)/**<Event bit for tap detection enable and read password */
#define pinentryKEY_PRESS	(1 << 2)/**<Key press event can be long/short*/
#define pinentryUSER_GET_PSWD	(1 << 3)/**<Event bit for pin entry and read password for user mode */

/** Text displayed on oled */
#define pinentryTEXT_ZERO	"0"
#define pinentryTEXT_ONE	"1"
#define pinentryTEXT_TWO	"2"
#define pinentryTEXT_THREE	"3"
#define pinentryTEXT_FOUR	"4"
#define pinentryTEXT_FIVE	"5"
#define pinentryTEXT_SIX	"6"
#define pinentryTEXT_SEVEN	"7"
#define pinentryTEXT_EIGHT	"8"
#define pinentryTEXT_NINE	"9"

/**<Right shift by 3 to get cursor position from random number*/
#define pinentryRIGHT_SHIFT	(4)
/*Mask the obtained random number to get 4bit number*/
#define pinentryMASK		(0xF)

/** enum to check which text to display */
typedef enum xDISPLAY_DIGIT
{
    ePINENRTY_ZERO = 0,
    ePINENRTY_ONE,
    ePINENRTY_TWO,
    ePINENRTY_THREE,
    ePINENRTY_FOUR,
    ePINENRTY_FIVE,
    ePINENRTY_SIX,
    ePINENRTY_SEVEN,
    ePINENRTY_EIGHT,
    ePINENRTY_NINE,
    ePINENTRY_TEN
}eDIGITS_t;

/** @brief This task displays pins on oled
 *         and on Tap event receives the user pin.
 *         Verifies user entered pin.
 *
 * @return void.
 */
void vPinentryTask(void *pvArg);
#endif
