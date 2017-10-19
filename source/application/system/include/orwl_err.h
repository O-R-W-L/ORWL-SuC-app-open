/**===========================================================================
 * @file orwl_err.h
 *
 * @brief This file defines all the possible errors at ORWL application layer
 * except those errors defined by COBRA stack/NFC stack/Display stack
 * and Mems stack
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
 ============================================================================
 *
 */
#ifndef _INCLUDE_ORWL_ERR_
#define _INCLUDE_ORWL_ERR_
#define errORWL_APPLICATION_ERR_CODE			0x06	/**< ORWL application base error code */
#define errORWL_APPLICATION_BASE_ERR_SHIFT		0xF		/**< ORWL application base err shift */
#define errORWL_APPLICATION_ERR_BASE_VAL		(errORWL_APPLICATION_ERR_CODE << errORWL_APPLICATION_BASE_ERR_SHIFT) /**< ORWL application error base value */

typedef enum xORWL_ERROR
{
	eORWL_ERROR_INVALID_REQUEST = errORWL_APPLICATION_ERR_BASE_VAL,
	eORWL_ERROR_USER_PIN,
	eORWL_ERROR_OOB_HASH_ERR,
	eORWL_ERROR_OOB_ASSOCIATION_ERR,
	ORWL_ERROR_USER,
	eORWL_ERROR_DEVICE_TAMPERED,
	eORWL_ERROR_NVSRAM_INVALID_ADDRESS,
	eORWL_ERROR_NVSRAM_INVALID_SIZE,
	eORWL_ERROR_NVSRAM_NOT_ACCESSIBLE,
	eORWL_ERROR_NVSRAM_INVALID_HEADER,
	eORWL_INVALID_ACTION,
}xOrwlError;

#endif //_INCLUDE_ORWL_ERR_
