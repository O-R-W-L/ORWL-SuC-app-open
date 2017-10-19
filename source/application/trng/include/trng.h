/**===========================================================================
 * @file trng.h
 *
 * @brief This file defines the TRNG interface to access a True random number
 * generate by the TRNG module
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
 * ============================================================================
 *
 */

#ifndef _TRNG_H_
#define _TRNG_H_

/* Global includes */
#include <config.h>
#include <errors.h>

/* cobra mml includes */
#include <mml.h>
#include <mml_gcr.h>
#include <mml_trng_regs.h>
#include <mml_uart.h>
#include <cobra_defines.h>

#define trngMIN_TRNG_GEN_WORDS			(1)		/**< Defines minimum RNG data request support by the module in Bytes
 	 	 	 	 	 	 	 	 	 	 	 	 There is no max. length defined. However, on a single TRNGF Generate
 	 	 	 	 	 	 	 	 	 	 	 	 call maxim supports only 128-bit TRNG generation. Post that minimum 32
 	 	 	 	 	 	 	 	 	 	 	 	 clock cycle wait is expected before generating the next set of
 	 	 	 	 	 	 	 	 	 	 	 	 random numbers. User is expected to call the same API again after 32
 	 	 	 	 	 	 	 	 	 	 	 	 clock cycle delay if he need random number of length anything greater
 	 	 	 	 	 	 	 	 	 	 	 	 than 16Bytes */
#define	trngTRNG_SINGLE_SHOT_READ_LEN	(4)		/**< 4Words can be read in single shot from TRNG [16Bytes] */
#define	trngMAX_WAIT_TIME		(100000)  	/**< Max wait time defined for TRNG module to generate random number */

#define trngWORD_TO_BYTE_FACT		(4)		/* Factor to convert word to bytes */

/**
 * @brief This is an interface function between application and TRNG driver.
 * On successful execution, this function will generate True Random number and
 * populate the application Buffer with the Random number of given length.
 *
 * @param ulpRandNumBuf Data buffer to store the random number generated.
 * 						Buffer of sufficient size must be supplied by the caller
 * @param ulNumWords	Number of of data words requested by the caller
 * 						Minimum count is '1 WORD'[4Bytes].
 *
 * @return error number will be returned on failure
 *			On success returns NO_ERROR(0)
 *
 *
 */
uint32_t ulGenerateRandomNumber(uint32_t *pulRandNumBuf, uint32_t ulNumWords);

#endif /* _TRNG_H_ */
