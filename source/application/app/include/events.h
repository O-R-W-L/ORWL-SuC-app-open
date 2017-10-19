/**===========================================================================
 * @file events.h
 *
 * @brief This file contains all the events used by ORWL application
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ============================================================================
 *
 */
#ifndef _INCLUDE_EVENTS_H_
#define _INCLUDE_EVENTS_H_

#define eventsEVENT_SET_DELAY	500	/* 500ms delay for each event set, in case previous set fails */

/**
 *	@brief This enum reserves 1 bit each for the listed events interpreted by
 *	the user mode application task.
 */
typedef enum xBIT_SHIFTS {
	eBIT_SHIFT0 = 0, /**< No BIT SHIFT */
	eBIT_SHIFT1, /**< BIT SHIFT 1: */
	eBIT_SHIFT2, /**< BIT SHIFT 2: */
	eBIT_SHIFT3, /**< BIT SHIFT 3: */
	eBIT_SHIFT4, /**< BIT SHIFT 4: */
	eBIT_SHIFT5, /**< BIT SHIFT 5: */
	eBIT_SHIFT6, /**< BIT SHIFT 6: */
	eBIT_SHIFT7, /**< BIT SHIFT 7: */
	eBIT_SHIFT8, /**< BIT SHIFT 8: */
	eBIT_SHIFT9, /**< BIT SHIFT 9: */
	eBIT_SHIFT10, /**< BIT SHIFT 10: */
	eBIT_SHIFT11, /**< BIT SHIFT 11: */
	eBIT_SHIFT12, /**< BIT SHIFT 12: */
	eBIT_SHIFT13, /**< BIT SHIFT 13: */
	eBIT_SHIFT14, /**< BIT SHIFT 14: */
	eBIT_SHIFT15, /**< BIT SHIFT 15: */
	eBIT_SHIFT16, /**< BIT SHIFT 16: */
	eBIT_SHIFT17, /**< BIT SHIFT 17: */
	eBIT_SHIFT18, /**< BIT SHIFT 18: */
	eBIT_SHIFT19, /**< BIT SHIFT 19: */
	eBIT_SHIFT20, /**< BIT SHIFT 20: */
	eBIT_SHIFT21, /**< BIT SHIFT 21: */
	eBIT_SHIFT22, /**< BIT SHIFT 22: */
	eBIT_SHIFT23, /**< BIT SHIFT 23: */
	eBIT_SHIFT24, /**< BIT SHIFT 24: */
} xBitShift;

/* user mode event handler bit definitions
 * These events are interpreted by the user mode application task
 */
/* Bit 0 and 1 reserved for future */
#define eventsKEYFOB_NFC_AUTH_SUCCESS		(1<<eBIT_SHIFT2)	/**< NFC Authentication success event */
#define eventsKEYFOB_NFC_AUTH_FAILURE		(1<<eBIT_SHIFT3)	/**< NFC Authentication Failure event */
#define eventsKEYFOB_NFC_ASSOCIATION_SUCCESS	(1<<eBIT_SHIFT4)	/**< NFC association success event */
#define eventsKEYFOB_NFC_ASSOCIATION_FAILURE	(1<<eBIT_SHIFT5)	/**< NFC association Failure event */
#define eventsKEYFOB_NFC_ENTRY_FULL		(1<<eBIT_SHIFT6)	/**< NFC KeyFob Entry Full event */
#define eventsKEYFOB_NFC_DELETE_SUCCESS		(1<<eBIT_SHIFT7)	/**< NFC KeyFob delete success event */
#define eventsKEYFOB_NFC_DELETE_FAILURE		(1<<eBIT_SHIFT8)	/**< NFC KeyFob delete failure event */
#define eventsKEYFOB_ENTRY_DELETE_SUCCESS	(1<<eBIT_SHIFT9)	/**< NFC KeyFob Entry delete success event */
#define eventsKEYFOB_ENTRY_DELETE_FAILURE	(1<<eBIT_SHIFT10)	/**< NFC KeyFob Entry delete failure event */
#define eventsKEYFOB_DETECT_TIMEOUT		(1<<eBIT_SHIFT11)	/**< NFC KeyFob detection timeout events */
#define eventsKEYFOB_GET_SERIAL_SUCCESS		(1<<eBIT_SHIFT12)	/**< NFC KeyFob getting serial success events */
#define eventsKEYFOB_GET_SERIAL_FAILURE		(1<<eBIT_SHIFT13)	/**< NFC KeyFob getting serial failure events */
#define eventsKEYFOB_NFC_UNKNOWN_COMMAND	(1<<eBIT_SHIFT14)	/**< NFC Unknown Command event */
#define eventsKEY_PRESS_LONG			(1<<eBIT_SHIFT15)	/**< Long key press event */
#define eventsKEY_PRESS_SHORT			(1<<eBIT_SHIFT16)	/**< Short key press event */
#define eventsROT_OS_INSTALLATION_COMPLETE	(1<<eBIT_SHIFT17)	/**< OS installed */
#define eventsROT_OS_INSTALLATION_FAIL		(1<<eBIT_SHIFT18)	/**< OS installation failed */
#define eventsLE_TIMER_OUT			(1<<eBIT_SHIFT19)	/**< Le seed update timer event*/
#define eventsKEYFOB_STOP_NFC_SUCCESS		(1<<eBIT_SHIFT20)	/**< NFC STOP Success event */
#define eventsKEYFOB_FIRST_DETECT_SUCCESS	(1<<eBIT_SHIFT21)	/**< NFC First Authentication success event */
#define eventsPROXIMITY_TIMEOUT			(1<<eBIT_SHIFT22)	/**< 24 hrs Proximity timeout */
#define eventsKEYFOB_NOT_ADMIN			(1<<eBIT_SHIFT23)	/**< NFC Keyfob is not admin event */

/* user mode NFC task event handlers
 * These events interpreted by the NFC task
 */
#define eventsNFC_DETECT_KEYFOB			(1<<eBIT_SHIFT0)	/**< start NFC detection and authentication */
#define eventsNFC_ASSOCIATE_KEYFOB		(1<<eBIT_SHIFT1)	/**< start NFC key association with ORWL */
#define eventsNFC_DELETE_KEYFOB			(1<<eBIT_SHIFT2)	/**< delete the Keyfob details from the Keyfob secure element */
#define eventsNFC_DELETE_KEYFOB_ENTRY		(1<<eBIT_SHIFT3)	/**< delete the Keyfob details from the flash */
#define eventsNFC_GET_KEYFOB_SERIAL		(1<<eBIT_SHIFT4)	/**< Event to get the keyfob serial number */
#define eventsNFC_STOP_EXEC			(1<<eBIT_SHIFT5)	/**< Stop nfc execution */
#define eventsNFC_FIRST_DETECT			(1<<eBIT_SHIFT6)	/**< First nfc authentication */
#endif //_INCLUDE_EVENTS_H_
