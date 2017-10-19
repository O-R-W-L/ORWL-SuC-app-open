/**===========================================================================
 * @file nfc_interface.h
 *
 * @brief This file contains the status check and debug configuration macros
 *
 * @author ravikiran@design-shift.com
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

#ifndef NFC_INTERFACE_H
#define NFC_INTERFACE_H

/* Status header */
#include <ph_Status.h>

/* Platform Header */
#include <phPlatform.h>

/* Discovery Loop Header */
#include <phacDiscLoop.h>

/* nfc lib header */
#include <phNfcLib.h>

/* log header */
#include <debug.h>

#include <keyfobid.h>

#ifdef NXPBUILD__PH_OSAL_FREERTOS
#ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
#define SIMPLIFIED_ISO_STACK    (2000/4)
#else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
#define NFC_APP_STACK    2048	/**< Stack allocated for application stack in freeRTOS mode */
#endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION*/
#define NFC_APP_PRIO     2	/**< Task priority for NFC stack in freeRTOS mode*/
#endif /* NXPBUILD__PH_OSAL_FREERTOS */

/* check status and print error message w.r.t component */
#define CHECK_STATUS(x)                                      \
    if ((x) != PH_ERR_SUCCESS)                               \
{                                                            \
    debugERROR_PRINT("Error - (0x%04X) has occurred : 0xCCEE CC-Component ID, EE-Error code. Refer-ph_Status.h\n",(x));    \
}

/* prints if error is detected */
#define CHECK_NFCLIB_STATUS(x)                               \
    if ((x) != PH_NFCLIB_STATUS_SUCCESS)                     \
{                                                            \
    debugERROR_PRINT("\nError - (0x%04X) has occurred in NFCLIB\n ",(x)); \
}

#endif /* NFC_INTERFACE_H */
