 /**===========================================================================
 * @file orwl_secmon.h
 *
 * @brief This file defines SuC MAX32550 security monitor setup and operations
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
#ifndef secmonSECURITYMON_H_
#define secmonSECURITYMON_H_

/* Global includes */
#include <cobra_defines.h>
#include <errors.h>
#include <orwl_sensoren.h>

/** For reseting the tamper */
#define secmonTAMPER_RESET				( 0 )
/* To decide EXTERNAL sensor magic refer data sheet
 * Present configuration as below:
 * 1. EXTS_EN 	= 0x3F; 	- all sensor ON
 * 2. EXTCNT  	= 0x0;	 	- no mismatch accepted
 * 3. EXTFRQ  	= 0x0;	 	- 2KHz
 * 4. DIVCLK  	= 0x0;		- 8KHz
 * 5. Lock 	= 0x1;		- locked
 */

/** sets external sensor bits along with lock bit */
#define secmonEXT_SENSOR_LOCK_EN_MAGIC			( 0x8000003F )

/** sets external sensor bit without lock so that we can reset and check */
#define secmonEXT_SENSOR_UNLOCK_EN_MAGIC		( 0x0000003F )

/* To decide INTERNAL sensor magic refer data sheet
 * Present configuration as below:
 * 1. SHIELD_EN		= 1; 	- die shield EN
 * 2. TEMP_EN  		= 1;	- temp. sensor. EN
 * 3. VBAT_EN  		= 1;	- BAT mon. EN
 * 4. LOTEMP_SEL  	= 0;	- low. temp. detect -50'C
 * 5. With all votage level check 5bits 18 to 22
 * 6. LOCK		= 1;    - locked
 */

/* sets internal sensor bits along with lock bit. (( 0x80000007 )|(0x1F<<18)).
 * Disabling VDD under voltage detect((( 0x80000007 )|(0x1B<<18)) enable bit
 * because of some hardware issue, please restore the same once its fixed.
 */
#define secmonINT_SENSOR_LOCK_EN_MAGIC			(( 0x80000007 )|(0x1B<<18))

/** set internal sensor without lock bit so that we can reset and check */
#define secmonINT_SENSOR_UNLOCK_EN_MAGIC		(( 0x00000007 )|(0x1B<<18))


/* Function Declaration */

/** @brief Interrupt service routine
*
* This is Non Maskable Interrupt Service Routine - DSR Security violation found
*
* @return void
*/
__COBRA_OPT_DEBUG__ void vSecmonDefaultTamperDetectNmi( void );

/** @brief configure secure controller on boot
*
* This function prints reason of tamper.
*
* @return void
*/
void vSecmonTamperInfo( void );

/**@brief for checking tamper
*
* This function is used for tamper check by reading Secalm register.
*
* @return NO_ERROR on success and error code on failure
*/
int32_t lSecmonTamperCheck( void );

#endif /* secmonSECURITYMON_H_ */

