 /**===========================================================================
 * @file user_config.h
 *
 * @brief This file contains all the data structures and macros for user config.
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
#ifndef configINCLUDE_USER_CONFIG_H_
#define configINCLUDE_USER_CONFIG_H_

/* Global includes */
#include <stdint.h>

/**
 * ORWL user config magic header size.
 */
#define configCONFIG_MAGIC_SIZE		(4)

/**
 * ORWL user config reserved bytes to make 32 bytes.
 */
#define configRESERVED			(19)

#define userconfigBLE_RANGE		(PROX_PROT_05M) /**<BLE range in meters*/
#define userconfigDISABLE		(PROX_PROT_IO_DIS) /**<IO disable status */
#define userconfigMODE			(PROX_PROT_MOD_STD_BY) /**<Intel subsystem mode */
#define userconfigPROXPROTACTION	(PROX_PROT_MOT_EN) /** system default action is shutdown on proximity lock */
#define userconfigASKPINONBOOT		(PROX_PROT_BOOTPIN_DIS) /** disable pin request on boot */
#define userconfigDISABLEWIFI		(1 << 1) /** Disable wifi, set bit 1 */
#define userconfigADVR_ROT_INT		(15) /**<Advertisement rotation interval (in minutes)*/

/**
 * TODO:
 * This structure not used efficiently to handle all user mode configs.
 * Need to use it for correct user experience.
 * ORWL user configuration structure.
 */
typedef struct xUSER_CONFIG
{
    /** User config magic header */
    uint32_t ulMagicHeader;
    /** BLE RANGE */
    uint8_t ucBLERange;
    /** Status of IO Enable/Disable */
    uint8_t ucIOEnDis;
    /** Advertisement rotation interval */
    uint8_t ucAdverRotInt;
    /** Mode of Intel subsytem */
    uint8_t ucMode;
    /** SuC action on proximity timeout */
    uint32_t ulSuCAction;
    /** Whether Pin required on boot or not */
    uint8_t ucAskPinOnBoot;
    /** reserved bytes */
    uint8_t ucReserved[configRESERVED];
}xUserConfig_t;

/** Function declaration */

/** @brief Writes user config data.
 *
 * This function writes user config data to flash.
 *
 * @param pxUserConfig pointer to user config structure.
 * @return error code..
 *
 */
int32_t lUserWriteUserConfig(xUserConfig_t *pxUserConfig);

/** @brief Reads user config data.
 *
 * This function reads user config data from flash.
 *
 * @param pxUserConfig pointer to user config structure.
 * @return error code..
 *
 */
int32_t lUserReadUserConfig(xUserConfig_t *pxUserConfig);

#endif /* configINCLUDE_USER_CONFIG_H_ */
