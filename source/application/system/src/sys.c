
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

/* standard headers */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
//#include <projdefs.h>
//#include <FreeRTOSConfig.h>
#include <portable.h>

/* local includes */
#include <sys.h>

/*****************************************************************************/
void *pvSysCalloc( uint32_t ulNumElem, uint32_t ulElemSize ) {
	uint32_t ulAllocSize = (ulNumElem * ulElemSize);
	void *vpPtr;

	vpPtr = pvPortMalloc( ulAllocSize );
	if( vpPtr == NULL ) {
		sysCRYPTO_DBG( "\n ptr is NULL: pvPortMalloc Failed \n" );
	} else {
		memset( vpPtr , 0x00 , ulAllocSize );
	}
	return vpPtr;
}

/*****************************************************************************/
void vSysFree( void *pv ) {
	if( pv != NULL )
		vPortFree( pv );
}

/*****************************************************************************/
