/**===========================================================================
 * @file app_init.h
 *
 * @brief This file contains the boot mode identification macros and helper
 * routine definitions.
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
#ifndef _INCLUDE_APP_INIT_H_
#define _INCLUDE_APP_INIT_H_

/* We need to have the state values mapping to Intel SuC Communication Dev State*/
#include <intel_suc_comm.h>
#include <stdint.h>

/**
 * Check if GPIO init has been done. This is need NMI context to check if the
 * GPIO init was done. Set bit 1 to inform GPIO init is done.
 */
#define initGPIO_INIT_DONE		(1 << 1)
/**
 * Check if Display init has been done. This is need NMI context to check if the
 * Display init was done. Set bit 2 to inform Display init is done.
 */
#define initDISPLAY_INIT_DONE		(1 << 2)

/**
 * @brief ORWL SuC boot mode definition.
 * This values must be stored in the product_lifecycle field of flash
 * Useful in determining mode for SuC on boot
 */
typedef enum EBOOT_MODES {
	eBOOTMODE_IVALID = 0x00,
	eBOOTMODE_PRODUCTION_PCBA= ORWL_DEV_STATE_PROD_PCBA,
	eBOOTMODE_PRODUCTION_FA=ORWL_DEV_STATE_PROD_FA,
	eBOOTMODE_PRODUCTION_TAMPER=ORWL_DEV_STATE_PROD_TAMPER,
	eBOOTMODE_DEV_ROT=ORWL_DEV_STATE_ROT,
	eBOOTMODE_DEV_OOB=ORWL_DEV_STATE_HLOS_OOB,
	eBOOTMODE_DEV_USER=ORWL_DEV_STATE_USER,
	eBOOTMODE_DEV_USER_TAMPER=ORWL_DEV_STATE_TAMPERED,
	eBOOTMODE_DEV_ENTER_BIOS=ORWL_DEV_STATE_ENTER_BIOS,
	eBOOTMODE_DEV_UNKOWN_FA_STATE,
	eBOOTMODE_DEV_ERROR,
	eBOOTMODE_MAX
} eBootModes;

/**
 * @brief This function reads the current boot mode of the device.
 *
 * @param pulProdCycle is populated with current boot mode of the
 * device from xBootModes enum
 *
 * @return NO_ERROR on success and error code on failure
 */
int32_t lInitGetSuCBootMode( uint32_t *pulProdCycle );

/**
 * @brief This function store the device boot mode on flash
 * Whenever there is a transition from one mode to other, it is ideal to
 * power cycle the device (bu writing to soft reset registers).
 *
 * @param xBootMode device boot mode to be set
 *
 * @return NO_ERROR on success and error code on failure
 */
uint32_t ulInitSetSuCBootMode( eBootModes eBootMode );

/**
 * @brief This function is just an infinite loop from which device will never
 * be able to come back. This will be called only if tamper is detected
 * @return void
 */
void vInitEnterErrorState( void );

/**
 * @brief This function is check for the system state.
 * i.e. it reads tamper module status and changes system state to ERROR &
 * then runs to infinite loop if device tampered.
 *
 * @param ulProdCycle Product cycle of ORWL device.
 *
 * @return error code.
 */
int32_t lInitSystemStateCheck( uint32_t ulProdCycle );

/**
 * @brief This function displays the ORWL Welcome screen
 *
 * @return void
 */
void vInitDisplayWelcomeScreen ( void );


/**
 * @brief This function displays the ORWL Welcome screen in user mode
 *
 * @return void
 */
void vInitDisplayUserModeWelcomeScreen ( void );

/**
 * @brief This function displays tampered Logo
 *
 * @return void
 */
void vInitDisplayTamperScreen ( void );

/**
 * @brief This function displays tampered Logo. This should be called from
 * NON RTOS context.
 *
 * @return void
 */
void vInitDisplayTamperScreenNONRTOS ( void );

/**
 * @brief This function calculates simple CRC based on addition
 * Overflow is ignored
 *
 * @return CRC value of the input provided
 */
uint16_t usInitCommonCalcCrc( const uint8_t *pucIn, uint32_t ulLen );
#endif //_INCLUDE_APP_INIT_H_
