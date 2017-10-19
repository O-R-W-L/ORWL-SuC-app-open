/**===========================================================================
 * @file sys.h
 *
 * @brief This file provide system interface for standard calloc, free and
 * printf functions
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

#ifndef INCLUDE_SYS_H_
#define INCLUDE_SYS_H_

#include <debug.h>

#define sysCRYPTO_DBG	debugERROR_PRINT	/**< defines system Crypto message interface as a replacement for standard printf */

/**
 * @brief Calloc allocation interface
 *
 * This function provides a workaround for calloc API in free RTOS using
 * portAlloc
 *
 * @param ulNumElem	number of elements required to be allocated
 * @param ulElemSize size of each element to be allocated
 *
 * @return void pointer on successful allocation
 *  		NULL on failure
 */
void *pvSysCalloc( uint32_t ulNumElem, uint32_t ulElemSize );

/**
 * @brief frees dynamically allocated memory
 *
 * @param *pvPtr pointer to the memory allocated using previos port
 * malloc/sysCalloc Api's
 *
 * @return void
 */
void vSysFree( void *pvPtr );

#endif /* INCLUDE_SYS_H_ */
