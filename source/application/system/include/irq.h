/**===========================================================================
 * @file irq.h
 *
 * @brief This file contains prototype for interrupt routines.
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

#ifndef IRQ_H_
#define IRQ_H_

/* driver includes */
#include <mml_gpio_regs.h>
#include <mml_gpio.h>
#include <mml_gpio_regs.h>
#include <mml_intc_regs.h>
#include <mml_intc.h>
#include <stdint.h>
#include <errors.h>

#define irqNBITS		(1)	/**<Number of bits*/

typedef struct irqVectorHadlerTable
{
	void (*irqFuncPtr)();
}irqHandler_t;

/** @brief lIrqSetup set interrupt mode, polarity and
 *  	  enables the interrupt for given pin.
 *
 * @param xDevId 	device identifier.
 * @param ulPinNo	gpio pin number.
 * @param ulMode 	Interrupt mode edge or level.
 * @param ulPol		Interrupt polarity.
 * @param vFuncPtr	Function pointer of ISR hander.
 *
 * @return 0 on success and error code on failure.
 */
int32_t lIrqSetup( mml_gpio_id_t xDevId, uint32_t ulPinNo, uint32_t ulMode,
		uint32_t ulPol, void (*vFuncptr)());

/** @brief iIrqEnableInterrupt enables the interrupt for
 * 	  a given gpio pin.
 *
 * @param  xDevId device identifier.
 * @param  ulPinNo gpio pin number.
 *
 * @return 0 on success and error code on failure.
 */
int32_t lIrqEnableInterrupt(mml_gpio_id_t xDevId, uint32_t ulPinNo);

/** @brief iIrqDisable disables the interrupt for
 * 	  a given gpio pin.
 *
 * @param  xDevId device identifier.
 * @param  ulPinNo gpio pin number.
 *
 * @return 0 on success and error code on failure.
 */
int32_t lIrqDisable(mml_gpio_id_t xDevId , uint32_t ulPinNo);

/** @brief vIrqDev0CallbackISR interrupt service routine
 * 	   for gpio port 0.
 *
 * @return void.
 */
void vIrqDev0CallbackISR(void);

/** @brief vIrqDev1CallbackISR interrupt service routine
 * 	   for gpio port 1.
 *
 * @return void.
 */
void vIrqDev1CallbackISR(void);

/** @brief vIrqDev2CallbackISR interrupt service routine
 * 	   for gpio port 2.
 *
 * @return void.
 */
void vIrqDev2CallbackISR(void);

/** @brief iIrqInit register the irq handler for
 * 	   for gpio port 0, 1, 2.
 *
 * @return 0 on success and error code on failure.
 */
int32_t lIrqInit(void);

/** @brief IrqClearInterrupt clears the interrupt for
 * 	  a given gpio pin.
 *
 * @param  xDevId device identifier.
 * @param  ulPinNo gpio pin number.
 *
 * @return 0 on success and error code on failure.
 */
int32_t lIrqClearInterrupt(mml_gpio_id_t xDevId, uint32_t ulPinNo);
#endif
