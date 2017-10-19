/**===========================================================================
 * @file orwl_sensoren.h
 *
 * @brief This file contains function to enable security monitor sensors
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

#ifndef secalmINCLUDE_SENSEOREN_H_
#define secalmINCLUDE_SENSEOREN_H_

/**
 * Busy bit 30 for external sensor register.
 */
#define sensorenEXTSEN_BUSY		(1 << 30)

/**
 * Security alarm register status.
 */
#define sensorenALARM_STATUS		(1 << 2)

/**@brief Enables the external sensor
*
* This function enables pairs of external sensors
*
* @param ulext_conf Bits for enabling external sensors, user can set
* value as per requirement.
*
* @return void
*/
void vSensorenSetExtSen( uint32_t ulext_conf );

/**@brief Enables the external sensor
*
* This function enables internal sensors -Die shield, temperature and voltage.
*
* @param ulint_conf Bits for enabling internal sensors, user can set
* value as per requirement.
*
* @return void
*/
void vSensorenSetIntSen( uint32_t ulint_conf );

/** @brief Interface API to configure security monitor
*
* This function enables sensors by calling set_sensor().
* Caller need to specify which all the sensors to be configured
* and prints info of tamper if user wants by calling vtamper_info()
*
* @return void
*/
void vSensorenSecConfig( void );

/**@brief For reseting tamper
*
* This function resets tamper by reseting SECALM register
*
* @return void
*/
void vSensorenResetTamper( void );

/**@brief For disabling tamper.
*
* This function disables all tamper registers so that, next
* time the system is ready to detect the tamper. This is needed
* for testing tamper.
*
* @return void
*/
void vSensorenDisableTamper( void );

#endif /* secalmINCLUDE_SENSEOREN_H_ */
