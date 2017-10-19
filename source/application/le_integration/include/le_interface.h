/**===========================================================================
 * @file le_interface.h
 *
 * @brief This file contains macros and function prototypes
 * used in BLE interfacing.
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

#ifndef LE_INTERFACE_H
#define LE_INTERFACE_H

#include <cobra_defines.h>
#include <stdint.h>
#include <string.h>
#include <errors.h>
#include <debug.h>
#include <mml_i2c.h>
#include <i2c.h>

/** 48-bit address, LSB format */
#define leBLE_GAP_ADDR_LEN	(6)

/** Nordic I2C addr */
#define leNORDIC_I2C_SLAVE_ADDR	(0xA0)

/** BLE Version info Address */
#define leVERSION_READ_ADDR	(0x22)

/** BLE Write Address */
#define leWRITE_ADDR		(0x40)

/** BLE Register starting address */
#define leNORDIC_REG_ADDR	(0x00)

/** BLE GPIO status register address */
#define leGPIO_REG_ADDR		(0x1F)

/** Maximum ble data broadcasting buffer length */
#define leMAX_BUF_LENGTH	(30)

/** Ble broadcasting data length */
#define leDATA_LENGTH		(29)

/** BLE Data specific for Manufacturer */
#define leMANF_SPEC_DATA	(0xFF)

/** Reserved size for making structure 8 byte align*/
#define leRESERVE_SIZE		(7)

/** BLE Random number length in the broadcast data */
#define lePRND_LENGTH		(28)

/** BLE address half length */
#define leBLE_GAP_ADDR_HALF_LEN (leBLE_GAP_ADDR_LEN/2)

/**
 * @brief Bluetooth Low Energy address.
 */
typedef struct
{
  uint8_t ucAddr_type;			/**< BLE_GAP_ADDR_TYPES. */
  uint8_t ucAddr[leBLE_GAP_ADDR_LEN];	/**< 48-bit address, LSB format. */
} leGap_addr_t;

/**
*@brief BLE device information
*/
typedef struct
{
	leGap_addr_t xle_mac_id;		/**< bt mac address */
	uint8_t ucBleSwMajor;			/**< SW major version info */
	uint8_t ucBleSwMinor;			/**< SW minor version info */
	uint8_t ucReserved[leRESERVE_SIZE];	/**< Reserved field */
} leDeviceInfo_t;

/**
 * @brief Reads the BLe stack version over I2C
 *
 * @param pxDeviceInfo structure pointer
 *
 * @return NO_ERROR on success or error codes
 */
int32_t lLeInterfaceRead_Version(leDeviceInfo_t * pxDeviceInfo);

/**
 * @brief Check the BLe status
 *
 * @return NO_ERROR on success or error codes
 */
int32_t lLeInterfaceCheck_LE_status(void);

/**
 * @brief Write Data to the BLE EEPROM
 *
 * @param pucData : pointer to data to be written
 * @param ulLength : Length of the data
 *
 * @return NO_ERROR on success or error codes
 */
int32_t lLeInterfaceLe_WriteData(uint8_t * pucData, uint32_t ulLength);

/**
 * @brief Function to read LE version and MAC address and print on console.
 *
 * @param None
 *
 * @return NO_ERROR on success or error codes
 */
int32_t lLeInterfacePrint_version(void);

#endif /* LE_INTERFACE_H*/
