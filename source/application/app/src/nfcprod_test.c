/**===========================================================================
 * @file nfcprod_test.c
 *
 * @brief This file contains the task to get the serial number of the keyfob.
 * This task is called for testing nfc in the production build only.
 *
 * @author viplav.roy@design-shift.com
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

#if ORWL_PRODUCTION_KEYFOB_SERIAL
#include <nfcprod_test.h>

void vNfcProdTestTask(void *pvArg)
{
	uint8_t ucTmp;
	uint32_t ulEventToSend = 0;
	uint32_t ulEventToWait = 0;
	EventBits_t uxBits = 0;
	const TickType_t xTicksToWait = portMAX_DELAY;
	NfcEventData_t xNfcData;
	xSMAppResources_t *pxResHandle;

	if( pvArg == NULL )
	{
		debugERROR_PRINT("\nFailed to create task, invalid pvArg \n" );
		return;
	}
	pxResHandle = (xSMAppResources_t *)pvArg;
	while(1)
	{
	    ulEventToSend = eventsNFC_GET_KEYFOB_SERIAL;
	    uxBits = xEventGroupSetBits( pxResHandle->xEventGroupNFC ,
			    ulEventToSend );
	    if((uxBits & eventsNFC_GET_KEYFOB_SERIAL) != eventsNFC_GET_KEYFOB_SERIAL)
	    {
		    debugERROR_PRINT("post event: "
			    "eventsNFC_GET_KEYFOB_SERIAL Failed, retry... ");
		    vTaskDelay( eventsEVENT_SET_DELAY );
	    }
	    else
	    {
		    debugPRINT_APP("post event: eventsNFC_GET_KEYFOB_SERIAL Done");
	    }
	    ulEventToWait = eventsKEYFOB_GET_SERIAL_SUCCESS |
			    eventsKEYFOB_GET_SERIAL_FAILURE |
			    eventsKEYFOB_DETECT_TIMEOUT;
	    uxBits = xEventGroupWaitBits(
				    pxResHandle->xEventGroupUserModeApp ,
				    ulEventToWait,
				    pdFALSE ,
				    pdFALSE ,
				    xTicksToWait );
	    uxBits = xEventGroupClearBits( pxResHandle->xEventGroupUserModeApp ,
		    ulEventToWait );
	    vTaskDelay(usermodeDELAY_MSEC_100);
	    /* change state based on event type */
	    if( ((uxBits & eventsKEYFOB_GET_SERIAL_SUCCESS)
		    == eventsKEYFOB_GET_SERIAL_SUCCESS)
		    && ((uxBits & eventsKEYFOB_GET_SERIAL_FAILURE)
			    == eventsKEYFOB_GET_SERIAL_FAILURE) )
	    {
		    debugERROR_PRINT("This is an error case. ");
		    goto END;
	    }
	    else if( (uxBits & eventsKEYFOB_GET_SERIAL_SUCCESS)
		    == eventsKEYFOB_GET_SERIAL_SUCCESS )
	    {
		    debugPRINT_APP("Receive event: eventsKEYFOB_GET_SERIAL_SUCCESS");
	    }
	    else if( (uxBits & eventsKEYFOB_GET_SERIAL_FAILURE)
		    == eventsKEYFOB_GET_SERIAL_FAILURE )
	    {
		    debugERROR_PRINT("Receive event: "
			    "eventsKEYFOB_GET_SERIAL_FAILURE");
		    goto END;
	    }
	    else if((uxBits & eventsKEYFOB_DETECT_TIMEOUT )
		    == eventsKEYFOB_DETECT_TIMEOUT)
	    {
		    debugERROR_PRINT("Receive event: eventsKEYFOB_DETECT_TIMEOUT ");
		    goto END;
	    }
	    else
	    {
		    debugERROR_PRINT(" Receive event: Invalid Event received ");
		    goto END;
	    }
	    if(xQueueReceive(pxResHandle->xNfcQueue, &xNfcData, portMAX_DELAY)
			    != pdTRUE)
	    {
		    debugERROR_PRINT(" Failed to receive data from queue ");
		    goto END;
	    }
	    for(ucTmp = 0;ucTmp < usermodeKEYFOB_SERIAL_LEN;ucTmp++)
	    {
		    debugPRINT(" xNfcData.ucKeyFobSerial[%d] %d", ucTmp,
			    xNfcData.ucKeyFobSerial[ucTmp]);
	    }
	    END:
	    vTaskDelay(usermodeDELAY_SEC_30);
	}
}
/*----------------------------------------------------------------------------*/
#endif
