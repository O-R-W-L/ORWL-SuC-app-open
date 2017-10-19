 /**===========================================================================
 * @file tamper_log_intel.h
 *
 * @brief This file contains macro related to tamper cause.
 *
 * @author viplav.roy@design-shift.com
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

/** String used in formatting tamper log sent to the intel */

#define tamperTAMPER_CAUSE		"Tamper Cause:\n"	/**< Tamper cause log */
#define tamperDIE_SHIELD_DETECTED	"\rDie Shield\n"	/**< Message if Die shield Flag set */
#define tamperLOW_TEMP_DETECTED		"\rLow Temp\n"		/**< Message on Low temperature detected */
#define tamperHIGH_TEMP_DETECTED	"\rHigh Temp\n"		/**< Message on High temperature detected */
#define tamperBATTERY_LOW_DETECTED	"\rBattery Low\n" 	/**< Mesasge on Battery Low */
#define tamperBATTERY_OVER_VOLT_DETECTED "\rBattery High\n"	/**< Message on Battery over voltage detected */
#define tamperEXTN_TAMPER_DETECTED	"\rExternal Tamper\n"	/**< Message On External Tamper detected*/
#define tamperOCCURED_TIME		"\rOccurred Time: "	/**< Occurred time message */
#define tamperFORMATED_TIME_LEN		(0x15)			/**< Length of the formatted time message */
