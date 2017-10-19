/**===========================================================================
 * @file i2c.c
 *
 * @brief This file is used for i2c operation.
 *
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ============================================================================
 *
 */

/*---------------------------------------------------------------------------*/

#include <stdint.h>
#include <stddef.h>
#include <mml_i2c_regs.h>
#include <i2c.h>
#include <debug.h>

void vI2cIRQHandler(void)
{
    debugPRINT("i2cIRQHandler\n");

}
/* ---------------------------------------------------------------------------*/

int32_t lI2cInit(void)
{
    mml_i2c_config_t xI2cConfig;

    /* Initialize I2C */
    xI2cConfig.baudrate = 400000;
    xI2cConfig.rxfifo_thr = 1;
    xI2cConfig.flags = MML_I2C_AUTOSTART_ENABLE;
    /* Just not to provide null pointer */
    xI2cConfig.irq_handler = (irq_handler_t) vI2cIRQHandler;

    return mml_i2c_init(&xI2cConfig);

}
/* ---------------------------------------------------------------------------*/

int32_t lI2cWrite(uint8_t ucSlaveAddr, uint8_t ucRegAddr,
	uint32_t ulLength, uint8_t *pucData)
{
    int32_t lRetVal;
    uint8_t ucMsg[2];
    uint32_t ulLen = 2;
    uint32_t ulSize;

    /* Length should be always greater than 0*/
    if (ulLength == 0)
    {
	return COMMON_ERR_INVAL;
    }
    if (pucData == NULL)
    {
	return COMMON_ERR_NULL_PTR;
    }
    ucMsg[i2cINDEX_0] = ucRegAddr;
    ucMsg[i2cINDEX_1] = *pucData;

    /* Initialize the I2C operation by writing slave address
     * and one byte data.
     */
    if ((lRetVal = mml_i2c_write_start(ucSlaveAddr,ucMsg,
	    (unsigned  int*)&ulLen))
	    != NO_ERROR)
    {
	return lRetVal;
    }

    /* As I2C operation is initialized check if data lengths is one*/
    if (ulLength > 1)
    {
	/* increment the data pointer as one byte
	 * of data is already written.
	 */
	pucData++;
	/* Already one by data is written decrement the length by one*/
	ulLen = ulLength - 1;

	/* If data length is greater than four, in each
	 * write operation four bytes of data is written
	 * until length becomes zero.Four byte approach
	 * is used to avoid limitation on write operation
	 * for greater length, if any and make code generic.
	 */
	for (; ulLen>0; )
	{
	    if (ulLen > i2cSIZE)
	    {
		ulSize = i2cSIZE;
	    }
	    else
	    {
		ulSize = ulLen;
	    }

	    if ((lRetVal = mml_i2c_write(pucData,
		    (unsigned int *)&ulSize)) != NO_ERROR)
	    {
		return lRetVal;
	    }
	    pucData += ulSize;
	    ulLen -= ulSize;
	}
    }
    return NO_ERROR;
}
/* ---------------------------------------------------------------------------*/

int32_t lI2cRead(uint8_t ucSlaveAddr, uint8_t ucRegAddr,
	uint32_t ulLength, uint8_t  *pucData)
{
    int32_t lRetVal;
    uint32_t ucBusStatus;
    uint32_t ulLen = 1;       /*Write one byte register address to read from*/
    uint32_t ulSize;

    if (ulLength == 0)
    {
	return COMMON_ERR_INVAL;
    }
    if (pucData == NULL)
    {
	return COMMON_ERR_NULL_PTR;
    }
    /* Write the register address to read from and start
     * i2c operation.
     */
    if ((lRetVal = mml_i2c_write_start(ucSlaveAddr, &ucRegAddr,
	    (unsigned int *)&ulLen)) != NO_ERROR)
     {
	return lRetVal;
     }
    ulLen = ulLength;

    /* If data length is greater than four, in each
     * read operation four bytes of data is read
     * until length becomes zero.Four byte approach
     * is used to avoid limitation on read operation
     * for greater length, if any and make code generic.
     */
    for (; ulLen>0; )
	{
	    if (ulLen > i2cSIZE)
	    {
		ulSize = i2cSIZE;
	    }
	    else
	    {
		ulSize = ulLen;
	    }
	    /* Read the register */
	    if((lRetVal = mml_i2c_ioctl(MML_I2C_SET_READCOUNT, &ulSize))
		    != NO_ERROR)
	    {
		return lRetVal;
	    }

	    if ((lRetVal = mml_i2c_read_start(ucSlaveAddr)) != NO_ERROR)
	    {
		return lRetVal;
	    }
	    do
	    {
		mml_i2c_bus_status((unsigned int *)&ucBusStatus);
	    }
	    while ((ucBusStatus & MML_I2C_SR_RXEMPTY_MASK)
			  || (ucBusStatus & MML_I2C_SR_BUSY_MASK));

	    if ((lRetVal = mml_i2c_read(pucData,
		    (unsigned int *)&ulSize)) != NO_ERROR )
	    {
		return lRetVal;
	    }

	    pucData += ulSize;
	    ulLen -= ulSize;
	}
	return NO_ERROR;
}
