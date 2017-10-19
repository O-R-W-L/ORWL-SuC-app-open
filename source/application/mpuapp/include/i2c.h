/**===========================================================================
 * @file i2c.h
 *
 * @brief This file contains function prototype for i2c.
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

#ifndef I2C_ORWL_H
#define I2C_ORWL_H

#include <stdint.h>
#include <mml_i2c.h>
#include <mml_i2c_regs.h>

#define i2cINDEX_0	(0) /**<Array index 0*/
#define i2cINDEX_1	(1) /**<Array index 1*/
#define i2cSIZE		(4) /**<I2c read/write size*/

/** @brief iI2cInit used to initialize I2C.
 *
 *  @return 0 on success -1 on failure.
 */
int32_t lI2cInit(void);

/** @brief iI2cWrite used to write data to I2C registers.
 * @param ucSlaveAddr salve address.
 * @param ucRegAddr register to write.
 * @param ucLength length of data to write.
 * @param ucData data to be written.
 *
 * @return 0 on success and negative value on failure.
 */
int32_t lI2cWrite(uint8_t ucSlaveAddr, uint8_t ucRegAddr,
	uint32_t ulLength, uint8_t *pucData);

/** @brief iI2cRead used to write data to I2C registers.
 *
 * @param ucSlaveAddr salve address.
 * @param ucRegAddr register to read.
 * @param ucLength length of data to read.
 * @param ucData data to be read.
 *
 * @return 0 on success and negative value on failure.
 */
int32_t lI2cRead(uint8_t ucSlaveAddr, uint8_t ucRegAddr,
	uint32_t ulLength, uint8_t *pucData);

/** @brief iI2cIRQHandler interrupt handler.
 *
 * @return void.
 */
void vI2cIRQHandler(void);
#endif
