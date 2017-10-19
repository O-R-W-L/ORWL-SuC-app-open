/**===========================================================================
 * @file access_key.c
 *
 * @brief This file contains all the API definition required to handle
 * ORWL unique pins, password and access control keys
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
#include <stdint.h>
#include <debug.h>
#include <printf_lite.h>
#include <string.h>
#include <mml_sflc.h>

/* Freertos includes */
#include <FreeRTOS.h>
#include <task.h>
#include <portable.h>

/* Local includes */
#include <mem_common.h>
#include <access_keys.h>
/* Not exposing flash.h to users */
#include "flash.h"

/**
 * Magic number for access keys.
 */
#define keyACCESS_KEY_MAGIC		(0xACCE551E)

/**
 * Magic number size for access keys is 4 bytes.
 */
#define keyACCESS_KEY_MAGIC_SIZE	(4)

/**
 * access keys partition 1 address.
 */
#define keysACCESS_KEY_PART1	(flashACCESS_KEY_START_ADDR)

/**
 * Access key partition 2(backup) address.
 */
#define keysACCESS_KEY_PART2	((keysACCESS_KEY_PART1 + flashPAGE_SIZE))

/** function definition */

int32_t lKeysWriteMasterKeys( keysDSFT_MASTER_KEYS_t *pxACKeys )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Partition to write */
    uint32_t ulPartition = 0;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxACKeys == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* access key structure for backup */
    keysDSFT_MASTER_KEYS_t *pxBackUpAKData = NULL;
    /* allocate dynamic memory */
    pxBackUpAKData = (keysDSFT_MASTER_KEYS_t *) pvPortMalloc(
	    sizeof(keysDSFT_MASTER_KEYS_t));
    /* check if memory was allocated properly */
    if(pxBackUpAKData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for access key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxBackUpAKData, commonDEFAULT_VALUE, sizeof(keysDSFT_MASTER_KEYS_t));

    /* We need to check which partition has a valid data, based on valid magic
     * number. If both partition does not have valid magic number than, data
     * will be written to partition 1.
     */
    ulPartition = lCommonChoosePartition(keysACCESS_KEY_PART1,
	    keysACCESS_KEY_PART2, keyACCESS_KEY_MAGIC,
	    keyACCESS_KEY_MAGIC_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	/* assuming partition 1 has latest updated data, copy the partition 1
	 * data to backup structure.
	 */
	lStatus = lKeysReadMasterKeys(pxBackUpAKData);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("failed to read access key data \r\n");
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
	lStatus = lCommonUpdatePartition(keysACCESS_KEY_PART2,
		sizeof(keysDSFT_MASTER_KEYS_t), (uint8_t *) pxBackUpAKData,
		keyACCESS_KEY_MAGIC, keyACCESS_KEY_MAGIC_SIZE);
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
	/* update error code */
	lStatus = ulPartition;
	goto CLEANUP;
    }

    /* Before writing the user data we must update the magic header because user
     * is not aware of it.
     */
    pxACKeys->ulAccessKeyMagic = keyACCESS_KEY_MAGIC;

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
    lStatus = lCommonUpdatePartition(keysACCESS_KEY_PART1,
	    sizeof(keysDSFT_MASTER_KEYS_t), (uint8_t *) pxACKeys,
	    keyACCESS_KEY_MAGIC, keyACCESS_KEY_MAGIC_SIZE);
    /* xTaskResumeAll(); */
    taskEXIT_CRITICAL();
    if(lStatus != NO_ERROR)
	{
	     debugERROR_PRINT("failed to write access key data \r\n");
	     goto CLEANUP;
	}

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxBackUpAKData)
    {
	vPortFree(pxBackUpAKData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeysWriteMasterKeysNONRTOS( keysDSFT_MASTER_KEYS_t *pxACKeys )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Partition to write */
    uint32_t ulPartition = 0;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxACKeys == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* access key structure for backup */
    keysDSFT_MASTER_KEYS_t *pxBackUpAKData = NULL;
    /* allocate dynamic memory */
    pxBackUpAKData = (keysDSFT_MASTER_KEYS_t *) pvPortMalloc(
	    sizeof(keysDSFT_MASTER_KEYS_t));
    /* check if memory was allocated properly */
    if(pxBackUpAKData == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for access key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxBackUpAKData, commonDEFAULT_VALUE, sizeof(keysDSFT_MASTER_KEYS_t));

    /* We need to check which partition has a valid data, based on valid magic
     * number. If both partition does not have valid magic number than, data
     * will be written to partition 1.
     */
    ulPartition = lCommonChoosePartition(keysACCESS_KEY_PART1,
	    keysACCESS_KEY_PART2, keyACCESS_KEY_MAGIC,
	    keyACCESS_KEY_MAGIC_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	/* assuming partition 1 has latest updated data, copy the partition 1
	 * data to backup structure.
	 */
	lStatus = lKeysReadMasterKeys(pxBackUpAKData);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("failed to read access key data \r\n");
	    goto CLEANUP;
	}
	/* Now write primary partition data to backup partition */
	lStatus = lCommonUpdatePartition(keysACCESS_KEY_PART2,
		sizeof(keysDSFT_MASTER_KEYS_t), (uint8_t *) pxBackUpAKData,
		keyACCESS_KEY_MAGIC, keyACCESS_KEY_MAGIC_SIZE);
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
	/* update error code */
	lStatus = ulPartition;
	goto CLEANUP;
    }

    /* Before writing the user data we must update the magic header because user
     * is not aware of it.
     */
    pxACKeys->ulAccessKeyMagic = keyACCESS_KEY_MAGIC;

    /* Now update the primary partition with latest data given by the user*/
    lStatus = lCommonUpdatePartition(keysACCESS_KEY_PART1,
	    sizeof(keysDSFT_MASTER_KEYS_t), (uint8_t *) pxACKeys,
	    keyACCESS_KEY_MAGIC, keyACCESS_KEY_MAGIC_SIZE);
    if(lStatus != NO_ERROR)
	{
	     debugERROR_PRINT("failed to write access key data \r\n");
	     goto CLEANUP;
	}

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxBackUpAKData)
    {
	vPortFree(pxBackUpAKData);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeysReadMasterKeys( keysDSFT_MASTER_KEYS_t *pxMKeys )
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
    if(pxMKeys == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* We need to check if data needs to be written to partition 1 or 2
     * based on the valid magic number. If both partition does not have valid
     * magic number than choose partition 1.
     */
    ulPartition = lCommonChoosePartition(keysACCESS_KEY_PART1,
	    keysACCESS_KEY_PART2, keyACCESS_KEY_MAGIC,
	    keyACCESS_KEY_MAGIC_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	ulAddress = keysACCESS_KEY_PART1;
    }
    /* check if partition two has valid header */
    else if (ulPartition == commonPARTITION2)
    {
	ulAddress = keysACCESS_KEY_PART2;
    }
    /* Some error has occurred. */
    else
    {
	debugERROR_PRINT("No valid header partition found \r\n");
	return ulPartition;
    }

    /* read the access key data from the flash */
    lStatus = mml_sflc_read(ulAddress, (uint8_t *) pxMKeys,
	    sizeof(keysDSFT_MASTER_KEYS_t));
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read accees key data \r\n");
	return lStatus;
    }
    /* return error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeysGetDefaultPIN( uint8_t *pucDefaultPin )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucDefaultPin == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* access key structure to read from flash */
    keysDSFT_MASTER_KEYS_t *pxAccessKey = NULL;
    /* allocate dynamic memory */
    pxAccessKey = (keysDSFT_MASTER_KEYS_t *) pvPortMalloc(
	    sizeof(keysDSFT_MASTER_KEYS_t));
    /* check if memory was allocated properly */
    if(pxAccessKey == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for access key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxAccessKey, commonDEFAULT_VALUE, sizeof(keysDSFT_MASTER_KEYS_t));

    /* read access key data from flash */
    lStatus = lKeysReadMasterKeys(pxAccessKey);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read access key \r\n");
	goto CLEANUP;
    }
    /* copy the default pin */
    memcpy((void *) pucDefaultPin, (const void *) pxAccessKey->ucDefaultPIN,
	    sizeof(pxAccessKey->ucDefaultPIN));

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxAccessKey)
    {
	vPortFree(pxAccessKey);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeysGetUserPIN( uint8_t *pucUserPin )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucUserPin == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* access key structure to read from flash */
    keysDSFT_MASTER_KEYS_t *pxAccessKey = NULL;
    pxAccessKey = (keysDSFT_MASTER_KEYS_t *) pvPortMalloc(
	    sizeof(keysDSFT_MASTER_KEYS_t));
    /* check if memory was allocated properly */
    if(pxAccessKey == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for access key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxAccessKey, commonDEFAULT_VALUE, sizeof(keysDSFT_MASTER_KEYS_t));

    /* read access key data from flash */
    lStatus = lKeysReadMasterKeys(pxAccessKey);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read access key \r\n");
	goto CLEANUP;
    }
    /* copy user pin */
    memcpy((void *) pucUserPin, (const void *) pxAccessKey->ucUserPIN,
	    sizeof(pxAccessKey->ucUserPIN));

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxAccessKey)
    {
	vPortFree(pxAccessKey);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeysUpdateUserPIN( uint8_t *pucUserPin )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucUserPin == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* access key structure to read from flash */
    keysDSFT_MASTER_KEYS_t *pxAccessKey = NULL;
    pxAccessKey = (keysDSFT_MASTER_KEYS_t *) pvPortMalloc(
	    sizeof(keysDSFT_MASTER_KEYS_t));
    /* check if memory was allocated properly */
    if(pxAccessKey == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for access key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxAccessKey, commonDEFAULT_VALUE, sizeof(keysDSFT_MASTER_KEYS_t));

    /* read access key data from flash */
    lStatus = lKeysReadMasterKeys(pxAccessKey);
    if(lStatus != NO_ERROR)
    {
	/* To update keys we are not updating if there is no valid data in the
	 * flash because default keys should be present before adding/upadting
	 * the user pin.
	 */
	debugERROR_PRINT(" failed to read access key data \r\n");
	goto CLEANUP;
    }

    /* copy the user pin from user */
    memcpy((void *) pxAccessKey->ucUserPIN, (const void *) pucUserPin,
	    sizeof(pxAccessKey->ucUserPIN));

    /* write back the access key data */
    lStatus = lKeysWriteMasterKeys(pxAccessKey);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write access key data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxAccessKey)
    {
	vPortFree(pxAccessKey);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeysGetDefISDKey( uint8_t *pucISDKey )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucISDKey == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* access key structure to read from flash */
    keysDSFT_MASTER_KEYS_t *pxAccessKey = NULL;
    pxAccessKey = (keysDSFT_MASTER_KEYS_t *) pvPortMalloc(
	    sizeof(keysDSFT_MASTER_KEYS_t));
    /* check if memory was allocated properly */
    if(pxAccessKey == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for access key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxAccessKey, commonDEFAULT_VALUE, sizeof(keysDSFT_MASTER_KEYS_t));

    /* read access key data from flash */
    lStatus = lKeysReadMasterKeys(pxAccessKey);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read access key \r\n");
	goto CLEANUP;
    }
    /* copy default ISD keys */
    memcpy((void *) pucISDKey, (const void *) pxAccessKey->ucDefaultISDKey,
	    sizeof(pxAccessKey->ucDefaultISDKey));

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxAccessKey)
    {
	vPortFree(pxAccessKey);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeysGetDefSSDKey( uint8_t *pucSSDKey )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucSSDKey == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* access key structure to read from flash */
    keysDSFT_MASTER_KEYS_t *pxAccessKey = NULL;
    pxAccessKey = (keysDSFT_MASTER_KEYS_t *) pvPortMalloc(
	    sizeof(keysDSFT_MASTER_KEYS_t));
    /* check if memory was allocated properly */
    if(pxAccessKey == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for access key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxAccessKey, commonDEFAULT_VALUE, sizeof(keysDSFT_MASTER_KEYS_t));

    /* read access key data from flash */
    lStatus = lKeysReadMasterKeys(pxAccessKey);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read access key \r\n");
	goto CLEANUP;
    }
    /* copy default ISD keys */
    memcpy((void *) pucSSDKey, (const void *) pxAccessKey->ucDefaultSSDKey,
	    sizeof(pxAccessKey->ucDefaultSSDKey));

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxAccessKey)
    {
	vPortFree(pxAccessKey);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeysGetDefAdminPasswd( uint8_t *pucPasswd )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucPasswd == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* access key structure to read from flash */
    keysDSFT_MASTER_KEYS_t *pxAccessKey = NULL;
    pxAccessKey = (keysDSFT_MASTER_KEYS_t *) pvPortMalloc(
	    sizeof(keysDSFT_MASTER_KEYS_t));
    /* check if memory was allocated properly */
    if(pxAccessKey == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for access key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxAccessKey, commonDEFAULT_VALUE, sizeof(keysDSFT_MASTER_KEYS_t));

    /* read access key data from flash */
    lStatus = lKeysReadMasterKeys(pxAccessKey);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read access key \r\n");
	goto CLEANUP;
    }
    /* copy default ISD keys */
    memcpy((void *) pucPasswd, (const void *) pxAccessKey->ucAdminPassword,
	    sizeof(pxAccessKey->ucAdminPassword));

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxAccessKey)
    {
	vPortFree(pxAccessKey);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeysGetSSDserialNum( uint8_t *pucSSDSerial )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucSSDSerial == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* access key structure to read from flash */
    keysDSFT_MASTER_KEYS_t *pxAccessKey = NULL;
    pxAccessKey = (keysDSFT_MASTER_KEYS_t *) pvPortMalloc(
	    sizeof(keysDSFT_MASTER_KEYS_t));
    /* check if memory was allocated properly */
    if(pxAccessKey == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for access key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxAccessKey, commonDEFAULT_VALUE, sizeof(keysDSFT_MASTER_KEYS_t));

    /* read access key data from flash */
    lStatus = lKeysReadMasterKeys(pxAccessKey);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read access key \r\n");
	goto CLEANUP;
    }
    /* copy SSD serial number */
    memcpy((void *) pucSSDSerial, (const void *) pxAccessKey->ucSSDSerialNum,
	    sizeof(pxAccessKey->ucSSDSerialNum));

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxAccessKey)
    {
	vPortFree(pxAccessKey);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lKeysUpdateSSDserialNum( uint8_t *pucSSDSerial )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucSSDSerial == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* access key structure to read from flash */
    keysDSFT_MASTER_KEYS_t *pxAccessKey = NULL;
    pxAccessKey = (keysDSFT_MASTER_KEYS_t *) pvPortMalloc(
	    sizeof(keysDSFT_MASTER_KEYS_t));
    /* check if memory was allocated properly */
    if(pxAccessKey == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for access key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(pxAccessKey, commonDEFAULT_VALUE, sizeof(keysDSFT_MASTER_KEYS_t));

    /* read access key data from flash */
    lStatus = lKeysReadMasterKeys(pxAccessKey);
    if(lStatus != NO_ERROR)
    {
	/* To update SSD serial we are not updating if there is no valid data in the
	 * flash because.
	 */
	debugERROR_PRINT(" failed to read access key data \r\n");
	goto CLEANUP;
    }

    /* copy the user pin from user */
    memcpy((void *) pxAccessKey->ucSSDSerialNum, (const void *) pucSSDSerial,
	    sizeof(pxAccessKey->ucSSDSerialNum));

    /* write back the access key data */
    lStatus = lKeysWriteMasterKeys(pxAccessKey);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write access key data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(pxAccessKey)
    {
	vPortFree(pxAccessKey);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/
