 /**===========================================================================
 * @file orwl_secmon_reg.h
 *
 * @brief This file contains address of all security monitoring registers.
 *
 * @author priya.gokani@design-shift.com
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
#ifndef secmonADDR_H_
#define secmonADDR_H_

/** Base address for secmon register */
#define secmonBASE			( 0x40004000 )

/**  Security monitor register offsets */
#define secmonEXTSC_OFST		( 0x00000000 )
#define secmonINTSC_OFST		( 0x00000004 )
#define secmonSECALM_OFST		( 0x00000008 )
#define secmonSECDIAG_OFST		( 0x0000000C )
#define secmonDLRTC_OFST		( 0x00000010 )
#define secmonSECST_OFST		( 0x00000034 )

/** Security monitor register addresses */
#define secmonEXTSC			( secmonBASE + secmonEXTSC_OFST )
#define secmonINTSC			( secmonBASE + secmonINTSC_OFST )
#define secmonSECALM			( secmonBASE + secmonSECALM_OFST )
#define secmonSECDIAG			( secmonBASE + secmonSECDIAG_OFST )
#define secmonDLRTC			( secmonBASE + secmonDLRTC_OFST )
#define secmonSECST			( secmonBASE + secmonSECST_OFST )

#endif /* secmonADDR_H_ */
