/**===========================================================================
 * @file systemRes.h
 *
 * @brief This file contains all data types and API' that are used across
 * different boot modes of the system.
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

#ifndef _INCLUDE_SYSTEMRES_H_
#define _INCLUDE_SYSTEMRES_H_
/* local includes */
#include <orwl_gpio.h>
#include <stdint.h>
#include <stddef.h>
#include <delay.h>
#include <reset.h>

#define SystemResINTEL_POWER_SEQ_DELAY_MS		(500)		/**< 500 ms delay for Intel power on sequence */
#define SystemResINTEL_LONG_PRESS_DELAY			(15000)		/**< 15 secs Delay for long button press */
#define SystemResINTEL_SHUTDOWN_WAIT_PERIOD		(3000)		/**< 3 sec wait for intel shutting down */
#define SystemResDELAY_INC_1S_30S_COUNT			(10)		/**< For having 30seconds with 3second increment delay */
/**
 * @brief structure to hold the GIO details required by the application
 * All these GIO mux config and init are done during boot by GIO module
 * software. Below structure to hold only the particular pin identity to
 * controller it at during SM execution
 */
typedef struct xGIO_HANDLE {
	/** describes the GIO the group to which GIO belongs to */
	uint32_t ulGropuId;

	/** holds the GIO PIN number*/
	uint32_t ulOPinId;
} xgGIOHandle_t;

/**
 * @brief global structure to hold information across various modes of SuC
 * operation. These are the global resource used in most of the operation
 * modes of SuC & hence this structure is common across all modes.
 *
 * No synchronization is required for this resource since
 * In a particular boot mode only one thread is expected
 * to access this resource
 */
typedef struct xSUC_SYSTEM_RESOURCES {
	/* GPIO's */
	/** system GIO0 is SuC out --> Intel CPU IN.
	 * This GIO is used to send power on/off command to the Intel system
	 * based on security state machine of SuC
	 */
	xgGIOHandle_t xUCSysGio0;

	/** system GIO3 is SuC out --> Intel CPU IN.
	 * This GIO is used to indicate the Intel CPU/BIOS to control the
	 * display subsystem based on security state machine state on SuC.
	 */
	xgGIOHandle_t xUCSysGio3;

	/** system GIO2 is Intel out --> SuC  IN.
	 * This GIO is sampled frequently by SuC to understand the Intel CPU's
	 * 'S' states.
	 */
	xgGIOHandle_t xUCSysGio2;

	/** system xUCSysECPwnBtn is SuC out --> Intel PWR System Control.
	 * This GIO is used for sending short press and long press event to
	 * Intel Sub-system
	 */
	xgGIOHandle_t xUCSysECPwnBtn;

	/** system xUCSysUSBPort1En is SuC out --> USB Port 1 Control.
	 * This GIO is used to control the MUX enable and disable
	 */
	xgGIOHandle_t xUCSysUSBPort1En;

	/** system xUCSysUSBPort2En is SuC out --> USB Port 1 Control.
	 * This GIO is used to control the MUX enable and disable
	 */
	xgGIOHandle_t xUCSysUSBPort2En;

	/** system xUCSysUSBPort1Sel is SuC out --> USB Port 1 Selection.
	 * This GIO is used to control the selection of USB Port 1 for
	 * BT programming or for USB port usage of the system
	 */
	xgGIOHandle_t xUCSysUSBPort1Sel;

	/** system xUCSysUSBPort2Sel is SuC out --> USB Port 2 Selection.
	 * This GIO is used to control the selection of USB Port 2 for
	 * UART debugging or for the USB port usage.
	 * In user mode for any debug print, this should be enabled before
	 * sending UART messages
	 */
	xgGIOHandle_t xUCSysUSBPort2Sel;

	/** It is SuC IN <-- LE out
	 * This GIO is sampled frequently by SuC to understand the proximity check
	 * status
	 */
	xgGIOHandle_t xProximityCheck;

	/*TODO: All other resources such as SPI/I2C to follow here */
} xgSysRes_t;

/**
 * @brief This function configures all the system resources such as GIO
 * required for ORWL operation
 *
 * @return error code on failure and 0 on success
 */
uint32_t ulSystemResInitGlobalSystemResources( void );

/**
 * @brief This function turn ON the Intel CPU by enabling a power and GPIO
 *
 * This enable API will power on the Intel CPU. It should be called only
 * in OS scheduled functions.
 *
 * @return void
 */
void vSystemResPowerOn_IntelCpu( void );

/**
 * @brief This function turn OFF the Intel CPU by asserting gpio.
 *
 * This will initiate the shutdown sequence for the Intel CPU.
 * This function should be called only from OS task
 *
 * @return void
 */
void vSystemResPowerOff_IntelCpu( void );

/**
 * @brief This function force shutdown the Intel CPU by asserting gpio
 * gpioUC_VIN_EN. This shutdown mimics the power cord removal shutdown.
 *
 * @return void
 */
void vSystemResForcePowerOff_IntelCpu( void );

/**
 * @brief This function will indicate BIOS to power ON display by setting a GIO
 *
 * This is an API which can be used for testing Intel CPU is OFF or not
 *
 * @return uint8_t
 * 0 -> Indicate Intel CPU has shutdown
 * 1 -> Indicate Indicate Intel CPU is still active
 */
uint32_t ulSystemResPowerState_Intel( void );

/**
 * @brief This function will put Intel in Proximity Protection State
 *
 * This is an API which can be used for disabling the IO interfaces and putting
 * Intel subsystem to proximity protection state
 *
 * @return void
 */
void vSystemResProxi_On( void );

/**
 * @brief This function will bring back Intel from Proximity Protection State
 *
 * This is an API which can be used for enabling the IO interfaces and brining
 * Intel subsystem from proximity protection state
 *
 * @return void
 */
void vSystemResProxi_Off( void );

/**
 * @brief This function will generate short key press event for Intel Subsystem
 *
 * This is an API which can be used for generating a short key press.
 * This is useful when the user has entered proximity protection state and
 * returned. Proxi OFF will bring back most of the HW IPC and other communications
 * to required level. Generating this event will bring back the Intel Subsystem
 *
 * @return void
 */
void vSystemResIntel_ShortPress( void );

/**
 * @brief This function will generate Long key press event for Intel Subsystem
 *
 * This is an API which can be used for generating a Long key press.
 * This is useful when the user wants to shutdown the system.
 *
 * @return void
 */
void vSystemResIntel_LongPress( void );

/**
 * @brief This function will provide support for realizing shutdown due to motion
 *
 * This is used for removing the power to Intel subsystem.
 * In proximity protection
 *
 * @return void
 */
void vSystemIntelMotion_ShutDown( void );

/**
 * @brief This function will indicate BIOS to power ON display by setting a GIO
 *
 *TODO: This is an optional API which may not be used in final SM
 *
 * @return void
 */
void vSystemResPowerOn_IntelDisp( void );

/**
 * @brief This function will indicate BIOS to power OFF display by
 * asserting a GIO
 *
 *TODO: This is an optional API which may not be used in final SM
 *
 * @return void
 */
void vSystemResPowerOff_IntelDisp( void );

/**
 * @brief This function erases all the NVSRAM contents and Flash 'KEY' area
 *
 * @return void
 */
void vSystemResEraseAllKeys( void );

/**
 * @brief This function will configure the LE Interrupt GPIO pin as input pin.
 *
 * @return void
 */
void vSystemResBT_GPIO_Config( void );

/**
 * @brief This function reads the LE GPIO pin status.
 *
 * @param pucStatus pointer to store pin status
 *
 * @return void
 */
void vSystemResBT_GPIO_Status( uint8_t *pucStatus );

/**
 * @brief This function enables the USB ports to Intel
 *
 * @return void
 */
void vEnableUSBToIntel( void );

/**
 * @brief This function Disables the USB ports to Intel
 * In other words, it enables SuC UART
 *
 * @return void
 */
void vDisableUSBToIntel( void );

/* error state for ROT and OOb to be written here */
#endif //_INCLUDE_SYSTEMRES_H_
