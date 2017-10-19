/**===========================================================================
 * @file enckey.c
 *
 * @brief This file contains all the API definition required to handle
 * ORWL encryption keys
 *
 * @author megharaj.agdesign-shift.com
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

/*Global includes*/
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
#include <enckeys.h>
#include <mem_common.h>
/* Not exposing flash.h to users */
#include "flash.h"

/**
 * magic number to access DesignShift encryption keys
 */
#define enckeyENC_KEY_MAGIC		(0xE9C801E5U)

/**
 * encryption key magic number size.
 */
#define enckeyENC_KEY_MAGIC_SIZE	(4)

/**
 * encryption key partition 1 address.
 */
#define	enckeysEN_KEY_PART1	(flashENC_KEY_START_ADDR)

/**
 * encryption key partition 2(backup) address.
 */
#define	enckeysEN_KEY_PART2	(enckeysEN_KEY_PART1 + flashPAGE_SIZE)

/** function definition */

int32_t lEnckeysWriteEncKey( enckeysDsftEncKeys_t *pxENKey )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* Partition to write */
    uint32_t ulPartition = 0;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pxENKey == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* encryption structure for backup */
    enckeysDsftEncKeys_t *xBackUpENKey = NULL;
    /* allocate dynamic memory */
    xBackUpENKey = (enckeysDsftEncKeys_t *) pvPortMalloc(
	    sizeof(enckeysDsftEncKeys_t));
    /* check if memory was allocated properly */
    if(xBackUpENKey == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(xBackUpENKey, commonDEFAULT_VALUE, sizeof(enckeysDsftEncKeys_t));

    /* We need to check which partition has a valid data, based on valid magic
     * number. If both partition does not have valid magic number than, data
     * will be written to partition 1.
     */
    ulPartition = lCommonChoosePartition(enckeysEN_KEY_PART1,
	    enckeysEN_KEY_PART2, enckeyENC_KEY_MAGIC, enckeyENC_KEY_MAGIC_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	/* assuming partition 1 has latest updated data, copy the partition 1
	 * data to backup structure.
	 */
	lStatus = lEnckeysReadEncKey(xBackUpENKey);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("failed to back up encryption key \r\n");
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
	lStatus = lCommonUpdatePartition(enckeysEN_KEY_PART2,
		sizeof(enckeysDsftEncKeys_t), (uint8_t *) xBackUpENKey,
		enckeyENC_KEY_MAGIC, enckeyENC_KEY_MAGIC_SIZE);
	/* xTaskResumeAll(); */
	taskEXIT_CRITICAL();
        if(lStatus != NO_ERROR)
	{
	     debugERROR_PRINT("failed to update encryption key \r\n");
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
    pxENKey->ulEncKeyMagic = enckeyENC_KEY_MAGIC;

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
    lStatus = lCommonUpdatePartition(enckeysEN_KEY_PART1,
	    sizeof(enckeysDsftEncKeys_t), (uint8_t *) pxENKey,
	    enckeyENC_KEY_MAGIC, enckeyENC_KEY_MAGIC_SIZE);
    /* xTaskResumeAll(); */
    taskEXIT_CRITICAL();
    if(lStatus != NO_ERROR)
    {
	     debugERROR_PRINT("failed to write encryption keys Data \r\n");
	     goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(xBackUpENKey)
    {
	vPortFree(xBackUpENKey);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lEnckeysReadEncKey( enckeysDsftEncKeys_t *pxENKey )
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
    if(pxENKey == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* We need to check if data needs to be written to partition 1 or 2
     * based on the valid magic number. If both partition does not have valid
     * magic number than choose partition 1.
     */
    ulPartition = lCommonChoosePartition(enckeysEN_KEY_PART1,
	    enckeysEN_KEY_PART2, enckeyENC_KEY_MAGIC, enckeyENC_KEY_MAGIC_SIZE);
    /* check if partition one has valid header */
    if(ulPartition == commonPARTITION1)
    {
	ulAddress = enckeysEN_KEY_PART1;
    }
    /* check if partition two has valid header */
    else if (ulPartition == commonPARTITION2)
    {
	ulAddress = enckeysEN_KEY_PART2;
    }
    /* Some error has occurred. */
    else
    {
	debugERROR_PRINT("No valid header partition found \r\n");
	return ulPartition;
    }

    /* read the encryption data from the flash */
    lStatus = mml_sflc_read(ulAddress, (uint8_t *) pxENKey,
	    sizeof(enckeysDsftEncKeys_t));
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read encryption key data \r\n");
	return lStatus;
    }
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lEnckeysGetPublicKey( uint8_t *pucPublicKey )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucPublicKey == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* access key structure to read from flash */
    enckeysDsftEncKeys_t *xENKey = NULL;
    /* allocate dynamic memory */
    xENKey = (enckeysDsftEncKeys_t *)pvPortMalloc(sizeof(enckeysDsftEncKeys_t));
    /* check if memory was allocated properly */
    if(xENKey == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(xENKey, commonDEFAULT_VALUE, sizeof(enckeysDsftEncKeys_t));

    /* read manufacture data from flash */
    lStatus = lEnckeysReadEncKey(xENKey);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to read encryption key \r\n");
	goto CLEANUP;
    }
    /* copy the public key to user buffer */
    memcpy((void *) pucPublicKey, (const void *) xENKey->ucEncDsftPubKey,
	    sizeof(xENKey->ucEncDsftPubKey));

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(xENKey)
    {
	vPortFree(xENKey);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lEnckeysUpdatePublicKey( uint8_t *pucPublicKey )
{
    /* status to return */
    int32_t lStatus = NO_ERROR;
    /* First we need to check if the pointer passed by the user is valid and
     * not NULL.
     */
    if(pucPublicKey == NULL)
    {
	debugERROR_PRINT("Invalid argument \n");
	return COMMON_ERR_NULL_PTR;
    }
    /* access key structure to read from flash */
    enckeysDsftEncKeys_t *xENKey = NULL;
    /* allocate dynamic memory */
    xENKey = (enckeysDsftEncKeys_t *)pvPortMalloc(sizeof(enckeysDsftEncKeys_t));
    /* check if memory was allocated properly */
    if(xENKey == NULL)
    {
	debugERROR_PRINT(
		"Failed to allocate memory for encryption key structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* After successful allocation of memory do memset to 0xff */
    memset(xENKey, commonDEFAULT_VALUE, sizeof(enckeysDsftEncKeys_t));

    /* read manufacture data from flash */
    lStatus = lEnckeysReadEncKey(xENKey);
    if(lStatus != NO_ERROR)
    {
	/* reading might have failed because both partitions does not have valid
	 * magic header, lets add valid header.
	 */
	if(lStatus == commonPARTITIONNONE)
	{
	    xENKey->ulEncKeyMagic = enckeyENC_KEY_MAGIC;
	}
	/* Some other flash error might have occurred report */
	else
	{
	    debugERROR_PRINT(" failed to read encryption data \r\n");
	    goto CLEANUP;
	}
    }

    /* copy the public key from user buffer */
    memcpy((void *) xENKey->ucEncDsftPubKey, (const void *) pucPublicKey,
	    sizeof(xENKey->ucEncDsftPubKey));

    /* write back the encryption key data */
    lStatus = lEnckeysWriteEncKey(xENKey);
    if(lStatus != NO_ERROR)
    {
	debugERROR_PRINT("failed to write encryption data \r\n");
	goto CLEANUP;
    }

    /* clean up the allocated memory before returning error */
    CLEANUP:
    if(xENKey)
    {
	vPortFree(xENKey);
    }
    /* return the error code */
    return lStatus;
}
/*----------------------------------------------------------------------------*/
