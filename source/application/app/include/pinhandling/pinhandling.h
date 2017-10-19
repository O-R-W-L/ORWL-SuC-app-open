/**===========================================================================
 * @file pinhandling.h
 *
 * @brief This file contains routines and definition for PIN handling routines.
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

#ifndef _INCLUDE_PINHANDLING_H_
#define _INCLUDE_PINHANDLING_H_

/* Global includes */
#include <stdint.h>
#include <oob.h>

/** Enum for type of pin entry */
typedef enum xPIN_ENTRY_TYPE
{
    /** For default pin entry */
    eDEFPINTYPE = 0,
    /** For custom pin entry */
    eCUSTPINTYPE,
    /** For user pin entry */
    eUSERPIN
}ePINTYPE;

/*macro definition */
/**
 * PIN retry count 4, from now on wards the delay will be added for each PIN entry.
 */
#define pinhandlingFOURTH_PIN_ENTRY		(4)

/**
 * Sixty seconds to wait for 4th entry.
 */
#define pinhandlingFOURTH_PIN_ENTRY_DELAY	(60)

/**
 * Multiply factor for delay time before each wron PIN entry.
 */
#define pinhandlingMULTIPLY_FACTOR_DELAY	(2)

/**
 * There is no delay till 3rd pin retry attempt.
 */
#define pinhandlingNODELAY_RETRYCOUNT		(3)

/**
 * Wait for 30 seconds before each RTC read
 */
#define pinhandlingWAIT_30_SECONDS		(30000)

/* Function declaration */
/**
 * @brief This function checks the PIN retry count and delay time. If the device
 * needs to wait for some delay time before entering the PIN entry.
 *
 * @param *pucPinBuf buffer containing the PIN. Pin must be terminated with '\0'
 *
 * @return error code.
 */
int32_t lPinhandlingCheckRetryDelay( void );

/**
 * @brief This function accepts the PIN entry from user and verify the PIN with previously
 * set custom PIN. This API must be called only if user PIN sets previously.
 *
 * @param *pusPinBuf buffer containing the PIN. Pin must be terminated with '\0'
 * @param *pvRes oob mode state resource handler pointer
 * @param ulEventToSend event to send either pinentryGET_PASSWORD/pinentryUSER_GET_PSWD
 *
 * @return error code.
 */
int32_t lPinhandlingAcceptPIN( uint8_t *pucPinBuf, xSMUOobModeResources_t *pxRes,
	uint32_t ulEventToSend);

/**
 * @brief This function is used to verify the USER/DEFAULT PIN entered from user.
 *
 * @param *pucPinBuf buffer containing the PIN. Pin must be terminated with '\0'
 *
 * @return error code.
 */
int32_t lPinhandlingVerifyPIN( uint8_t *pucPinBuf, ePINTYPE ePIN);

/**
 * @brief This function is used increment and update retry count.
 *
 *
 * @return error code.
 */
int32_t lPinhandlingIncrementRetryCount( void );

#endif /* _INCLUDE_PINHANDLING_H_ */
