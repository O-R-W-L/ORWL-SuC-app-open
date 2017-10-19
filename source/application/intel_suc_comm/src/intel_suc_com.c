/**===========================================================================
 * @file intel_suc_com.c
 *
 * @brief This file implements the complete implementation of
 *        Intel SuC communication for realization of the application.
 *
 * @author gupta@design-shift.com
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
 ============================================================================
 *
 */

/* Global includes */
#include <config.h>
#include <errors.h>
#include <cobra_defines.h>
#include <mml.h>
#include <mml_uart.h>
#include <mml_uart_regs.h>

/* FreeRtos includes */
#include <FreeRTOS.h>
#include <queue.h>
#include <stdlib.h>
#include <task_config.h>
#include <task.h>
#include <event_groups.h>
#include <semphr.h>
#include <portmacro.h>

/* Local includes */
#include <printf_lite.h>
#include <private.h>
#include <uart_config.h>
#include <debug.h>
#include <Intel_SuC_Comm.h>
#include <intel_suc_comm_manager.h>
#include <init.h>
#include <mfgdata.h>
#include <task_config.h>
#include <rtc.h>
#include <hist_devtamper.h>
#include <keyfobid.h>
#include <access_keys.h>
#include <orwl_err.h>
#include <user_config.h>
#include <usermode.h>
#include <nvsram.h>
#include <orwl_gpio.h>
#include <reset.h>
#include <orwl_disp_interface.h>
#include <systemRes.h>
#include <tamper_log_intel.h>
#include <orwl_secalm.h>
#include <oled_ui.h>
#include <pinhandling.h>

/** Size of receiving Que */
#define intelUART_RX_QUE_SIZE		(5)


/* Static Data payload for ACK&NANK */
static IntelSuCAckNack_t xAckNackPkt  ;

xUartBuffer_t xTxBuffer;
xUartBuffer_t xRxProcessBuff ;
EventGroupHandle_t xUartTxRXSync ;
SemaphoreHandle_t xsUartTxSync ;


/** global resource handler for user mode */
/* TODO: this is a global hanlde. Ideally we need to pass this a
 * arg to the func/thread during init
 */
extern xSMAppResources_t xgResource;

/**
 * @brief Interrupt service routine
 *
 * This isr is used to check if there is some data received
 * or rx error interrupt received by reading interrupt status reg.
 *
 * @return void
 */
static void prvIntelSucUartIsr( void );

/**
 * @brief For initializing uart port
 *
 * This function is used for initializing uart1 and queue for receiving data.
 *
 * @return error code on failure.
 */
static int32_t prvInitUartPort( void );
/**
 * @brief For reading current state
 *
 * This function is used for reading the current state of device.
 *
 * @return error code on failure..
 */
static int32_t prvDevStateRead( void );

/**
 * @brief For reading SSD password
 *
 * This function is used reading SSD password i.e user password and admin
 * password from flash.
 *
 * @return error code
 */
static int32_t prvSSDRead( void );

/**
 * @brief For configuring the device
 *
 * This function is used for reading current time,serial number,
 * production date date and time.. etc.
 *
 * @return error code
 */
static int32_t prvDevProdConfig( void );

/**
 * @brief For Key Management
 *
 * This function is used for printing action,key fob name and keyfob attribute
 * and transmit the key management option i.e adding or deleting keys.
 *
 * @return error code
 */
static int32_t prvKeyMgtAdd( void );

/**
 * @brief For keyfob data
 *
 * This function is used for reading the keyfob data i.e keyfob name
 * and transmitting the same.
 *
 * @return error code
 */
static int32_t prvKeyData( void );

/**
 * @brief For tamper log
 *
 * This function is used for reading tamper log and transmitting the same.
 *
 * @return void
 */
static int32_t prvTamperLog( void );

/**
 * @brief For security check of level1.
 *
 * This function is used for security check of level1. i.e NFC +BLE check.
 *
 * @return void
 */
static int32_t prvCheckSecLevel1( void );

/**
 * @brief For security check of level2
 *
 * This function is used for security check of level2. i.e NFC + BLE + PIN check.
 *
 * @return NO_ERROR on success or error code on failure.
 */
static int32_t prvCheckSecLevel2(void);

/**
 * @brief For creating the transmitting packets.
 *
 * This function is used for creating the transmitting packets and copying
 * that in to tx buffer.
 *
 * @return NO_ERROR on success or error code on failure.
 */
static void prvCreateTxPacket(uint8_t *pucMsg, uint16_t usLen);

/**
 * @brief For establishing ROT
 *
 * This function is used for establishing root of trust. i.e Erasing NVSRAM.
 *
 * @return error code
 */
static int32_t prvRotRegen( void );

/**
 * @brief For configuring the device
 *
 * This function is used for reading the proximity range.
 *
 * @return error code.
 */
static int32_t prvDevReadProxi( void );

/**
 * @brief For configuring the ORWL for proximity protection.
 *
 * This function is used for configuring the ORWL for proximity protection.
 *
 * @return error code
 */
static int32_t prvDevWriteProxi( void );

/**
 * @brief For updating the pin
 *
 * This function is used for erasing the pin from NVSRAM and and SSD data
 * will be lost.
 *
 * @return error code
 */
static int32_t prvSucPinUpdate( void );

/**
 * @brief For updating the device state
 *
 * This function is used for updating the state to Api state.
 *
 * @return error code..
 */
static int32_t prvIntelDevStateUpdate( void );

/**
 * @brief For updating the state
 *
 * This function is used for updating the state on OS installation i.e state transition.
 *
 * @return error code.
 */
static int32_t prvHlosStatUpdate( void );

/**
 * @brief For getting RTC time from SUC
 *
 * This function is used for getting RTC time from SUC.
 *
 * @return error code.
 */
static int32_t prvGetRTCTime( void );

/**
 * @brief For setting RTC time to SUC
 *
 * This function is used for setting RTC time to SUC.
 *
 * @return error code.
 */
static int32_t prvSetRTCTime( void );

/**
 * @brief Send data to supervisor task.
 *
 * This function is used for sending command and data to supervisory task.
 *
 * @param pxData pointer to the keyfob data.
 *
 * @return NO_ERROR on success and error code on failure.
 */
static int32_t prvIntelSendKeyFobData( OrwlKeyMgt_t *pxData );

/**
 * @brief Send SSD password generate command to supervisor task.
 *        Waits for the data from supervisor to be sent to BIOS
 *
 * @param pxData pointer to the data.
 *
 * @return NO_ERROR in success or error code on failure.
 */
static int32_t prvIntelSSDGenCmd(BiosSuc1B_t *pxData);

/**
 * @brief Send security check command to supervisor task.
 *
 * @param ucCommand level check command.
 *
 * @return NO_ERROR in success or error code on failure.
 */
static int32_t prvIntelSecCommand( uint32_t ucCommand );

/**
 * @brief Displays message on oled .
 *
 *@param pcMsg message to display on oled.
 *
 * @return void
 */
static void prvShutdownAndDisplayMessage(const int8_t *pcMsg );

/**
 * @brief Function used to formatting the tamper log send to intel.
 *
 * @param ulTamperCause  Variable containing tamper cause bit set
 * @param pxRtcDateTime  pointer conatining updated rtcDateTime_t structure
 * @param pucString      pointer to formatted output
 *
 * @return void
 */
static void prvFormatLog(uint32_t ulTamperCause, rtcDateTime_t* pxRtcDateTime,
				uint8_t* pucString);

/**
 * @brief To handle data error.
 *
 * This function is to handle data error command from Intel. Just send ACK for
 * the same.
 *
 * @return error code
 */
static int32_t prvHandleDataError( void );

static xIntelSucCommHandling xIntelSucLookUpAction[] =
	{   /* Command Packet Type      cmd_read		, cmd_write*/
	    {ORWL_DEV_STATE	 	,prvDevStateRead	,NULL},
	    {ORWL_PROX_CONFIG	 	,prvDevReadProxi	,prvDevWriteProxi},
	    {ORWL_DEV_PROD_CONFIG	,prvDevProdConfig	,NULL},
	    {ORWL_KEY_MGT		,NULL			,prvKeyMgtAdd},
	    {ORWL_KEY_FOR_DAT		,prvKeyData		,NULL},
	    {SUC_ROT_REGEN		,NULL			,prvRotRegen},
	    {ORWL_SEC_PIN_UPDATE	,NULL			,prvSucPinUpdate},
	    {INTEL_DEV_ACT		,NULL			,prvIntelDevStateUpdate},
	    {INT_SSD_MGMT_INTEL		,prvSSDRead		,NULL},
	    {INT_SSD_MGMT_SUC		,NULL			,NULL},
	    {ORWL_TAMPER_LOG		,prvTamperLog		,NULL},
	    {ORWL_TAMPER_MGT		,NULL			,NULL},
	    {ORWL_TAMPER_METR		,NULL			,NULL},
	    {ORWL_SEC_STATE		,NULL			,NULL},
	    {ORWL_SEC_CHECK_LEVEL0	,prvCheckSecLevel1	,NULL},
	    {ORWL_SEC_CHECK_LEVEL1	,prvCheckSecLevel2	,NULL},
	    {INTEL_HLOS_STATUS		,NULL			,prvHlosStatUpdate},
	    {COREBOOT_PUBLIC_KEY	,NULL			,NULL},
	    {GET_RTC_TIME	        ,prvGetRTCTime		,NULL},
	    {SET_RTC_TIME	        ,NULL			,prvSetRTCTime},
	    {DATA_ERROR_STATAUS         ,NULL			,prvHandleDataError}
	};
/*---------------------------------------------------------------------------*/
static uint8_t ucPreAmble[intelPRE_AMBLE_SIZE] = {0xFE, 0xDF,0x02, 0x0, 0x0} ;
xSplRxUartBuffer_t xBuffer;
static int32_t lSizxIntelSucLookUpAction ;
int32_t *plSuCState;
/**
 * Static functions
 */
/*---------------------------------------------------------------------------*/

static int32_t prvIntelSendKeyFobData( OrwlKeyMgt_t *pxData )
{
    IntelUserData_t xKeyFobData;
    uint8_t ucAck;

    /* Check for NULL */
    configASSERT( pxData != NULL );

    /* Keyfob data to Add / delete */
    xKeyFobData.ucCommand = ORWL_KEY_MGT;
    /* Check if the action requested is to add KeyFob */
    if(pxData->action == ORWL_KEY_FOB_MGT_ADD)
    {
	xKeyFobData.ucSubCommand = ORWL_KEY_FOB_MGT_ADD;
    }
    /* Check if the action requested is to delete KeyFob */
    else if (pxData->action == ORWL_KEY_FOB_MGT_DEL)
    {
	xKeyFobData.ucSubCommand = ORWL_KEY_FOB_MGT_DEL;
    }
    /* If the action is not to add/delete KeyFob then we are not going to send
     * this event to waiting task because just send SUC_WRITE_STATUS_FAIL_INVALID.
     */
    else
    {
	return SUC_WRITE_STATUS_FAIL_INVALID;
    }

    xKeyFobData.pvData = pxData;
    /* Send the Keyfob Add/Delete command to supervisory task */
    xQueueSend(xgResource.xSucBiosSendQueue, &xKeyFobData, portMAX_DELAY);
    /* Receive the acknowledgment for the sent command */
    xQueueReceive(xgResource.xSucBiosReceiveQueue, &ucAck, portMAX_DELAY);
    /* Return the acknowledgment received */
    return (ucAck);
}
/*---------------------------------------------------------------------------*/

static int32_t prvIntelSSDGenCmd(BiosSuc1B_t *pxData)
{
    IntelUserData_t xSSData;
    uint8_t ucAck;

    xSSData.ucCommand = SUC_ROT_REGEN;
    xSSData.pvData = pxData;

    /* Send the ssd password generate command to supervisory task */
    xQueueSend(xgResource.xSucBiosSendQueue, &xSSData, portMAX_DELAY);
    /* Receive the acknowledgment for the sent command */
    xQueueReceive(xgResource.xSucBiosReceiveQueue, &ucAck, portMAX_DELAY);

    /* Return the acknowledgment received */
    return (ucAck);
}
/*---------------------------------------------------------------------------*/

static int32_t prvIntelSecCommand( uint32_t ucCommand )
{
    IntelUserData_t xSecData;
    uint8_t ucAck;

    /* Security check 1 or 2 command*/
    xSecData.ucCommand = ucCommand;
    /* Send the Security check 1 or 2  command to supervisory task */
    xQueueSend(xgResource.xSucBiosSendQueue, &xSecData, portMAX_DELAY);
    /* Receive the acknowledgment for the sent command */
    xQueueReceive(xgResource.xSucBiosReceiveQueue, &ucAck, portMAX_DELAY);

    /* Return the acknowledgment received */
    return (ucAck);
}
/*---------------------------------------------------------------------------*/

static void prvShutdownAndDisplayMessage(const int8_t *pcMsg )
{
    /* Display the message on the OLED */
    vDisplayClearScreen(dispWELCOME_IMGLTX, dispWELCOME_IMGLTY,
	    dispWELCOME_IMGRBX, dispWELCOME_IMGRBY, dispBACKGROUND_BLACK);
    vDisplaySetTextPos( dispWELCOME_STR_XCOR, dispWELCOME_STR_YCOR );
    vDisplayText((const int8_t *)pcMsg);

    /* send SHUTDOWN signal using long press. This is safe shutdown */
    vSystemResIntel_LongPress();

    /* 30 sec wait. Allow Intel to shutdown smoothly */
    vTaskDelay(SystemResINTEL_SHUTDOWN_WAIT_PERIOD*SystemResDELAY_INC_1S_30S_COUNT) ;

    /* Intel has shutdown by now, reboot the ORWL */
    resetSYSTEM_RESET;
}
/*---------------------------------------------------------------------------*/

static int32_t prvDevStateRead( void )
{
    int32_t lResult = NO_ERROR;
    BiosSuc1B_t xResPack ;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__) ;
    plSuCState = pvPortMalloc(sizeof(plSuCState));
    if(plSuCState == NULL)
    {
	debugERROR_PRINT("Failed to allocate Memory for dev state..\r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Reading the device state */
    lResult = lMfgdataReadProductCycle( (uint32_t*)plSuCState );
    if(lResult != NO_ERROR )
    {
	debugERROR_PRINT("Failed to read mode\n");
	if(plSuCState)
	{
	    vPortFree(plSuCState);
	}
	return lResult;
    }
    xResPack.action.cmd = RESP_READ ;
    xResPack.action.dataPktTyp = ORWL_DEV_STATE ;
    xResPack.statUpdate = *plSuCState;

    prvCreateTxPacket ((uint8_t *)&xResPack, sizeof(xResPack));

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX);

    /* release the allocated memory */
    if(plSuCState)
    {
	vPortFree(plSuCState);
    }
    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__) ;
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvSSDRead( void )
{
    BiosSucActionWithData_t xResPack;
    BiosSsdMgt_t xDataToSend;
    /* Variable for reading administrator password */
    uint8_t *pucAdminPasswd;
    /* Variable for reading SSD encryption password */
    uint32_t *pulSSDEnKey;
    int32_t lResult;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__) ;
    memset(&xDataToSend,0,sizeof(xDataToSend)) ;
    /* Allocate memory for administrator password */
    pucAdminPasswd =(uint8_t *) pvPortMalloc((keysADMIN_PASSWD_LEN));
    if(pucAdminPasswd == NULL)
    {
	debugERROR_PRINT(""
		"Failed to allocate Memory for administrator password\r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Reading administrator password from flash */
    lResult = lKeysGetDefAdminPasswd(pucAdminPasswd);
    if(lResult != NO_ERROR)
    {
	debugERROR_PRINT("Failed to read administrator password\n");
	if(pucAdminPasswd)
	{
	    vPortFree(pucAdminPasswd);
	}
	return lResult;
    }

    /* Allocate memory for SSD encryption key */
    pulSSDEnKey =(uint32_t *) pvPortMalloc(nvsramSNVSRAM_ENC_KEY_LEN_MAX
	    *sizeof(uint32_t));
    if(pulSSDEnKey == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate Memory for SSD encryption key for NVSRAM\r\n");
	if(pucAdminPasswd)
	{
	    vPortFree(pucAdminPasswd);
	}
	return COMMON_ERR_NULL_PTR;
    }

    /* Reading user SSD encryption key */
    lResult = lNvsramReadSSDEncKey(nvsramSNVSRAM_ENC_KEY_LEN_MAX,pulSSDEnKey);
    if(lResult != NO_ERROR)
    {
	debugERROR_PRINT("Failed to read SSD encryption key\n");
	if(pulSSDEnKey)
	{
	    vPortFree(pulSSDEnKey);
	}
	if(pucAdminPasswd)
	{
	    vPortFree(pucAdminPasswd);
	}
	return lResult;
    }

    /*
     * We have allocated larger memory to
     * "pucAdminPasswd" compare to xDataToSend.adminPwd,
     * So, let us use the lower size to be compatible.
     * Ideally both should have same size.
     */
    memcpy(xDataToSend.adminPwd, pucAdminPasswd,ORWL_MAX_SSD_PASSWORD-1) ;
    xDataToSend.adminPwd[ORWL_MAX_SSD_PASSWORD-1] = '\0';

    lite_snprintf(xDataToSend.userPwd,ORWL_MAX_SSD_PASSWORD,"%08x%08x%08x%08x"
	    ,pulSSDEnKey[0],pulSSDEnKey[1],pulSSDEnKey[2],pulSSDEnKey[3]);

    /* Populate the data payload */
    xResPack.action.cmd = RESP_READ;
    xResPack.action.dataPktTyp = INT_SSD_MGMT_INTEL;
    memcpy(&xResPack.data[0],&xDataToSend,sizeof(xDataToSend));

    /* Two bytes to compensate for the Cmd + Pkttype */
    prvCreateTxPacket ((uint8_t *)&xResPack, (sizeof(xDataToSend)+2));

    /* Start the transmission */
    xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX);

    /* release the allocated memory */
    if(pucAdminPasswd)
    {
	vPortFree(pucAdminPasswd);
    }
    if(pulSSDEnKey)
    {
	vPortFree(pulSSDEnKey);
    }

    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__);

    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvDevProdConfig( void )
{
    int32_t lResult =NO_ERROR;
    BiosSucActionWithData_t xResPack;
    UIBiosOrwlConf_t xDataToSend;
    /* Variable for reading date and time */
    rtcDateTime_t *pxRTCDateTime = NULL;
    /* Variable for reading mfg date and time */
    mfgdataManufactData_t *pxMFData = NULL;
    uint8_t ucRetryCount = 0 ;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);
    memset(&xDataToSend,0,sizeof(xDataToSend));

    debugPRINT_SUC_INTEL_COMM("Data size to be sent %d\n\r",(sizeof(xDataToSend)+2));
    /* Allocate memory for reading date */
    pxRTCDateTime = (rtcDateTime_t *)pvPortMalloc(sizeof(rtcDateTime_t));
    if(pxRTCDateTime == NULL )
    {
	debugERROR_PRINT("Failed to allocate Memory for Date&Time structure\r\n");
	lResult = COMMON_ERR_NULL_PTR;
	goto CLEANUP;
    }

    /* Allocate memory for reading mfgdata */
    pxMFData = (mfgdataManufactData_t *)pvPortMalloc(sizeof(mfgdataManufactData_t));
    if(pxMFData == NULL )
    {
	debugERROR_PRINT("Failed to allocate Memory for mfg data\r\n");
	lResult = COMMON_ERR_NULL_PTR;
	goto CLEANUP;
    }
    /* Reading of mfg data */
    lResult = lMfgdateReadMfgData(pxMFData);
    if( lResult != NO_ERROR )
    {
	debugERROR_PRINT("Failed to read mfg data.\n");
	goto CLEANUP;
    }

    do{
	lResult = lRtcGetDateTime(pxRTCDateTime);
	if( lResult != NO_ERROR )
	{
	    debugERROR_PRINT("Failed to read current Date & time..\n");
	}
	else
	{
	    ucRetryCount = 5;
	}
	ucRetryCount++ ;
    }while (ucRetryCount<3);

    /* We didn't manage to read the time. So pass MFG date & time*/
    if(ucRetryCount == 3)
    {
	memcpy(pxRTCDateTime,&pxMFData->xMFGDateTime,sizeof(rtcDateTime_t)) ;
    }

    /*Update date and time in string format for sending */
    /* MM-DD-YYYY HH:MM:SS in this format*/
    lite_snprintf(&xDataToSend.prodTime[0],intelMAX_DATE_TIME_SIZE,
	    "%02d-%02d-%04d %02d:%02d:%02d",
	    pxMFData->xMFGDateTime.ucMonth,
	    pxMFData->xMFGDateTime.ucDate,
	    pxMFData->xMFGDateTime.usYear,
	    pxMFData->xMFGDateTime.ucHour,
	    pxMFData->xMFGDateTime.ucMinute,
	    pxMFData->xMFGDateTime.ucSecond);

    lite_snprintf(&xDataToSend.currTime[0],intelMAX_DATE_TIME_SIZE,
	    "%02d-%02d-%04d %02d:%02d:%02d",
	    pxRTCDateTime->ucMonth,
	    pxRTCDateTime->ucDate,
	    pxRTCDateTime->usYear,
	    pxRTCDateTime->ucHour,
	    pxRTCDateTime->ucMinute,
	    pxRTCDateTime->ucSecond);

    memcpy(&xDataToSend.srNum[0], &pxMFData->ucSerialNum[0], mfgdataSERIAL_NUM_SIZE);
    memcpy(&xDataToSend.sucSerialNum[0],&pxMFData->ucSUCSerialNum[0],mfgdataSERIAL_NUM_SIZE);
    xDataToSend.hwVer = pxMFData->ulHardwareRev;;
    xDataToSend.swVerMajor = intelSUC_SW_MAJOR_RELEASE_NO;
    xDataToSend.swVerMinor = intelSUC_SW_MINOR_RELEASE_NO;

    /*Populate the data payload*/
    xResPack.action.cmd = RESP_READ ;
    xResPack.action.dataPktTyp = ORWL_DEV_PROD_CONFIG ;
    memcpy(&xResPack.data[0],&xDataToSend,sizeof(xDataToSend));

    /* Two bytes to compensate for the Cmd+Pkttype */
    prvCreateTxPacket((uint8_t *)&xResPack, (sizeof(xDataToSend)+ 2));

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX);

    CLEANUP:
    /* Release allocated memory */
    if(pxRTCDateTime)
    {
	vPortFree(pxRTCDateTime);
    }
    if(pxMFData)
    {
	vPortFree(pxMFData);
    }
    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__);
    return lResult;
}
/*---------------------------------------------------------------------------*/

static int32_t prvKeyMgtAdd( void )
{
    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);
    BiosSuc1B_t xResPack;
    OrwlKeyMgt_t * pxDataReceived;
    int32_t lResult = NO_ERROR;

    /* Point to the data buffer */
    pxDataReceived =(OrwlKeyMgt_t *) &xRxProcessBuff.xBuff.actWithData.data[0];

    debugPRINT_SUC_INTEL_COMM("Key Management Action Received %d\n\r",pxDataReceived->action);
    debugPRINT_SUC_INTEL_COMM("Key Management Key Name Received %s\n\r",pxDataReceived->keyFobNam);
    debugPRINT_SUC_INTEL_COMM("Key Management Key Name Attribute %c\n\r",pxDataReceived->preAttr);

    /* Add new keyfob or delete the existing one.
     * Check for add/delete operation failure.
     */
    if ((lResult = prvIntelSendKeyFobData(pxDataReceived))
	    != SUC_WRITE_STATUS_SUCCESS)
    {
	debugERROR_PRINT("Failed to Add/Delete key\n");
    }

    /* Update the response packet */
    xResPack.action.cmd = RESP_WRITE;
    xResPack.action.dataPktTyp = ORWL_KEY_MGT;
    /* Update the response type ACK/NACK */
    xResPack.statUpdate = lResult;

    prvCreateTxPacket ((uint8_t *)&xResPack, sizeof(xResPack));

    /* Start the transmission */
    xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX);

    if(lResult == SUC_WRITE_STATUS_FAIL_AUTH)
    {
	/* Got Auth fail response from supervisory task.
	 * Set intelSESSION_AUTH_FAIL bit.
	 */
	xEventGroupSetBits(xUartTxRXSync, intelSESSION_AUTH_FAIL);
    }
    else if(lResult == SUC_WRITE_STATUS_FAIL_UNKOWN)
    {
	/* Got unknown error response from supervisory task.
	 * Set intelSESSION_COM_FAIL bit.
	 */
	xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX_RX_COM_FAIL);
    }
    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__);

    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvKeyData( void )
{
    BiosSucActionWithData_t xResPack;
    /* Read KeyFob data to send to BIOS */
    IntelUserData_t xReadKeyFOb;
    /* ORWL keyfob data */
    OrwlKeyData_t xDataToSend;
    /* ACK from supervisory/waiting task */
    uint8_t ucACK = 0;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);
    memset(&xDataToSend,0,sizeof(xDataToSend));

    /* Asign xDataToSend to xReadKeyFOb.pvData, so that KeyFob information will
     * be read at waiting task and updated the same to xReadKeyFOb.pvData.
     */
    xReadKeyFOb.pvData = &xDataToSend;

    /* Send the event to the task waiting for this event */
    xReadKeyFOb.ucCommand = ORWL_KEY_FOR_DAT;

    /* Send the Intel device state to supervisory/waiting task */
    xQueueSend(xgResource.xSucBiosSendQueue, &xReadKeyFOb, portMAX_DELAY);
    /* Receive the acknowledgment for the sent command */
    xQueueReceive(xgResource.xSucBiosReceiveQueue, &ucACK, portMAX_DELAY);

    /* Populate the data payload */
    xResPack.action.cmd = RESP_READ;
    xResPack.action.dataPktTyp = ORWL_KEY_FOR_DAT;
    /* Copy the xDataToSend(xReadKeyFOb.pvData) to response packet. */
    memcpy(&xResPack.data[0],&xDataToSend,sizeof(xDataToSend));
    /* Two bytes to compensate for the Cmd+Pkttype */
    prvCreateTxPacket ((uint8_t *)&xResPack, (sizeof(xDataToSend)+2));

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX);
    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__);
    /* return error code */
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvTamperLog( void )
{
    BiosSucActionWithData_t xResPack ;
    OrwlTamperLog_t xDataToSend ;
    int32_t lResult;

    /* Variable for reading tamper history */
    devtamperTamperHist_t *pxDevTamHist;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);
    memset(&xDataToSend,0,sizeof(xDataToSend));

    /* Memory allocation for variable of tamper history */
    pxDevTamHist = pvPortMalloc(sizeof(devtamperTamperHist_t));
    if( pxDevTamHist == NULL )
    {
	debugERROR_PRINT("Failed to allocate memory.\n");
	return COMMON_ERR_NULL_PTR;
    }

    /* Reading of tamper history */
    lResult = lDevtamperReadTampHistory(pxDevTamHist);
    if(lResult !=NO_ERROR)
    {
	debugERROR_PRINT("Failed to read tamper history..\n");
	if( pxDevTamHist )
	{
	    vPortFree(pxDevTamHist);
	}
	return lResult;
    }

    /* Formatting the tamper log */
    prvFormatLog(pxDevTamHist->xTamperEvent[0].ultamperCause,
	    &pxDevTamHist->xTamperEvent[0].xTamperDateTime,
	    xDataToSend.formatLog);

    /*Populate the data payload*/
    xResPack.action.cmd = RESP_READ;
    xResPack.action.dataPktTyp = ORWL_TAMPER_LOG;
    memcpy(&xResPack.data[0],&xDataToSend,sizeof(xDataToSend));

    /* Two bytes to compensate for the Cmd + Pkttype */
    prvCreateTxPacket ((uint8_t *)&xResPack, (sizeof(xDataToSend)+2)) ;

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX) ;

    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__) ;

    /* Release allocated memory */
    if( pxDevTamHist )
    {
	vPortFree(pxDevTamHist);
    }
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvCheckSecLevel1( void )
{
    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);
    BiosSuc1B_t xResPack;
    int32_t lResult = NO_ERROR;

    if((lResult = prvIntelSecCommand(ORWL_SEC_CHECK_LEVEL0)) !=
	    SUC_WRITE_STATUS_SUCCESS)
    {
	debugERROR_PRINT("Failed to check security  level 1\n");
    }
    /* Update the response packet */
    xResPack.action.cmd = RESP_WRITE;
    xResPack.action.dataPktTyp = ORWL_SEC_CHECK_LEVEL0;
    xResPack.statUpdate = lResult;

    prvCreateTxPacket((uint8_t *)&xResPack, sizeof(xResPack));

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX);

    if(lResult == SUC_WRITE_STATUS_FAIL_AUTH)
    {
	/* Got Auth fail response from supervisory task.
	 * Set intelSESSION_AUTH_FAIL bit.
	 */
	xEventGroupSetBits(xUartTxRXSync, intelSESSION_AUTH_FAIL);
    }
    else if(lResult == SUC_WRITE_STATUS_FAIL_UNKOWN)
    {
	/* Got unknown error response from supervisory task.
	 * Set intelSESSION_COM_FAIL bit.
	 */
	xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX_RX_COM_FAIL);
    }
    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__);
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvCheckSecLevel2( void )
{
    BiosSuc1B_t xResPack;
    int32_t lResult = NO_ERROR;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);

    if((lResult = prvIntelSecCommand(ORWL_SEC_CHECK_LEVEL1)) !=
	    SUC_WRITE_STATUS_SUCCESS)
    {
	debugERROR_PRINT("Failed to check security  level 1\n");
    }

    /* Update the response packet */
    xResPack.action.cmd = RESP_WRITE ;
    xResPack.action.dataPktTyp = ORWL_SEC_CHECK_LEVEL1;
    xResPack.statUpdate = lResult ;

    prvCreateTxPacket ((uint8_t *)&xResPack, sizeof(xResPack));

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX);

    if(lResult == SUC_WRITE_STATUS_FAIL_AUTH)
    {
	/* Got Auth fail response from supervisory task.
	 * Set intelSESSION_AUTH_FAIL bit.
	 */
	xEventGroupSetBits(xUartTxRXSync, intelSESSION_AUTH_FAIL);
    }
    else if(lResult == SUC_WRITE_STATUS_FAIL_UNKOWN)
    {
	/* Got unknown error response from supervisory task.
	 * Set intelSESSION_COM_FAIL bit.
	 */
	xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX_RX_COM_FAIL);
    }

    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__);
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvRotRegen( void )
{
    BiosSuc1B_t xResPack;
    int32_t lResult = NO_ERROR;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);

    /* Send command to supervisory task to generate SSD password*/
    if((lResult = prvIntelSSDGenCmd(&xResPack)) !=
	    SUC_WRITE_STATUS_SUCCESS)
    {
	debugERROR_PRINT("Failed to generate SSD password\n");
    }

    /* Update the response packet */
    xResPack.action.cmd = RESP_WRITE;
    xResPack.action.dataPktTyp = SUC_ROT_REGEN;
    xResPack.statUpdate = lResult;

    prvCreateTxPacket ((uint8_t *)&xResPack, sizeof(xResPack));

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX);

    if(lResult != SUC_WRITE_STATUS_SUCCESS )
    {
	/* Got negative response from supervisory task. Set SUC_COM_FAIL bit.*/
	xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX_RX_COM_FAIL);
    }

    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__);
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvDevReadProxi( void )
{
    BiosSucActionWithData_t xResPack;
    UIBiosProxiProt_t xDataToSend;
    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);

    xDataToSend.ioEnDis = xgResource.ucIOEnDis;
    xDataToSend.range = xgResource.ucBLERange;
    xDataToSend.mode = xgResource.ucMode;
    xDataToSend.motionAct = xgResource.ulSuCAction;
    xDataToSend.AskPinOnBoot = xgResource.ucAskPinOnBoot;

    debugPRINT_SUC_INTEL_COMM(
	    "Proxiconfig ioEnDis %d\n\r",xgResource.ucIOEnDis);
    debugPRINT_SUC_INTEL_COMM(
	    "Proxiconfig range %d\n\r",xgResource.ucBLERange);
    debugPRINT_SUC_INTEL_COMM(
	    "Proxiconfig mode %d\n\r",xgResource.ucMode);
    debugPRINT_SUC_INTEL_COMM(
	    "Proxiconfig motionAct %d\n\r",xgResource.ulSuCAction);
    debugPRINT_SUC_INTEL_COMM(
	    "Proxiconfig AskPinOnBoot %d\n\r",xgResource.ucAskPinOnBoot);

    /* Populate the data payload */
    xResPack.action.cmd = RESP_READ;
    xResPack.action.dataPktTyp = ORWL_PROX_CONFIG;
    memcpy(&xResPack.data[0],&xDataToSend,sizeof(xDataToSend));

    /* Two bytes to compensate for the Cmd+Pkttype */
    prvCreateTxPacket ((uint8_t *)&xResPack, (sizeof(xDataToSend)+2));

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX);

    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__);
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvDevWriteProxi( void )
{
    BiosSuc1B_t xResPack;
    UIBiosProxiProt_t *pxDataReceived;
    xUserConfig_t *pxUserConfig;
    int32_t lResult;
    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);
    pxUserConfig = pvPortMalloc(sizeof(xUserConfig_t));
    if(pxUserConfig == NULL)
    {
	debugERROR_PRINT("Failed to allocate Memory for reading"
		" proximity data\r\n");
	return COMMON_ERR_NULL_PTR;
    }

    /* Point to the databuffer */
    pxDataReceived = (UIBiosProxiProt_t *)&xRxProcessBuff.xBuff.actWithData.data[0];

    debugPRINT_SUC_INTEL_COMM(
	    "Proxiconfig ioEnDis %d\n\r",pxDataReceived->ioEnDis);
    debugPRINT_SUC_INTEL_COMM(
	    "Proxiconfig range %d\n\r",pxDataReceived->range);
    debugPRINT_SUC_INTEL_COMM("Proxiconfig mode %d\n\r",pxDataReceived->mode);
    debugPRINT_SUC_INTEL_COMM(
	    "Proxiconfig motionAct %d\n\r",pxDataReceived->motionAct);
    debugPRINT_SUC_INTEL_COMM(
	    "Proxiconfig AskPinOnBoot %d\n\r",pxDataReceived->AskPinOnBoot);

    pxUserConfig->ucBLERange = pxDataReceived->range;
    /* Irrespective of user configuration we are making IO disable, to make
     * sure user cannot access IO on proximity lock situation.
     */
    pxUserConfig->ucIOEnDis = PROX_PROT_IO_DIS;
    pxUserConfig->ucMode = pxDataReceived->mode;
    pxUserConfig->ulSuCAction = pxDataReceived->motionAct;
    pxUserConfig->ucAskPinOnBoot = pxDataReceived->AskPinOnBoot;

    /* Updating the global resource */
    xgResource.ucAskPinOnBoot = pxDataReceived->AskPinOnBoot;
    xgResource.ucBLERange = pxDataReceived->range;
    xgResource.ucIOEnDis = PROX_PROT_IO_DIS;
    xgResource.ucMode = pxDataReceived->mode;
    xgResource.ulSuCAction = pxDataReceived->motionAct;

    lResult = lUserWriteUserConfig(pxUserConfig);
    if(lResult != NO_ERROR)
    {
	debugERROR_PRINT("Failed to update proximity data..\n");
	if(pxUserConfig)
	{
	    vPortFree(pxUserConfig);
	}
	/* Failed to write the user configuration.update the status to failure */
	lResult = SUC_WRITE_STATUS_FAIL_INVALID;
    }
    else
    {
	/* Update the status to success */
	lResult = SUC_WRITE_STATUS_SUCCESS;
    }

    /* let us print for the Key management Add options */
    xResPack.action.cmd = RESP_WRITE;
    xResPack.action.dataPktTyp = ORWL_PROX_CONFIG;
    xResPack.statUpdate = lResult;

    prvCreateTxPacket ((uint8_t *)&xResPack, sizeof(xResPack));

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX);

    /* Release allocated memory */
    if(pxUserConfig)
    {
	vPortFree(pxUserConfig);
    }

    if(lResult != SUC_WRITE_STATUS_SUCCESS )
    {
	/* Got negative response from supervisory task. Set SUC_COM_FAIL bit.*/
	xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX_RX_COM_FAIL);
    }

    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__);
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvSucPinUpdate( void )
{
    BiosSuc1B_t xResPack ;
    /* ACK to Intel. As of now this feature is not supported. Error will be reported
     * to Intel.
     */
    int8_t ucACK = SUC_WRITE_STATUS_FAIL_INVALID;
    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__) ;

    /* Display on OLED to user feature not supported */
    vUiMessageDisplay((const int8_t *)uiUSER_PIN_NOT_SUPPORTED, uiXCORDINATE_0 ,uiYCORDINATE_C);
    vTaskDelay(uiTHREE_SEC_DELAY);

    /* Update response packet */
    xResPack.action.cmd = RESP_WRITE ;
    xResPack.action.dataPktTyp = ORWL_SEC_PIN_UPDATE;
    xResPack.statUpdate = ucACK ;

    prvCreateTxPacket ((uint8_t *)&xResPack, sizeof(xResPack));

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX);

    if(ucACK != SUC_WRITE_STATUS_SUCCESS )
    {
	/* Got negative response from supervisory task. Set SUC_COM_FAIL bit.*/
	xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX_RX_COM_FAIL);
    }

    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__);
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvIntelDevStateUpdate( void )
{
    /* Intel device state to send to waiting task */
    IntelUserData_t xIntelDevState;
    /* ACK from supervisory/waiting task */
    uint8_t ucACK = 0;
    BiosSuc1B_t xResPack;
    IntelSubState_t *pxDataReceived;
    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);

    /* Point to the databuffer */
    pxDataReceived =(IntelSubState_t *)&xRxProcessBuff.xBuff.actWithData.data[0];

    /* Check the Intel device state and update the sub-command accordingly */
    if(*pxDataReceived == INTEL_DEV_STATE_S0)
    {
	xIntelDevState.ucSubCommand = INTEL_DEV_STATE_S0;
    }
    else if(*pxDataReceived == INTEL_DEV_STATE_STD_BY)
    {
	xIntelDevState.ucSubCommand = INTEL_DEV_STATE_STD_BY;
    }
    else if(*pxDataReceived == INTEL_DEV_STATE_HIBER)
    {
	xIntelDevState.ucSubCommand = INTEL_DEV_STATE_HIBER;
    }
    else if(*pxDataReceived == INTEL_DEV_STATE_SHT_DWN)
    {
	xIntelDevState.ucSubCommand = INTEL_DEV_STATE_SHT_DWN;
    }
    else if(*pxDataReceived == INTEL_DEV_STATE_RST)
    {
	xIntelDevState.ucSubCommand = INTEL_DEV_STATE_RST;
    }
    else
    {
	/* Don't send the event to waiting task since this is not the expected
	 * device state from Intel. RESP_WRITE Intel with error code.
	 */
	    /* Send the response write to the Intel */
	    xResPack.action.cmd = RESP_WRITE ;
	    xResPack.action.dataPktTyp = INTEL_DEV_ACT ;
	    /* Return SUC_WRITE_STATUS_FAIL_INVALID as error code to Intel */
	    xResPack.statUpdate = SUC_WRITE_STATUS_FAIL_INVALID ;

	    /* Create the packet and add to TX buffer */
	    prvCreateTxPacket ((uint8_t *)&xResPack, sizeof(xResPack)) ;

	    /* Start the transmission*/
	    xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX);
	    /* Set Suc com fail event if invalid command is issued */
	    xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX_RX_COM_FAIL);

	    /* Return the error code */
	    return COMMON_ERR_INVAL;
    }

    /* Send the event to the task waiting for this event */
    xIntelDevState.ucCommand = INTEL_DEV_ACT;

    /* Send the Intel device state to supervisory/waiting task */
    xQueueSend(xgResource.xSucBiosSendQueue, &xIntelDevState, portMAX_DELAY);
    /* Receive the acknowledgment for the sent command */
    xQueueReceive(xgResource.xSucBiosReceiveQueue, &ucACK, portMAX_DELAY);

    /* Send the response write to the Intel */
    xResPack.action.cmd = RESP_WRITE ;
    xResPack.action.dataPktTyp = INTEL_DEV_ACT ;
    /* Return ucACK as error code to Intel, supervisory/waiting task should send
     * the ucACK that is understood by Intel. */
    xResPack.statUpdate = ucACK ;

    prvCreateTxPacket ((uint8_t *)&xResPack, sizeof(xResPack)) ;

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX);

    if(ucACK != SUC_WRITE_STATUS_SUCCESS )
    {
	/* Got negative response from supervisory task. Set SUC_COM_FAIL bit.*/
	xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX_RX_COM_FAIL);
    }

    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__);
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvHlosStatUpdate( void )
{
    BiosSuc1B_t xResPack ;
    /* OS installation status update */
    IntelUserData_t xOSInstallationStatus;
    /* ACK from waiting task */
    uint8_t ucACK = 0;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);

    /* memset xOSInstallationStatus to zero */
    memset(&xOSInstallationStatus, 0, sizeof(IntelUserData_t));

    /* Point to the data buffer from BIOS. Give the pointer to xOSInstallationStatus.pvData
     * so that it can be passed to waiting task.
     */
    xOSInstallationStatus.pvData = (HlosInstallStat_t *)&xRxProcessBuff.xBuff.actWithData.data[0] ;

    /* Update the command */
    xOSInstallationStatus.ucCommand = INTEL_HLOS_STATUS;

    /* Send xOSInstallationStatus to supervisory/waiting task */
    xQueueSend(xgResource.xSucBiosSendQueue, &xOSInstallationStatus, portMAX_DELAY);
    /* Receive the acknowledgment for the sent command */
    xQueueReceive(xgResource.xSucBiosReceiveQueue, &ucACK, portMAX_DELAY);

    /* Update the response packet */
    xResPack.action.cmd = RESP_WRITE;
    xResPack.action.dataPktTyp = INTEL_HLOS_STATUS;
    xResPack.statUpdate = ucACK;

    prvCreateTxPacket ((uint8_t *)&xResPack, sizeof(xResPack)) ;

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX) ;

    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__) ;
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvGetRTCTime( void )
{
    BiosSucActionWithData_t xResPack;
    /* RTC date time structure, to communicate between SUC and BIOS */
    OrwlRTCTimeStat_t xRTCTimeComData;
    /* ACK from waiting task */
    uint8_t ucACK = ORWL_RTC_RD_STAT_SUCC;
    int32_t lStatus = NO_ERROR;
    /* RTC structure to read from RTC hardware */
    rtcDateTime_t xReadRTCDateTime;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);

    /* memset OrwlRTCTimeStat_t to zero */
    memset(&xRTCTimeComData, 0, sizeof(OrwlRTCTimeStat_t));

    /* for get RTC we are handling in Intel SUC for now */
    /* Read the RTC time from RTC hardware */
    lStatus = lRtcGetDateTime(&xReadRTCDateTime);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed read RTC time.\n");
	/* This should usually not occur if RTC is initialized, we don't need to
	 * retry multiple times since it being done in lRtcGetDateTime.
	 */
	if(lStatus == COMMON_ERR_NOT_INITIALIZED)
	{
	    /* ORWL RTC not initialized */
	    ucACK = ORWL_RTC_RD_STAT_ERR;
	    /* Handle error */
	    goto ERROR;
	}
	/* ORWL RTC read second register was zero, must not occur but still BIOS
	 * can retry again. This is in else because these are the only two possibilities
	 * of error that can occur.
	 */
	else
	{
	    /* ORWL RTC second register read wrong data, but can be retried */
	    ucACK = ORWL_RTC_RD_STAT_BUSY;
	    goto ERROR;
	}
    }

    /* Now we have read the RTC, update the same to xRTCTimeComData */
    xRTCTimeComData.currTime.ucDate = xReadRTCDateTime.ucDate;
    xRTCTimeComData.currTime.ucHour = xReadRTCDateTime.ucHour;
    xRTCTimeComData.currTime.ucMinute = xReadRTCDateTime.ucMinute;
    xRTCTimeComData.currTime.ucMonth = xReadRTCDateTime.ucMonth;
    xRTCTimeComData.currTime.ucSecond = xReadRTCDateTime.ucSecond;
    xRTCTimeComData.currTime.ucWeekday = xReadRTCDateTime.ucWeekday;
    /* Year should be updated in terms of offset to 2000. Example 2015 offset is
     * 15 for 2000.
     */
    xRTCTimeComData.currTime.ucYearOffset2000 = xReadRTCDateTime.usYear - 2000;

    ERROR:
    /* Update the error code */
    xRTCTimeComData.ucReadRTCStat = ucACK;

    /* Update the response packet */
    xResPack.action.cmd = RESP_READ;
    xResPack.action.dataPktTyp = GET_RTC_TIME;
    /* Copy the xRTCTimeComData to response packet. */
    memcpy(&xResPack.data[0],&xRTCTimeComData,sizeof(xRTCTimeComData));
    /* Two bytes to compensate for the Cmd+Pkttype */
    prvCreateTxPacket ((uint8_t *)&xResPack, (sizeof(xRTCTimeComData)+2));

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX) ;

    /* If there was error in reading RTC from hardware send event com failure */
    if(ucACK == ORWL_RTC_RD_STAT_ERR)
    {
	/* Got not initialized error from RTC driver .
	 * Set intelSESSION_COM_FAIL bit.
	 */
	xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX_RX_COM_FAIL);
    }

    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__) ;
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvSetRTCTime( void )
{
    BiosSuc1B_t xResPack ;
    /* RTC data time structure, to communicate between SUC and BIOS */
    OrwlRTCTime_t *pxRTCTimeComData;
    /* ACK from waiting task */
    uint8_t ucACK = SUC_WRITE_STATUS_SUCCESS;
    int32_t lStatus = NO_ERROR;
    int32_t lError = NO_ERROR;
    /* RTC structure to read from RTC hardware */
    rtcDateTime_t xReadRTCDateTime;
    /* PIN entry retry count */
    uint16_t usPswdRetryCount = 0;
    /* RTC delay seconds before next PIN entry */
    uint32_t ulRTCDelaySec = 0;
    /* RTC raw seconds snapshot at time of modifying delay */
    uint32_t ulRTCSnapShot = 0;
    /* RTC current seconds read from hardware */
    uint32_t ulRTCCurrentSeconds = 0;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);

    /* Point to the data buffer from BIOS. Give the pointer to xUserRTCDateTime.pvData
     * so that it can be passed to waiting task.
     */
    pxRTCTimeComData = (OrwlRTCTime_t *)&xRxProcessBuff.xBuff.actWithData.data[0] ;

    /* update rtcDateTime_t to write to RTC hardware */
    xReadRTCDateTime.ucDate = pxRTCTimeComData->ucDate;
    xReadRTCDateTime.ucHour = pxRTCTimeComData->ucHour;
    xReadRTCDateTime.ucMinute = pxRTCTimeComData->ucMinute;
    xReadRTCDateTime.ucMonth = pxRTCTimeComData->ucMonth;
    xReadRTCDateTime.ucSecond = pxRTCTimeComData->ucSecond;
    /* Year sent by BIOS is offset to 2000, so add the same to 2000 to calculate
     * actual year.
     */
    xReadRTCDateTime.usYear = pxRTCTimeComData->ucYearOffset2000 + 2000;
    /* WeekDay is not necessary to set RTC data and time */
    /* xReadRTCDateTime.ucWeekday = xRTCTimeComData.ucWeekday; */

    /* Before setting the new RTC time from user, we need to calculate the actual
     * wait delay for next pin entry.
     */
    /* Read the NVSRAM to know wait delay and retry count */
    /* Read the retry count and RTC delay seconds from NVSRAM */
    lStatus = lNvsramReadPswdRetryCountDelay(&usPswdRetryCount, &ulRTCDelaySec,
	    &ulRTCSnapShot);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to read retry count and RTC delay seconds \n");
	/* This should never fail */
	ucACK = SUC_WRITE_STATUS_RTC_FAIL;
    }

    /* Pin entry delay comes to effect only when usPswdRetryCount is greater than 3*/
    if(usPswdRetryCount > pinhandlingNODELAY_RETRYCOUNT)
    {
	/* Before proceeding further first get the RTC seconds */
	while(lRtcGetRTCSeconds(&ulRTCCurrentSeconds) != NO_ERROR)
	{
	    /* if RTC read fails, which should never occur re-try until its success
	     * wait for 1 second before each re-try.
	     */
	    vTaskDelay(uiONE_SEC_DELAY);
	}

	/* usPswdRetryCount is greater than 3, we need to calculate wait delay.
	 * Before calculating the wait delay we need to check if user has already
	 * waited till the wait delay, if so wait delay will be zero.
	 */
	if((ulRTCDelaySec + ulRTCSnapShot) <= ulRTCCurrentSeconds)
	{
	    /* user has already waited till the wait delay */
	    ulRTCDelaySec = 0;
	}
	else
	{
	    /* if user has not waited, calculate new wait delay */
	    ulRTCDelaySec = (ulRTCDelaySec + ulRTCSnapShot) - ulRTCCurrentSeconds;
	}

	/* ulRTCDelaySec has been calculated, set the RTC snapshot dirty bit */
	lStatus = lNvsramSetRTCSnapshotDirtyBit();
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("Failed to set RTCSnapshot \n");
	    /* This should never fail */
	    ucACK = SUC_WRITE_STATUS_RTC_FAIL;
	}
    }

    /* Now we have updated xReadRTCDateTime write the same to RTC hardware */
    lStatus = lRtcSetDateTime(&xReadRTCDateTime);
    if(lStatus != NO_ERROR)
    {
	/* if new RTC time update has failed we will not update the wait delay
	 * or RTC snapshot. And reset RTC snapshot.
	 */
	lError = lNvsramResetRTCSnapshotDirtyBit();
	if(lError != NO_ERROR)
	{
	    /* This should never fail */
	    debugERROR_PRINT("Failed to reset RTCSnapshot \n");
	}

	/* Check if RTC values are out of range */
	if(lStatus == COMMON_ERR_OUT_OF_RANGE)
	{
	    debugERROR_PRINT("RTC set values out of range.\n");
	    ucACK = SUC_WRITE_STATUS_RTC_INVALID;
	}
	else
	{
	    debugERROR_PRINT("Failed to set RTC time.\n");
	    ucACK = SUC_WRITE_STATUS_RTC_FAIL;
	}
    }

    /* Now user has updated RTC, we need to update the ulRTCSnapShot */
    if(usPswdRetryCount > pinhandlingNODELAY_RETRYCOUNT)
    {
	/* Before proceeding further first get updated RTC seconds */
	while(lRtcGetRTCSeconds(&ulRTCCurrentSeconds) != NO_ERROR)
	{
	    /* if RTC read fails, which should never occur re-try until its success
	     * wait for 1 second before each re-try.
	     */
	    vTaskDelay(uiONE_SEC_DELAY);
	}
	/* Now this latest RTC seconds will be ulRTCSnapShot, so that user needs
	 * to wait as per the latest RTC time while the wait delay has already
	 * been calculated.
	 */
	ulRTCSnapShot = ulRTCCurrentSeconds;
	/* update the wait delay and snapshot rtc to NVSRAM */
	lStatus = lNvsramWritePswdRetryCountDelay(&usPswdRetryCount,
		&ulRTCDelaySec, &ulRTCSnapShot);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("Failed to update retry count and RTC delay seconds\n");
	    /* updating NVSRAm should never fail, if failed report RTC fail */
	    ucACK = SUC_WRITE_STATUS_RTC_FAIL;
	}
	/* Now we have written the RTCdelay and RTC snapshot to NVSRAM now clear
	 * the RTC snapshot dirty bit.
	 */
	lStatus = lNvsramResetRTCSnapshotDirtyBit();
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("Failed to reset RTCSnapshot \n");
	    /* This should never fail */
	    ucACK = SUC_WRITE_STATUS_RTC_FAIL;
	}
    }

    /* Update the response packet */
    xResPack.action.cmd = RESP_WRITE;
    xResPack.action.dataPktTyp = SET_RTC_TIME;
    xResPack.statUpdate = ucACK;

    prvCreateTxPacket ((uint8_t *)&xResPack, sizeof(xResPack)) ;

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX) ;

    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__) ;
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static int32_t prvHandleDataError( void )
{
    BiosSuc1B_t xResPack ;
    /* ACK to BIOS */
    uint8_t ucACK = SUC_WRITE_STATUS_SUCCESS;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__);

    /* Update the response packet */
    xResPack.action.cmd = RESP_WRITE;
    xResPack.action.dataPktTyp = DATA_ERROR_STATAUS;
    /* As of now we just send success for the data error command. */
    xResPack.statUpdate = ucACK;

    prvCreateTxPacket ((uint8_t *)&xResPack, sizeof(xResPack)) ;

    /* Start the transmission*/
    xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX) ;

    /* After sending the ACK send event intelSESSION_TX_RX_COM_FAIL so that ORWL
     * will shutdown after turning off the Intel. This is being done because we
     * are not expecting this command from the Intel.
     */
    xEventGroupSetBits(xUartTxRXSync, intelSESSION_TX_RX_COM_FAIL);

    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__) ;
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

static void prvFormatLog(uint32_t ulTamperCause, rtcDateTime_t* pxRtcDateTime,
			uint8_t* pucString)
{
    uint8_t uclen;
    strcat((char *)pucString,tamperTAMPER_CAUSE);
    /* Checking for all the bits set due to tamper */
    if(ulTamperCause&secalmDIESHIELD_FAULT)
    {
	strcat((char *)pucString,tamperDIE_SHIELD_DETECTED);
    }
    if(ulTamperCause&secalmLOW_TEMP)
    {
	strcat((char *)pucString,tamperLOW_TEMP_DETECTED);
    }
    if(ulTamperCause&secalmHIGH_TEMP)
    {
	strcat((char *)pucString,tamperHIGH_TEMP_DETECTED);
    }
    if(ulTamperCause&secalmBAT_LOW)
    {
	strcat((char *)pucString,tamperBATTERY_LOW_DETECTED);
    }
    if(ulTamperCause&secalmBAT_OVERVOLT)
    {
	strcat((char *)pucString,tamperBATTERY_OVER_VOLT_DETECTED);
    }
    if(ulTamperCause&secalmBAT_EXT_FAULT)
    {
	strcat((char *)pucString,tamperEXTN_TAMPER_DETECTED);
    }
    /* Concatenating the time of tamper occurrence */
    strcat((char *)pucString,tamperOCCURED_TIME);
    uclen = (uint8_t)strlen((char *)pucString);
    lite_snprintf(&pucString[uclen], tamperFORMATED_TIME_LEN,
	    "%02u-%02u-%04u %02u:%02u:%02u\0",
	    pxRtcDateTime->ucMonth,
	    pxRtcDateTime->ucDate,
	    pxRtcDateTime->usYear,
	    pxRtcDateTime->ucHour,
	    pxRtcDateTime->ucMinute,
	    pxRtcDateTime->ucSecond);
}
/*----------------------------------------------------------------------------*/

static int32_t prvInitUartPort( void )
{
    int32_t lResult = COMMON_ERR_UNKNOWN;
    mml_uart_config_t xUartConfig;

    /* Initialize the buffer. */
    memset(&xBuffer, 0 ,sizeof(xBuffer)) ;
    xBuffer.usByteFree = intelSUC_BUFF_RX_SIZE ;
    xRxProcessBuff.eState = eINIT_STATE ;

    /* Let us initialize the UART1 for Receiving the data */
    xUartConfig.baudrate = K_LITE_UART0_DEFAULT_BAUDRATE;
    xUartConfig.data_bits = MML_UART_DATA_TRANSFER_SIZE_8_BITS;
    xUartConfig.flwctrl = MML_UART_HW_FLOW_CTL_DISABLE;
    xUartConfig.parity = MML_UART_PARITY_NONE;
    xUartConfig.parity_mode = MML_UART_PARITY_MODE_ONES;
    xUartConfig.rts_ctl = MML_UART_RTS_IO_LEVEL_LOW;
    xUartConfig.stop_bits = MML_UART_STOPBITS_ONE;
    xUartConfig.handler = (volatile mml_uart_handler_t) prvIntelSucUartIsr;
    lResult = mml_uart_init(MML_UART_DEV1, xUartConfig);

    debugPRINT_SUC_INTEL_COMM("Installed ISR for UART1 \n\r") ;


    if( NO_ERROR == lResult )
    {
	/* Disable interruption */
	lResult = M_MML_UART_INTERRUPT_DISABLE(MML_UART_DEV1);
	if ( lResult )
	{
		goto echo_uart_init_out;

	}
	    debugPRINT_SUC_INTEL_COMM(
		    "Installed ISR setting the interrupts rightly \n\r") ;

	/* Clear all IRQ ... */
	mml_uart_interrupt_clear(MML_UART_DEV1, ( MML_UART_ISR_FRAMIS_MASK |\
						     MML_UART_ISR_PARIS_MASK |\
						     MML_UART_ISR_SIGIS_MASK |\
						     MML_UART_ISR_OVERIS_MASK |\
						     MML_UART_ISR_FFRXIS_MASK |\
						     MML_UART_ISR_FFTXOIS_MASK |\
						     MML_UART_ISR_FFTXHIS_MASK ));
	/* ... set wanted interruption(s) */
	mml_uart_interrupt_set(MML_UART_DEV1, ( MML_UART_IER_FFRXIE_MASK |\
						 MML_UART_IER_FRAMIE_MASK |\
						 MML_UART_IER_PARIE_MASK |\
						 MML_UART_IER_SIGIE_MASK |\
						 MML_UART_IER_OVERIE_MASK ));
	M_MML_UART_INTERRUPT_ENABLE(MML_UART_DEV1) ;
    }
    echo_uart_init_out:
    return lResult;

}
/*---------------------------------------------------------------------------*/

static void prvCreateTxPacket (uint8_t *pucMsg, uint16_t usLen)
{
    uint8_t ucIndex;
    xTxBuffer.ucLen = usLen;
    xTxBuffer.ucPtr = 0;
    xTxBuffer.ucChksum = 0;
    for(ucIndex=0; ucIndex < usLen; ucIndex++)
    {
	xTxBuffer.ucChksum =  xTxBuffer.ucChksum + pucMsg[ucIndex] ;
    }

    debugPRINT_SUC_INTEL_COMM(" %s Received length 0x%x\n\r",__FUNCTION__,usLen) ;

    /*Copy the message to tx_buffer*/
    memcpy(&xTxBuffer.xBuff.ucRawData[0], pucMsg, usLen ) ;
}
/*---------------------------------------------------------------------------*/

void vProcessRxInterrupt( void )
{
    register uint32_t ulSr;
    volatile mml_uart_regs_t *pxReg_uart = (volatile mml_uart_regs_t*)MML_UART1_IOBASE;
    volatile uint8_t ucChar ;
    do
    {
	ucChar = pxReg_uart->dr ;

	xBuffer.ucBuff[xBuffer.usInPtr++] = ucChar ;

	if(xBuffer.usInPtr == intelSUC_BUFF_RX_SIZE)
	{
	    xBuffer.usInPtr = 0 ;
	}
	xBuffer.usByteFree-- ;

	ulSr = pxReg_uart->sr ;

    }while((ulSr & MML_UART_SR_RXEMPTY_MASK ) == 0);
}
/*---------------------------------------------------------------------------*/

static void prvIntelSucUartIsr( void )
{
    register uint32_t ulIsr;
    volatile mml_uart_regs_t *pxRegUart = (volatile mml_uart_regs_t*)MML_UART1_IOBASE;

    /* Read the interrupt status register */
    ulIsr = pxRegUart->isr ;

    /*Check if any rx error interrupts are there */
    if (ulIsr & (MML_UART_IER_FRAMIE_MASK|MML_UART_IER_PARIE_MASK))
    {
	/* Flush the rx FIFO */
	mml_uart_flush_raw(MML_UART_DEV1, ( MML_UART_CR_RXFLUSH_MASK ));

	/* Check this is in data packet stage or header */
	xBuffer.usInPtr = xBuffer.usOutPtr ;
	xBuffer.usByteFree = intelSUC_BUFF_RX_SIZE ;
    }
    /* Check if there is some data received */
    else if (ulIsr & MML_UART_ISR_FFRXIS_MASK)
    {
	vProcessRxInterrupt() ;
    }

    /*Lets clear the interrupt at UART level and then at UART peripheral level*/
    /** Clear all IRQ ... */
    mml_uart_interrupt_clear(MML_UART_DEV1, ulIsr);

    /** Acknowledge interrupt at platform level */
    mml_uart_interrupt_ack(MML_UART_DEV1);
}
/*---------------------------------------------------------------------------*/

int32_t lIntelSendPacket(uint8_t *pucMsg, uint8_t *pucLen, TickType_t * pxTimeout )
{
    int32_t lResult = NO_ERROR;
    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__) ;

    /*Get the Mutex*/
    if(xSemaphoreTake( xsUartTxSync, ( TickType_t ) *pxTimeout ) == pdTRUE)
    {
	/* We have now exclusive transmission */
	prvCreateTxPacket(pucMsg, *pucLen) ;

	/*Indicate TxRx Task to Transmit*/

	/* Wait for reply from TxRx Task */
    }
    else /* Timeout occurred. */
    {
	lResult = intelSEND_TIMEOUT ;
    }

    /* Release mutex */
    xSemaphoreGive(xsUartTxSync) ;
    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__) ;

    return lResult ;
}
/*---------------------------------------------------------------------------*/

uint8_t ucIntelTxUartData (uint8_t *pucMsg, uint8_t ucLen)
{

    uint8_t ucTxLen = 0;
    volatile mml_uart_regs_t *xReg_uart = (volatile mml_uart_regs_t*)MML_UART1_IOBASE;

    debugPRINT_SUC_INTEL_COMM("Entry %s\n\r",__FUNCTION__) ;

    do
    {
	xReg_uart->dr = pucMsg[ucTxLen] ;

	debugPRINT_SUC_INTEL_COMM("TxData - 0x%x\n\r",pucMsg[ucTxLen]) ;
	ucTxLen++ ;

	if(ucTxLen == ucLen)
	{
	    break ;
	}

    /* Tx FIFO Is not full */
    }while((xReg_uart->sr & MML_UART_SR_TXFULL_MASK) != MML_UART_SR_TXFULL_MASK);
    debugPRINT_SUC_INTEL_COMM("Exit %s\n\r",__FUNCTION__) ;

    return ucTxLen ;
}
/*---------------------------------------------------------------------------*/

void vIntelProcessRxPacket( void )
{
    uint8_t ucTxLen;
    uint8_t ucIndex;
    uint8_t ucChecksum;
    uint8_t ucSizeToRead;
    uint8_t ucTemp ;

    EventBits_t xTempSetBit ;

    if(xBuffer.usInPtr == xBuffer.usOutPtr)
    {
	/* Indicates that there is no received data */
	return;
    }
    else
    {
	debugPRINT_SUC_INTEL_COMM(
		" vProcessRxPacket Entry free buffer size %d \n\r",
		xBuffer.usByteFree);
	/**
	 * Received some data.
	 * let us see whether we have right data.
	 */
	ucSizeToRead = intelSUC_BUFF_RX_SIZE - xBuffer.usByteFree;
	if(ucSizeToRead)
	{
	    /**
	     * We have received some data and let us see whether it makes
	     * packet for processing or packet for ACK/NACK
	     */
	    do
	    {
		ucTemp = xBuffer.ucBuff[xBuffer.usOutPtr++];
		debugPRINT_SUC_INTEL_COMM(" Received buffer data 0x%2x \n\r"
			,ucTemp);
		if (xBuffer.usOutPtr == intelSUC_BUFF_RX_SIZE)
		{
		    xBuffer.usOutPtr = 0 ;
		}
		xBuffer.usByteFree++ ;
		switch(xRxProcessBuff.eState)
		{
		case eINIT_STATE:
		    if (ucTemp == 0xFE)
			xRxProcessBuff.eState = eSOF_STATE;
		    else
			xRxProcessBuff.eState = eINIT_STATE;
		    break ;

		case eSOF_STATE:
		    if (ucTemp == 0xDF)
			xRxProcessBuff.eState = ePACKET_TYPE_STATE;
		    else
			xRxProcessBuff.eState = eINIT_STATE;
		    break ;

		case ePACKET_TYPE_STATE:
		    if (ucTemp == UART_TYP_ENC ||ucTemp ==UART_TYP_ENC_NO )
		    {
			xRxProcessBuff.eState = eLENGTH_STATE;
		    }
		    else
		     {
			/* Received ACK/NACK Packet */
			xRxProcessBuff.eState = eACK_NACK_STATE ;
		     }
		    break ;

		case eACK_NACK_STATE:
		    if(ucTemp == UART_TRANS_NACK)
		    {
			xTempSetBit = xEventGroupSetBits(xUartTxRXSync,
				intelSESSION_RX_NACK);
		    }
		    else
		    {
			xTempSetBit = xEventGroupSetBits(xUartTxRXSync,
				intelSESSION_RX_ACK);

		    }
		    xRxProcessBuff.eState = eINIT_STATE ;
		    debugPRINT_SUC_INTEL_COMM(
			    "RECEIVED ACK-NACK Packet and processed "
			    "information for 0x%x\n\r",xTempSetBit);
		    break ;

		case eLENGTH_STATE:
		    xRxProcessBuff.ucLen = ucTemp ;
		    xRxProcessBuff.eState = eCHECKSUM_STATE ;
		    break ;

		case eCHECKSUM_STATE:
		    xRxProcessBuff.ucChksum = ucTemp ;
		    xRxProcessBuff.eState = ePAYLOAD_STATE ;
		    xRxProcessBuff.ucPtr = 0 ;
		    break ;

		case ePAYLOAD_STATE:
		    xRxProcessBuff.xBuff.ucRawData[xRxProcessBuff.ucPtr++] = ucTemp ;
		    if(xRxProcessBuff.ucPtr == xRxProcessBuff.ucLen )
		    {
			/**
			 *Received the complete packet. Let us process the same
			 *received packet from Intel
			 *check the checksum
			 */
			ucChecksum = 0 ;
			for(ucIndex=0; ucIndex<(xRxProcessBuff.ucLen);ucIndex++)
			{
			    ucChecksum = xRxProcessBuff.xBuff.ucRawData[ucIndex]
									+ ucChecksum ;
			}

			debugPRINT_SUC_INTEL_COMM(" "
				"Calculated Ck=0x%x Received 0x%x\n\r",
				ucChecksum, xRxProcessBuff.ucChksum);

			/* we received checksum correctly */
			if(ucChecksum == xRxProcessBuff.ucChksum)
			{
			    /* Let us transmit */
			    xAckNackPkt.ackStatus = UART_TRANS_ACK ;
			    debugPRINT_SUC_INTEL_COMM(
				    "Transmitting ACK Packet\n\r") ;

			}
			else
			{
			    /* Let us transmit */
			    xAckNackPkt.ackStatus = UART_TRANS_NACK ;
			    debugPRINT_SUC_INTEL_COMM(
				    " Transmitting NACK Packet\n\r") ;

			}

			ucTxLen = ucIntelTxUartData((uint8_t *)&xAckNackPkt,
				sizeof(xAckNackPkt));

			if(ucTxLen != sizeof(xAckNackPkt))
			{
			    debugPRINT_SUC_INTEL_COMM(
				    " Couldn't transmit ACK Packet\n\r") ;
			}
			else
			{
			    debugPRINT_SUC_INTEL_COMM(
				    " Transmitted NACK/ACK Packet\n\r") ;
			}

			if(ucChecksum == xRxProcessBuff.ucChksum)
			{
			    for(ucIndex=0 ; ucIndex<(sizeof(xIntelSucLookUpAction)
				    /sizeof(xIntelSucCommHandling));ucIndex++)
			    {
				if(xRxProcessBuff.xBuff.action.dataPktTyp ==
					xIntelSucLookUpAction[ucIndex].ucPktType )
				    break ;
			    }
			    /* There might be case where BIOS can send some command
			     * which are not yet supported from SUC. In that case
			     * handle this error case to avoid SUC hanging.
			     */
			    if(ucIndex >= (sizeof(xIntelSucLookUpAction)
					    / sizeof(xIntelSucCommHandling)))
			    {
				/* Send invalid command event */
				xEventGroupSetBits(xUartTxRXSync, intelSESSION_INVALID_COMMAND);
				/* We don't want to proceed further in this case
				 * return the context.
				 */
				return;
			    }
			    debugPRINT_SUC_INTEL_COMM(" Received Pkt type 0x%x\n\r",
				    xRxProcessBuff.xBuff.action.dataPktTyp) ;

			    /* We received right data. Let us process the command*/
			    if(xRxProcessBuff.xBuff.action.cmd == CMD_READ)
			    {
				if (xIntelSucLookUpAction[ucIndex].cmd_read
					!= NULL)
				{
				    debugPRINT_SUC_INTEL_COMM(
					    " Processing Read of packet type "
					    "0x%x\n\r",
					    xRxProcessBuff.xBuff.action.dataPktTyp);
				    if( xIntelSucLookUpAction[ucIndex].cmd_read()
					    != NO_ERROR )
				    {
					prvShutdownAndDisplayMessage(
						(int8_t *)"Intel SuC Com   "
							"Failed Reboot...");
				    }

				}
				else
				{
				    debugPRINT_SUC_INTEL_COMM(
					    " No Read functionality defined"
					    "for packet type 0x%x\n\r",xRxProcessBuff.
					    xBuff.action.dataPktTyp);
				}
			    }
			    else
			    {
				if (xIntelSucLookUpAction[ucIndex].cmd_write != NULL)
				{
				    debugPRINT_SUC_INTEL_COMM(
					    " Processing Write of packet type"
					    "  0x%x\n\r",
					    xRxProcessBuff.xBuff.action.dataPktTyp);
				    if(xIntelSucLookUpAction[ucIndex].cmd_write() !=
					    NO_ERROR)
				    {
					prvShutdownAndDisplayMessage(
						(int8_t *)"Intel SuC Com   "
							"Failed Reboot...");
				    }
				}
				else
				{
				    debugPRINT_SUC_INTEL_COMM(
					    " No Write functionality defined"
					    " for packet type  0x%x\n\r",
					    xRxProcessBuff.xBuff.action.dataPktTyp) ;
				}
			    }
			}

			/*
			 * we have finished packet receiving.
			 * Let us restart the packet reception
			 */
			xRxProcessBuff.eState = eINIT_STATE ;

			/* We have received one packet for processing.
			 * Let us not process further data
			 * complete the action for this received packet */
			return ;
		    }
		    break ;

		default:
		    break ;

		}
		ucSizeToRead-- ;

	    /* Indicates that we have completed all the bytes to be read */
	    }while(ucSizeToRead);
	}

    }
    debugPRINT_SUC_INTEL_COMM("Exiting the function %s\n\r",__FUNCTION__);
}
/*---------------------------------------------------------------------------*/

void vUartTxRx( void )
{
    EventBits_t xRetBits;
    EventBits_t xRetBitsTx ;
    uint32_t ulTxLen,ulTotTxLen ,ulRetryCount;
    /* Block for 5ms. */
    const TickType_t xDelay = intelDELAY_MS / portTICK_PERIOD_MS;
    ulRetryCount = 0 ;

    debugPRINT_SUC_INTEL_COMM(" In UART Rx TX Task\n\r") ;

    while(1)
    {
	/* Wait for the events that need to be set for activity to be performed.*/
	xRetBits = xEventGroupWaitBits(xUartTxRXSync,
		( intelSESSION_TX | intelSESSION_TX_RX_COM_FAIL |
		  intelSESSION_AUTH_FAIL | intelSESSION_INVALID_COMMAND), /* const EventBits_t uxBitsToWaitFor */
		    pdTRUE,		/* const BaseType_t xClearOnExit */
			pdFALSE ,	/*const BaseType_t xWaitForAllBits */
			    0 );	/* Return immediately */


	vIntelProcessRxPacket();

	if(xRetBits & intelSESSION_TX)
	{
	    debugPRINT_SUC_INTEL_COMM(" Received Transmit Session\n\r") ;
	    ulTotTxLen = 0 ;
	    ucPreAmble[3] = xTxBuffer.ucLen ;
	    ucPreAmble[4] = xTxBuffer.ucChksum;
	    ulTotTxLen = 0 ;

	    /* Lets Transmit ucPreAmble*/
	    do
	    {
		ulTxLen = ucIntelTxUartData((uint8_t *)&ucPreAmble[ulTotTxLen], sizeof( ucPreAmble ) - ulTotTxLen);
		ulTotTxLen = ulTotTxLen + ulTxLen ;
		vTaskDelay(xDelay) ;
	    }while (ulTotTxLen != sizeof(ucPreAmble));

	    ulTotTxLen = 0 ;
	    do
	    {
		ulTxLen = ucIntelTxUartData(&xTxBuffer.xBuff.ucRawData[ulTotTxLen],
		    xTxBuffer.ucLen);
		ulTotTxLen = ulTotTxLen + ulTxLen ;
		vTaskDelay(xDelay) ;
	    }while (ulTotTxLen != xTxBuffer.ucLen) ;

	    do
	    {
		/* Wait for the ACK/NACK Packet */
		xRetBitsTx = xEventGroupWaitBits(xUartTxRXSync,
			( intelSESSION_RX_NACK | intelSESSION_RX_ACK) /* const EventBits_t uxBitsToWaitFor */,
			    pdTRUE 		/* const BaseType_t xClearOnExit */,
				pdFALSE		/* const BaseType_t xWaitForAllBits */,
				    0 );
		debugPRINT_SUC_INTEL_COMM("Waiting for ACK/NACK Packet 0x%x\n\r",xRetBitsTx);

		if ((xRetBitsTx & intelSESSION_RX_ACK))
		{
		    /* Recieved ACK. So we are success*/
		    xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX_SUCCESS);
		    ulRetryCount = 0 ;
		    debugPRINT_SUC_INTEL_COMM(
			    "Received ACK. Success in transmission \n\r");

		    /* exit */
		    break ;
		}
		else if((xRetBitsTx & intelSESSION_RX_NACK))
		{
		    if(ulRetryCount != 3)
		    {
			ulRetryCount++ ;
			/* Set event for self to retransmit the packet */
			xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX);
			debugPRINT_SUC_INTEL_COMM(
				"Received NACK. Retransmitting %d try \n\r",
				ulRetryCount);
			    break ;
		    }
		    else
		    {
			ulRetryCount = 0 ;
			xEventGroupSetBits(xUartTxRXSync,intelSESSION_TX_FAIL) ;
			debugPRINT_SUC_INTEL_COMM(
				"Received NACK. After %d try, fail to transmit\n\r",
				ulRetryCount) ;
			break ;
		    }
		}
		vTaskDelay(xDelay) ;
		vIntelProcessRxPacket();
	    }while(1) ; /* Do for the reception of ACK/NACK packet */
	}
	if(xRetBits & intelSESSION_TX_RX_COM_FAIL)
	{
	    debugERROR_PRINT("Intel write command got negative response\n");
	    debugERROR_PRINT("System halted please reboot\n");
	    prvShutdownAndDisplayMessage(
		    (int8_t *)"Intel SuC Com   Failed Reboot...");
	}
	if(xRetBits & intelSESSION_AUTH_FAIL)
	{
	    prvShutdownAndDisplayMessage(
		    (int8_t *)"Authentication  Failed Reboot...");
	}
	if(xRetBits & intelSESSION_INVALID_COMMAND)
	{
	    prvShutdownAndDisplayMessage(
		    (int8_t *)"Invalid command received Reboot...");
	}
	/* Let us sleep for the scheduler to spawn */
	vTaskDelay(xDelay) ;
    }
}
/*---------------------------------------------------------------------------*/

int32_t lIntelSucComInit( void )
{
    int32_t lResult = NO_ERROR;

    /**
     * Initialize the ACK/NACK pkt
     * checksum and status to be updated as needed to send
     * ACK or NACKs
     */
    xAckNackPkt.sof[0] = 0xFE ;
    xAckNackPkt.sof[1] = 0xDF ;
    xAckNackPkt.typ = UART_TYP_ACK_NACK ;
    TaskHandle_t xTempHandle;
    lSizxIntelSucLookUpAction = sizeof(xIntelSucLookUpAction)/sizeof(xIntelSucCommHandling);

    /* Initialize the data link layer of IntelSuC Communication. i.e. UART */
    lResult = prvInitUartPort();
    if ( lResult!= NO_ERROR )
    {
	return lResult ;
    }

    /**
     * Create Event Flag for getting the events from different tasks and
     * for management
     */
    xUartTxRXSync = xEventGroupCreate();
    configASSERT( xUartTxRXSync != NULL );

    xsUartTxSync = xSemaphoreCreateMutex ();
    configASSERT( xsUartTxSync != NULL );

    debugPRINT_SUC_INTEL_COMM(" Completed Creation of Event and Semaphore\n\r") ;

    /* Create task for managing data reception and sending to session task */
    lResult = xTaskCreate(vUartTxRx, "IntelSuC",
		configSTACK_SIZE_INTEL_SUC_MANAGE_DATA, NULL,
		ePRIORITY_IDLE_TASK, &xTempHandle);
    if( lResult != pdPASS ) {
	    while ( 1 )
		    ;
    }

    debugPRINT_SUC_INTEL_COMM(" Task Created Successfully\n\r") ;

    return NO_ERROR ;
}
/*---------------------------------------------------------------------------*/
