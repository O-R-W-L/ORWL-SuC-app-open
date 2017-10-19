/**===========================================================================
 * @file nvsram.c
 *
 * @brief This file contains rotines to access and modify the nvsram contents
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

/* Global includes */
#include <stdint.h>
#include <errors.h>
#include <string.h>
#include <stddef.h>
#include <freeRTOS.h>
/* Local include */
#include <mml.h>	/** include Cobra api's and macros*/
#include <nvsram.h>	/** include nvsram header */
#include <orwl_err.h>
#include <mml_trng_regs.h>

/** MAX32550 NVSRAM configurations */

/**
 * magic number to access NVSRAM
 */
#define nvsramSNVSRAM_MAGIC		( 0x909758A3 )

/**
 * snvram start address
 */
#define nvsramSNVSRAM_BASE		( MML_MEM_SNVSRAM_BASE )

/**
 * snvram size
 */
#define nvsramSNVSRAM_SIZE		( MML_MEM_SNVSRAM_SIZE )

/**
 * size of magic header
 */
#define nvsramSNVSRAM_MAGIC_SIZE	( 4 )

/**
 * length of Encryption key
 */
#define nvsramSNVSRAM_ENCYKEY_SIZE	( 4 )

/**
 * Size of nvsram partition
 */
#define nvsramSNVSRAM_PART_SIZE		( 512 )

/**
 * Start address of partition 1
 */
#define nvsramPART1_ADDRESS		( nvsramSNVSRAM_BASE )

/**
 * Start Address of partition 2
 */
#define nvsramPART2_ADDRESS		( nvsramSNVSRAM_BASE + nvsramSNVSRAM_PART_SIZE )

/**
 * data for erasing NVSRAM
 */
#define nvsramErase_DATA		( 0x00 )

/**
 * Base address of trng registers
 */
#define nvsramTRNG_IOBASE		( MML_TRNG_IOBASE )

/**
 * Enable new AES generation
 */
#define nvsramENABLE_AESKG		( MML_TRNGCN_AESKG_MASK )

/** Offset of ssd password from base address */
#define nvsramSSD_OFFSET	(8)

/** function declaration */

/** @brief Erase NVSRAM
 *
 * This function erase the NVSRAM according to address and size.
 *
 * @param ulAddr is the starting address of NVRAM partition to be erased.
 * @param ulSize is size of NVSRAM to be erased
 *
 * @return error code..
 */
static int32_t prvNvsramErase( uint32_t ulAddr, uint32_t ulSize );

/** @brief Write NVSRAM
 *
 * This function writes on NVSRAM from the structure passed to the function.
 *
 * @param ulAddr is the starting address of NVRAM partition to written.
 * @param ulSize is size of NVSRAM to be written.
 * @param pxData is pointer to the structure.
 *
 * @return error code..
 */
static int32_t prvNvsramWrite( uint32_t ulAddr, uint32_t ulSize,
	NvsramData_t *pxData );

/** @brief Update magic header
 *
 * This function writes valid magic header.
 *
 * @param ulAddr is the starting address of NVRAM partition to write valid
 * magic header.
 *
 * @return error code..
 */
static int32_t prvNvsramUpdateHeader( uint32_t ulAddr );

/** @brief Invalidate magic header
 *
 * This function Invalidates magic header by writing 0xFFFF to header.
 *
 * @param ulAddr is the starting address of NVRAM partition to write
 * invalid header.
 *
 * @return error code..
 */
static int32_t prvNvsramInvalidateHeader( uint32_t ulAddr );

/** @brief Select proper partition
 *
 * This function selects proper partition from user partition and backup
 * partition by checking magic header.
 *
 * @param ulPar1Address is the starting address of NVRAM partition 1.
 * @param ulPar2Address is the starting address of NVRAM partition 1..
 *
 * @return partition address in case of success and error code in
 * case of failure..
 */
static uint32_t prvNvsramChoosePartition(uint32_t ulPar1Address, uint32_t
	ulPar2Address );

/** @brief Select proper partition
 *
 * This function selects proper partition from user partition and backup
 * partition by checking magic header.
 *
 * @param ulAddr is the address for read.
 * @param ulSize is the size of the data to be read
 * @param pucData is the pointer which populated with data
 *
 * @return error code..
 */
static int32_t prvNvsramReadPartition(uint32_t ulAddr, uint32_t ulSize ,
	NvsramData_t *pucData);
/*----------------------------------------------------------------------------*/

/* function definition */

static int32_t prvNvsramInvalidateHeader( uint32_t ulAddr )
{
	/* Validate Address of Magic header */
	if( ulAddr == nvsramPART1_ADDRESS || ulAddr == nvsramPART2_ADDRESS)
	{
	    NvsramData_t *pxData =(NvsramData_t *)ulAddr;
	    if(pxData == NULL)
	    {
		return COMMON_ERR_NULL_PTR;
	    }
	    /* Erase magic header */
	    pxData -> ulMagic = nvsramErase_DATA;
	    return NO_ERROR;
	}
	return eORWL_ERROR_NVSRAM_INVALID_ADDRESS;
}
/*----------------------------------------------------------------------------*/

static int32_t prvNvsramUpdateHeader( uint32_t ulAddr )
{
	/* Validate address of Magic header */
	if( ulAddr == nvsramPART1_ADDRESS || ulAddr == nvsramPART2_ADDRESS)
	{
	    NvsramData_t *pxData =(NvsramData_t *)ulAddr;
	    if(pxData == NULL)
	    {
		return COMMON_ERR_NULL_PTR;
	    }
	    /* Write Valid magic header */
	    pxData -> ulMagic = nvsramSNVSRAM_MAGIC;
	    return NO_ERROR;
	}
	return eORWL_ERROR_NVSRAM_INVALID_ADDRESS;
}
/*----------------------------------------------------------------------------*/

static int32_t prvNvsramErase( uint32_t ulAddr, uint32_t ulSize )
{
	volatile uint32_t *pulnvAddr;

	/* Validate NVSRAM address */

	if (( ulAddr ) > (nvsramSNVSRAM_BASE + nvsramSNVSRAM_SIZE))
	{
	    return eORWL_ERROR_NVSRAM_INVALID_ADDRESS;
	}
	else
	{
	    pulnvAddr = (volatile uint32_t *) ulAddr;
	    if( pulnvAddr == NULL )
	    {
		return COMMON_ERR_NULL_PTR;
	    }
	}
	 /*
	 * ensure that requested size (erase bytes ulSize) falls within
	 * NVRAM address range
	 */
	if (( ulAddr + ulSize )
			> ( nvsramSNVSRAM_BASE + nvsramSNVSRAM_SIZE ))
	{
		return eORWL_ERROR_NVSRAM_INVALID_SIZE;
	}
	/* fill 0(zero) in NVSRAM for 'ulSize' bytes */
	memset((void *)pulnvAddr, nvsramErase_DATA, ulSize);
	/* erase completed */
	return NO_ERROR;
}
/*----------------------------------------------------------------------------*/

static int32_t prvNvsramReadPartition(uint32_t ulAddr, uint32_t ulSize ,
	NvsramData_t *pucData)
{
	volatile uint32_t *pulnvAddr;
	if(pucData == NULL)
	{
	    return COMMON_ERR_NULL_PTR;
	}
	else if( ulAddr != nvsramPART1_ADDRESS && ulAddr != nvsramPART2_ADDRESS)
	{
	    return eORWL_ERROR_NVSRAM_INVALID_ADDRESS;
	}
	else
	{
	    pulnvAddr = (volatile uint32_t *) ulAddr;
	}

	/* ensure that requested size (read bytes Size) falls within
	 * NVRAM address range */
	if (( ulAddr + ulSize ) > ( nvsramSNVSRAM_BASE + nvsramSNVSRAM_SIZE ))
	{
	    return eORWL_ERROR_NVSRAM_INVALID_SIZE;
	}
	/* Reading of data from given address to Structure */
	memcpy((void *)pucData, (const void *)pulnvAddr, ulSize);
	return NO_ERROR;
}
/*----------------------------------------------------------------------------*/

static int32_t prvNvsramWrite( uint32_t ulAddr, uint32_t ulSize,
	NvsramData_t *pxData )
{
	uint8_t *pucNvAddr;
	if(pxData == NULL)
	{
	    return COMMON_ERR_NULL_PTR;
	}
	/* Validate NVSRAM address */
	else if( ulAddr != nvsramPART1_ADDRESS && ulAddr != nvsramPART2_ADDRESS)
	{
	    return eORWL_ERROR_NVSRAM_INVALID_ADDRESS;
	}
	else
	{
	    pucNvAddr = ((uint8_t *)ulAddr) + nvsramSNVSRAM_MAGIC_SIZE;
	}

	/* ensure that requested size (write bytes size) falls within
	 * NVRAM address range */
	if(( ulAddr + ulSize ) > ( nvsramSNVSRAM_BASE + nvsramSNVSRAM_SIZE ))
	{
	    return eORWL_ERROR_NVSRAM_INVALID_SIZE;
	}
	/* write the data of structure to NVSRAM */
	memcpy( pucNvAddr, ((const uint8_t *)(pxData))+ nvsramSNVSRAM_MAGIC_SIZE,
		((uint8_t)ulSize) - nvsramSNVSRAM_MAGIC_SIZE);
	return NO_ERROR;
}
/*----------------------------------------------------------------------------*/

int32_t lNvsramWriteData( NvsramData_t *pxData )
{
	/* status to return */
	int32_t lStatus = NO_ERROR;
	/* Partition to write */
	uint32_t ulPartition = 0;
	/* Check for valid pointer */
	if(pxData == NULL)
	{
	    return COMMON_ERR_NULL_PTR;
	}
	/* We need to check if data needs to be written to partition 1 or 2
	 * based on the valid magic number. If both partition does not have valid
	 * magic number than choose partition 1.
	 */
	ulPartition = prvNvsramChoosePartition( nvsramPART1_ADDRESS,
							nvsramPART2_ADDRESS );
	/* check if partition one has valid header */
	if(ulPartition == nvsramPART1_ADDRESS)
	{
	    /* Erase partition2 before taking backup */
	    lStatus = prvNvsramErase((uint32_t)(nvsramPART2_ADDRESS),
		    (uint32_t)(sizeof(NvsramData_t)));
	    if(lStatus != NO_ERROR)
	    {
		debugERROR_PRINT("failed to erase..!! \n");
		return lStatus;
	    }
	    /* Taking backup of partition1 to partition2 */
	    memcpy(((uint8_t *)nvsramPART2_ADDRESS ) + nvsramSNVSRAM_MAGIC_SIZE,
		((const uint8_t *)nvsramPART1_ADDRESS) + nvsramSNVSRAM_MAGIC_SIZE,
			nvsramSNVSRAM_PART_SIZE - nvsramSNVSRAM_MAGIC_SIZE);
	    /* write magic to backup partition */
	    lStatus = prvNvsramUpdateHeader(nvsramPART2_ADDRESS);
	    if(lStatus != NO_ERROR)
	    {
		debugERROR_PRINT("failed to validate header \r\n");
		return lStatus;
	    }
	    /* Before Writing Invalidate Header */
	    lStatus = prvNvsramInvalidateHeader(ulPartition);
	    if(lStatus != NO_ERROR)
	    {
		debugERROR_PRINT("failed to Invalidate header \r\n");
		return lStatus;
	    }
	}
	/* check if partition two has valid header */
	else if (ulPartition == nvsramPART2_ADDRESS)
	{
	    /* Erase partition1 before taking backup */
	    lStatus = prvNvsramErase((uint32_t)(nvsramPART1_ADDRESS),
		    (uint32_t)(sizeof(NvsramData_t)));
	    if(lStatus != NO_ERROR)
	    {
		debugERROR_PRINT("failed to erase..!! \n");
		return lStatus;
	    }
	    /* Taking backup of partition2 to partition1 */
	    memcpy(((uint8_t *)nvsramPART1_ADDRESS ) + nvsramSNVSRAM_MAGIC_SIZE,
		((const uint8_t *)nvsramPART2_ADDRESS)+ nvsramSNVSRAM_MAGIC_SIZE,
			nvsramSNVSRAM_PART_SIZE - nvsramSNVSRAM_MAGIC_SIZE);
	    /* write magic to backup partition */
	    lStatus = prvNvsramUpdateHeader(nvsramPART1_ADDRESS);
	    if(lStatus != NO_ERROR)
	    {
		debugERROR_PRINT("failed to validate header \r\n");
		return lStatus;
	    }
	    /* Before Writing Invalidate Header */
	    lStatus = prvNvsramInvalidateHeader(ulPartition);
	    if(lStatus != NO_ERROR)
	    {
		debugERROR_PRINT("failed to Invalidate header \r\n");
		return lStatus;
	    }
	}
	/* Some error has occurred. */
	else
	{
		debugERROR_PRINT("No valid header partition found \r\n");
		return eORWL_ERROR_NVSRAM_INVALID_HEADER;
	}

	/* Write data to nvsram */
	lStatus = prvNvsramWrite (ulPartition, sizeof(NvsramData_t), pxData);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("failed to write nvsram data \r\n");
	    return lStatus;
	}
	/*  Validate header after writing data */
	lStatus = prvNvsramUpdateHeader(ulPartition);
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("failed to validate header \r\n");
	    return lStatus;
	}
	return NO_ERROR;
}
/*----------------------------------------------------------------------------*/

static uint32_t prvNvsramChoosePartition(uint32_t ulPar1Address,
						uint32_t ulPar2Address )
{
	/* status to return */
	uint32_t ulStatus = NO_ERROR;
	/* Allocate memory for NVSRAM data structure */
	NvsramData_t *pxData = (NvsramData_t *)pvPortMalloc(sizeof(NvsramData_t));
	if( pxData == NULL )
	{
	    debugERROR_PRINT(
		"Failed to allocate memory for NVSRAM data structure \r\n");
	    return COMMON_ERR_NULL_PTR;
	}

	/* check if partition one contains valid magic header */
	ulStatus = prvNvsramReadPartition( ulPar1Address, sizeof(NvsramData_t),
						pxData);
	if(ulStatus != NO_ERROR)
	{
	    debugERROR_PRINT("Error in reading magic header of partition 1..!!\r\n");
	    goto CLEANUP;
	}
	/* check if partition 1 contains valid header */
	if( pxData->ulMagic == nvsramSNVSRAM_MAGIC )
	{
	    /* partition one contains valid magic header */
	    return nvsramPART1_ADDRESS;
	}

	/* check if partition two contains valid magic header */
	ulStatus = prvNvsramReadPartition(ulPar2Address, sizeof(NvsramData_t),
						pxData);
	if(ulStatus != NO_ERROR)
	{
	    debugERROR_PRINT("Error in reading magic header of partition 2..!!\r\n");
	    goto CLEANUP;
	}
	/* check if partition 2 contains valid data */
	if( pxData->ulMagic == nvsramSNVSRAM_MAGIC )
	{
	    /* partition two contains valid magic header */
	    return nvsramPART2_ADDRESS;
	}
	else
	{
	    /* Both partition contains Invalid magic */
	    ulStatus = nvsramPART1_ADDRESS;
	}
CLEANUP:
	/* Release the memory before returning */
	if( pxData != NULL )
	{
	    vPortFree(pxData);
	}
	return ulStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lNvsramReadSSDEncKey ( uint32_t ulEncKeyLen, uint32_t *pulSSDEncKey )
{
	int32_t lResult = 0;
	uint32_t ulPartition = 0;
	/* variable to store address */
	uint32_t ulAddr = 0;
	if( pulSSDEncKey == NULL )
	{
	    return COMMON_ERR_NULL_PTR;
	}
	/* Allocate memory for NVSRAM data structure */
	NvsramData_t *pxData = (NvsramData_t *)pvPortMalloc(sizeof(NvsramData_t));
	if( pxData == NULL )
	{
	    debugERROR_PRINT(
		"Failed to allocate memory for NVSRAM data structure \r\n");
	    return COMMON_ERR_NULL_PTR;
	}
	/* We need to check if data needs to be written to partition 1 or 2
	 * based on the valid magic number. If both partition does have valid
	 * magic number than choose partition 1.
	 */
	ulPartition = prvNvsramChoosePartition(nvsramPART1_ADDRESS,
		nvsramPART2_ADDRESS );
	if(ulPartition == nvsramPART1_ADDRESS || ulPartition == nvsramPART2_ADDRESS)
	{
	    ulAddr = ulPartition;
	}
	/* Some error has occurred. */
	else
	{
	    debugERROR_PRINT("No valid header partition found \r\n");
	    /* Release the memory before returning */
	    if(pxData)
	    {
		vPortFree(pxData);
	    }
	    return eORWL_ERROR_NVSRAM_INVALID_HEADER;
	}

	/* Read of Encryption key */
	lResult = prvNvsramReadPartition( ulAddr, sizeof(NvsramData_t), pxData);
	if(lResult != NO_ERROR)
	{
	    debugERROR_PRINT("Error in reading encryption key..\n");
	    /* Release the memory before returning */
	    if(pxData)
	    {
		vPortFree(pxData);
	    }
	    return lResult;
	}
	/* copy 16 byte encrypted ssd password */
	memcpy((void *)pulSSDEncKey,(const void *) pxData->ulSSDEncKey,
		sizeof(uint32_t) * ulEncKeyLen);
	/* Release the memory before returning */
	if(pxData)
	{
	    vPortFree(pxData);
	}
	return NO_ERROR;
}
/*----------------------------------------------------------------------------*/

int32_t lNvsramReadHash ( uint8_t *pucHashBuf )
{
	int32_t lResult = 0;
	uint32_t ulPartition = 0;
	/* variable to store address */
	uint32_t ulAddr = 0;
	if( pucHashBuf == NULL )
	{
	    return COMMON_ERR_NULL_PTR;
	}
	/* Allocate memory for NVSRAM data structure */
	NvsramData_t *pxData = (NvsramData_t *)pvPortMalloc(sizeof(NvsramData_t));
	if( pxData == NULL )
	{
	    debugERROR_PRINT(
		"Failed to allocate memory for NVSRAM data structure \r\n");
	    return COMMON_ERR_NULL_PTR;
	}
	/* We need to check if data needs to be written to partition 1 or 2
	 * based on the valid magic number. If both partition does have valid
	 * magic number than choose partition 1.
	 */
	ulPartition = prvNvsramChoosePartition(nvsramPART1_ADDRESS,
		nvsramPART2_ADDRESS );
	if(ulPartition == nvsramPART1_ADDRESS || ulPartition == nvsramPART2_ADDRESS)
	{
	    ulAddr = ulPartition;
	}
	/* Some error has occurred. */
	else
	{
	    debugERROR_PRINT("No valid header partition found \r\n");
	    /* Release the memory before returning */
	    if(pxData)
	    {
		vPortFree(pxData);
	    }
	    return eORWL_ERROR_NVSRAM_INVALID_HEADER;
	}

	/* Read of unique hash number */
	lResult = prvNvsramReadPartition( ulAddr, sizeof(NvsramData_t), pxData);
	if(lResult != NO_ERROR)
	{
	    debugERROR_PRINT("Error in reading unique hash number..\n");
	    /* Release the memory before returning */
	    if(pxData)
	    {
		vPortFree(pxData);
	    }
	    return lResult;
	}
	/* Update pointer */
	memcpy((void *)pucHashBuf,(const void *) pxData->ucHASH,
		nvsramSNVSRAM_HASH_LEN_MAX);
	    /* Release the memory before returning */
	if(pxData)
	{
	    vPortFree(pxData);
	}
	return NO_ERROR;
}
/*----------------------------------------------------------------------------*/

int32_t lNvsramReadPswdRetryCountDelay(uint16_t *pusPswdRetryCount,
	uint32_t *pulRTCDelaySec, uint32_t *pulRTCSnapShot)
{
	int32_t lResult = 0;
	uint32_t ulPartition = 0;
	/* variable to store address */
	uint32_t ulAddr = 0;
	NvsramData_t *pxData;
	/* validate the input params */
	if ((pusPswdRetryCount == NULL) || (pulRTCDelaySec == NULL)
	    || (pulRTCSnapShot == NULL))
	{
	    return COMMON_ERR_NULL_PTR;
	}
	/* Allocate memory for NVSRAM data structure */
	pxData = (NvsramData_t *)pvPortMalloc(sizeof(NvsramData_t));
	if( pxData == NULL )
	{
	    debugERROR_PRINT(
		"Failed to allocate memory for NVSRAM data structure \r\n");
	    return COMMON_ERR_NULL_PTR;
	}
	/* We need to check if data needs to be written to partition 1 or 2
	 * based on the valid magic number. If both partition does have valid
	 * magic number than choose partition 1.
	 */
	ulPartition = prvNvsramChoosePartition(nvsramPART1_ADDRESS,
		nvsramPART2_ADDRESS );
	if(ulPartition == nvsramPART1_ADDRESS || ulPartition == nvsramPART2_ADDRESS)
	{
	    ulAddr = ulPartition;
	}
	/* Some error has occurred. */
	else
	{
	    debugERROR_PRINT("No valid header partition found \r\n");
	    /* Release the memory before returning */
	    if(pxData)
	    {
		vPortFree(pxData);
	    }
	    return eORWL_ERROR_NVSRAM_INVALID_HEADER;
	}

	/* Read of unique hash number */
	lResult = prvNvsramReadPartition( ulAddr, sizeof(NvsramData_t), pxData);
	if(lResult != NO_ERROR)
	{
	    debugERROR_PRINT("Error in reading unique hash number..\n");
	    /* Release the memory before returning */
	    if(pxData)
	    {
		vPortFree(pxData);
	    }
	    return lResult;
	}

	/* Update Retry count and RTC delay */
	*pusPswdRetryCount = pxData->usPswdRetryCount;
	*pulRTCDelaySec = pxData->ulRTCDelaySec;
	*pulRTCSnapShot = pxData->ulRTCSnapShot;

	/* Release the memory before returning */
	if(pxData)
	{
	    vPortFree(pxData);
	}
	return NO_ERROR;
}
/*----------------------------------------------------------------------------*/

int32_t lNvsramWritePswdRetryCountDelay(uint16_t *pusPswdRetryCount,
	uint32_t *pulRTCDelaySec, uint32_t *pulRTCSnapShot)
{
	NvsramData_t *pxData;
	int32_t lResult = 0;
	uint32_t ulPartition = 0;
	/* variable to store address */
	uint32_t ulAddr = 0;

	pxData = (NvsramData_t *)pvPortMalloc(sizeof(NvsramData_t));
	if( pxData == NULL )
	{
	    debugERROR_PRINT(
		"Failed to allocate memory for NVSRAM data structure \r\n");
	    return COMMON_ERR_NULL_PTR;
	}
	/* We need to check if data needs to be written to partition 1 or 2
	 * based on the valid magic number. If both partition does have valid
	 * magic number than choose partition 1.
	 */
	ulPartition = prvNvsramChoosePartition(nvsramPART1_ADDRESS,
		nvsramPART2_ADDRESS );
	if(ulPartition == nvsramPART1_ADDRESS || ulPartition == nvsramPART2_ADDRESS)
	{
	    ulAddr = ulPartition;
	}
	/* Some error has occurred. */
	else
	{
	    debugERROR_PRINT("No valid header partition found \r\n");
	    if(pxData)
	    {
		vPortFree(pxData);
	    }
	    return eORWL_ERROR_NVSRAM_INVALID_HEADER;
	}

	lResult = prvNvsramReadPartition( ulAddr, sizeof(NvsramData_t), pxData);
	if(lResult != NO_ERROR)
	{
	    debugERROR_PRINT("Error in reading NVSRAM\n");
	    goto cleanup;
	}

	/* Update the new retry coutn and RTC delays */
	pxData->usPswdRetryCount = *pusPswdRetryCount;
	pxData->ulRTCDelaySec = *pulRTCDelaySec;
	pxData->ulRTCSnapShot = *pulRTCSnapShot;

	/* Write the updated retry count to NVSRAM */
	lResult = lNvsramWriteData(pxData);
	if(lResult != NO_ERROR)
	{
	    debugERROR_PRINT("Failed to update the Password retry count\n");
	}
cleanup:
	if(pxData)
	{
	    vPortFree(pxData);
	}

	return lResult;

}
/*----------------------------------------------------------------------------*/

int32_t lNvsramResetPswdRetryCountDelay ( void )
{
	NvsramData_t *pxData;
	int32_t lResult = 0;
	uint32_t ulPartition = 0;
	/* variable to store address */
	uint32_t ulAddr = 0;

	pxData = (NvsramData_t *)pvPortMalloc(sizeof(NvsramData_t));
	if( pxData == NULL )
	{
	    debugERROR_PRINT(
		"Failed to allocate memory for NVSRAM data structure \r\n");
	    return COMMON_ERR_NULL_PTR;
	}
	/* We need to check if data needs to be written to partition 1 or 2
	 * based on the valid magic number. If both partition does have valid
	 * magic number than choose partition 1.
	 */
	ulPartition = prvNvsramChoosePartition(nvsramPART1_ADDRESS,
		nvsramPART2_ADDRESS );
	if(ulPartition == nvsramPART1_ADDRESS || ulPartition == nvsramPART2_ADDRESS)
	{
	    ulAddr = ulPartition;
	}
	/* Some error has occurred. */
	else
	{
	    debugERROR_PRINT("No valid header partition found \r\n");
	    if(pxData)
	    {
		vPortFree(pxData);
	    }
	    return eORWL_ERROR_NVSRAM_INVALID_HEADER;
	}

	lResult = prvNvsramReadPartition( ulAddr, sizeof(NvsramData_t), pxData);
	if(lResult != NO_ERROR)
	{
	    debugERROR_PRINT("Error in reading NVSRAM..\n");
	    goto cleanup;
	}
	/* reset the password retry count to 1 */
	pxData->usPswdRetryCount = 1;
	/* reset the RTC delay and RTC snapshot seconds to 0 */
	pxData->ulRTCDelaySec = 0;
	pxData->ulRTCSnapShot = 0;

	lResult = lNvsramWriteData(pxData);
	if(lResult != NO_ERROR)
	{
	    debugERROR_PRINT("Failed to reset password retry count\n");
	}
cleanup:
	/* Release the memory before returning */
	if(pxData)
	{
	    vPortFree(pxData);
	}

	return lResult;
}
/*----------------------------------------------------------------------------*/

int32_t lNvsramSetRTCSnapshotDirtyBit ( void )
{
    NvsramData_t *pxData;
    int32_t lResult = 0;
    uint32_t ulPartition = 0;
    /* variable to store address */
    uint32_t ulAddr = 0;

    pxData = (NvsramData_t *)pvPortMalloc(sizeof(NvsramData_t));
    if( pxData == NULL )
    {
	debugERROR_PRINT(
	    "Failed to allocate memory for NVSRAM data structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* We need to check if data needs to be written to partition 1 or 2
     * based on the valid magic number. If both partition does have valid
     * magic number than choose partition 1.
     */
    ulPartition = prvNvsramChoosePartition(nvsramPART1_ADDRESS,
	    nvsramPART2_ADDRESS );
    if(ulPartition == nvsramPART1_ADDRESS || ulPartition == nvsramPART2_ADDRESS)
    {
	ulAddr = ulPartition;
    }
    /* Some error has occurred. */
    else
    {
	debugERROR_PRINT("No valid header partition found \r\n");
	if(pxData)
	{
	    vPortFree(pxData);
	}
	return eORWL_ERROR_NVSRAM_INVALID_HEADER;
    }
    /* Read NVSRAM data */
    lResult = prvNvsramReadPartition( ulAddr, sizeof(NvsramData_t), pxData);
    if(lResult != NO_ERROR)
    {
	debugERROR_PRINT("Error in reading NVSRAM..\n");
	goto cleanup;
    }
    /* set the RTC snapshot dirty bit */
    pxData->ucRTCSnapshotDirtyBit = 1;
    /* write the updated data */
    lResult = lNvsramWriteData(pxData);
    if(lResult != NO_ERROR)
    {
	debugERROR_PRINT("Failed to reset password retry count\n");
    }
cleanup:
    /* Release the memory before returning */
    if(pxData)
    {
	vPortFree(pxData);
    }
    /* return error code */
    return lResult;
}
/*----------------------------------------------------------------------------*/

int32_t lNvsramResetRTCSnapshotDirtyBit ( void )
{
    NvsramData_t *pxData;
    int32_t lResult = 0;
    uint32_t ulPartition = 0;
    /* variable to store address */
    uint32_t ulAddr = 0;

    pxData = (NvsramData_t *)pvPortMalloc(sizeof(NvsramData_t));
    if( pxData == NULL )
    {
	debugERROR_PRINT(
	    "Failed to allocate memory for NVSRAM data structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* We need to check if data needs to be written to partition 1 or 2
     * based on the valid magic number. If both partition does have valid
     * magic number than choose partition 1.
     */
    ulPartition = prvNvsramChoosePartition(nvsramPART1_ADDRESS,
	    nvsramPART2_ADDRESS );
    if(ulPartition == nvsramPART1_ADDRESS || ulPartition == nvsramPART2_ADDRESS)
    {
	ulAddr = ulPartition;
    }
    /* Some error has occurred. */
    else
    {
	debugERROR_PRINT("No valid header partition found \r\n");
	if(pxData)
	{
	    vPortFree(pxData);
	}
	return eORWL_ERROR_NVSRAM_INVALID_HEADER;
    }
    /* Read NVSRAM data */
    lResult = prvNvsramReadPartition( ulAddr, sizeof(NvsramData_t), pxData);
    if(lResult != NO_ERROR)
    {
	debugERROR_PRINT("Error in reading NVSRAM..\n");
	goto cleanup;
    }
    /* reset the RTC snapshot dirty bit */
    pxData->ucRTCSnapshotDirtyBit = 0;
    /* write the updated data */
    lResult = lNvsramWriteData(pxData);
    if(lResult != NO_ERROR)
    {
	debugERROR_PRINT("Failed to reset password retry count\n");
    }
cleanup:
    /* Release the memory before returning */
    if(pxData)
    {
	vPortFree(pxData);
    }
    /* return error code */
    return lResult;
}
/*----------------------------------------------------------------------------*/

int32_t lNvsramReadRTCSnapshotDirtyBit ( uint8_t *pucRTCSnapshotDirtyBit )
{
    NvsramData_t *pxData;
    int32_t lResult = 0;
    uint32_t ulPartition = 0;
    /* variable to store address */
    uint32_t ulAddr = 0;

    pxData = (NvsramData_t *)pvPortMalloc(sizeof(NvsramData_t));
    if( pxData == NULL )
    {
	debugERROR_PRINT(
	    "Failed to allocate memory for NVSRAM data structure \r\n");
	return COMMON_ERR_NULL_PTR;
    }
    /* We need to check if data needs to be written to partition 1 or 2
     * based on the valid magic number. If both partition does have valid
     * magic number than choose partition 1.
     */
    ulPartition = prvNvsramChoosePartition(nvsramPART1_ADDRESS,
	    nvsramPART2_ADDRESS );
    if(ulPartition == nvsramPART1_ADDRESS || ulPartition == nvsramPART2_ADDRESS)
    {
	ulAddr = ulPartition;
    }
    /* Some error has occurred. */
    else
    {
	debugERROR_PRINT("No valid header partition found \r\n");
	if(pxData)
	{
	    vPortFree(pxData);
	}
	return eORWL_ERROR_NVSRAM_INVALID_HEADER;
    }
    /* Read NVSRAM data */
    lResult = prvNvsramReadPartition( ulAddr, sizeof(NvsramData_t), pxData);
    if(lResult != NO_ERROR)
    {
	debugERROR_PRINT("Error in reading NVSRAM..\n");
	goto cleanup;
    }
    /* read the RTC snapshot dirty bit */
    *pucRTCSnapshotDirtyBit = pxData->ucRTCSnapshotDirtyBit;
cleanup:
    /* Release the memory before returning */
    if(pxData)
    {
	vPortFree(pxData);
    }
    /* return error code */
    return lResult;
}
/*----------------------------------------------------------------------------*/

void vNvsramAESKeySet(void)
{
	volatile mml_trng_regs_t *pxRegTrng =
		(volatile mml_trng_regs_t*) nvsramTRNG_IOBASE;
	volatile uint32_t ulTempVal ;

	ulTempVal =  pxRegTrng->trngcr ;

	pxRegTrng->trngcr = ulTempVal|nvsramENABLE_AESKG;

	/* Check whether AES generation is complete and transferred */
	do
	{
		ulTempVal =  pxRegTrng->trngcr ;
	}while( ulTempVal&nvsramENABLE_AESKG) ;
}
/*----------------------------------------------------------------------------*/

int32_t lNvsramCheckHeader(void)
{
	/* status to return */
	int32_t lStatus = NO_ERROR;
	/* Allocate memory for NVSRAM data structure */
	NvsramData_t *pxData = (NvsramData_t *)pvPortMalloc(sizeof(NvsramData_t));
	if( pxData == NULL )
	{
	    debugERROR_PRINT(
		"Failed to allocate memory for NVSRAM data structure \r\n");
	    return COMMON_ERR_NULL_PTR;
	}
	/* check if partition one contains valid magic header */
	lStatus = prvNvsramReadPartition(nvsramPART1_ADDRESS,
					sizeof(NvsramData_t) , pxData );
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("Error in reading magic header of partition 1..!!\r\n");
	    lStatus = eORWL_ERROR_NVSRAM_NOT_ACCESSIBLE;
	    goto CLEANUP;
	}
	/* check if partition 1 contains valid header */
	if( pxData->ulMagic == nvsramSNVSRAM_MAGIC)
	{
	    /* partition one contains valid magic header */
	    lStatus = NO_ERROR;
	    goto CLEANUP;
	}

	/* check if partition two contains valid magic header */
	lStatus = prvNvsramReadPartition(nvsramPART2_ADDRESS,
					sizeof(NvsramData_t) , pxData );
	if(lStatus != NO_ERROR)
	{
	    debugERROR_PRINT("Error in reading magic header of partition 2..!!\r\n");
	    lStatus = eORWL_ERROR_NVSRAM_NOT_ACCESSIBLE;
	    goto CLEANUP;
	}
	/* check if partition 2 contains valid data */
	if( pxData->ulMagic == nvsramSNVSRAM_MAGIC)
	{
	    /* partition two contains valid magic header */
	    lStatus = NO_ERROR;
	    goto CLEANUP;
	}
	else
	{
	    /* Both partition contains Invalid magic */
	    lStatus = eORWL_ERROR_NVSRAM_INVALID_HEADER;
	}
CLEANUP:
	/* Release the memory before returning */
	if( pxData != NULL )
	{
	    vPortFree(pxData);
	}
	return lStatus;
}
/*----------------------------------------------------------------------------*/

int32_t lNvsSSDErasePassword( void )
{
	uint32_t ulPartition = 0;
	int32_t lResult = 0;
	uint32_t ulAddr = 0;
	NvsramData_t *pxData;

	pxData = (NvsramData_t *)pvPortMalloc(sizeof(NvsramData_t));
	if( pxData == NULL )
	{
	    debugERROR_PRINT(
		"Failed to allocate memory for NVSRAM data structure \r\n");
	    return COMMON_ERR_NULL_PTR;
	}
	/* We need to check if data needs to be written to partition 1 or 2
	 * based on the valid magic number. If both partition does have valid
	 * magic number than choose partition 1.
	 */
	ulPartition = prvNvsramChoosePartition(nvsramPART1_ADDRESS,
		nvsramPART2_ADDRESS );
	if(ulPartition == nvsramPART1_ADDRESS ||
		ulPartition == nvsramPART2_ADDRESS)
	{
	    ulAddr = ulPartition;
	}
	/* Some error has occurred. */
	else
	{
	    debugERROR_PRINT("No valid header partition found \r\n");
	    if(pxData)
	    {
		vPortFree(pxData);
	    }
	    return eORWL_ERROR_NVSRAM_INVALID_HEADER;
	}

	lResult = prvNvsramReadPartition( ulAddr, sizeof(NvsramData_t), pxData);
	if(lResult != NO_ERROR)
	{
	    debugERROR_PRINT("Error in reading NVSRAM\n");
	    goto cleanup;
	}

	/* Erase SSD encryption key */
	memset( pxData->ulSSDEncKey, nvsramErase_DATA,
		sizeof(pxData->ulSSDEncKey));
	/* Write the updated data to NVSRAM */
	lResult = lNvsramWriteData(pxData);
	if(lResult != NO_ERROR)
	{
	    debugERROR_PRINT("Failed to update the Password retry count\n");
	}
cleanup:
	if(pxData)
	{
	    vPortFree(pxData);
	}
	return lResult;
}
/*----------------------------------------------------------------------------*/

int32_t lEraseNvsramComplete( void )
{
	return prvNvsramErase(nvsramSNVSRAM_BASE, nvsramSNVSRAM_SIZE);
}
/*----------------------------------------------------------------------------*/
