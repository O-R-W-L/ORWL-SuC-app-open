/**===========================================================================
 * @file oled_ui.h
 *
 * @brief This file contains function prototype for OLED UI.
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
#ifndef OLED_UI_H
#define OLED_UI_H
/* Global include */
#include<stdint.h>

/**Display digits */
#define uiDIGITS	"0123456789<"

/* "*" is displayed when password is accepted*/
#define uiDISP_STAR	"*"
/* To clear the Star*/
#define uiCLEAR_STAR	"      "

/** Digit display coordinate */
#define uiXCORDINATE_0	(0)
#define uiXCORDINATE	(3)
#define uiYCORDINATE_0	(0)
#define uiYCORDINATE_1	(1)
#define uiXCORDINATE_M	(127)
#define uiYCORDINATE_M	(127)
#define uiYCORDINATE_C	(35)

/**Coordinate to display star*/
#define uiXSTAR_COORD	(5)
#define uiYSTAR_COORD	(3)

/** To display digit at bottom */
#define uiPINDIS_XCORD	(39)
#define uiPINDIS_YCORD	(5)

/** Rectangle coordinates*/
#define uiRECT_TOP_XCORD	(1)
#define uiRECT_TOP_YCORD	(32)
#define uiRECT_BOT_XCORD	(127)
#define uiRECT_BOT_YCORD	(64)

/** Rectangle color white*/
#define uiBACKGROUND_CLR	(0xFF)

/**Display background color white*/
#define uiCLEAR_SCREEN		(0x00)

/*Cursor postion on display*/
#define uiCURSOR_POS_0		(3)
#define uiCURSOR_POS_1		(4)
#define uiCURSOR_POS_2		(5)
#define uiCURSOR_POS_3		(6)
#define uiCURSOR_POS_4		(7)
#define uiCURSOR_POS_5		(8)
#define uiCURSOR_POS_6		(9)
#define uiCURSOR_POS_7		(10)
#define uiCURSOR_POS_8		(11)
#define uiCURSOR_POS_9		(12)
#define uiCURSOR_POS_10		(13)

/* Pin entry retry count */
#define uiRETRY_COUNT		(10)

/*Delay after displaying message on OLED*/
#define uiONE_SEC_DELAY		(1000)
#define uiTWO_SEC_DELAY		(2000)
#define uiTHREE_SEC_DELAY	(3000)
#define uiEIGHT_SEC_DELAY	(8000)
#define uiONE_MIN_DELAY		(60000)
/** ORWL  messages */
#define uiPOWERON		"Welcome to ORWL"

#ifdef TAP_FEATURE
#define uiTAPMSG		"Please tap ORWL to enter PIN.."
#else
#define uiTAPMSG		" Please press    power button    to proceed"
#endif

#define uiPREDEF_PIN		"  Please enter    shipping PIN"
#define uiPREDEF_PIN_MATCH	"PIN verified"

#define uiPREDEF_PIN_FAIL	"  Verification     Failed!\n"
#define uiORWL_LOCK		"ORWL LOCKED !!!"
#define uiCUSTOM_PIN_MSG	" Set custom PIN"
#define uiREENTER_PIN		"Re-enter PIN"
#define uiUSER_INCORRECT_PIN	"PIN didn't match Please retry"
#define uiUSER_PIN_MATCH	"  Your custom     PIN is set"

#define uiUSER_SET_PIN		"Enter your PIN"
#define uiLOGIN_SUCCESS		"Login successful"
#define uiLOGIN_FAIL		"  Login failed\n"
#define uiWAIT_TILL_NEXT_TRY	"  Wait till next     PIN entry       is allowed"

#ifdef TAP_FEATURE
#define uiHASH_DISP		"VERIFY HASH     Tap to confirm"
#else
#define uiHASH_DISP		"VERIFY HASH     please press    power button    to confirm"
#endif

#define uiHASH_CONFIRM_MSG	" ORWL Received     untampered"

#define uiHASH_REJECT_MSG	" Hash acceptance   timeout..!!       Rebooting"

#define uiUSER_KEY_MSG		"  Pair Keyfob\n  to associate"

#define uiTIMEOUT_MSG		"   Timeout         Rebooting"

#define uiASSOCIATION_FAIL	"Pairing Failed - Retry by        presenting again"

#define uiASSOCIATION_SUC	"Associated"

#define uiCONFIG_SUC		"ORWL configured Rebooting"

#define uiTAMPER		"Tamper detected"

#define uiSELECTED		"selected"

/* 000000 is invalid password */
#define uiINVALID_PSWD		"Invalid password\ntry different   password"

/* ROT Mode error messages */
#define uiSSD_ADMIN_ERR		"ERR: Flash ssd\nadmin void"
#define uiNVSRAM_INIT_FAIL	"NVSRAM init fail"

/* User mode display messages */
/** Nfc Authenticated message */
#define uiNFC_AUTHENTICATED	"       NFC        Authenticated "

/** BT proximity checking message */
#define uiBT_PROXIMITY_CHECK	"  BT Proximity       Check      "

/** Nfc Not associated message */
#define uiNFC_NOT_ASSOCIATED	" Not Associated "

/** Device in motion message */
#define uiDEVICE_IN_MOTION	"Device In Motion"

/** KeyFob delete success message */
#define uiKEYFOB_DELETE_SUCCESS	" Delete Success "

/** KeyFob delete failure message */
#define uiKEYFOB_DELETE_FAILURE	" Delete Failure "

/** Present new keyfob message */
#define uiPRESENT_KEY		"   Present new      KeyFob for       pairing    "

/** Security check Time out message*/
#define uiTIME_OUT		"    Time out      Please Retry..  "

/** L1 or L2 security failed message */
#define uiSECURITY_CHECK_FAIL	" Security Check      Failed     "

/** Use Admin keyfob message */
#define uiUSE_ADMIN_KEYFOB	" Insufficient     Privilege.       Use Admin       Keyfob"

/** ORWL error message in user mode */
#define uiERROR_MSG		"   ORWL ERROR   "

/** Association in progress message*/
#define uiASSOCIATION_PROGRESS	" Association in     progress..  "

/** Max Password retry exceeded and owl locked message */
#define uiMAX_RETRY_EXCEEDED	"Pin Retry Exceed  ORWL LOCKED!"

/** USER PIN update is not supported from USER mode as of now */
#define uiUSER_PIN_NOT_SUPPORTED "  Feature not      supported"

/** Inform short button press on OLED */
#define uiSHORT_BUTTON_PRESS	 "  Short button        press"

/** @brief this function draws rectangle.
 *
 * @return void
 */
void vUiDrawRectangle(void);

/** @brief this function displays the digit
 *         on OLED.
 *
 *  @return void.
 */
void vUiDisplayDigit(void);

/** @brief this function displays cursor under
 *         the current pin number.
 *
 * @param ucXPos x-coordinate.
 * @param ucYPos y-coordinate.
 *
 * @return void
 */
void vUiDisplayCusrsor(uint8_t ucXPos, uint8_t ucYPos);

/** brief this function clear the cursor.
 *
 * @param ucXPos x-coordinate.
 * @param ucYPos y-coordinate.
 *
 * @return void
 */
void vUiClearCursor(uint8_t ucXPos, uint8_t ucYPos);

/** @brief this function set the x and y coordinate
 *         and display the message at the set position.
 *
 *  @param ucXPos x-coordinate.
 *  @param ucYPas y-coordinate.
 *  @param pcMsg message to display.
 *
 *  @return void
 */
void vUiSetCoordAndDisplay(uint8_t ucXPos, uint8_t ucYPos, const int8_t  *pcMsg);

/** @brief this function clears the screen
 *         and displays message on screen.
 *
 * @param ucXPos x-coordinate.
 * @param ucYPas y-coordinate.
 * @param pcMsg message to display.
 *
 * @return void
 */
void vUiMessageDisplay(const int8_t *pcMsg, uint8_t ucXcord, uint8_t ucYcord);
#endif
