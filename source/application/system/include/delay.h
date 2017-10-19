/**===========================================================================
 * @file delay.h
 *
 * @brief This file contains ORWL delay macros.
 *
 * @author priya.gokani@design-shift.com
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

#ifndef _INCLUDE_DELAY_H_
#define _INCLUDE_DELAY_H_
/** Driver includes */
#include <mml_intc.h>

/** Delay factor */
#define delayUS_FACT			108

/** for generating delay in micro second & must be used
 * before schedular starts */
#define delayMICRO_SEC(us_to_delay)					\
{									\
	int32_t iloop, idelay;						\
	cpsid(); 							\
	for (iloop = 0; iloop < us_to_delay; iloop++) 			\
	{								\
		for (idelay = 0; idelay < delayUS_FACT; idelay++) 	\
		{							\
			asm ("mov r0, r0"); 				\
		}							\
	}								\
	cpsie();							\
}


/** for generating delay in micro seconds. This delay macros doesn't
 * disables the interrupt and hence safe to use from both RTOS/NonRTOS
 * context
 */
#define delayuSEC(us_to_delay)						\
{									\
	int32_t iloop, idelay;						\
	for (iloop = 0; iloop < us_to_delay; iloop++) 			\
	{								\
		for (idelay = 0; idelay < delayUS_FACT; idelay++) 	\
		{							\
			asm ("mov r0, r0"); 				\
		}							\
	}								\
}
#endif /* _INCLUDE_DELAY_H_ */
