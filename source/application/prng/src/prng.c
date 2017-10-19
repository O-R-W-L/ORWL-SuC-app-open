/**===========================================================================
 * @file prng.c
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

#include <cyassl/ctaocrypt/settings.h>
#include <cyassl/ctaocrypt/random.h>
#include <cyassl/ctaocrypt/arc4.h>
#include <debug.h>
#include <prng.h>
#include <errors.h>
#define prngSEEDMAXSIZE    (32)             /**< Seed maximum size */

/* Global variables */
uint8_t ucSeed[ prngSEEDMAXSIZE ] = {0};    /* To store the current seed
					       given by the user*/
RNG xRng;			    /* random number generator instance */

uint8_t ucPrngGetSeedByte(uint32_t ulCount)
{
    if(ulCount >= 32)
    {
	return 0;
    }
    /* return the byte at location given in the argument */
    return ucSeed[ ulCount ];
}
/*-----------------------------------------------------------------------------*/

int32_t lPrngReadSeed(uint8_t* pucSeed, size_t xSize)
{
    uint32_t ulRet;
    if( xSize > prngSEEDMAXSIZE )
    {
	debugERROR_PRINT("\nseed cannot be greater than 32 bytes\n");
        return COMMON_ERR_INVAL;
    }

    if(pucSeed == NULL)
    {
	debugERROR_PRINT("\nseed invalid\n");
	return COMMON_ERR_NULL_PTR;
    }

    /* copy the seed given by user into the ucseed array */
    memcpy(ucSeed, pucSeed, xSize);

    /* If there is a current instance in use, remove it */
    memset(&xRng, 0, sizeof(xRng));

/* Initialize the RNG module. Seed will be passed internally to the module */
    ulRet = InitRng(&xRng);
    if (ulRet != NO_ERROR)
    {
        debugERROR_PRINT("\nunable to do init\n");
        return COMMON_ERR_NOT_INITIALIZED;
    }
    return NO_ERROR;
}
/*-----------------------------------------------------------------------------*/

int32_t lPrngGenerateRandom(uint8_t* pucBlock, size_t xSize)
{
    uint32_t ulRet;

    if(pucBlock == NULL)
    {
	debugERROR_PRINT("\nNULL pointer given as argument\n");
	return COMMON_ERR_NULL_PTR;
    }

    ulRet = RNG_GenerateBlock(&xRng, pucBlock, xSize);
    if (ulRet != NO_ERROR)
    {
	debugERROR_PRINT("\nunable to do generate random number\n");
	return COMMON_ERR_UNKNOWN;
    }
    return NO_ERROR;
}

