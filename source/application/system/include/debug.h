/**===========================================================================
 * @file debug.h
 *
 * @brief This file contains ORWL debug api/macros
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
 ============================================================================
 *
 */

#ifndef INCLUDE_DEBUG_H_
#define INCLUDE_DEBUG_H_
/*---------------------------------------------------------------------------*/
/** Application includes */
#include <printf_lite.h>

/**
 *  Macros internal to the file. Do not modify !!!!
 * These Macros shouldn't be used directly.
 */

/** debug prefix and informations */
#define debugORWL_DBG_PREFIX		(lite_printf("[ORWL_DBG]:"))
#define debugORWL_DBG_MSG( fmt )	(lite_printf( fmt ))
#define debugORWL_DBG_INFO		(lite_printf("%s():%d:",__func__,__LINE__))

/** error prefix and informations */
#define debugORWL_ERR_PREFIX		(lite_printf("[ORWL_ERR]:"))
#define debugORWL_DBG_MSG( fmt )	(lite_printf( fmt ))
#define debugORWL_DBG_INFO		(lite_printf("%s():%d:",__func__,__LINE__))

/** error prefix and informations, for ISR */
#define debugISR_ERR_PREFIX		(lite_printf_isr("[ISR_ERR]:"))
#define debugISR_DBG_MSG( fmt )		(lite_printf_isr( fmt ))
#define debugISR_DBG_INFO		(lite_printf_isr("%s():%d:",__func__,__LINE__))

/** warning prefix and information */
#define debugORWL_WARN_PREFIX		(lite_printf("[ORWL_WARNING]:"))
#define debugORWL_DBG_MSG( fmt )	(lite_printf( fmt ))
#define debugORWL_DBG_INFO		(lite_printf("%s():%d:",__func__,__LINE__))

/** print debug logs */
#define debugPRINT(...) do {\
	debugORWL_DBG_PREFIX;\
	debugORWL_DBG_INFO;\
	lite_printf(__VA_ARGS__);\
	lite_printf( "\n" );\
} while(0)

/** print error logs */
#define debugERROR_PRINT(...) do {\
	debugORWL_ERR_PREFIX;\
	debugORWL_DBG_INFO;\
	lite_printf(__VA_ARGS__);\
	lite_printf( "!!!!\n" );\
} while(0)

/** print warning logs */
#define debugWARNING_PRINT(...) do {\
	debugORWL_WARN_PREFIX;\
	debugORWL_DBG_INFO;\
	lite_printf(__VA_ARGS__);\
	lite_printf( "!!!!\n" );\
} while(0)

/** Print plain, without function name and log type */
#define debugPLAIN_PRINT(...) do {\
        lite_printf(__VA_ARGS__); \
} while(0)

/** Print plain, without function name and log type. Should be called from ISR */
#define debugPRINT_ISR(...) do {\
        lite_printf_isr(__VA_ARGS__); \
} while(0)

/** print error logs, this should be called from ISR context. */
#define debugERROR_PRINT_ISR(...) do {\
	debugISR_ERR_PREFIX;\
	debugISR_DBG_INFO;\
	lite_printf_isr(__VA_ARGS__);\
	lite_printf_isr( "!!!!\n" );\
} while(0)

/*---------------------------------------------------------------------------*/

/**
 * Macros to be used by module for debug
 * Debug enable/disable for each module can be selected independently from
 * configuration file.
 */
/** OLED debug */
#ifdef _DEBUG_MODULE_OLED_
    #define debugPRINT_OLED	debugPRINT
#else
    #define debugPRINT_OLED
#endif

/** NFC debug */
#ifdef _DEBUG_MODULE_NFC_
    #define debugPRINT_NFC	debugPRINT
#else
    #define debugPRINT_NFC
#endif

/** BLE debug */
#ifdef _DEBUG_MODULE_BLE_
    #define debugPRINT_BLE	debugPRINT
#else
    #define debugPRINT_BLE
#endif

/** MEMS debug */
#ifdef _DEBUG_MODULE_MEMS_
    #define debugPRINT_MEMS	debugPRINT
#else
    #define debugPRINT_MEMS	debugPRINT
#endif

/** STORAGE debug */
#ifdef _DEBUG_MODULE_STORAGE_
    #define debugPRINT_STORAGE	debugPRINT
#else
    #define debugPRINT_STORAGE
#endif

/** TRNG debug */
#ifdef _DEBUG_MODULE_TRNG_
    #define debugPRINT_TRNG	debugPRINT
#else
    #define debugPRINT_TRNG
#endif

/** Application debug  */
#ifdef _DEBUG_MODULE_SUC_APP_
    #define debugPRINT_APP	debugPRINT
#else
    #define debugPRINT_APP
#endif

/** SYSTEM debug */
#ifdef _DEBUG_SYS_
    #define debugPRINT_SYS	debugPRINT
#else
    #define debugPRINT_SYS
#endif

/** Intel-Suc Comm debug */
#ifdef _DEBUG_INTEL_SUC_COMM_
    #define debugPRINT_SUC_INTEL_COMM	debugPRINT
#else
    #define debugPRINT_SUC_INTEL_COMM
#endif
/*---------------------------------------------------------------------------*/

/**
 * Macros to print error message.
 * Always enabled irrespective of debug configurations
 */
#define debugPRINT_ERROR	debugERROR_PRINT

/*---------------------------------------------------------------------------*/
#endif /* INCLUDE_DEBUG_H_ */
