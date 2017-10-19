/**===========================================================================
 * @file keyfobid.c
 *
 * @brief This file contains all the API definition required to handle
 * ORWL keyfobId section access
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

/* Local include */
#include <keyfobid.h>
#include <mem_common.h>
/* Not exposing flash.h to users */
#include "flash.h"

/**
 * Size of magic header.
 */
#define keyfobidKEYFOB_ID_MAGIC_SIZE	(4)

/**
 * Keyfob partition 1 address.
 */
#define keyfobidKEYFOB_PART1_ADDRESS	(flashKEYFOB_ID_START_ADDR)

/**
 * Keyfob partition 2(backup) address. Partition 1, plus 2 page size because
 * the size of keyfob entry structure is 2 pages(please refer flash.h).
 */
#define keyfobidKEYFOB_PART2_ADDRESS	(keyfobidKEYFOB_PART1_ADDRESS + (flashPAGE_SIZE * 2))

/** function definition */

int32_t lKeyfobidWriteKeyFobEntry(keyfobidKeyFobEntry_t *pxKeyFobEntry)
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Partition to write */
    uint32_t ulPartition = 0;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxKeyFobEntry == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Keyfob entry structure for backup */
    keyfobidKeyFobEntry_t *pxBackUpKeyfobData = NULL;
    /* allocate dynamic memory */
    pxBackUpKeyfobData = (keyfobidKeyFobEntry_t *) pvPortMalloc(
	    sizeof(keyfobidKeyFobEntry_t));
    /* check if memory was allocated properly */
    if(pxBackUpKeyfobData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for keyfob entry structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxBackUpKeyfobData, commonDEFAULT_VALUE, sizeof(keyfobidKeyFobEntry_t));

    /* We need to check which partition has a valid data, based on valid magic
     * number. If both partition does not have valid magic number than, data
     * will be written to partition 1.
     */
    ulPartition = lCommonChoosePartition(keyfobidKEYFOB_PART1_ADDRESS,
	    keyfobidKEYFOB_PART2_ADDRESS, keyfobidKEYFOB_ID_MAGIC,
	    keyfobidKEYFOB_ID_MAGIC_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	/* assuming partition 1 has latest updated data, copy the partition 1
	 * data to backup structure.
	 */
	lStatus = lKeyfobidReadKeyFobEntry(pxBackUpKeyfobData);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("failed to back up keyfob data \r\n");
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
	lStatus = lCommonUpdatePartition(keyfobidKEYFOB_PART2_ADDRESS,
		sizeof(keyfobidKeyFobEntry_t),
		(uint8_t *) pxBackUpKeyfobData, keyfobidKEYFOB_ID_MAGIC,
		keyfobidKEYFOB_ID_MAGIC_SIZE);
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
	/* update the error code */
	lStatus = ulPartition;
	goto CLEANUP;
    }

    /* Before writing the user data we must update the magic header because user
     * is not aware of it.
     */
    pxKeyFobEntry->ulKeyFobIdMagic = keyfobidKEYFOB_ID_MAGIC;

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
    lStatus = lCommonUpdatePartition(keyfobidKEYFOB_PART1_ADDRESS,
	    sizeof(keyfobidKeyFobEntry_t), (uint8_t *) pxKeyFobEntry,
	    keyfobidKEYFOB_ID_MAGIC, keyfobidKEYFOB_ID_MAGIC_SIZE);
    /* xTaskResumeAll(); */
    taskEXIT_CRITICAL();
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write keyfob Data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxBackUpKeyfobData)
    {
	vPortFree(pxBackUpKeyfobData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeyfobidReadKeyFobEntry(keyfobidKeyFobEntry_t *pxKeyFobEntry)
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
    if(pxKeyFobEntry == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* We need to check if data needs to be written to partition 1 or 2
     * based on the valid magic number. If both partition does not have valid
     * magic number than choose partition 1.
     */
    ulPartition = lCommonChoosePartition(keyfobidKEYFOB_PART1_ADDRESS,
	    keyfobidKEYFOB_PART2_ADDRESS, keyfobidKEYFOB_ID_MAGIC,
	    keyfobidKEYFOB_ID_MAGIC_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	ulAddress = keyfobidKEYFOB_PART1_ADDRESS;
    }
    /* check if partition two has valid header */
    else if (ulPartition == commonPARTITION2)
    {
	ulAddress = keyfobidKEYFOB_PART2_ADDRESS;
    }
    /* Some error has occurred. */
    else
    {
	debugERROR_PRINT("No valid header partition found \r\n");
	return ulPartition;
    }

    /* read the manufacture data from the flash */
    lStatus = mml_sflc_read(ulAddress, (uint8_t *) pxKeyFobEntry,
	    sizeof(keyfobidKeyFobEntry_t));
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read keyfob data \r\n");
	return lStatus;
    }
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeyfobidReadKey( keyfobidKeyFobInfo_t *pxKeyInfo, uint8_t ucIndex)
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxKeyInfo == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Keyfob entry structure to read */
    keyfobidKeyFobEntry_t *pxBackUpKeyfobData = NULL;
    /* allocate dynamic memory */
    pxBackUpKeyfobData = (keyfobidKeyFobEntry_t *) pvPortMalloc(
	    sizeof(keyfobidKeyFobEntry_t));
    /* check if memory was allocated properly */
    if(pxBackUpKeyfobData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for keyfob entry structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxBackUpKeyfobData, commonDEFAULT_VALUE, sizeof(keyfobidKeyFobEntry_t));

    /* Now read the keyfob entry structure */
    lStatus = lKeyfobidReadKeyFobEntry(pxBackUpKeyfobData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to read Keyfob data \r\n");
	goto CLEANUP;
    }

    /* copy the desired keyfob info based on the index */
    if (pxBackUpKeyfobData->xKeyFobInfo[ucIndex].ulKeyMagic
	    == keyfobidKEYFOB_INFO_MAGIC)
    {
	memcpy((void *) pxKeyInfo,
		(void *) &pxBackUpKeyfobData->xKeyFobInfo[ucIndex],
		sizeof(keyfobidKeyFobInfo_t));
    }
    else
    {
	debugERROR_PRINT("keyfob info for index has no valid data \r\n");
	lStatus = COMMON_ERR_INVAL;
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxBackUpKeyfobData)
    {
	vPortFree(pxBackUpKeyfobData);
    }
    /* return error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeyfobidAddKey( keyfobidKeyFobInfo_t *pxKeyInfo)
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxKeyInfo == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Keyfob entry structure for backup */
    keyfobidKeyFobEntry_t *pxBackUpKeyfobData = NULL;
    /* Index for keyfob info */
    uint8_t ucIndex = 0;
    /* Index to add key */
    int8_t ucIndexAdd = -1;
    /* allocate dynamic memory */
    pxBackUpKeyfobData = (keyfobidKeyFobEntry_t *) pvPortMalloc(
	    sizeof(keyfobidKeyFobEntry_t));
    /* check if memory was allocated properly */
    if(pxBackUpKeyfobData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for keyfob entry structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxBackUpKeyfobData, commonDEFAULT_VALUE, sizeof(keyfobidKeyFobEntry_t));

    /* Now read the keyfob entry structure */
    lStatus = lKeyfobidReadKeyFobEntry(pxBackUpKeyfobData);
    if(lStatus != NO_ERROR)
    {
	/* reading might have failed because both partitions does not have valid
	 * magic header, lets add valid header.
	 */
	if(lStatus == commonPARTITIONNONE)
	{
	    pxBackUpKeyfobData->ulKeyFobIdMagic = keyfobidKEYFOB_ID_MAGIC;
	}
	/* Some other flash error might have occurred report error in this case*/
	else
	{
	    debugERROR_PRINT(" failed to read keyfobid entry data \r\n");
	    goto CLEANUP;
	}
    }

    /* Now find which index is free to hold the key */
    for(ucIndex = 0; ucIndex < keyfobidMAX_KEFOB_ENTRIES; ucIndex++)
    {
	if (pxBackUpKeyfobData->xKeyFobInfo[ucIndex].ulKeyMagic
		!= keyfobidKEYFOB_INFO_MAGIC)
	{
	    ucIndexAdd = ucIndex;
	    break;
	}
    }

    /* Now check if we have found index to hold new key */
    if(ucIndexAdd < 0)
    {
	lStatus = COMMON_ERR_OUT_OF_RANGE;
	goto CLEANUP;
    }

    /* Now we have the free index to add the new key, copy the user keyinfo
     * to the free index. Before that write the magic header of kefobid
     * because user is not aware of it.
     */
    pxKeyInfo->ulKeyMagic = keyfobidKEYFOB_INFO_MAGIC;
    memcpy((void *) &pxBackUpKeyfobData->xKeyFobInfo[ucIndexAdd],
	    (void *) pxKeyInfo, sizeof(keyfobidKeyFobInfo_t));

    /* Now we have updated the keyfob entry structure write it back to flash */
    lStatus = lKeyfobidWriteKeyFobEntry(pxBackUpKeyfobData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to update Keyfob data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxBackUpKeyfobData)
    {
	vPortFree(pxBackUpKeyfobData);
    }
    /* return error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeyfobidRemoveKeyKeyFobID( uint32_t ulKeyFobId )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Keyfob entry structure for backup */
    keyfobidKeyFobEntry_t *pxBackUpKeyfobData = NULL;
    /* Index for keyfob info */
    uint8_t ucIndex = 0;
    /* Keyfob index to delete */
    int8_t ucIndexDelete = -1;
    /* allocate dynamic memory */
    pxBackUpKeyfobData = (keyfobidKeyFobEntry_t *) pvPortMalloc(
	    sizeof(keyfobidKeyFobEntry_t));
    /* check if memory was allocated properly */
    if(pxBackUpKeyfobData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for keyfob entry structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxBackUpKeyfobData, commonDEFAULT_VALUE, sizeof(keyfobidKeyFobEntry_t));

    /* Now read the keyfob entry structure */
    lStatus = lKeyfobidReadKeyFobEntry(pxBackUpKeyfobData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to read Keyfob data \r\n");
	goto CLEANUP;
    }

    /* Find the index of key which needs to be deleted */
    for(ucIndex = 0; ucIndex < keyfobidMAX_KEFOB_ENTRIES; ucIndex++)
    {
	/* first need to check if the index has valid magic header */
	if (pxBackUpKeyfobData->xKeyFobInfo[ucIndex].ulKeyMagic
		== keyfobidKEYFOB_INFO_MAGIC)
	{
	    /* check if the keyfobID is found */
	    if (pxBackUpKeyfobData->xKeyFobInfo[ucIndex].ulKeyFobId
		    == ulKeyFobId)
	    {
		ucIndexDelete = ucIndex;
		break;
	    }
	}
    }

    /* Now check if we found key with valid ID to delete */
    if(ucIndexDelete < 0)
    {
	debugERROR_PRINT("No key found with this ID \r\n");
	lStatus = COMMON_ERR_OUT_OF_RANGE;
	goto CLEANUP;
    }

    /* Now we known index for the keyfobid to be deleted, make the magic
     * number zero that will make the key invalid */
    pxBackUpKeyfobData->xKeyFobInfo[ucIndexDelete].ulKeyMagic = 0;

    /* Now we have updated the keyfob entry structure write it back to flash */
    lStatus = lKeyfobidWriteKeyFobEntry(pxBackUpKeyfobData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to update Keyfob data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxBackUpKeyfobData)
    {
	vPortFree(pxBackUpKeyfobData);
    }
    /* return error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeyfobidRemoveKeyKeyName( uint8_t *pucKeyName )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucKeyName == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* Keyfob entry structure for backup */
    keyfobidKeyFobEntry_t *pxBackUpKeyfobData = NULL;
    /* Index for keyfob info */
    uint8_t ucIndex = 0;
    /* Keyfob index to delete */
    int8_t ucIndexDelete = -1;
    /* allocate dynamic memory */
    pxBackUpKeyfobData = (keyfobidKeyFobEntry_t *) pvPortMalloc(
	    sizeof(keyfobidKeyFobEntry_t));
    /* check if memory was allocated properly */
    if(pxBackUpKeyfobData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for keyfob entry structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxBackUpKeyfobData, commonDEFAULT_VALUE, sizeof(keyfobidKeyFobEntry_t));

    /* Now read the keyfob entry structure */
    lStatus = lKeyfobidReadKeyFobEntry(pxBackUpKeyfobData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to read Keyfob data \r\n");
	goto CLEANUP;
    }

    /* Find the index of key which needs to be deleted */
    for(ucIndex = 0; ucIndex < keyfobidMAX_KEFOB_ENTRIES; ucIndex++)
    {
	/* first need to check if the index has valid magic header */
	if (pxBackUpKeyfobData->xKeyFobInfo[ucIndex].ulKeyMagic
		== keyfobidKEYFOB_INFO_MAGIC)
	{
	    /* Now compare if the string is equal to ORWL */
	    if (!memcmp(
		(const char *) &pxBackUpKeyfobData->xKeyFobInfo[ucIndex].ucKeyName,
		(const char *) pucKeyName,
		keyfobidNAME_LEN_ACT))
	    {
		/* Keyfob name matches with name provided by user */
		ucIndexDelete = ucIndex;
		break;
	    }
	}
    }

    if(ucIndexDelete == 0)
    {
	debugERROR_PRINT("This key is First Keyfob.\n  Cannot be deleted \r\n");
	lStatus = COMMON_ERR_UNKNOWN;
	goto CLEANUP;
    }
    /* Now check if we found key with valid name, to delete */
    if(ucIndexDelete < 0)
    {
	debugERROR_PRINT("No key found with this name \r\n");
	lStatus = COMMON_ERR_OUT_OF_RANGE;
	goto CLEANUP;
    }

    /* Now we known index for the keyfobid to be deleted, make the magic
     * number zero that will make the key invalid */
    pxBackUpKeyfobData->xKeyFobInfo[ucIndexDelete].ulKeyMagic = 0;

    /* Now we have updated the keyfob entry structure write it back to flash */
    lStatus = lKeyfobidWriteKeyFobEntry(pxBackUpKeyfobData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to update Keyfob data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxBackUpKeyfobData)
    {
	vPortFree(pxBackUpKeyfobData);
    }
    /* return error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeyfobidUpdateKeyPrivilages( uint32_t ulKeyFobId, uint32_t ulKeyPrevilage )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Keyfob entry structure for backup */
    keyfobidKeyFobEntry_t *pxBackUpKeyfobData = NULL;
    /* Index for keyfob info */
    uint8_t ucIndex = 0;
    /* Keyfob index to update privilage settings */
    int8_t ucIndexUpdate = -1;
    /* allocate dynamic memory */
    pxBackUpKeyfobData = (keyfobidKeyFobEntry_t *) pvPortMalloc(
	    sizeof(keyfobidKeyFobEntry_t));
    /* check if memory was allocated properly */
    if(pxBackUpKeyfobData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for keyfob entry structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxBackUpKeyfobData, commonDEFAULT_VALUE, sizeof(keyfobidKeyFobEntry_t));

    /* Now read the keyfob entry structure */
    lStatus = lKeyfobidReadKeyFobEntry(pxBackUpKeyfobData);
    if(lStatus != NO_ERROR)
    {
	/* reading might have failed because both partitions does not have valid
	 * magic header, lets add valid header.
	 */
	if(lStatus == commonPARTITIONNONE)
	{
	    pxBackUpKeyfobData->ulKeyFobIdMagic = keyfobidKEYFOB_ID_MAGIC;
	}
	/* Some other flash error might have occurred report error */
	else
	{
	    debugERROR_PRINT(" failed to read keyfobid entry data \r\n");
	    goto CLEANUP;
	}
    }

    /* Find the index of key which needs to be deleted */
    for(ucIndex = 0; ucIndex < keyfobidMAX_KEFOB_ENTRIES; ucIndex++)
    {
	/* first need to check if the index has valid magic header */
	if (pxBackUpKeyfobData->xKeyFobInfo[ucIndex].ulKeyMagic
		== keyfobidKEYFOB_INFO_MAGIC)
	{
	    /* check the keyfobID that matches to user provided keyfobID */
	    if (pxBackUpKeyfobData->xKeyFobInfo[ucIndex].ulKeyFobId
		    == ulKeyFobId)
	    {
		ucIndexUpdate = ucIndex;
		break;
	    }
	}
    }

    /* Now check if we found key with valid ID to update */
    if(ucIndexUpdate < 0)
    {
	debugERROR_PRINT("No key found with this ID \r\n");
	lStatus = COMMON_ERR_OUT_OF_RANGE;
	goto CLEANUP;
    }

    /* Now we now known index for the keyfobid to be updated*/
    pxBackUpKeyfobData->xKeyFobInfo[ucIndexUpdate].ulKeyPrevilage =
	    ulKeyPrevilage;

    /* Now we have updated the keyfob entry structure write it back to flash */
    lStatus = lKeyfobidWriteKeyFobEntry(pxBackUpKeyfobData);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("Failed to update Keyfob data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxBackUpKeyfobData)
    {
	vPortFree(pxBackUpKeyfobData);
    }
    /* return error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeyfobidCheckFreeIndex( uint8_t *pucIndexAdd)
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Keyfob entry structure for backup */
    keyfobidKeyFobEntry_t *pxBackUpKeyfobData = NULL;
    /* Index for keyfob info */
    uint8_t ucIndex = 0;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucIndexAdd == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* allocate dynamic memory */
    pxBackUpKeyfobData = (keyfobidKeyFobEntry_t *) pvPortMalloc(
	    sizeof(keyfobidKeyFobEntry_t));
    /* check if memory was allocated properly */
    if(pxBackUpKeyfobData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for keyfob entry structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxBackUpKeyfobData, commonDEFAULT_VALUE, sizeof(keyfobidKeyFobEntry_t));

    /* Now read the keyfob entry structure */
    lStatus = lKeyfobidReadKeyFobEntry(pxBackUpKeyfobData);
    if(lStatus != NO_ERROR)
    {
	/* reading might have failed because both partitions does not have valid
	 * magic header, lets add valid header.
	 */
	if(lStatus == commonPARTITIONNONE)
	{
	    pxBackUpKeyfobData->ulKeyFobIdMagic = keyfobidKEYFOB_ID_MAGIC;
	    lStatus = NO_ERROR;
	}
	/* Some other flash error might have occurred report error in this case*/
	else
	{
	    debugERROR_PRINT(" failed to read keyfobid entry data \r\n");
	    goto CLEANUP;
	}
    }

    *pucIndexAdd = keyfobidMAX_KEFOB_ENTRIES;
    /* Now find which index is free to hold the key */
    for(ucIndex = 0; ucIndex < keyfobidMAX_KEFOB_ENTRIES; ucIndex++)
    {
	if (pxBackUpKeyfobData->xKeyFobInfo[ucIndex].ulKeyMagic
		!= keyfobidKEYFOB_INFO_MAGIC)
	{
	    (*pucIndexAdd)--;
	}
    }
    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxBackUpKeyfobData)
    {
	vPortFree(pxBackUpKeyfobData);
    }
    /* return error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/
