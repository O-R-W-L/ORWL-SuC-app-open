/**===========================================================================
 * @file orwl_SecMon.c
 *
 * @brief This file defines SuC MAX32550 security monitor setup and operations
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
#include <stdint.h>
#include <errors.h>
#include <stddef.h>
#include <freeRtos.h>
#include <string.h>

/* driver includes */
#include <mml.h>
#include <mml_scbr_regs.h>

/* application include */
#include <orwl_secalm.h>
#include <orwl_secmon_reg.h>
#include <orwl_sensoren.h>
#include <orwl_err.h>
#include <debug.h>
#include <orwl_gpio.h>
#include <hist_devtamper.h>
#include <init.h>
#include <reset.h>
#include <mfgdata.h>
#include <rtc.h>
#include <access_keys.h>
#include <mem_common.h>
#include <orwl_disp_interface.h>

/* Global variables */
/* Flag to check if display init is success */
extern uint8_t ucGPIODisplayInitDone;

/*---------------------------------------------------------------------------*/

__attribute__((section(".nmi_code")))void vSecmonDefaultTamperDetectNmi( void )
{
    int32_t lResult;
    devtamperTamperHist_t *pxTamperHistory;
    uint32_t ulDevState = eBOOTMODE_DEV_USER_TAMPER;
    keysDSFT_MASTER_KEYS_t *pxMasterKeys;
    uint32_t ulSecalm = 0;
    uint32_t ulSecStatus = 0;

    /* Check if GPIO initialization has been done, if done display tamper
     * logo on OLED. Otherwise initialize the GPIO hardware.
     */
    if(!(ucGPIODisplayInitDone & initGPIO_INIT_DONE))
    {
	/* Initialization of GPIO */
	vOrwlGpioInit( );
    }

    /* Check if Display initialization has been done, if done display tamper
     * logo on OLED. Otherwise initialize the Display hardware.
     */
    if(!(ucGPIODisplayInitDone & initDISPLAY_INIT_DONE))
    {
	/* Init the display hardware */
	vDisplayInterfaceEnable( );
    }

    /* Display tamper image on OLED */
    vInitDisplayTamperScreenNONRTOS();

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
	debugERROR_PRINT_ISR("sensorenALARM_STATUS is set not authorized access \n");
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

    /* If the interrupt occurred because tamper, save the tamper history to
     * flash, erase secure data from flash.
     */
    if(ulSecalm & secalmTAMPER_DETECTION_MAGIC)
    {
	/* power OFF Intel sub-system */
	gpioIntelCpuShutdown();

	/* Memory allocation for tamper history storing */
	pxTamperHistory = pvPortMalloc(sizeof(devtamperTamperHist_t));
	configASSERT(pxTamperHistory != NULL);

	/* Memory allocation for reading access key structure */
	pxMasterKeys = pvPortMalloc(sizeof(keysDSFT_MASTER_KEYS_t));
	configASSERT(pxMasterKeys != NULL);

	/* Read DRS Log RTC register. This might return zero if the RTC is not
	 * initialized. This register reads the second register from RTC. This
	 * will be updated if there is new DRS or if battery is removed.
	 */
	IO_READ_U32(secmonDLRTC, pxTamperHistory->xTamperEvent[0].ulDRSLogRTC);

	/* Now convert the 32 bit second DRS Log RTC register value to human
	 * readable values in rtcDateTime_t format.
	 */
	lResult = lRtcConvertRawToDateTime(
		pxTamperHistory->xTamperEvent[0].ulDRSLogRTC,
		&pxTamperHistory->xTamperEvent[0].xTamperDateTime);
	if(lResult != NO_ERROR)
	{
	    debugERROR_PRINT_ISR("Failed to convert raw to date and time..\n");
	    goto ERRORSTATE;
	}

	/* Update the tamper cause */
	pxTamperHistory->xTamperEvent[0].ultamperCause = ulSecalm;

	/* Event type */
	pxTamperHistory->xTamperEvent[0].ulEventTyype = devtamperEVENT_TAMPER;

	/*Write tamper history */
	lResult = lDevtamperWriteTampHistoryNONRTOS(pxTamperHistory);
	if( lResult != NO_ERROR )
	{
	    debugERROR_PRINT_ISR("Failed to write tamper history..\n");
	    goto ERRORSTATE;
	}

	/* reading of all keys */
	lResult = lKeysReadMasterKeys(pxMasterKeys);
	if( lResult != NO_ERROR )
	{
	    debugERROR_PRINT_ISR("Failed to read keys..\n");
	    goto ERRORSTATE;
	}

	/* Erase required field from flash */
	memset(pxMasterKeys->ucDefaultPIN, 0xff, keysPIN_LEN);
	memset(pxMasterKeys->ucUserPIN, 0xff, keysPIN_LEN);
	memset(pxMasterKeys->ucDefaultISDKey, 0xff, keysISD_LEN);

	lResult = lKeysWriteMasterKeysNONRTOS(pxMasterKeys);
	if( lResult != NO_ERROR )
	{
	    debugERROR_PRINT_ISR("Failed to write keys..\n");
	    goto ERRORSTATE;
	}

	/* Erase UserConfig data */
	lResult = lcommonEraseUserConfigData();
	if( lResult != NO_ERROR )
	{
	    debugERROR_PRINT_ISR("Failed to erase UserConfig data..\n");
	    goto ERRORSTATE;
	}

	ERRORSTATE :
	/* Moving the device in to tamper state */
	lResult = lMfgdataUpdateProductCycleNONRTOS(&ulDevState);
	if( lResult != NO_ERROR )
	{
	    debugERROR_PRINT_ISR("Failed to update state..\n");
	}

	/* release allocated resources */
	if(pxTamperHistory)
	{
	    vPortFree(pxTamperHistory);
	}
	if(pxMasterKeys)
	{
	    vPortFree(pxMasterKeys);
	}
	/** reboot ORWL */
	resetSYSTEM_RESET;
    }
    /* Check if its warning, if its warning clear the warning message and reboot
     * if needed.
     */
    else if(ulSecalm & secalmWARN_MAGIC)
    {
	debugPRINT_ISR("Tamper ISR occurred because of warning\n");
	/* Reset the tamper ISR warnings and reboot */
	ulSecalm &= ~(secalmWARN_MAGIC);
	/* Update the warning reset value to security alarm register. */
	IO_WRITE_U32(secmonSECALM, ulSecalm);
	/* Reboot ORWL */
	resetSYSTEM_RESET;
    }
    /* Tamper might have occurred for unknown reason */
    else
    {
	debugPRINT_ISR("Tamper ISR occurred for unknown reason \n");
	/* Write the security alarm register with zero, so that it does not keep
	 * on occurring all the time.
	 */
	ulSecalm = 0;
	/* Update the reset value to security alarm register. */
	IO_WRITE_U32(secmonSECALM, ulSecalm);
	/* Reboot ORWL */
	resetSYSTEM_RESET;
    }
}
/*---------------------------------------------------------------------------*/

void vSecmonTamperInfo( void )
{
	uint32_t ulValue;
	uint16_t usReasonCount = 1 ;
	uint32_t ulSecalm = 0;
	uint32_t ulSecdiag = 0;
	/** Read secure monitor status */
	/* All the print in this functions are 'debugPRINT_ISR', because this
	 * function can be called from ISR context to display the tamper reason.
	 */
	IO_READ_U32(secmonEXTSC, ulValue);
	debugPRINT_ISR("External sensors registers: 0x%08X\n", ulValue);
	IO_READ_U32(secmonINTSC, ulValue);
	debugPRINT_ISR("Internal sensors registers: 0x%08X\n", ulValue);
	IO_READ_U32(secmonSECALM, ulSecalm);
	debugPRINT_ISR("Security alarm registers: 0x%08X\n", ulSecalm);
	IO_READ_U32(secmonSECDIAG, ulSecdiag);
	debugPRINT_ISR("Security diagnostic registers: 0x%08X\n", ulSecdiag);

	/** if tampered in previous boot - display reason for tamper */
	if(ulSecalm & secalmTAMPER_DETECTION_MAGIC)
	{
	    if(ulSecalm & secalmDIESHIELD_FAULT)
	    {
		debugPRINT_ISR(" \n Reason No. %d: Die shield fault detected !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmLOW_TEMP)
	    {
		debugPRINT_ISR(" \n  Reason No. %d: Device temperature below low temperature threshold detected. !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmHIGH_TEMP)
	    {
		debugPRINT_ISR(" \n Reason No. %d: Device temperature above high \
			 temperature threshold detected !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmBAT_LOW)
	    {
		debugPRINT_ISR(" \n Reason No. %d: Back up Battery under voltage detected !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmBAT_OVERVOLT)
	    {
		debugPRINT_ISR(" \n Reason No. %d: Back up Battery over voltage detected !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmBAT_EXT_FAULT)
	    {
		debugPRINT_ISR(" \n !!!! External tamper detected. !!!! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmVDDLO)
	    {
		debugPRINT_ISR(" \n  Reason No. %d:VDD undervoltage detected !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmVCORELO)
	    {
		debugPRINT_ISR(" \n  Reason No. %d: VCORE undervoltage detected. !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmVCOREHI)
	    {
		debugPRINT_ISR(" \n Reason No. %d: VCORE overvoltage detected !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmVDDHI)
	    {
		debugPRINT_ISR(" \n  Reason No. %d:VDD overvoltage detected. !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmVGL)
	    {
		debugPRINT_ISR(" \n  Reason No. %d:Voltage glitch detected !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmEXT_SENS0)
	    {
		debugPRINT_ISR(" \n  Reason No. %d:External tamper_SENS0 !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmEXT_SENS1)
	    {
		debugPRINT_ISR(" \n  Reason No. %d:External tamper_SENS1 !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmEXT_SENS2)
	    {
		debugPRINT_ISR(" \n  Reason No. %d:External tamper_SENS2 !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmEXT_SENS3)
	    {
		debugPRINT_ISR(" \n  Reason No. %d:External tamper_SENS3 !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmEXT_SENS4)
	    {
		debugPRINT_ISR(" \n Reason No. %d: External tamper_SENS4 !! \n",usReasonCount++);
	    }
	    if(ulSecalm & secalmEXT_SENS5)
	    {
		debugPRINT_ISR(" \n  Reason No. %d: External tamper_SENS5 !! \n",usReasonCount++);
	    }
	}
	else if (ulSecalm & secalmWARN_MAGIC)
	{
	    /** Display the cause of WARNINGS; but Ignore those warnings
	     *  and continue to boot */
	    if((ulSecalm & secalmRFU22) || (ulSecalm & secalmRFU23) ||
		    (ulSecalm & secalmRFU30) || (ulSecalm & secalmRFU31))
	    {
		debugPRINT_ISR(" \n UNEXPECTED WARNINGS: secalmRFUXX !! "
			    "- Ignore this message \n");
	    }
	    if(ulSecalm & secalmEXTSWARN_SENS0)
	    {
		debugPRINT_ISR(" \n External tamper__SENS0 !! \n");
	    }
	    if(ulSecalm & secalmEXTSWARN_SENS1)
	    {
		debugPRINT_ISR(" \n External tamper__SENS1 !! \n");
	    }
	    if(ulSecalm & secalmEXTSWARN_SENS2)
	    {
		debugPRINT_ISR(" \n External tamper__SENS2 !! \n");
	    }
	    if(ulSecalm & secalmEXTSWARN_SENS3)
	    {
		debugPRINT_ISR(" \n External tamper__SENS3 !! \n");
	    }
	    if(ulSecalm & secalmEXTSWARN_SENS4)
	    {
		debugPRINT_ISR(" \n External tamper_SENS4 !! \n");
	    }
	    if(ulSecalm & secalmEXTSWARN_SENS5)
	    {
		debugPRINT_ISR(" \n External tamper__SENS5 !! \n");
	    }
	    debugPRINT_ISR(" \n Removed all WARNINGS & "
		    "continue with normal boot !! \n");
	}
}
/*---------------------------------------------------------------------------*/

int32_t lSecmonTamperCheck( void )
{
	uint32_t ulSecalm = 0;
	uint32_t ulSecStatus = 0;

	/* Read security monitor status register */
	IO_READ_U32(secmonSECST, ulSecStatus);

	/* When the corresponding BIT is 0, the associated register has been updated
	 * and access is authorized. While the corresponding bit is 1, the associated
	 * register is being updated and access is not authorized. So wait till
	 * the BIT gets cleared.
	 */
	while(ulSecStatus & sensorenALARM_STATUS);

	/* Read security alarm register */
	IO_READ_U32(secmonSECALM, ulSecalm);
	if(ulSecalm & secalmTAMPER_DETECTION_MAGIC)
	{
	    return eORWL_ERROR_DEVICE_TAMPERED;
	}
	return NO_ERROR;
}
/*---------------------------------------------------------------------------*/
