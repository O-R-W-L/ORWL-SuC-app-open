/**===========================================================================
 * @file tamper.c
 *
 * @brief This file contains function definition for tamper mode.
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

/* RTOS includes */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

/* Global includes */
#include <Init.h>
#include <errors.h>
#include <debug.h>
#include <stdint.h>
#include <stdlib.h>
#include <systemRes.h>
#include <orwl_disp_interface.h>
#include <intel_suc_comm.h>
#include <orwl_err.h>
#include <usermode.h>
#include <orwl_gpio.h>
#include <intel_SuC_Comm.h>
#include <reset.h>

/* Local includes */
#include <tamper.h>

/* Function definition */

void vTamperTamperModeTask( void *pvArg )
{
    /* thread resources */
    xSMAppResources_t *pxResHandle;
    /* Intel user data */
    IntelUserData_t xIntelUserData;
    /* ACK the command received */
    uint8_t ucACK = SUC_WRITE_STATUS_SUCCESS;

    /* For displaying tamper image on OLED */
    vInitDisplayTamperScreen();

    /* Check if the parameter is not NULL */
    if( pvArg == NULL )
    {
	debugERROR_PRINT("Invalid argument received for tamper mdode task \n");
	while(1);
    }
    /* If the argument is not NULL, assign to local variable */
    pxResHandle = (xSMAppResources_t *)pvArg;

    /* POwer on the Intel CPU, so that it can take the tamper logs. */
    vSystemResPowerOn_IntelCpu();

    /* Delay of 30 seconds, so that Intel booting is completed */
    vTaskDelay(usermodeDELAY_SEC_30);

    /* Once the Intel reads product cycle, it takes the tamper log and then it
     * will shutdown. Keep probing for the Intel shutdown.
     */
    while(1)
    {
	/* Check for Intel Command */
	if (xQueueReceive(pxResHandle->xSucBiosSendQueue, &xIntelUserData,
		usermodeDELAY_SEC_1) == pdTRUE)
	{
	    /* We have received the command from Intel, check if Intel has
	     * sent shutdown command.
	     */
	    if ((xIntelUserData.ucCommand == INTEL_DEV_ACT)
		    && (xIntelUserData.ucSubCommand == INTEL_DEV_STATE_SHT_DWN))
	    {
		/* We have received the device state from Intel ACK the same */
		if(xQueueSend(pxResHandle->xSucBiosReceiveQueue, &ucACK, portMAX_DELAY) != pdTRUE)
		{
		    debugERROR_PRINT(" ERROR in Queue data send");
		    while(1);
		}
		/* Intel has send shutdown command, poll GPIO to check if Intel
		 * has shutdown.
		 */
		while(1)
		{
		    /* Poll every 1 second */
		    vTaskDelay(usermodeDELAY_SEC_1);
		    /* Read Intel shutdown GPIO */
		    if ( ulSystemResPowerState_Intel() == gpioGIO_LOW )
		    {
			debugPRINT_APP(" Intel has ShutDown \n");
			/* Restart the ORWL device */
			resetSYSTEM_RESET;
			while(1);
		    } /* End of if */
		}
	    }
	    else
	    {
		/* Got the command from Intel, but its not device state ACK failure */
		ucACK = SUC_WRITE_STATUS_FAIL_INVALID;
		/* We have received the command from Intel ack the same */
		if(xQueueSend(pxResHandle->xSucBiosReceiveQueue, &ucACK, portMAX_DELAY) != pdTRUE)
		{
		    debugERROR_PRINT(" ERROR in Queue data send");
		    while(1);
		}
	    }
	}
    } /* End of while(1) */
}
