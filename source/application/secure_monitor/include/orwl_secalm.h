 /**===========================================================================
 * @file orwl_secalm.h
 *
 * @brief This file contains tamper detection flags and seclam register
 * bit definition for tamper detection.
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

#ifndef secalmINCLUDE_SEC_ALAM_H_
#define secalmINCLUDE_SEC_ALAM_H_

/** secalm tamper detection flag */
#define secalmTAMPER_DETECTION_MAGIC	( 0x003F1FFC )
#define secalmWARN_MAGIC		( 0x3F000000 )

/** secalm register bit definition */
#define secalmDIESHIELD_FAULT		( 0x00000004 ) /**<check for dieshield - BIT2*/
#define secalmLOW_TEMP			( 0x00000008 ) /**<check for low temp - BIT3*/
#define secalmHIGH_TEMP			( 0x00000010 ) /**<check for high temp - BIT4*/
#define secalmBAT_LOW			( 0x00000020 ) /**<check for low voltage - BIT5*/
#define secalmBAT_OVERVOLT		( 0x00000040 ) /**<check for high voltage - BIT6*/
#define secalmBAT_EXT_FAULT		( 0x00000080 ) /**<check for ext tamper - BIT7*/
#define secalmVDDLO			( 0x00000100 ) /**<check for VDD under vtg - BIT8*/
#define secalmVCORELO			( 0x00000200 ) /**<check for VCORE under vtg - BIT9*/
#define secalmVCOREHI			( 0x00000400 ) /**<check for VCORE over vtg - BIT10*/
#define secalmVDDHI			( 0x00000800 ) /**<check for VDD over vtg - BIT11*/
#define secalmVGL			( 0x00001000 ) /**<check for vtg glitch - BIT12*/
#define secalmEXT_SENS0			( 0x00010000 ) /**<check for SEN0 tamper- BIT16*/
#define secalmEXT_SENS1			( 0x00020000 ) /**<check for SEN1 tamper - BIT17*/
#define secalmEXT_SENS2			( 0x00040000 ) /**<check for SEN2 tamper - BIT18*/
#define secalmEXT_SENS3			( 0x00080000 ) /**<check for SEN3 tamper - BIT19*/
#define secalmEXT_SENS4			( 0x00100000 ) /**<check for SEN4 tamper - BIT20*/
#define secalmEXT_SENS5			( 0x00200000 ) /**<check for SEN5 tamper - BIT21*/

/** Check for warnings */
#define secalmRFU22			( 0x00400000 ) /**<check for reserved bit*/
#define secalmRFU23			( 0x00800000 ) /**<check for reserved bit*/
#define secalmEXTSWARN_SENS0		( 0x01000000 ) /**<check for SEN0 tamper*/
#define secalmEXTSWARN_SENS1		( 0x02000000 ) /**<check for SEN1 tamper*/
#define secalmEXTSWARN_SENS2		( 0x04000000 ) /**<check for SEN2 tamper*/
#define secalmEXTSWARN_SENS3		( 0x08000000 ) /**<check for SEN3 tamper*/
#define secalmEXTSWARN_SENS4		( 0x10000000 ) /**<check for SEN4 tamper*/
#define secalmEXTSWARN_SENS5		( 0x20000000 ) /**<check for SEN5 tamper*/
#define secalmRFU30			( 0x40000000 ) /**<check for reserved bit*/
#define secalmRFU31			( 0x08000000 ) /**<check for reserved bit*/

#endif /* secalmINCLUDE_SEC_ALAM_H_ */
