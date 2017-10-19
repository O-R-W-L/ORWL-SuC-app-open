/**===========================================================================
 * @file mpuinterface.c
 *
 * @brief This application is used for tap and motion detection.
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

/*---------------------------------------------------------------------------*/
/*Global Includes*/
#include <stdint.h>
#include <stddef.h>
#include <errors.h>

/* Application Includes*/
#include <mpuinterface.h>
#include <mpuapp.h>
#include <debug.h>
#include <inv_mpu.h>
#include <usermode.h>

#define mpuappBITMOTINTEN	(0x40)  /**<Motion interrupt mask*/

/*---------------------------------------------------------------------------*/

int32_t lMpuinterfaceIntializeMpu(void)
{
    return lMpuappmain();
}
/*---------------------------------------------------------------------------*/

int32_t lMpuinterfaceEnableDmp(void)
{
    return mpu_set_dmp_state(1);
}
/*---------------------------------------------------------------------------*/

int32_t lMpuinterfaceDisableDmp(void)
{
    return mpu_set_dmp_state(0);
}
/*---------------------------------------------------------------------------*/

int32_t lMpuinterfaceEnableMotionDetection(void)
{
    return lMpuappEnableMotion();
}
/*---------------------------------------------------------------------------*/

int32_t lMpuinterfaceDisableMotionDetection(void)
{
    return lMpuappDisableMotion();
}
/*---------------------------------------------------------------------------*/

BaseType_t xMpuinterfaceIsMotionDetected( void )
{
    int16_t sIntStatus = 0;

    mpu_get_int_status(&sIntStatus);
    if (sIntStatus & mpuappBITMOTINTEN)
    {
	debugPRINT("MOTION Detected !!");
	return pdTRUE;
    }
    return pdFALSE;
}
/*---------------------------------------------------------------------------*/

#ifndef ENABLE_MPU_GIO_INTERRUPT
extern SemaphoreHandle_t prvI2CSemaphore;
void vMpuinterfaceReadFifoTsk(void *vArgs)
{
    xSMAppResources_t *pxRes;
    configASSERT(vArgs != NULL);
    pxRes = (xSMAppResources_t *)vArgs;
	while(1)
	{
	    /* acquire semaphore: Wait till you acquire it*/
	    if( xSemaphoreTake( pxRes->prvI2CSemaphore, portMAX_DELAY  ) == pdTRUE )
	    {

		/* read FIFO */
		vMpuappGyroDataReadyCb();
		 xSemaphoreGive( pxRes->prvI2CSemaphore );
	    }
		/* sleep 100ms to allow others to run */
		vTaskDelay(mpuinterfaceFIFOREAD_DELAY);

	}
	vTaskDelete( NULL );
}
#endif
/*---------------------------------------------------------------------------*/


