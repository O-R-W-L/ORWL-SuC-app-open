/**===========================================================================
 * @file user_config.c
 *
 * @brief This file contains all the API definition required to handle
 * user config data.
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
#include <user_config.h>
#include <mem_common.h>
/* Not exposing flash.h to users */
#include "flash.h"

/* user config data Macros, that are not exposed to user. */
/**
 * Magic number for accessing user config data.
 */
#define configUSER_MAGIC		(0x3FFDDAA1)

/**
 * user config data base address for partition 1, size of partition is 1 page.
 * This is primary partition with latest changes.
 */
#define configUSER_PART1_ADDRESS (flashUSER_CONFIG_START_ADDR)

/**
 * user config data base address for partition 2, size of partition is 1 page.
 * this is backup partition for primary partition.
 */
#define configUSER_PART2_ADDRESS (configUSER_PART1_ADDRESS + flashPAGE_SIZE)

/* Function definition */

int32_t lUserWriteUserConfig(xUserConfig_t *pxUserConfig)
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Partition to write */
    uint32_t ulPartition = 0;
    /* user config structure for backup */
    xUserConfig_t *xBackUpUserCFG = NULL;

    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxUserConfig == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }

    /* allocate dynamic memory */
    xBackUpUserCFG = (xUserConfig_t *) pvPortMalloc(sizeof(xUserConfig_t));
    /* check if memory was allocated properly */
    if(xBackUpUserCFG == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for user config backup structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(xBackUpUserCFG, commonDEFAULT_VALUE, sizeof(xUserConfig_t));

    /* We need to check which partition has a valid data, based on valid magic
     * number. If both partition does not have valid magic number than, data
     * will be written to partition 1.
     */
    ulPartition = lCommonChoosePartition(configUSER_PART1_ADDRESS,
	    configUSER_PART2_ADDRESS, configUSER_MAGIC,
	    configCONFIG_MAGIC_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	/* assuming partition 1 has latest updated data, copy the partition 1
	 * data to backup structure.
	 */
	lStatus = lUserReadUserConfig(xBackUpUserCFG);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("failed to read user config data \r\n");
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
	lStatus = lCommonUpdatePartition(configUSER_PART2_ADDRESS,
		sizeof(xUserConfig_t), (uint8_t *) xBackUpUserCFG,
		configUSER_MAGIC, configCONFIG_MAGIC_SIZE);
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
    pxUserConfig->ulMagicHeader = configUSER_MAGIC;

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
    lStatus = lCommonUpdatePartition(configUSER_PART1_ADDRESS,
	    sizeof(xUserConfig_t), (uint8_t *) pxUserConfig,
	    configUSER_MAGIC, configCONFIG_MAGIC_SIZE);
    /* xTaskResumeAll(); */
    taskEXIT_CRITICAL();
    if(lStatus != NO_ERROR)
	{
	     debugERROR_PRINT("failed to write user config Data \r\n");
	     goto CLEANUP;
	}

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(xBackUpUserCFG)
    {
	vPortFree(xBackUpUserCFG);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lUserReadUserConfig(xUserConfig_t *pxUserConfig)
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
    if(pxUserConfig == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* We need to check if data needs to be written to partition 1 or 2
     * based on the valid magic number. If both partition does not have valid
     * magic number than choose partition 1.
     */
    ulPartition = lCommonChoosePartition(configUSER_PART1_ADDRESS,
	    configUSER_PART2_ADDRESS, configUSER_MAGIC,
	    configCONFIG_MAGIC_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	ulAddress = configUSER_PART1_ADDRESS;
    }
    /* check if partition two has valid header */
    else if (ulPartition == commonPARTITION2)
    {
	ulAddress = configUSER_PART2_ADDRESS;
    }
    /* Some error has occurred. */
    else
    {
	debugERROR_PRINT("No valid header partition found \r\n");
	return ulPartition;
    }

    /* read the manufacture data from the flash */
    lStatus = mml_sflc_read(ulAddress, (uint8_t *) pxUserConfig,
	    sizeof(xUserConfig_t));
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read user config data \r\n");
	return lStatus;
    }
    return lStatus;
}
/*----------------------------------------------------------------------------*/
