/**===========================================================================
 * @file irq.c
 *
 * @brief This file is used for interrupt handling.
 *
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ============================================================================
 *
 */
#include <irq.h>
#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
/** Interrupt vector table*/
static irqHandler_t xIrqVectorTable[MML_GPIO_DEV_COUNT][MML_GPIO_BIT_RANGE_NB] = {NULL};
/**<Gpio pin number*/
static uint8_t ucIrqPin[MML_GPIO_DEV_COUNT][MML_GPIO_BIT_RANGE_NB];

/* Same implementation must be done for port 0 and 2 if required.*/
void vIrqDev1CallbackISR(void)
{
    int32_t lStatus;
    uint32_t ulIndex;

    lStatus = 0;
    mml_gpio_get_interrupt_status(MML_GPIO_DEV1,MML_GPIO_BIT_RANGE_MIN,
	    MML_GPIO_BIT_RANGE_MAX, (int *)&lStatus);
    for(ulIndex = 0; ulIndex < MML_GPIO_BIT_RANGE_NB ; ulIndex++)
    {
	if (ucIrqPin[MML_GPIO_DEV1][ulIndex] != 0xff)
	{
	    if((lStatus & (1 << ucIrqPin[MML_GPIO_DEV1][ulIndex])))
	    {
		xIrqVectorTable[MML_GPIO_DEV1][ucIrqPin[MML_GPIO_DEV1][ulIndex]].irqFuncPtr();
		mml_gpio_clear_interrupt(MML_GPIO_DEV1,
		ucIrqPin[MML_GPIO_DEV1][ulIndex], irqNBITS);
	    }
	    else
	    {
		continue;
	    }
	}
	else
	{
	    break;
	}
    }
}
/*---------------------------------------------------------------------------*/

int32_t lIrqInit(void)
{
    memset(ucIrqPin, -1, sizeof(ucIrqPin));

    /* Irq setup for port 0 and 2 must done in same way as port 1,if required*/
    return mml_intc_setup_irq(MML_INTNUM_GPIO1, MML_INTC_PRIO_15,
			    &vIrqDev1CallbackISR);
}
/*---------------------------------------------------------------------------*/

int32_t lIrqSetup(mml_gpio_id_t xDevId, uint32_t ulPinNo, uint32_t ulMode,
		uint32_t iPol, void (*vFuncPtr)(void))
{
    int32_t lRetVal;
    static uint8_t ucIndexPortZero = 0;
    static uint8_t ucIndexPortOne  = 0;
    static uint8_t ucInexPortTwo   = 0;

    if(xDevId > MML_GPIO_DEV_MAX ||
	    ulPinNo > MML_GPIO_BIT_RANGE_MAX ||
	    ucIndexPortZero > MML_GPIO_BIT_RANGE_MAX ||
	    ucIndexPortOne > MML_GPIO_BIT_RANGE_MAX ||
	    ucInexPortTwo > MML_GPIO_BIT_RANGE_MAX  )
    {
	return MML_GPIO_ERR_OUT_OF_RANGE;
    }
    if(vFuncPtr)
    {
	xIrqVectorTable[xDevId][ulPinNo].irqFuncPtr = vFuncPtr;
    }
    else
    {
	return COMMON_ERR_NULL_PTR;
    }

    /* Gpio port 0*/
    if ( xDevId == MML_GPIO_DEV0)
    {
	ucIrqPin[xDevId][ucIndexPortZero] = ulPinNo;
	ucIndexPortZero++;
    }
    /* Gpio port 1*/
    else if( xDevId == MML_GPIO_DEV1)
    {
	ucIrqPin[xDevId][ucIndexPortOne] = ulPinNo;
	ucIndexPortOne++;
    }
    /* Gpio port 2*/
    else if( xDevId == MML_GPIO_DEV2)
    {
	ucIrqPin[xDevId][ucInexPortTwo] = ulPinNo;
	ucInexPortTwo++;
    }
    lRetVal = mml_gpio_set_int_trigger_mode(xDevId, ulPinNo, ulMode);
    if ( lRetVal != NO_ERROR)
    {
	return lRetVal;
    }
    return mml_gpio_set_int_pol(xDevId, ulPinNo, iPol);
}
/*---------------------------------------------------------------------------*/

int32_t lIrqDisable(mml_gpio_id_t xDevId, uint32_t ulPinNo)
{
    return mml_gpio_disable_interrupt(xDevId, ulPinNo, irqNBITS);
}
/*---------------------------------------------------------------------------*/

int32_t lIrqEnableInterrupt(mml_gpio_id_t xDevId, uint32_t ulPinNo)
{
    return mml_gpio_enable_interrupt(xDevId, ulPinNo, irqNBITS);
}
/*---------------------------------------------------------------------------*/

int32_t lIrqClearInterrupt(mml_gpio_id_t xDevId, uint32_t ulPinNo)
{
    return mml_gpio_clear_interrupt(xDevId, ulPinNo, irqNBITS);
}
/*---------------------------------------------------------------------------*/
