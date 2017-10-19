/**===========================================================================
 * @file app_init.c
 *
 * @brief This file contains function to initialize ORWL application
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

/* Global includes */
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <errors.h>

/* application includes */
#include <Init.h>
#include <task_config.h>
#include <usermode.h>
#include <mfgdata.h>
#include <keyfobid.h>
#include <orwl_disp_interface.h>
#include <delay.h>
#include <orwl_err.h>
#include <mem_common.h>
#include <orwl_secmon.h>
#include <user_config.h>
#include <orwl_gpio.h>
#include <orwl_secmon_reg.h>
#include <orwl_sensoren.h>
#include <hist_devtamper.h>
#include <access_keys.h>
#include <systemRes.h>
#include <rtc.h>

/*---------------------------------------------------------------------------*/

int32_t lInitGetSuCBootMode( uint32_t *pulProdCycle )
{
	int32_t lResult;

	/* Read Product cycle */
	lResult = lMfgdataReadProductCycle( pulProdCycle );
	if(lResult != NO_ERROR)
	{
	    debugERROR_PRINT("Failed to read mode\n");
	    return COMMON_ERR_UNKNOWN;
	}
	return lResult;
}
/*---------------------------------------------------------------------------*/

int32_t lInitSystemStateCheck( uint32_t ulProdCycle )
{
    int32_t lResult = NO_ERROR;
    devtamperTamperHist_t *pxTamperHistory;
    uint32_t ulDevState = eBOOTMODE_DEV_USER_TAMPER;
    keysDSFT_MASTER_KEYS_t *pxMasterKeys;
    uint32_t ulSecalm = 0;
    uint32_t ulSecStatus = 0;
    uint32_t ulSecDLRTC = 0;
    /* Structure to read RTC */
    rtcDateTime_t xReadRTC;

    /* This API should only be called from ROT, OOB and USER mode. Check if this
     * is called from other than these modes, if so report error.
     */
    if ((ulProdCycle != eBOOTMODE_DEV_ROT) && (ulProdCycle != eBOOTMODE_DEV_OOB)
	    && (ulProdCycle != eBOOTMODE_DEV_USER))
    {
	debugERROR_PRINT("lInitSystemStateCheck called from non ROT/OOB/USER\n");
	return COMMON_ERR_OUT_OF_RANGE;
    }

    /* check if the device is tampered. Also check if RTC ready bit is not set*/
    if(( lSecmonTamperCheck() == eORWL_ERROR_DEVICE_TAMPERED ) || \
        ( lRtcGetDateTime(&xReadRTC) == COMMON_ERR_NOT_INITIALIZED ))
    {
	/* Display tamper image on OLED. We dont need to check if display
	 * hardware was initiated.
	 */
	vInitDisplayTamperScreenNONRTOS();

	/* power OFF Intel sub-system. Should be NON-RTOS API */
	gpioIntelCpuShutdown();

	/* Memory allocation for tamper history storing */
	pxTamperHistory = pvPortMalloc(sizeof(devtamperTamperHist_t));
	configASSERT(pxTamperHistory != NULL);
	/* memset tamper history buffer to 0xff */
	memset(pxTamperHistory, 0xff, sizeof(devtamperTamperHist_t));

	/* Memory allocation for reading access key structure */
	pxMasterKeys = pvPortMalloc(sizeof(keysDSFT_MASTER_KEYS_t));
	configASSERT(pxMasterKeys != NULL);
	/* memset pxMasterKeys buffer to 0xff */
	memset(pxMasterKeys, 0xff, sizeof(keysDSFT_MASTER_KEYS_t));

	/* Read DRS Log RTC register. This might return zero if the RTC is not
	 * initialized. This register reads the second register from RTC. This
	 * will be updated if there is new DRS or if battery is removed.
	 */
	IO_READ_U32(secmonDLRTC, ulSecDLRTC);

	/* Read the tamper structure from flash */
	lResult = lDevtamperReadTampHistory(pxTamperHistory);
	if(lResult != NO_ERROR)
	{
	    debugPRINT("There is no tamper history in flash, same will be updated \n");
	    /* Even if read failed, must not go to error state because there can
	     * be case where tamper might have occurred when ORWL was turned off
	     * and tamper register might have been updated since it battery backed.
	     * In the next boot ISR will not occur and tamper will be detected as
	     * tamper in previous boot, in this case we need to update the tamper
	     * history to flash.
	     */
	}

	/* Now check the secmonDLRTC updated in flash with what we have read,
	 * if both matches than we have updated the tamper history into the flash,
	 * if it does matches than this tamper is different than that of updated
	 * in flash so update the tamper structure with new data in flash.
	 */
	if(ulSecDLRTC != pxTamperHistory->xTamperEvent[0].ulDRSLogRTC)
	{
	    /* Update new DRS log RTC to tamper structure */
	    pxTamperHistory->xTamperEvent[0].ulDRSLogRTC = ulSecDLRTC;

	    /* Now convert the 32 bit second DRS Log RTC register value to human
	     * readable values in rtcDateTime_t format.
	     */
	    lResult = lRtcConvertRawToDateTime(
		    pxTamperHistory->xTamperEvent[0].ulDRSLogRTC,
		    &pxTamperHistory->xTamperEvent[0].xTamperDateTime);
	    if(lResult != NO_ERROR)
	    {
		/* Just show error message on console. Should not go to error state
		 * because this tamper might have occurred because of RTC.
		 */
		debugERROR_PRINT("Failed to convert raw to date and time..\n");
	    }

	    /* Read security monitor status register */
	    IO_READ_U32(secmonSECST, ulSecStatus);

	    /* When the corresponding BIT is 0, the associated register has been updated
	     * and access is authorized. While the corresponding bit is 1, the associsted
	     * register is being updated and access is not authorized.
	     */
	    if(ulSecStatus & sensorenALARM_STATUS)
	    {
		/* Secure alarm status bit is set, the hardware might be updating the
		 * register.
		 */
		debugERROR_PRINT("sensorenALARM_STATUS is set not authorized access \n");
		/* Read the secure alarm register value even its not authorized access
		 * since we don't want to stop here, we want to proceed even if the register
		 * is being updated at the time we are reading this register since its and
		 * interrupt and tamper actions needs to taken
		 */
		IO_READ_U32(secmonSECALM, ulSecalm);
	    }
	    else
	    {
		/* Secure alarm status bit is not set. Reading security alarm register
		 * is authorized access.
		 */
		IO_READ_U32(secmonSECALM, ulSecalm);
	    }

	    /* Update the tamper cause */
	    pxTamperHistory->xTamperEvent[0].ultamperCause = ulSecalm;

	    /* Event type */
	    pxTamperHistory->xTamperEvent[0].ulEventTyype = devtamperEVENT_TAMPER;

	    /*Write tamper history */
	    lResult = lDevtamperWriteTampHistoryNONRTOS(pxTamperHistory);
	    if( lResult != NO_ERROR )
	    {
		debugERROR_PRINT("Failed to write tamper history..\n");
		goto ERRORSTATE;
	    }
	}

	/* reading of all keys */
	lResult = lKeysReadMasterKeys(pxMasterKeys);
	if( lResult != NO_ERROR )
	{
	    debugERROR_PRINT("Failed to read keys..\n");
	    goto ERRORSTATE;
	}

	/* Erase required field from flash */
	memset(pxMasterKeys->ucDefaultPIN, 0xff, keysPIN_LEN);
	memset(pxMasterKeys->ucUserPIN, 0xff, keysPIN_LEN);
	memset(pxMasterKeys->ucDefaultISDKey, 0xff, keysISD_LEN);

	lResult = lKeysWriteMasterKeysNONRTOS(pxMasterKeys);
	if( lResult != NO_ERROR )
	{
	    debugERROR_PRINT("Failed to write keys..\n");
	    goto ERRORSTATE;
	}

	/* Erase UserConfig data */
	lResult = lcommonEraseUserConfigData();
	if( lResult != NO_ERROR )
	{
	    debugERROR_PRINT("Failed to erase UserConfig data..\n");
	    goto ERRORSTATE;
	}

	ERRORSTATE :
	/* release allocated resources */
	if(pxTamperHistory)
	{
	    vPortFree(pxTamperHistory);
	}
	if(pxMasterKeys)
	{
	    vPortFree(pxMasterKeys);
	}

	/* Check if the ulProdCycle is user mode, if its user mode return
	 * eORWL_ERROR_DEVICE_TAMPERED so that the tamper task will be started.
	 */
	if(ulProdCycle == eBOOTMODE_DEV_USER)
	{
	    /* Update the mode to user tamper mode */
	    lResult = lMfgdataUpdateProductCycleNONRTOS(&ulDevState);
	    if( lResult != NO_ERROR )
	    {
		debugERROR_PRINT("Failed to update state..\n");
	    }
	    return eORWL_ERROR_DEVICE_TAMPERED;
	}

	/* If its not user mode, update the product cycle to eORWL_ERROR_DEVICE_TAMPERED
	 * and block the CPU.
	 */
	lResult = lMfgdataUpdateProductCycleNONRTOS(&ulDevState);
	if( lResult != NO_ERROR )
	{
	    debugERROR_PRINT("Failed to update state..\n");
	}
	/* Block the CPU */
	while (1);
    }
    /* return success if no tamper */
    return lResult;
}
/*---------------------------------------------------------------------------*/

void vInitDisplayUserModeWelcomeScreen ( void )
{
	vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
	vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR );

	/* For displaying WELCOME msg on OLED */
	vDisplayText((const int8_t *)"WELCOME TO ORWL\n");
	vDisplayText((const int8_t *)"www.orwl.org\n");

	/* delay */
	delayMICRO_SEC(100000);

	/* For displaying image on OLED */
	lDisplayImageFullScreen(eDipsInt_NFC_AUTH_REQUEST);

	/* delay */
	delayMICRO_SEC(100000);
}
/*---------------------------------------------------------------------------*/

void vInitDisplayWelcomeScreen ( void )
{
	vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);

	vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR );

	/* For displaying WELCOME msg on OLED */
	vDisplayText((const int8_t *)"WELCOME TO ORWL\n");
	vDisplayText((const int8_t *)"hello.orwl.org\n");

	/* delay */
	delayMICRO_SEC(100000);

	/* For displaying image on OLED */
	lDisplayImageFullScreen(eDipsInt_ORWL_LOGO);

	/* delay */
	delayMICRO_SEC(100000);
}
/*---------------------------------------------------------------------------*/

void vInitDisplayTamperScreen ( void )
{
	vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);

	vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR );

	/* For displaying image on OLED */
	lDisplayImageFullScreen(eDipsInt_TAMPER_DETECTION);

	/* delay */
	delayMICRO_SEC(100000);
}
/*---------------------------------------------------------------------------*/

void vInitDisplayTamperScreenNONRTOS ( void )
{
    vDisplayClearScreenNONRTOS(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);

    vDisplaySetTextPosNONRTOS( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR );

    /* For displaying image on OLED */
    lDisplayImageFullScreenNONRTOS(eDipsInt_TAMPER_DETECTION);

    /* delay */
    delayuSEC(100000);
}
/*---------------------------------------------------------------------------*/

uint16_t usInitCommonCalcCrc( const uint8_t *pucIn, uint32_t ulLen )
{
    configASSERT(pucIn != NULL);

    uint16_t usCrc = 0xFFFF;

    while(ulLen > 0)
    {
	/* allow over flow. don't care */
	usCrc += *pucIn++;
	ulLen--;
    }

    return usCrc;
}
/*---------------------------------------------------------------------------*/
