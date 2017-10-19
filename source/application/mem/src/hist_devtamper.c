/**===========================================================================
 * @file hist_devtamper.c
 *
 * @brief This file contains all the API definition required to handle
 * tamper and clear event section of flash
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
#include <stdint.h>
#include <errors.h>
#include <debug.h>
#include <printf_lite.h>
#include <mml_sflc.h>
#include <string.h>

/* Freertos includes */
#include <FreeRTOS.h>
#include <task.h>
#include <portable.h>

/* Local includes */
#include <hist_devtamper.h>
#include <mem_common.h>
/* Not exposing flash.h to users */
#include "flash.h"

/**
 * tamper and clear event section magic
 */
#define devtamperTAMPER_CLEAR_MAGIC	(0x1A39E8C1)

/*
 * Tamper and clear event section magic size.
 */
#define devtamperTAMPER_CLEAR_MAGIC_SIZE (4)

/**
 * device tamper partition 1 address.
 */
#define devtamperDEVTAM_PART1_ADDRESS	(flashTAMP_HIST_START_ADDR)

/**
 * device tamper partition 2(backup) address.
 */
#define devtamperDEVTAM_PART2_ADDRESS	(devtamperDEVTAM_PART1_ADDRESS + flashPAGE_SIZE)

#if 0
/* To be Done */
static uint32_t sulEraseTamperEventEntry( );
static uint32_t sulEraseClearEventEntry( );
#endif

/** function prototypes */

int32_t lDevtamperWriteTampHistory(devtamperTamperHist_t *pxDevTamHist)
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Partition to write */
    uint32_t ulPartition = 0;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxDevTamHist == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Devtamper history structure for backup */
    devtamperTamperHist_t *pxBackUpdevtamper = NULL;
    /* allocate dynamic memory */
    pxBackUpdevtamper = (devtamperTamperHist_t *) pvPortMalloc(
	    sizeof(devtamperTamperHist_t));
    /* check if memory was allocated properly */
    if(pxBackUpdevtamper == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for devtamper history entry structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxBackUpdevtamper, commonDEFAULT_VALUE, sizeof(devtamperTamperHist_t));

    /* We need to check which partition has a valid data, based on valid magic
     * number. If both partition does not have valid magic number than, data
     * will be written to partition 1.
     */
    ulPartition = lCommonChoosePartition(devtamperDEVTAM_PART1_ADDRESS,
	    devtamperDEVTAM_PART2_ADDRESS, devtamperTAMPER_CLEAR_MAGIC,
	    devtamperTAMPER_CLEAR_MAGIC_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	/* assuming partition 1 has latest updated data, copy the partition 1
	 * data to backup structure.
	 */
	lStatus = lDevtamperReadTampHistory(pxBackUpdevtamper);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("failed to back up device tamper data \r\n");
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
	lStatus = lCommonUpdatePartition(devtamperDEVTAM_PART2_ADDRESS,
		sizeof(devtamperTamperHist_t),
		(uint8_t *) pxBackUpdevtamper,
		devtamperTAMPER_CLEAR_MAGIC, devtamperTAMPER_CLEAR_MAGIC_SIZE);
	/* xTaskResumeAll(); */
	taskEXIT_CRITICAL();
        if(lStatus != NO_ERROR)
	{
	     debugERROR_PRINT("failed to update backup device tamper data \r\n");
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
	/* update the error code */
	lStatus = ulPartition;
	goto CLEANUP;
    }

    /* Before writing the user data we must update the magic header because user
     * is not aware of it.
     */
    pxDevTamHist->ulKeyTamperMagic = devtamperTAMPER_CLEAR_MAGIC;

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
    lStatus = lCommonUpdatePartition(devtamperDEVTAM_PART1_ADDRESS,
	    sizeof(devtamperTamperHist_t), (uint8_t *) pxDevTamHist,
	    devtamperTAMPER_CLEAR_MAGIC, devtamperTAMPER_CLEAR_MAGIC_SIZE);
    /* xTaskResumeAll(); */
    taskEXIT_CRITICAL();
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write device tamper Data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxBackUpdevtamper)
    {
	vPortFree(pxBackUpdevtamper);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lDevtamperWriteTampHistoryNONRTOS(devtamperTamperHist_t *pxDevTamHist)
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Partition to write */
    uint32_t ulPartition = 0;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxDevTamHist == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Devtamper history structure for backup */
    devtamperTamperHist_t *pxBackUpdevtamper = NULL;
    /* allocate dynamic memory */
    pxBackUpdevtamper = (devtamperTamperHist_t *) pvPortMalloc(
	    sizeof(devtamperTamperHist_t));
    /* check if memory was allocated properly */
    if(pxBackUpdevtamper == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for devtamper history entry structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxBackUpdevtamper, commonDEFAULT_VALUE, sizeof(devtamperTamperHist_t));

    /* We need to check which partition has a valid data, based on valid magic
     * number. If both partition does not have valid magic number than, data
     * will be written to partition 1.
     */
    ulPartition = lCommonChoosePartition(devtamperDEVTAM_PART1_ADDRESS,
	    devtamperDEVTAM_PART2_ADDRESS, devtamperTAMPER_CLEAR_MAGIC,
	    devtamperTAMPER_CLEAR_MAGIC_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	/* assuming partition 1 has latest updated data, copy the partition 1
	 * data to backup structure.
	 */
	lStatus = lDevtamperReadTampHistory(pxBackUpdevtamper);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("failed to back up device tamper data \r\n");
	    goto CLEANUP;
	}
	/* Now write primary partition data to backup partition */
	lStatus = lCommonUpdatePartition(devtamperDEVTAM_PART2_ADDRESS,
		sizeof(devtamperTamperHist_t),
		(uint8_t *) pxBackUpdevtamper,
		devtamperTAMPER_CLEAR_MAGIC, devtamperTAMPER_CLEAR_MAGIC_SIZE);
        if(lStatus != NO_ERROR)
	{
	     debugERROR_PRINT("failed to update backup device tamper data \r\n");
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
	/* update the error code */
	lStatus = ulPartition;
	goto CLEANUP;
    }

    /* Before writing the user data we must update the magic header because user
     * is not aware of it.
     */
    pxDevTamHist->ulKeyTamperMagic = devtamperTAMPER_CLEAR_MAGIC;

    /* Now update the primary partition with latest data given by the user*/
    lStatus = lCommonUpdatePartition(devtamperDEVTAM_PART1_ADDRESS,
	    sizeof(devtamperTamperHist_t), (uint8_t *) pxDevTamHist,
	    devtamperTAMPER_CLEAR_MAGIC, devtamperTAMPER_CLEAR_MAGIC_SIZE);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write device tamper Data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxBackUpdevtamper)
    {
	vPortFree(pxBackUpdevtamper);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lDevtamperReadTampHistory(devtamperTamperHist_t *pxDevTamHist)
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
    if(pxDevTamHist == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* We need to check if data needs to be written to partition 1 or 2
     * based on the valid magic number. If both partition does not have valid
     * magic number than choose partition 1.
     */
    ulPartition = lCommonChoosePartition(devtamperDEVTAM_PART1_ADDRESS,
	    devtamperDEVTAM_PART2_ADDRESS, devtamperTAMPER_CLEAR_MAGIC,
	    devtamperTAMPER_CLEAR_MAGIC_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	ulAddress = devtamperDEVTAM_PART1_ADDRESS;
    }
    /* check if partition two has valid header */
    else if (ulPartition == commonPARTITION2)
    {
	ulAddress = devtamperDEVTAM_PART2_ADDRESS;
    }
    /* Some error has occurred. */
    else
    {
	debugERROR_PRINT("No valid header partition found \r\n");
	return ulPartition;
    }

    /* read the manufacture data from the flash */
    lStatus = mml_sflc_read(ulAddress, (uint8_t *) pxDevTamHist,
	    sizeof(devtamperTamperHist_t));
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read device tamper data \r\n");
	return lStatus;
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/
