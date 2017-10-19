 /**===========================================================================
 * @file hist_devtamper.h
 *
 * @brief This file contains all the data structures and macros for tracking all
 * ORWL tamper and clear events
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

#ifndef devtamperINCLUDE_TAMPER_CLEAR_H_
#define devtamperINCLUDE_TAMPER_CLEAR_H_

/*Global includes */
#include <stdint.h>
#include <rtc.h>
/** ORWL Tamper history management macros */
/**
 * length of time field
 */
#define	devtamperTIME_LEN		(4)

/**
 * length of date field
 */
#define	devtamperDATE_LEN		(4)

/**
 * tamper event reserved field
 */
#define	devtamperTAMP_RESERVED		(3)

/**
 * clear event reserved field
 */
#define	devtamperCLR_RESERVED		(7)

/**
 * max tamper/clr history that can be stored on flash
 */
#define devtamperEVENT_HIST_MAX_COUNT	(1)

/**
 * max tamper/clr history that can be stored on flash
 */
#define devtamperEVENT_TAMPER		(1)

/**
 * max tamper/clr history that can be stored on flash
 */
#define devtamperEVENT_CLEAR		(0)

/**
 * @brief This structure defines tamper events identification elements
 */
typedef struct
{
    /** differentiate clear or tamper */
    uint32_t ulEventTyype;
    /** occurrence ID of tamper event*/
    uint8_t  ucInstaceId;
    /** Reserved */
    uint8_t  ucReserved[devtamperTAMP_RESERVED];
    /** Occurrence Time and date of tamper event */
    rtcDateTime_t xTamperDateTime;
    /* cause of tamper */
    uint32_t ultamperCause;
    /* DRS RTC log register */
    uint32_t ulDRSLogRTC;
} devtamperTamperEvent_t;

/**
 * This structure defines tamper clear events.
 */
typedef struct
{
    /** differentiate clear or tamper */
    uint32_t ulEventTyype;
    /** occurrence ID of clear event*/
    uint8_t  ucInstaceId;
    /** Reserved */
    uint8_t  ucReserved[devtamperCLR_RESERVED];
    /** Occurrence Time of clear event */
    uint8_t  ucTime[devtamperTIME_LEN];
    /** Occurrence Day of clear event */
    uint8_t  ucDate[devtamperDATE_LEN];
} devtamperClearEvent_t;

/**
 * @brief This structure defines * @brief This structure defines tamper clear event
 * identification elements
 */
typedef struct
{
    /** Tamper Magic */
    uint32_t	  ulKeyTamperMagic;
    /** tamper event history */
    devtamperTamperEvent_t xTamperEvent[devtamperEVENT_HIST_MAX_COUNT];
    /** clear event history */
    devtamperClearEvent_t  xClearEvent[devtamperEVENT_HIST_MAX_COUNT];
    /** Reserved */
    uint32_t	  Reserved;
} devtamperTamperHist_t;

/* Function declaration */

/** @brief Writes device tamper history structure.
 *
 * This function writes device tamper history structure to flash.
 *
 * @pxDevTamHist pointer to devtamperTamperHist_t structure.
 * @return error code.
 *
 */
int32_t lDevtamperWriteTampHistory(devtamperTamperHist_t *pxDevTamHist);

/** @brief Writes device tamper history structure, NON RTOS.
 *
 * This function writes device tamper history structure to flash. This function
 * should be called from NON RTOS context.
 *
 * @pxDevTamHist pointer to devtamperTamperHist_t structure.
 * @return error code.
 *
 */
int32_t lDevtamperWriteTampHistoryNONRTOS(devtamperTamperHist_t *pxDevTamHist);

/** @brief Reads device tamper history structure.
 *
 * This function reads device tamper history structure from flash.
 *
 * @pxDevTamHist pointer to devtamperTamperHist_t structure.
 * @return error code.
 *
 */
int32_t lDevtamperReadTampHistory(devtamperTamperHist_t *pxDevTamHist);

#if 0
/** TO BE DONE */
int32_t ulAddTamperEventEntry( devtamperTamperHist_t *pData );
int32_t ulAddClearEventEntry( devtamperClearEvent_t *pData );
int32_t ulGetLastTamperEvent( devtamperTamperHist_t *pData );
int32_t ulGetLastClearEvent( devtamperClearEvent_t *pData );
int32_t ulGetTamperEventList( devtamperTamperHist_t *pData[] );
int32_t ulGetClearEventList( devtamperClearEvent_t *pData[] );
#endif
#endif /* devtamperINCLUDE_TAMPER_CLEAR_H_ */
