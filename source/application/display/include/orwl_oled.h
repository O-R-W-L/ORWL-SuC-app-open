 /**===========================================================================
 * @file orwl_oled.h
 *
 * @brief This file contains common data types and macros used for the
 * initialization of OLED.
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
#ifndef INCLUDE_ORWL_OLED_H_
#define INCLUDE_ORWL_OLED_H_
/* Global includes */
#include <stdint.h>
#include <mml_spi.h>

#define oledSPI_DEV				( MML_SPI_DEV1 )
/** Bits for OLED power on */
#define oledCMD_POWER_ON			( 0xAF )
/** Default value of pin is One i.e high */
#define oledGPIO_HIGH				( 0x01 )
/** Default value of pin is One i.e low */
#define oledGPIO_LOW				( 0x00 )
/** bits for slave select */
#define oledSLAVE_SELECT			( 0x01 )
/** Delay for compensation */
#define oledDELAY				( 0xFFFF )
/** SPI baudrate */
#define oledSPI_BAUD_RATE			( 4000000 )
/** SPI word size */
#define oledSPI_WORD_SIZE			( 8 )
/** bits for sending command */
#define oledCMD_SESSION				( 0 )
/** bits for sending data */
#define oledDATA_SESSION			( 1 )
/** Cmd for NOP instruction */
#define oledNOP_CMD				( 0xB2 )
/** Delay for setting SPI reg */
#define oledSPI_DELAY				( 3000 )
/** Status for data */
#define oledDATA_STATUS				( 'D' )
/** Status for command */
#define oledCMD_STATUS				( 'C' )
/** Delay for stabilization of pins */
#define oledDELAY_STABILIZATION			( 1000 )
/** Delay between toggle of reset pin */
#define oledTOGGLE_DELAY			( 1000 )
/** Delay before powering on OLED */
#define oledPOWER_ON_DELAY			( 2000 )
/* Function Declaration */

/**@brief For setting DC line
*
* This function is used to set oled_dc line according to data or command.
* for data DC must be high  & for command it must be low
*
* @param ulcmd used to send data bits according to data/command.
*
* @return void
*/
void vOledSessionStartCmd( uint32_t ulCmd );

/**@brief OLED initialization
*
* This function initialize the OLED and sets default value
* to corresponding GPIO pins.
*
* @return NO_ERROR in case of success and error code in case of failure
*/
uint32_t ulOledInit( void );

/**@brief configuring SPI registers for OLED
*
* This function configures SPI registers and initialize spi interface
*
* @return NO_ERROR in case of success and error code in case of failure
*/
uint32_t ulOledspiRegister( void );

/**@brief Power sequence for OLED
*
* This function defines the power sequence for OLED - SSD1327
* SOLEMON TECH DEFINES
*
* @return NO_ERROR in case of success and error code in case of failure
*/
uint32_t ulOledPowerOn( void );

/**@brief for transmitting data
*
* This function is used to writing/reading data on spi interface.
*
* @param pucdata is pointer to the buffer
*
* @return NO_ERROR in case of success and error code in case of failure
*/
int32_t lOledspiWrite(uint8_t *pucData);

/**@brief For initializing and configuring spi interface
*
* This function is used to activating, resting and configuring spi interface.
*
* @param pxSpiConfig is a pointer on SPI parameters structure allocated and
* filled by the caller.
*
* @return NO_ERROR in case of success and error code in case of failure
*/
int32_t lOledspiInit(mml_spi_params_t *pxSpiConfig);
#endif /* INCLUDE_ORWL_OLED_H_ */
