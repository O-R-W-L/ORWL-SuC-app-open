/**===========================================================================
 * @file mfgdata.c
 *
 * @brief This file contains all the API definition required to handle
 * ORWL MFG data
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
#include <debug.h>
#include <printf_lite.h>
#include <stdint.h>
#include <mml_sflc.h>
#include <string.h>

/* Freertos includes */
#include <FreeRTOS.h>
#include <task.h>
#include <portable.h>

/* Local includes */
#include <mfgdata.h>
#include <mem_common.h>
/* Not exposing flash.h to users */
#include "flash.h"

/* Manufacture data Macros, that are not exposed to user. */

/**
 * Magic number accessing manufacture data.
 */
#define mfgdataMANUFACTURE_MAGIC	(0x3FD0DA1A)

/**
 * Magic number size in bytes
 */
#define mfgdataMAGIC_NUM_SIZE		(4)

/**
 * Manufacture data base address for partition 1, size of partition is 1 page.
 * This is primary partition with latest changes.
 */
#define mfgdataMFG_PART1_ADDRESS (flashMANUFACT_DATA_START_ADDR)

/**
 * Manufacture data base address for partition 2, size of partition is 1 page.
 * this is backup partition for primary partition.
 */
#define mfgdataMFG_PART2_ADDRESS (mfgdataMFG_PART1_ADDRESS + flashPAGE_SIZE)

/** function defination */

int32_t lMfgdataWriteMfgData( mfgdataManufactData_t *pxMFData )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Partition to write */
    uint32_t ulPartition = 0;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxMFData == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Manufacuture structure for backup */
    mfgdataManufactData_t *xBackUpMFData = NULL;
    /* allocate dynamic memory */
    xBackUpMFData = (mfgdataManufactData_t *) pvPortMalloc(
	    sizeof(mfgdataManufactData_t));
    /* check if memory was allocated properly */
    if(xBackUpMFData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(xBackUpMFData, commonDEFAULT_VALUE, sizeof(mfgdataManufactData_t));

    /* We need to check which partition has a valid data, based on valid magic
     * number. If both partition does not have valid magic number than, data
     * will be written to partition 1.
     */
    ulPartition = lCommonChoosePartition(mfgdataMFG_PART1_ADDRESS,
	    mfgdataMFG_PART2_ADDRESS, mfgdataMANUFACTURE_MAGIC,
	    mfgdataMAGIC_NUM_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	/* assuming partition 1 has latest updated data, copy the partition 1
	 * data to backup structure.
	 */
	lStatus = lMfgdateReadMfgData(xBackUpMFData);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("failed to read MF data \r\n");
	    goto CLEANUP;
	}
	/* Now write primary partition data to backup partition */
	/* Before writing flash scheduler and interrupts should be disabled,
	 * once the write is done,then both can be enabled
	 */
	/* Commenting suspend and resume scheduler since, it is observed
	 * Flash hangs sometimes because of this. Please revisit the same
	 * if required.
	 */
	taskENTER_CRITICAL();
	/* vTaskSuspendAll(); */
	lStatus = lCommonUpdatePartition(mfgdataMFG_PART2_ADDRESS,
		sizeof(mfgdataManufactData_t), (uint8_t *) xBackUpMFData,
		mfgdataMANUFACTURE_MAGIC, mfgdataMAGIC_NUM_SIZE);
	/* xTaskResumeAll(); */
	taskEXIT_CRITICAL();
        if(lStatus != NO_ERROR)
	{
	     debugERROR_PRINT("failed to update backup partition \r\n");
	     goto CLEANUP;
	}
    }
    /* check if partition two has valid header */
    else if (ulPartition == commonPARTITION2)
    {
	/* Backup Partition has valid magic header, this can occur in case where
	 * there was power off while writing primary partition. we don't need to
	 * restore the same for primary partition because we will be writing the
	 * latest data given by user to primary partition. Also in case if user
	 * does not write latest data to primary partition, while reading data
	 * will be read from secondary partition.
	 */
	/* Do nothing for this else if */
    }
    /* check if both the partition does not have valid header */
    else if (ulPartition == commonPARTITIONNONE)
    {
	/* Both partitions does not have valid data, inform user but,
	 * don't return. Update latest user data to primary partition.
	 */
	debugPRINT("Both partition does not have valid header \r\n");
    }
    /* Some error has occurred. */
    else
    {
	debugERROR_PRINT("failed to choose partition \r\n");
	lStatus = ulPartition;
	goto CLEANUP;
    }

    /* Before writing the user data we must update the magic header because user
     * is not aware of it.
     */
    pxMFData->ulMagicHeader = mfgdataMANUFACTURE_MAGIC;

    /* Now update the primary partition with latest data given by the user*/
    /* Before writing flash scheduler and interrupts should be disabled,
     * once the write is done,then both can be enabled
     */
    /* Commenting suspend and resume scheduler since, it is observed
     * Flash hangs sometimes because of this. Please revisit the same
     * if required.
     */
    taskENTER_CRITICAL();
    /* vTaskSuspendAll(); */
    lStatus = lCommonUpdatePartition(mfgdataMFG_PART1_ADDRESS,
	    sizeof(mfgdataManufactData_t), (uint8_t *) pxMFData,
	    mfgdataMANUFACTURE_MAGIC, mfgdataMAGIC_NUM_SIZE);
    /* xTaskResumeAll(); */
    taskEXIT_CRITICAL();
    if(lStatus != NO_ERROR)
	{
	     debugERROR_PRINT("failed to write MF Data \r\n");
	     goto CLEANUP;
	}

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(xBackUpMFData)
    {
	vPortFree(xBackUpMFData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lMfgdataWriteMfgDataNONRTOS( mfgdataManufactData_t *pxMFData )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Partition to write */
    uint32_t ulPartition = 0;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxMFData == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Manufacuture structure for backup */
    mfgdataManufactData_t *xBackUpMFData = NULL;
    /* allocate dynamic memory */
    xBackUpMFData = (mfgdataManufactData_t *) pvPortMalloc(
	    sizeof(mfgdataManufactData_t));
    /* check if memory was allocated properly */
    if(xBackUpMFData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(xBackUpMFData, commonDEFAULT_VALUE, sizeof(mfgdataManufactData_t));

    /* We need to check which partition has a valid data, based on valid magic
     * number. If both partition does not have valid magic number than, data
     * will be written to partition 1.
     */
    ulPartition = lCommonChoosePartition(mfgdataMFG_PART1_ADDRESS,
	    mfgdataMFG_PART2_ADDRESS, mfgdataMANUFACTURE_MAGIC,
	    mfgdataMAGIC_NUM_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	/* assuming partition 1 has latest updated data, copy the partition 1
	 * data to backup structure.
	 */
	lStatus = lMfgdateReadMfgData(xBackUpMFData);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("failed to read MF data \r\n");
	    goto CLEANUP;
	}
	/* Now write primary partition data to backup partition */
	lStatus = lCommonUpdatePartition(mfgdataMFG_PART2_ADDRESS,
		sizeof(mfgdataManufactData_t), (uint8_t *) xBackUpMFData,
		mfgdataMANUFACTURE_MAGIC, mfgdataMAGIC_NUM_SIZE);
        if(lStatus != NO_ERROR)
	{
	     debugERROR_PRINT("failed to update backup partition \r\n");
	     goto CLEANUP;
	}
    }
    /* check if partition two has valid header */
    else if (ulPartition == commonPARTITION2)
    {
	/* Backup Partition has valid magic header, this can occur in case where
	 * there was power off while writing primary partition. we don't need to
	 * restore the same for primary partition because we will be writing the
	 * latest data given by user to primary partition. Also in case if user
	 * does not write latest data to primary partition, while reading data
	 * will be read from secondary partition.
	 */
	/* Do nothing for this else if */
    }
    /* check if both the partition does not have valid header */
    else if (ulPartition == commonPARTITIONNONE)
    {
	/* Both partitions does not have valid data, inform user but,
	 * don't return. Update latest user data to primary partition.
	 */
	debugPRINT("Both partition does not have valid header \r\n");
    }
    /* Some error has occurred. */
    else
    {
	debugERROR_PRINT("failed to choose partition \r\n");
	lStatus = ulPartition;
	goto CLEANUP;
    }

    /* Before writing the user data we must update the magic header because user
     * is not aware of it.
     */
    pxMFData->ulMagicHeader = mfgdataMANUFACTURE_MAGIC;

    /* Now update the primary partition with latest data given by the user*/
    lStatus = lCommonUpdatePartition(mfgdataMFG_PART1_ADDRESS,
	    sizeof(mfgdataManufactData_t), (uint8_t *) pxMFData,
	    mfgdataMANUFACTURE_MAGIC, mfgdataMAGIC_NUM_SIZE);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write MF Data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(xBackUpMFData)
    {
	vPortFree(xBackUpMFData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lMfgdateReadMfgData( mfgdataManufactData_t *pxMFData )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Partition to write */
    uint32_t ulPartition = 0;
    /* address to write data */
    uint32_t ulAddress = 0;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxMFData == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* We need to check if data needs to be written to partition 1 or 2
     * based on the valid magic number. If both partition does not have valid
     * magic number than choose partition 1.
     */
    ulPartition = lCommonChoosePartition(mfgdataMFG_PART1_ADDRESS,
	    mfgdataMFG_PART2_ADDRESS, mfgdataMANUFACTURE_MAGIC,
	    mfgdataMAGIC_NUM_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	ulAddress = mfgdataMFG_PART1_ADDRESS;
    }
    /* check if partition two has valid header */
    else if (ulPartition == commonPARTITION2)
    {
	ulAddress = mfgdataMFG_PART2_ADDRESS;
    }
    /* Some error has occurred. */
    else
    {
	debugERROR_PRINT("No valid header partition found \r\n");
	return ulPartition;
    }

    /* read the manufacture data from the flash */
    lStatus = mml_sflc_read(ulAddress, (uint8_t *) pxMFData,
	    sizeof(mfgdataManufactData_t));
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read manufacture data \r\n");
	return lStatus;
    }
    return NO_ERROR;
}
/*----------------------------------------------------------------------------*/

int32_t lMfgdataReadProductCycle( uint32_t *pulProductCycle )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pulProductCycle == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Manufacture structure to read from flash */
    mfgdataManufactData_t *pxMFData = NULL;
    /* allocate dynamic memory */
    pxMFData = (mfgdataManufactData_t *) pvPortMalloc(
	    sizeof(mfgdataManufactData_t));
    /* check if memory was allocated properly */
    if(pxMFData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxMFData, commonDEFAULT_VALUE, sizeof(mfgdataManufactData_t));

    /* read manufacture data from flash */
    lStatus = lMfgdateReadMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read manufacture data \r\n");
	goto CLEANUP;
    }
    /* read product cycle pointer */
    *pulProductCycle = pxMFData->ulProductCycle;

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxMFData)
    {
	vPortFree(pxMFData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lMfgdataUpdateProductCycle( uint32_t *pulProductCycle )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pulProductCycle == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Manufacture structure to read from flash */
    mfgdataManufactData_t *pxMFData = NULL;
    /* allocate dynamic memory */
    pxMFData = (mfgdataManufactData_t *) pvPortMalloc(
	    sizeof(mfgdataManufactData_t));
    /* check if memory was allocated properly */
    if(pxMFData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxMFData, commonDEFAULT_VALUE, sizeof(mfgdataManufactData_t));

    /* read manufacture data from flash */
    lStatus = lMfgdateReadMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	/* reading might have failed because both partitions does not have valid
	 * magic header, lets add valid header.
	 */
	if(lStatus == commonPARTITIONNONE)
	{
	    pxMFData->ulMagicHeader = mfgdataMANUFACTURE_MAGIC;
	}
	/* Some other flash error might have occurred report error in this case*/
	else
	{
	    debugERROR_PRINT(" failed to read Manufacture data \r\n");
	    goto CLEANUP;
	}
    }

    /* write new product cycle */
    pxMFData->ulProductCycle = *pulProductCycle;

    /* write back the manufacture data */
    lStatus = lMfgdataWriteMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write manufacture data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxMFData)
    {
	vPortFree(pxMFData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lMfgdataUpdateProductCycleNONRTOS( uint32_t *pulProductCycle )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pulProductCycle == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Manufacture structure to read from flash */
    mfgdataManufactData_t *pxMFData = NULL;
    /* allocate dynamic memory */
    pxMFData = (mfgdataManufactData_t *) pvPortMalloc(
	    sizeof(mfgdataManufactData_t));
    /* check if memory was allocated properly */
    if(pxMFData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxMFData, commonDEFAULT_VALUE, sizeof(mfgdataManufactData_t));

    /* read manufacture data from flash */
    lStatus = lMfgdateReadMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	/* reading might have failed because both partitions does not have valid
	 * magic header, lets add valid header.
	 */
	if(lStatus == commonPARTITIONNONE)
	{
	    pxMFData->ulMagicHeader = mfgdataMANUFACTURE_MAGIC;
	}
	/* Some other flash error might have occurred report error in this case*/
	else
	{
	    debugERROR_PRINT(" failed to read Manufacture data \r\n");
	    goto CLEANUP;
	}
    }

    /* write new product cycle */
    pxMFData->ulProductCycle = *pulProductCycle;

    /* write back the manufacture data, NON RTOS API version should be called */
    lStatus = lMfgdataWriteMfgDataNONRTOS(pxMFData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write manufacture data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxMFData)
    {
	vPortFree(pxMFData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lMfgdataReadORWLSerial( uint8_t *pucORWLSerial )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucORWLSerial == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Manufacture structure to read from flash */
    mfgdataManufactData_t *pxMFData = NULL;
    /* allocate dynamic memory */
    pxMFData = (mfgdataManufactData_t *) pvPortMalloc(
	    sizeof(mfgdataManufactData_t));
    /* check if memory was allocated properly */
    if(pxMFData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxMFData, commonDEFAULT_VALUE, sizeof(mfgdataManufactData_t));

    /* read manufacture data from flash */
    lStatus = lMfgdateReadMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read manufacture data \r\n");
	goto CLEANUP;
    }
    /* copy the ORWL serial number to pucORWLSerial */
    memcpy((void *) pucORWLSerial, (const void *) pxMFData->ucSerialNum,
	    sizeof(pxMFData->ucSerialNum));

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxMFData)
    {
	vPortFree(pxMFData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lMfgdataUpdateORWLSerial( uint8_t *pucORWLSerial )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucORWLSerial == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Manufacture structure to read from flash */
    mfgdataManufactData_t *pxMFData = NULL;
    /* allocate dynamic memory */
    pxMFData = (mfgdataManufactData_t *) pvPortMalloc(
	    sizeof(mfgdataManufactData_t));
    /* check if memory was allocated properly */
    if(pxMFData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxMFData, commonDEFAULT_VALUE, sizeof(mfgdataManufactData_t));

    /* read manufacture data from flash */
    lStatus = lMfgdateReadMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	/* reading might have failed because both partitions does not have valid
	 * magic header, lets add valid header.
	 */
	if(lStatus == commonPARTITIONNONE)
	{
	    pxMFData->ulMagicHeader = mfgdataMANUFACTURE_MAGIC;
	}
	/* Some other flash error might have occurred report error */
	else
	{
	    debugERROR_PRINT(" failed to read Manufacture data \r\n");
	    goto CLEANUP;
	}
    }
    /* update the ORWL serial number */
    memcpy((void *) pxMFData->ucSerialNum, (const void *) pucORWLSerial,
	    sizeof(pxMFData->ucSerialNum));

    /* write back the manufacture data */
    lStatus = lMfgdataWriteMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write manufacture data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxMFData)
    {
	vPortFree(pxMFData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lMfgdataCheckMagicHDR(void)
{
    /* status to return */
    int32_t lStatus = NO_ERROR;

    /* We need to check if data needs to be written to partition 1 or 2
     * based on the valid magic number. If both partition does not have valid
     * magic number than choose partition 1.
     */
    lStatus = lCommonChoosePartition(mfgdataMFG_PART1_ADDRESS,
	    mfgdataMFG_PART2_ADDRESS, mfgdataMANUFACTURE_MAGIC,
	    mfgdataMAGIC_NUM_SIZE);
    /* return error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lMfgdataUpdateMFGDateTime(rtcDateTime_t *pxDateTime)
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxDateTime == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Manufacture structure to read from flash */
    mfgdataManufactData_t *pxMFData = NULL;
    /* allocate dynamic memory */
    pxMFData = (mfgdataManufactData_t *) pvPortMalloc(
	    sizeof(mfgdataManufactData_t));
    /* check if memory was allocated properly */
    if(pxMFData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for MFG data structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxMFData, commonDEFAULT_VALUE, sizeof(mfgdataManufactData_t));

    /* read manufacture data from flash */
    lStatus = lMfgdateReadMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	/* reading might have failed because both partitions does not have valid
	 * magic header, lets add valid header.
	 */
	if(lStatus == commonPARTITIONNONE)
	{
	    pxMFData->ulMagicHeader = mfgdataMANUFACTURE_MAGIC;
	}
	/* Some other flash error might have occurred report error */
	else
	{
	    debugERROR_PRINT(" failed to read Manufacture data \r\n");
	    goto CLEANUP;
	}
    }
    /* update the ORWL serial number */
    memcpy((void *) &pxMFData->xMFGDateTime, (const void *) pxDateTime,
	    sizeof(rtcDateTime_t));

    /* write back the manufacture data */
    lStatus = lMfgdataWriteMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write manufacture data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxMFData)
    {
	vPortFree(pxMFData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lMfgdataGetMFGDateTime(rtcDateTime_t *pxDateTime)
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxDateTime == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Manufacture structure to read from flash */
    mfgdataManufactData_t *pxMFData = NULL;
    /* allocate dynamic memory */
    pxMFData = (mfgdataManufactData_t *) pvPortMalloc(
	    sizeof(mfgdataManufactData_t));
    /* check if memory was allocated properly */
    if(pxMFData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxMFData, commonDEFAULT_VALUE, sizeof(mfgdataManufactData_t));

    /* read manufacture data from flash */
    lStatus = lMfgdateReadMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read manufacture data \r\n");
	goto CLEANUP;
    }
    /* copy the ORWL serial number to pucORWLSerial */
    memcpy((void *) pxDateTime, (const void *) &pxMFData->xMFGDateTime,
	    sizeof(rtcDateTime_t));

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxMFData)
    {
	vPortFree(pxMFData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lMfgdataReadSUCSerial( uint8_t *pucSUCSerial )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucSUCSerial == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Manufacture structure to read from flash */
    mfgdataManufactData_t *pxMFData = NULL;
    /* allocate dynamic memory */
    pxMFData = (mfgdataManufactData_t *) pvPortMalloc(
	    sizeof(mfgdataManufactData_t));
    /* check if memory was allocated properly */
    if(pxMFData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxMFData, commonDEFAULT_VALUE, sizeof(mfgdataManufactData_t));

    /* read manufacture data from flash */
    lStatus = lMfgdateReadMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read manufacture data \r\n");
	goto CLEANUP;
    }
    /* copy the SUC serial number to pucSUCSerial */
    memcpy((void *) pucSUCSerial, (const void *) pxMFData->ucSUCSerialNum,
	    sizeof(pxMFData->ucSUCSerialNum));

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxMFData)
    {
	vPortFree(pxMFData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lMfgdataUpdateSUCSerial( uint8_t *pucSUCSerial )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucSUCSerial == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Manufacture structure to read from flash */
    mfgdataManufactData_t *pxMFData = NULL;
    /* allocate dynamic memory */
    pxMFData = (mfgdataManufactData_t *) pvPortMalloc(
	    sizeof(mfgdataManufactData_t));
    /* check if memory was allocated properly */
    if(pxMFData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxMFData, commonDEFAULT_VALUE, sizeof(mfgdataManufactData_t));

    /* read manufacture data from flash */
    lStatus = lMfgdateReadMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	/* reading might have failed because both partitions does not have valid
	 * magic header, lets add valid header.
	 */
	if(lStatus == commonPARTITIONNONE)
	{
	    pxMFData->ulMagicHeader = mfgdataMANUFACTURE_MAGIC;
	}
	/* Some other flash error might have occurred report error */
	else
	{
	    debugERROR_PRINT(" failed to read Manufacture data \r\n");
	    goto CLEANUP;
	}
    }
    /* update the SUC serial number */
    memcpy((void *) pxMFData->ucSUCSerialNum, (const void *) pucSUCSerial,
	    sizeof(pxMFData->ucSUCSerialNum));

    /* write back the manufacture data */
    lStatus = lMfgdataWriteMfgData(pxMFData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write manufacture data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxMFData)
    {
	vPortFree(pxMFData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/
