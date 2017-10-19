/**===========================================================================
 * @file user.h
 *
 * @brief This file contains routines and definition for ORWL User mode
 *
 * @author ravikiran@design-shift.com
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

#ifndef _INCLUDE_USERMODE_H_
#define _INCLUDE_USERMODE_H_
/* stddef include */
#include <stddef.h>

#include <FreeRTOS.h>
#include <task.h>
#include <portable.h>
#include <semphr.h>

/* event group include */
#include <event_groups.h>

/* debug header */
#include <debug.h>

/* queue */
#include <queue.h>

#include <orwl_disp_interface.h>
#include <keyfobid.h>
#include <nfc_common.h>
#include <le_interface.h>
#include <prng.h>
#include <mpuinterface.h>
#include <intel_suc_comm_manager.h>
#include <nvsram.h>
#include <orwl_timer.h>
#include <user_config.h>
#include <oled_ui.h>

#define usermodeDELAY_0				(0)		/**< 0 second delay */
#define usermodeDELAY_MSEC_100			(100)		/**< 100 millisecond delay */
#define usermodeDELAY_SEC_1			(1000)		/**< 1 second delay */
#define usermodeDELAY_SEC_2			(2000)		/**< 2 second delay */
#define usermodeDELAY_SEC_3			(3000)		/**< 3 second delay */
#define usermodeDELAY_SEC_10			(10000)		/**< 10 second delay */
#define usermodeDELAY_SEC_30			(30000)		/**< 30 second delay */
#define usermodeDELAY_SEC_120			(120000)	/**< 120 second delay */
#define usermodeINDEX_0				(0)		/**< 0th index */
#define usermodeINDEX_1				(1)		/**< 1st index */
#define usermodeINDEX_2				(2)		/**< 2nd index */

#define usermodeMSG_DISPLAY_TIME		(usermodeDELAY_SEC_3)	/**< display text on OLED time*/
#define usermodeMIN_WAIT_DELAY			(usermodeDELAY_MSEC_100)/**< Minimum delay for xEventGroupWaitBits */

/* LE related macro */
#define usermodeLE_GPIO_LOW			(0)		/**< GPIO status low */
#define usermodeLE_GPIO_HIGH			(1)		/**< GPIO status high */
#define usermodeLE_MAX_RETRY			(300)		/**< Maximum LE alive check retry after NFC authentication */
#define usermodeLE_NUM_OF_BYTES			(3)		/**< No of bytes to read to check GPIO status */
#define usermodeLE_MAX_RETRY_UNLOCK_STATE	(15)		/**< Value for 9 seconds LE alive check retry in unlock state */

#define usermodeKEYFOB_SERIAL_LEN		(4)		/**< KeyFob serial length */
#define usermodePWRBTN_LOW			(0)		/**< Power button status low */
#define usermodePWRBTN_HIGH			(1)		/**< Power button status high */

/** Macro to enable Le broadcast status check using GPIO pins.
 * If 0 - EEPROM read is used for checking the Le status.
 * else if 1 - GPIO pin is used for checking the Le status.
 */
#define usermodeENABLE_GPIO_READ		(1)

/** User mode error state return value*/
#define usermodeERROR_STATE			(0xAF)

/** When proxi locked, ORWL will ask shutdown/standby Intel wait for 10 seconds
 * to handle the INTEL_DEV_ACT. In standby case 10 seconds is enough.
 */
#define usermodeINTEL_DEV_WAIT_STDBY		(10)

/** When proxi locked, ORWL will ask shutdown/standby Intel wait for 10 seconds
 * to handle the INTEL_DEV_ACT. In shutdown case 30 seconds is enough.
 */
#define usermodeINTEL_DEV_WAIT_SHTDWN		(30)

/**
 * @brief enum indicates all the state index number.
 * Each state is provided an index starting from 0 till 'eSTATE_SUSC_ST_MAX'
 * for identification.
 */
typedef enum xORWL_SUC_USER_STATES
{
	eSTATE_SUC_UST_INVALID = 0,
	eSTATE_SUC_UST_1,			/**< locked state: GIO & OLED update */
	eSTATE_SUC_UST_2,			/**< NFC authentication only */
	eSTATE_SUC_UST_3,			/**< Le Alive Check State */
	eSTATE_SUC_UST_4,			/**< Access Denial state */
	eSTATE_SUC_UST_5,			/**< Power ON Intel CPU */
	eSTATE_SUC_UST_6,			/**< Proximity Error */
	eSTATE_SUC_UST_7,			/**< Unlock state */
	eSTATE_SUC_UST_8,			/**< Proximity Lock */
	eSTATE_SUC_UST_9,			/**< Device motion detected */
	eSTATE_SUC_UST_10,			/**< LE Alive check */
	eSTATE_SUC_UST_11,			/**< Repeat NFC authentication */
	eSTATE_SUC_UST_12,			/**< Error State */
	eSTATE_SUC_UST_MAX,
} eUserSuCUserStates;

/**
 * @brief: global structure to hold information across various states of SuC
 * Only State machine application can access and modify this data
 * TODO: define all the necessary contents of the app.
 */
typedef struct xSUC_SM_GLOBAL_RESOURCE_APPLICATION {

	/* Event group handle */

	/** Event group handle for user mode application thread.
	 *  user mode application thread to wait on this handle for events.
	 */
	EventGroupHandle_t xEventGroupUserModeApp;

	/** Event group handler for NFC thread
	 */
	EventGroupHandle_t xEventGroupNFC;

	/* task handle for MPU FIFO read */
	TaskHandle_t xMpuFifoTskHandle;

	/** Event group handler for Pin entry */
	EventGroupHandle_t xEventGroupPinEntry;

	/** event handle for power button press/release events */
	EventGroupHandle_t xPowerBtnEventHandle;

	/** Queue to send pin to caller */
	QueueHandle_t xPinQueue;

	/** Queue to data transfer between nfc task and user application */
	QueueHandle_t xNfcQueue;

	/** Binary semaphore for I2C synchronization */
	SemaphoreHandle_t prvI2CSemaphore;

	/** Queue to transfer data between Intel suc task and user app
	 * This Queue will be updated by Intel */
	QueueHandle_t xSucBiosSendQueue;

	/** Queue to transfer data between Intel suc task and user app
	 * Intel suc task will be waiting on this queue for receiving data */
	QueueHandle_t xSucBiosReceiveQueue;

	/** BLE RANGE */
	uint8_t ucBLERange;

	/** Status of IO Enable/Disable */
	uint8_t ucIOEnDis;

	/** Mode of Intel subsytem */
	uint8_t ucMode;

	/** SuC action on proximity timeout */
	uint32_t ulSuCAction;

	/** Whether Pin required on boot or not */
	uint8_t ucAskPinOnBoot;
}xSMAppResources_t;

/** Buffer for data transfer between nfc and app */
typedef struct
{
	/** Key Fob name */
	uint8_t ucKeyFobName[keyfobidNAME_LEN];
	/** Seed for BLE Broad Cast Data */
	uint8_t ucBleSeed[nfccommonSEED_SIZE];
	/** KeyFob serial number */
	uint8_t ucKeyFobSerial[usermodeKEYFOB_SERIAL_LEN];
	/** Key Fob privilege */
	uint32_t ulKeyPrivilege;
	/** Ble seed increment factor */
	uint8_t ucLeIncrFactor[nfccommonSEED_INC_SIZE];
	/** Reserved field */
	uint32_t ulReserved;
}NfcEventData_t;

/** Buffer for data transfer between intel suc task and user app */
typedef struct
{
    /** Command/Response */
    uint8_t ucCommand;
    /** Sub command */
    uint8_t ucSubCommand;
    /** pointer to Data Buffer */
    void * pvData;
}IntelUserData_t;

/** Structure to keep LE seed and increment factor. This structure is used by
 * application task for updating the LE seed every 15mins.
 */
typedef struct
{
	uint8_t ucSeed[nfccommonSEED_SIZE];			/**< random number seed */
	uint8_t ucSeedIncrFactor[nfccommonSEED_INC_SIZE];	/**< Seed Increment factor */
}usermodeLeData_t;

/* User mode events */

/* NFC events */

/* user mode API's */

/**
 * @brief This function sends NFC detect command to NFC thread
 *
 * @return NO_ERROR on success and error code on failure
 */
uint32_t ulUserModeDetectKeyFobNFC( void );

/**
 * @brief This function runs ISO7816 authentication procedure with KeyFob
 * including channel encryption and data encryption procedures
 *
 * This is responsible for card Identity verification (CVM) also.
 *
 * @return 	0 on success
 * 			error codes on failure
 */
uint32_t ulUserModeAuthenticateKeyFobNFC( void );

/**
 * @brief This function exchange a seed or token with the KeyFob.
 * This seed will be further used by the KeyFob BLE for broadcast.
 *
 * @return 	0 on success
 * 			error codes on failure
 */
uint32_t ulUserModeExchangeToken( void );

/**
 * @brief This function will be called on NFC authentication failure.
 * On calling this API, OLED display will be updated with tamper
 * logo & message. Post 30 sec timeout device will change the state.
 *
 * @return 	0 on success
 * 			error codes on failure
 */
uint32_t ulUserModeAuthentiationFailureNFC( void );

/**
 * @brief This function waits for LE broadcast for 5 seconds.
 *
 * @return 	0 on success
 * 			error codes on timeout
 */
uint32_t ulUserModeBroadcastDataVerificationLE( void );

/**
 * @brief This function updates the OLED with below strings
 * unlocked logo,
 * Welcome to ORWL
 * www.orwl.org
 *
 * @return 	0 on success
 * 			error codes on failure
 */
uint32_t ulUserModeUpdateUnlockedLogoOLED( void );

/**
 * @brief This function read the proximity status of KeyFob.
 *
 * @return 	0 on success
 * 			error codes on failure
 */
uint32_t ulPUserModeroximityCheck( void );

/**
 *	@brief This function initializes all the global resources requiered in
 *	User mode operation of SuC.
 *
 *	This includes initialization of event handler and queues.
 *
 * @param *pvRes user mode state resource handler pointer
 *
 *	@return 0 on success
 *			error code on failure.
 */
uint32_t ulUserModeInitGlobalApplicationResources( xSMAppResources_t *pxRes );

/**
 * @brief this is user mode application thread. This thread is created by the main
 * application and utilized by
 *
 * @param *pvArg is a void pointer containing user mode data structure provided
 * by the caller
 *
 * @return void
 */
void vUserModeUserModeTask( void *pvArg );

#endif /*_INCLUDE_USERMODE_H_*/
