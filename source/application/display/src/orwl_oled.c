/**===========================================================================
 * @file orwl_oled.c
 *
 * @brief This file is used for initialization,powering on of OLED and
 * configuring corresponding SPI registers for OLED.
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

/* Application includes */
#include <orwl_gpio.h>
#include <orwl_oled.h>
#include <debug.h>
#include <delay.h>
/* driver includes */
#include <mml_spi.h>
#include <mml_gcr.h>

uint32_t ulOledInit( void )
{
	/** SSD 1327 SOLOMON TECH - GPIO CONFIGURATIONS */
	uint32_t ulResult = 0;

	/** set default value to each GPIO */
	ulResult = mml_gpio_write_bit_pattern(MML_GPIO_DEV0, gpioOLED_VCI_EN,
		gpioNO_OF_PINS, oledGPIO_LOW);
	ulResult |= mml_gpio_write_bit_pattern(MML_GPIO_DEV0, gpioOLED_DC_3V3,
		gpioNO_OF_PINS, oledGPIO_LOW);
	ulResult |= mml_gpio_write_bit_pattern(MML_GPIO_DEV1, gpioOLED_RESET_L_3V3,
		gpioNO_OF_PINS, oledGPIO_HIGH);
	ulResult |= mml_gpio_write_bit_pattern(MML_GPIO_DEV0, gpioEN_VDD_BL,
		gpioNO_OF_PINS, oledGPIO_HIGH);

	if (ulResult != NO_ERROR)
	{
	    debugERROR_PRINT("\n OLED: GPIO default value set failed \n");
		return ulResult;
	}

	/** set vci & wait for 1ms atleast * vci stable time -Nick request */
	ulResult |= mml_gpio_write_bit_pattern(MML_GPIO_DEV0, gpioOLED_VCI_EN,
		gpioNO_OF_PINS, oledGPIO_HIGH);
	return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

uint32_t ulOledPowerOn( void )
{
	uint32_t ulResult = COMMON_ERR_UNKNOWN;

	/** keep reset line & d/c line high */
	ulResult = mml_gpio_write_bit_pattern(MML_GPIO_DEV0,
		gpioEN_VDD_BL, gpioNO_OF_PINS, oledGPIO_HIGH);
	ulResult |= mml_gpio_write_bit_pattern(MML_GPIO_DEV1,
		gpioOLED_RESET_L_3V3, gpioNO_OF_PINS, oledGPIO_HIGH);
	ulResult |= mml_gpio_write_bit_pattern(MML_GPIO_DEV0,
		gpioOLED_DC_3V3, gpioNO_OF_PINS, oledGPIO_HIGH);

	delayMICRO_SEC(oledDELAY_STABILIZATION);

	/** set vci & wait for 1ms atleast * vci stable time */
	ulResult |= mml_gpio_write_bit_pattern(MML_GPIO_DEV0,
		gpioOLED_VCI_EN, gpioNO_OF_PINS, oledGPIO_HIGH);

	delayMICRO_SEC(oledDELAY_STABILIZATION);

	/** Toggle reset line with 100us delay at-least*/
	ulResult |= mml_gpio_write_bit_pattern(MML_GPIO_DEV1,
		gpioOLED_RESET_L_3V3, gpioNO_OF_PINS, oledGPIO_LOW);

	delayMICRO_SEC(oledTOGGLE_DELAY);

	ulResult |= mml_gpio_write_bit_pattern(MML_GPIO_DEV1 ,
		gpioOLED_RESET_L_3V3 ,gpioNO_OF_PINS, oledGPIO_HIGH);

	delayMICRO_SEC(oledDELAY_STABILIZATION);

	/** set dc & wait for 1ms at-least [for command d/c must be low] */
	ulResult |= mml_gpio_write_bit_pattern(MML_GPIO_DEV0 ,
		gpioOLED_DC_3V3 ,gpioNO_OF_PINS, oledGPIO_LOW);

	delayMICRO_SEC(oledDELAY_STABILIZATION);

	/** send command POWER_ON to OLED */
	ulResult |= lOledspiWrite(oledSPI_DEV);

	delayMICRO_SEC(oledPOWER_ON_DELAY);

	if (ulResult != NO_ERROR)
	{
		return ulResult;
	}
	return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

void vOledSessionStartCmd( uint32_t ulCmd )
{
	/** For data DC must be high  & for command it must be low */
	mml_gpio_write_bit_pattern(MML_GPIO_DEV0, gpioOLED_DC_3V3,
		gpioNO_OF_PINS, ulCmd);
	delayMICRO_SEC(oledDELAY_STABILIZATION);
}
/*---------------------------------------------------------------------------*/
