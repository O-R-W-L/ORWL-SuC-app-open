/**===========================================================================
 * @file le_interface.c
 *
 * @brief This file contains functions used for the BLE interfacing
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
#include <le_interface.h>

/**
 * Shift by 8 used for endianess conversion
 */
#define leSHIFT_8		(8)

/**
 * Mask 8 bits used for endianess conversion
 */
#define leMASK_8_BITS		(0xFF)

int32_t lLeInterfaceRead_Version(leDeviceInfo_t * pxDeviceInfo)
{
    int32_t lRet = NO_ERROR;
    uint8_t ucTmp = 0;
    uint8_t ucCnt = 0;

    if(pxDeviceInfo == NULL)
    {
	debugERROR_PRINT(" Uninitialized Version Info pointer");
	return COMMON_ERR_INVAL;
    }
    lRet = mml_i2c_ioctl(MML_I2C_RXFLUSH, 0);
    lRet |= mml_i2c_ioctl(MML_I2C_TXFLUSH, 0);
    if( lRet != NO_ERROR)
    {
	debugERROR_PRINT(" mml_i2c_ioctl failed");
        return lRet;
    }
    lRet = lI2cRead((uint8_t)leNORDIC_I2C_SLAVE_ADDR,
		    (uint8_t)leVERSION_READ_ADDR,
		    sizeof(leDeviceInfo_t),
		    (uint8_t *)pxDeviceInfo);

    /* Convert Endianess */
    /* LE eeprom gives data in Big endianess */
    for(ucCnt=0 ; ucCnt<leBLE_GAP_ADDR_HALF_LEN ; ucCnt++)
    {
	ucTmp=pxDeviceInfo->xle_mac_id.ucAddr[ucCnt];
	pxDeviceInfo->xle_mac_id.ucAddr[ucCnt] =
		pxDeviceInfo->xle_mac_id.ucAddr[leBLE_GAP_ADDR_LEN - ucCnt -1];
	pxDeviceInfo->xle_mac_id.ucAddr[leBLE_GAP_ADDR_LEN - ucCnt -1]=ucTmp;
    }
    return lRet;
}
/*----------------------------------------------------------------------------*/

int32_t lLeInterfaceCheck_LE_status(void)
{
    int32_t lRet = NO_ERROR;
    leDeviceInfo_t xDeviceInfo;

    lRet = lLeInterfaceRead_Version(&xDeviceInfo);
    return lRet;
}
/*----------------------------------------------------------------------------*/

int32_t lLeInterfaceLe_WriteData(uint8_t * pucData, uint32_t ulLength)
{
    int32_t lRet = NO_ERROR;

    if(ulLength > leMAX_BUF_LENGTH)
    {
    	debugERROR_PRINT(" Invalid Data Length ");
    	return COMMON_ERR_INVAL;
    }

    lRet = lI2cWrite((uint8_t)leNORDIC_I2C_SLAVE_ADDR,
		    (uint8_t)leWRITE_ADDR,
		    ulLength,
		    pucData);
    return lRet;
}
/*----------------------------------------------------------------------------*/

int32_t lLeInterfacePrint_version(void)
{
    int32_t lRet = NO_ERROR;
    uint8_t ucCnt = 0;
    leDeviceInfo_t xDeviceInfo;

    lRet = lLeInterfaceRead_Version(&xDeviceInfo);
    if(!lRet)
    {
	debugPRINT(" Le Address type %d",xDeviceInfo.xle_mac_id.ucAddr_type);
	debugPLAIN_PRINT(" LE MAC Address : ");
	for(ucCnt=0;ucCnt<leBLE_GAP_ADDR_LEN;ucCnt++)
	{
	    debugPLAIN_PRINT("%X ",xDeviceInfo.xle_mac_id.ucAddr[ucCnt]);
	}
	debugPLAIN_PRINT("\n");
	debugPRINT(" Le SW major version : %d",xDeviceInfo.ucBleSwMajor);
	debugPRINT(" Le SW minor version : %d",xDeviceInfo.ucBleSwMinor);
    }
    return lRet;
}
/*----------------------------------------------------------------------------*/
