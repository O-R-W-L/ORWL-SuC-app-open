/**===========================================================================
 * @file rot.h
 *
 * @brief This file contains routines and definition for ORWL ROT mode
 * This mode is also referenced as non tampered production mode
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

/* ROT mode API's */
#ifndef _INCLUDE_ROT_H_
#define _INCLUDE_ROT_H_

#define rotSECRET_PRINT_PREIOD		(3000) /** print secrets for every 3 secs */
/**
 * @brief enum indicates all the state index number.
 * Each state is provided an index starting from 0 till 'MAX'
 * for identification.
 */
typedef enum xORWL_SUC_ROT_STATES
{
	eSTATE_ROT_SET_SECRETS = 0,	/*< generate and store ORWL Unique Secrets */
	eSTATE_ROT_POWER_ON,		/*< power on state */
	eSTATE_ROT_COMPLETE,		/*< Rot completed */
	eSTATE_ROT_ERR,			/*< Error state */
	eSTATE_ROT_MAX,			/*< Max state */
} eSuCRotStates;

/**
 * @brief This function Reads SSD password stored on NVSRAM
 *
 *@param pucPassBuf buffer to hold password read
 *
 * @return NO_ERROR on success
 *			error code on failure
 */
uint32_t ulRotReadSSDPassword( uint8_t *pucPassBuf );

/**
 * @brief This function implements the ROT mode task
 *
 * @return void
 */
void vRotModeTask( void *pvArg );

/**
 * @brief This function encrypt the message
 *
 * @param pucMessage buffer to hold message to be encrypted
 * @param pucEncryptedMsg to hold encrypted message
 *
 * @return void
 */

void vRotRsaEncryption(uint8_t *pucMessage ,uint8_t *pucEncryptedMsg);

/**
 * @brief This function generates SSD password and store on NVSRAM
 *
 * @param pucBuf pointer to SSDEnckey buffer.
 * @param ulSSDKeyLen length of SSDKey (in Words).
 *
 * @return NO_ERROR on success
 *	   error code on failure
 */
uint32_t ulRotGenerateUserSSDPassword( uint32_t *pulBuf, uint32_t ulSSDKeyLen );
#endif //_INCLUDE_ROT_H_
