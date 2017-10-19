/**===========================================================================
 * @file prng.h
 *
 * @brief This file contains APIs for generating random numbers using
 * a given seed.
 *
 * @author aakash.sarkar@design-shift.com
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

#include <errors.h>
#ifdef __cplusplus
extern "C" {
#endif

/**@brief This function will be used internally by cyassl library
 *        to read seed given by the user
 *
 * @param ulCount for indexing purposes
 *
 * @return Byte returns the byte of seed at the index given in the argument
 */
uint8_t ucPrngGetSeedByte(uint32_t ulCount);

/**@brief This function will accept seed from the user and initializes
 * 	   the RNG library to generate random numbers.
 *
 * @param puCseed give the address of your seed array here.
 * @param xSize give size of your seed array in bytes here.
 *
 * @return NO_ERROR Success
 * @return other error
 *
 */
int32_t lPrngReadSeed(uint8_t* pucSeed, size_t xSize);

/**@brief This function will generate a random number for the seed given
 *        by user in vPrngReadSeed() function.
 *
 * @param xSize give here the size of random number to be generated.
 * @param pucBlock this is where the random number will be generated.
 * 	    	   pass your random number buffer to this parameter.
 *
 * @return NO_ERROR Success
 * @return other error
 *
 */
int32_t lPrngGenerateRandom(uint8_t* pucBlock, size_t xSize);


#ifdef __cplusplus
}  /* extern "C" */
#endif
