/**===========================================================================
 * @file orwl_sensoren.c
 *
 * @brief This file enables external and internal sensors.
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


/* Global includes */
#include <stdint.h>
#include <errors.h>

/* application includes */
#include <orwl_secmon.h>
#include <orwl_secalm.h>
#include <orwl_secmon_reg.h>
#include <debug.h>
#include <io.h>
/*---------------------------------------------------------------------------*/

void vSensorenSecConfig( void )
{
	#ifdef ENABLE_EXTERNAL_SENSOR
	/** Enable external sensors */
	vSensorenSetExtSen( secmonEXT_SENSOR_LOCK_EN_MAGIC );
	#endif /* ENABLE_EXTERNAL_SENSOR */

	#ifdef ENABLE_INTERNAL_SENSOR
        /** Enable internal sensors */
        vSensorenSetIntSen( secmonINT_SENSOR_LOCK_EN_MAGIC );
	#endif /* ENABLE_INTERNAL_SENSOR */

	#ifdef DEBUG_TAMPER
	/** Print tamper debug info */
        vSecmonTamperInfo();
	#endif /* DEBUG_TAMPER */
}
/*---------------------------------------------------------------------------*/

void vSensorenSetExtSen( uint32_t ulext_conf )
{
	/* Read the external sensor register */
	uint32_t ulReadEXTSC = 0;
	IO_READ_U32(secmonEXTSC, ulReadEXTSC);
	/* Before writing external sensor register, check for the busy bit. If its
	 * set to 1, than the secure monitor hardware is busy. Please wait until
	 * its cleared by hardware. Busy bit is 30.
	 */
	while(ulReadEXTSC & sensorenEXTSEN_BUSY);

	/** enable external  sensor */
	IO_WRITE_U32(secmonEXTSC, ulext_conf);
	debugPRINT("\n enabled external sensor \n");
}
/*---------------------------------------------------------------------------*/

void vSensorenSetIntSen( uint32_t ulint_conf )
{
	/** enable internal sensor */
	IO_WRITE_U32(secmonINTSC, ulint_conf);
	debugPRINT("\n enabled internal sensor \n");
}
/*---------------------------------------------------------------------------*/

void vSensorenResetTamper( void )
{
	/** clear SECALAM - tamper recovery */
	IO_WRITE_U32(secmonSECALM, secmonTAMPER_RESET);
	debugPRINT(" \n Recovering from tamper ..... \n");
	debugPRINT(" \n Please do a COMPLETE POWER OFF AND ON ONCE  \n");
}
/*---------------------------------------------------------------------------*/

void vSensorenDisableTamper( void )
{
	/** clear SECALAM - tamper recovery */
	IO_WRITE_U32(secmonINTSC, secmonTAMPER_RESET);
	IO_WRITE_U32(secmonEXTSC, secmonTAMPER_RESET);
	debugPRINT(" \n Disabling Tamper ..... \n");
}
/*---------------------------------------------------------------------------*/
