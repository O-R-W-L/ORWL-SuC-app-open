/**===========================================================================
 * @file oob.h
 *
 * @brief This file contains routines and definition for ORWL OOB mode
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

#ifndef _INCLUDE_OOB_H_
#define _INCLUDE_OOB_H_

/* event group include */
#include <event_groups.h>
#include <queue.h>

/* local includes */
#include <usermode.h>
#include <access_keys.h>

/* Global Include */
#include <stdint.h>

/** Buffer to receive pin on event */
typedef struct XTAP_EVENT_DATA
{
    uint8_t ucPin[keysPIN_LEN];	/**<Password buffer */
    int32_t lStatus;	/**<Status Timeout */
}TapEventData_t;

typedef xSMAppResources_t xSMUOobModeResources_t;
/**
 * @brief enum indicates all the state index number.
 * Each state is provided an index starting from 0 till 'eSTATE_SUSC_ST_MAX'
 * for identification.
 */
typedef enum xORWL_SUC_OOB_STATES
{
	eSTATE_CHECK_PIN_STAT = 0,	/**< read flash and check if pins are set or not */
	eSTATE_VERIFY_DEFAULT_PIN,	/**< accept pin from user and verify the default pin */
	eSTATE_SET_USER_PIN,		/**< accept pin from user and store it */
	eSTATE_VERIFY_USER_PIN,		/**< accept pin from user and verify the user pin */
	eSTATE_OOB_UNLOCKED,		/**< accept pin from user and verify the user pin */
	eSTATE_OOB_ERROR,		/**< Error state in OOB mode */
	eSTATE_OOB_MAX,			/**< Error state in OOB mode */
} eSuCOobStates;

/** Integer to ascii conversion range MACRO*/
#define oobINTEGER_MIN_RANGE	(0)
#define oobINTEGER_MAX_RANGE	(9)
#define oobALPABHET_MIN_RANGE	(0xA)
#define oobALPHABET_MAX_RANGE	(0xF)
/** Right shift random number to get Msb 4 bits*/
#define oobROT_NIBBLE_SHIFT	(4)
/** Mask to get 4 bit value*/
#define oobMASK			(0xf)
/**Ascii character*/
#define oobASCII_A		('A' - 10)
#define oobASCII_0		('0')

/**Tap time out for Hash acceptance */
#define oobTAP_WAIT_TIMEOUT_SECS	(300000)
/**Time out for keyfob association*/
#define oobKEYFOB_ASSOC_TIMEOUT		(120000)

/**
 * Size of HASH string in OLED format.
 */
#define oobHASH_LEN_OLED_FORMAT		(86)

/**
 * We need to have 4 digit grouping on OLED of HASH.
 */
#define oob4DIGIT_GROUP			(4)

/* OOB Api's */
/**
 * @brief This function checks if USER has configured the custom PIN
 * for the ORWL or not.
 *
 * @return 	0 if PIN is Set
 * 			eORWL_ERROR_USER_PIN if PIN not set
 *			error code on failure
 */
uint32_t ulOobIsUserPINSet( void );

/**
 * @brief This function displays HASH on OLED
 *
 *@param *pucHashBuf buffer containing the unique Hash value for the device.
 	 	  Hash must be terminated with '\0'
 *
 * @return	0 on success
 * 			eORWL_ERROR_USER_PIN
 */
uint32_t ulOobDisplayHash( uint8_t *pucHashBuf  );

/**
 * @brief This function waits for the tap event to confirm HASH
 *
 * @return 	0 on tap detection
 * 			eORWL_ERROR_OOB_HASH_ERR on timeout
 */
uint32_t ulOobWaitForHashConfirmation( void );

/**
 * @brief This function waits for a first KeyFob association
 * First keyFob will be admin keyFob by default
 *
 * @return 	0 on association
 * 			eORWL_ERROR_OOB_ASSOCIATION_ERR on timeout
 */
uint32_t ulOobAdminKeyFobAssociation( void );

/**
 *@brief This function initializes all the global resources required in
 *	oob mode operation of SuC.
 *
 *	This includes initialization of event handler and queues.
 *
 * @param *pvRes oob mode state resource handler pointer
 *
 * @return 0 on success
 *           error code on failure.
 */
uint32_t ulOobModeInitGlobalResources(xSMUOobModeResources_t *pxRes);

/**
 * @brief This is OOB mode task function which will be running in by default in
 * Out of box mode of ORWL.
 *
 * @return void
 */
void vOobModeTask(void *pvArg);

#endif //_INCLUDE_OOB_H_
