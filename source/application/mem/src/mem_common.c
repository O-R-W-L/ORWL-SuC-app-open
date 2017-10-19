/**===========================================================================
 * @file mem_common.c
 *
 * @brief This file contains common API's for flash operation.
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

/* Global includes */
#include <errors.h>
#include <mml_sflc.h>
#include <debug.h>
#include <printf_lite.h>
#include <stdlib.h>
#include <stdint.h>

/* Freertos includes */
#include <FreeRTOS.h>
#include <task.h>
#include <portable.h>

/* local includes */
#include <mem_common.h>
#include <../src/flash.h>

/* function definition */

int32_t lCommonChoosePartition(uint32_t ulPar1Address, uint32_t ulPar2Address,
	uint32_t ulMagicNum, uint32_t ulSize)
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Magic number to read*/
    uint32_t ulMagicHDR = 0;
    /* check if partition one contains valid magic header */
    lStatus = mml_sflc_read(ulPar1Address,(uint8_t *)&ulMagicHDR, ulSize);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Error reading magic header in partition 1 \r\n");
	return N_MML_SFLC_ERR_NOT_ACCESSIBLE;
    }
    /* check if partition 1 contains valid header */
    if(ulMagicHDR == ulMagicNum)
    {
	/* partition one contains valid magic header */
	return commonPARTITION1;
    }

    /* check if partition two contains valid magic header */
    lStatus = mml_sflc_read(ulPar2Address,(uint8_t *)&ulMagicHDR, ulSize);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Error reading magic header in partition 2 \r\n");
	return N_MML_SFLC_ERR_NOT_ACCESSIBLE;
    }
    /* check if partition 2 contains valid data */
    if(ulMagicHDR == ulMagicNum)
    {
	/* partition two contains valid magic header */
	return commonPARTITION2;
    }
    /* if none of the partition contains valid header than return 3 */
    return commonPARTITIONNONE;
}
/*----------------------------------------------------------------------------*/

int32_t lCommonUpdatePartition(uint32_t ulAddress, uint32_t ulDataSize,
	uint8_t *pucDataBuffer, uint32_t ulMagicNum, uint32_t ulMagicSize)
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucDataBuffer == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* char pointer to increment magic header size from both write address and
     * structure address.
     */
    uint8_t *ucData;
    /* assign the address of data buffer */
    ucData = pucDataBuffer;
    /* skip the magic header from structure, we will write magic header once
     * manufacture data is completely written.Make sure magic header size is
     * 32bits aligned, otherwise it will fail.
     */
    ucData = ucData + ulMagicSize;
    /* erase the flash */
    lStatus = mml_sflc_erase(ulAddress, ulDataSize);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to erase for address 0x%X \r\n",ulAddress);
	return lStatus;
    }

    /* write to flash, excluding magic header */

    lStatus = mml_sflc_write((ulAddress + ulMagicSize), (uint8_t *) ucData,
	    (ulDataSize - ulMagicSize));

    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write for address 0x%x \r\n",
		(ulAddress + ulMagicSize));
	return lStatus;
    }

    /* Now write the magic header */
    lStatus = mml_sflc_write(ulAddress, (uint8_t *) (&ulMagicNum),
	    ulMagicSize);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write for address 0x%x \r\n",ulAddress);
	return lStatus;
    }
    /* return error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lCommonEraseFlashRTOS (uint32_t ulAddress, uint32_t ulLength)
{
    /* Status to return */
    int32_t lStatus = NO_ERROR;
    /* Before erasing the flash we need to disable interrupt and scheduler
     * and can be resumed once the flash erase is completed.
     */
    /* Commenting suspend and resume scheduler since, it is observed
     * Flash hangs sometimes because of this. Please revisit the same
     * if required.
     */
    taskENTER_CRITICAL();
    /* vTaskSuspendAll(); */
    lStatus = mml_sflc_erase(ulAddress,ulLength);
    /* xTaskResumeAll(); */
    taskEXIT_CRITICAL();

    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lcommonEraseEncyKey ( void )
{
   return lCommonEraseFlash(flashENC_KEY_START_ADDR,
	   flashENC_KEY_SIZE);
}
/*----------------------------------------------------------------------------*/

int32_t lcommonEraseAccessKey ( void )
{
    return lCommonEraseFlash(flashACCESS_KEY_START_ADDR,
	    flashACCESS_KEY_SIZE);
}
/*----------------------------------------------------------------------------*/

int32_t lcommonEraseKeyFobId( void )
{
    return lCommonEraseFlash(flashKEYFOB_ID_START_ADDR,
	    flashKEYFOB_ID_SIZE);
}
/*----------------------------------------------------------------------------*/

int32_t lcommonEraseUserConfigData( void )
{
    return lCommonEraseFlash(flashUSER_CONFIG_START_ADDR,
	    flashUSER_CONFIG_SIZE);
}
/*----------------------------------------------------------------------------*/

int32_t lCommonEraseFlash (uint32_t ulAddress, uint32_t ulLen)
{
    /* Erase Flash */
    return mml_sflc_erase(ulAddress,ulLen);
}
/*----------------------------------------------------------------------------*/
