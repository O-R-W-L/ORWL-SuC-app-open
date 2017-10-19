/**===========================================================================
 * @file mpuapp.h
 *
 * @brief This file contains function prototype for mpu operation.
 *
 * @author vikram.k@design-shift.com
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ============================================================================
 *
 */
#ifndef MPUAPP_H
#define MPUAPP_H

#define mpuappNBIT		(1)    /**<Gpio offset*/
#define mpuappMPLINITFAILURE	(-1)   /**<MPL INIT error code*/
#define mpuappMPLLIBFAILURE	(-2)   /**<MPL Library init error code*/
#define mpuappMPLSTATLIBFAIL	(-3)   /**<MPL library start error */
#define mpuappNOTAUTH		(-4)   /**<Authentication error */
#define mpuappACCEL_ON		(0x01) /**<Accel on*/
#define mpuappGYRO_ON		(0x02) /**<Gyro on*/
#define mpuappDEFAULT_MPU_HZ	(20)   /**<Sampling rate*/
#define mpuappTap_TRESH		(50)   /**<Tap threshold*/
#define mpuappBIT0		(1 << 0) /**Bit to wait for event*/

/** @brief vMpuappGyroDataReadyCb
 * Every time new gyro data is available, this function is called in an
 * ISR context. In this example, it sets a flag protecting the FIFO read
 * function.
 */
void vMpuappGyroDataReadyCb(void);
/** @brief lMpuappmain  entry point.
  *
  * @return 0 on success and error code on failure.
  *
  */
int32_t lMpuappmain(void);
/** @brief lMpuappEnableMotion enables motion interrupt.
  *
  * @return 0 on success and error code on failure.
  *
  */
int32_t lMpuappEnableMotion(void);
/** @brief lMpuappDisableMotion disables motion interrupt.
  *
  * @return 0 on success and error code on failure.
  *
  */
int32_t lMpuappDisableMotion(void);
#endif
