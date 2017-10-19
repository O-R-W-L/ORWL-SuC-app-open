/**===========================================================================
 * @file mpumotion.h
 *
 * @brief This file contains function prototype and macros used
 * 	  for motion detection.
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
 */
#ifndef MPU_MOTION_H
#define MPU_MOTION_H

#define motionI2C_SLAVE_ADDR	(0xD0)	/**< I2C slave address */
#define motionNBIT_WRITE	(1)	/**< N bits to write */
#define motionNBIT_READ	(1)	/**< N bits to read */
#define motionPWR_MAGMT_1	(0x6B)	/**< Power management reg 1*/
#define motionPWR_MAGMT_2	(0x6C)	/**< Power management reg 2*/
#define motionINT_ENABLE	(0x38)	/**< Interrupt enable reg */
#define motionMOT_DETECT_CTRL	(0x69)	/**< Motion detection reg */
#define motionWOM_THR		(0x1F)	/**< Threshold reg */
#define motionLP_ACCEL_ODR	(0x1E)	/**< Low power ODR reg */
#define motionACCEL_CONFIG_2	(0x1D)	/**< Accelometer config reg */
#define motionINT_CONFIG	(0x37)	/**< Interrupt config reg */
#define motionUSER_CNTRl	(0x6A)	/**< User control reg */
#define motionSTATUS_REG	(0x3A)	/**< Interrupt status reg */
#define motionNREG_CONFIG	(9)	/**< Number of reg to config*/
#define motionNGPIO_PINS	(32)	/**< Total Number of gpio pins*/
#define motionGPIOPIN_ZERO	(0)	/**< First gpio pin offset */

/** This structure is used to configure register for motion detection */
typedef struct mpu_reg_config
{
    uint8_t ucDevId;
    uint8_t ucRegAddr;
    uint8_t ucLength;
    uint8_t data;
} mpuRegConfig_t;
/** @brief this function configures gpio as interrupt lin.
 *
 * @return 0 on success and error code on failure.
 */
int32_t lMpuMotionGpioConfig(void);
/** @brief this function is interrupt handler.
 *
 * @return void.
 */
void vMpuMotionISR(void);
/** @brief Task to monitor motion.
 *
 * @return void.
 */
void vMpuMotionTask(void);
/** @brief this function configures the register
 *         required for motion detection.
 *
 * @return void.
 */
int32_t lMpuMotionRegConfig(void);
#endif
