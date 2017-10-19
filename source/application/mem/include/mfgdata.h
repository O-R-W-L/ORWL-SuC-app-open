 /**===========================================================================
 * @file mfgdata.h
 *
 * @brief This file contains all the data structures and macros for manufacturer
 * data of ORWL
 *
 * @author ravikiran.hv@design-shift.com
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
#ifndef mfgdataINCLUDE_MFG_DATA_H_
#define mfgdataINCLUDE_MFG_DATA_H_

/*Global includes */
#include <stdint.h>
#include <rtc.h>

/* Manufacture data Macros */
/**
 * Serial number size
 */
#define mfgdataSERIAL_NUM_SIZE		(32)

/**
 * @brief This structure defines contents for manufacturer data field in flash
 */
typedef struct
{
    /** Manufacturer data magic number */
    uint32_t ulMagicHeader;
    /** ORWL Serial number */
    uint8_t ucSerialNum[mfgdataSERIAL_NUM_SIZE];
    /** ORWL Hardware Rev */
    uint32_t ulHardwareRev;
    /** SuC software revision */
    uint32_t ulSoftwareRev;
    /** NFC Library version */
    uint32_t ulNFCLibVer;
    /** BLE Library version */
    uint32_t ulBLELibVer;
    /** Display Library version */
    uint32_t ulDispLibVer;
    /** MEMS Library version */
    uint32_t ulMEMSLibVer;
    /** Holds details ORWL life cycle stages */
    uint32_t ulProductCycle;
    /** RTC date time structure to hold manufacture date and time. */
    rtcDateTime_t xMFGDateTime;
    /** SUC Serial number */
    uint8_t ucSUCSerialNum[mfgdataSERIAL_NUM_SIZE];
} mfgdataManufactData_t;

/** function prototypes */

/** @brief Reads manufacture data.
 *
 * This function reads the manufacture data from flash memory.
 *
 * @param pxMFData pointer to manufacture structure.
 * @return error code..
 *
 */
int32_t lMfgdateReadMfgData( mfgdataManufactData_t *pxMFData );

/** @brief Write manufacture data.
 *
 * This function writes the manufacture data to flash memory.
 *
 * @param pxMFData pointer to manufacture structure.
 * @return error code..
 *
 */
int32_t lMfgdataWriteMfgData( mfgdataManufactData_t *pxMFData );

/** @brief Write manufacture data, NON RTOS.
 *
 * This function writes the manufacture data to flash memory. This function
 * should be called from NON RTOS context.
 *
 * @param pxMFData pointer to manufacture structure.
 * @return error code..
 *
 */
int32_t lMfgdataWriteMfgDataNONRTOS( mfgdataManufactData_t *pxMFData );

/** @brief Read product cycle.
 *
 * This function reads product cycle from flash.
 *
 * @param pulProductCycle pointer to product cycle.
 * @return error code..
 *
 */
int32_t lMfgdataReadProductCycle( uint32_t *pulProductCycle );

/** @brief Write product cycle.
 *
 * This function writes product cycle to flash.
 *
 * @param pulProductCycle pointer to product cycle.
 * @return error code..
 *
 */
int32_t lMfgdataUpdateProductCycle( uint32_t *pulProductCycle );

/** @brief Write product cycle, NON RTOS.
 *
 * This function writes product cycle to flash. This function should be called
 * from NON RTOS context.
 *
 * @param pulProductCycle pointer to product cycle.
 * @return error code..
 *
 */
int32_t lMfgdataUpdateProductCycleNONRTOS( uint32_t *pulProductCycle );

/** @brief Read ORWL serial number.
 *
 * This function Reads ORWL serial number from flash.
 *
 * @param pucORWLSerial pointer to ORWL serial number buffer.
 * @return error code..
 *
 */
int32_t lMfgdataReadORWLSerial( uint8_t *pucORWLSerial );

/** @brief Update ORWL serial number.
 *
 * This function updates ORWL serial number to flash.
 *
 * @param pucORWLSerial pointer to ORWL serial number buffer.
 * @return error code..
 *
 */
int32_t lMfgdataUpdateORWLSerial( uint8_t *pucORWLSerial );

/** @brief Check magic header.
 *
 * This function checks if there is valid magic header in manufacture partitions.
 *
 * @return error code.
 *
 */
int32_t lMfgdataCheckMagicHDR(void);

/** @brief Update MFG date and time.
 *
 * This function updates manufacture date and time to flash.
 *
 * @param pxDateTime pointer to date time structure.
 * @return error code.
 *
 */
int32_t lMfgdataUpdateMFGDateTime(rtcDateTime_t *pxDateTime);

/** @brief Reads MFG date and time.
 *
 * This function reads manufacture date and time to flash.
 *
 * @param pxDateTime pointer to date time structure.
 * @return error code.
 *
 */
int32_t lMfgdataGetMFGDateTime(rtcDateTime_t *pxDateTime);

/** @brief Read SUC serial number.
 *
 * This function Reads SUC serial number from flash.
 *
 * @param pucORWLSerial pointer to ORWL serial number buffer.
 * @return error code..
 *
 */
int32_t lMfgdataReadSUCSerial( uint8_t *pucORWLSerial );

/** @brief Update SUC serial number.
 *
 * This function updates SUC serial number to flash.
 *
 * @param pucORWLSerial pointer to ORWL serial number buffer.
 * @return error code..
 *
 */
int32_t lMfgdataUpdateSUCSerial( uint8_t *pucORWLSerial );

#endif /* mfgdataINCLUDE_MFG_DATA_H_ */
