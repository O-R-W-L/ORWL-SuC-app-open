/**===========================================================================
 * @file orwl_spi.c
 *
 * @brief This file is used for configuring the spi interface.
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

/* Global Includes */
#include <errors.h>
/* Driver includes */
#include <mml_spi.h>
/* Application includes */
#include <orwl_oled.h>
#include <delay.h>
/*---------------------------------------------------------------------------*/

int32_t lOledspiWrite( uint8_t *pucData )
{
	return mml_spi_transmit(oledSPI_DEV, pucData, sizeof(int8_t));
}
/*---------------------------------------------------------------------------*/

uint32_t ulOledspiRegister( void )
{
	uint32_t ulResult;
	mml_spi_params_t    xSpiConfig;

	/* Fill parameters for spi configuration */
	xSpiConfig.baudrate = oledSPI_BAUD_RATE; /* 4MHZ */
	xSpiConfig.word_size = oledSPI_WORD_SIZE;
	xSpiConfig.mode = MML_SPI_MODE_MASTER;
	xSpiConfig.wor = MML_SPI_WOR_NOT_OPEN_DRAIN;
	xSpiConfig.clk_pol = MML_SPI_SCLK_LOW;
	xSpiConfig.phase = MML_SPI_PHASE_LOW;
	xSpiConfig.brg_irq = MML_SPI_BRG_IRQ_DISABLE;
	xSpiConfig.ssv = MML_SPI_SSV_LOW;
	xSpiConfig.ssio = MML_SPI_SSIO_OUTPUT;
	xSpiConfig.tlj = MML_SPI_TLJ_DIRECT;
	xSpiConfig.dma_tx.active = MML_SPI_DMA_DISABLE;
	xSpiConfig.dma_rx.active = MML_SPI_DMA_DISABLE;

	/* Set CS1
	 * bit0 - SSELx0
	 * bit1 - SSELx1
	 * bit2 - SSELx2
	 * bit3 - SSELx3
	 */
	xSpiConfig.ssel = oledSLAVE_SELECT; /* 0x02 */

	/* Call initialization function from driver */
	ulResult = lOledspiInit(&xSpiConfig);
	if (NO_ERROR == ulResult)
	{
	    /** Now enable SPI interface */
	    M_MML_SPI_ENABLE(oledSPI_DEV);
	}

	/* compensate delay b/w SPI actual enable & enabled called here above */
	{
	    uint32_t ulloop = 0;
	    for (ulloop = 0; ulloop < oledDELAY; ulloop++);
	}
	delayMICRO_SEC(oledSPI_DELAY);
	/** We're done */
	return ulResult;
}
/*---------------------------------------------------------------------------*/

int32_t lOledspiInit (mml_spi_params_t *pxSpiConfig)
{
	return mml_spi_init(oledSPI_DEV, pxSpiConfig);
}
/*---------------------------------------------------------------------------*/
