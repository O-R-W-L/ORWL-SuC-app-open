/*
 * private.c --
 *
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, Maxim Integrated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Maxim Integrated nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MAXIM INTEGRATED ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* [INTERNAL] ------------------------------------------------------------------
 * Created on: April 20, 2012
 * Author: Dayananda H.B. (dayananda.hb@maximintegrated.com)
 *
 * ---- Subversion keywords (need to set the keyword property)
 * $Revision:: $: Revision of last commit
 * $Author:: $: Author of last commit
 * $Date:: $: Date of last commit
 * [/INTERNAL] -------------------------------------------------------------- */

/** Global includes */
#include <config.h>
#include <errors.h>

/** Other includes */
#include <cobra_defines.h>
#include <mml.h>
#ifdef _WITH_DEVICES_
#include <mml_gcr.h>
#include <mml_uart.h>
#include <mml_uart_regs.h>
#include <mml_gpio.h>
#include <mml_gpio_regs.h>
#include <mml_spi.h>
#include <mml_spi_regs.h>
#endif /* _WITH_DEVICES_ */

/** Local includes */
#include <printf_lite.h>
#include <private.h>
#include <uart_config.h>

#ifdef _WITH_DEVICES_
volatile mml_gpio_regs_t dev_gpio[MML_GPIO_DEV_COUNT];
volatile mml_spi_regs_t dev_spi[MML_SPI_DEV_COUNT];
volatile mml_uart_regs_t dev_uart[MML_UART_DEV_COUNT];
#endif /* _WITH_DEVICES_ */

/******************************************************************************/
int uart_write_char(const char c)
{
#ifdef _WITH_DEVICES_
	return mml_uart_write_char(MML_UART_DEV0, c);
#else
	return (unsigned int)c;
#endif /* _WITH_DEVICES_ */
}

/******************************************************************************/
void init_interfaces(void)
{
#ifdef _WITH_DEVICES_
#if !defined (_GPIO_RESET_AT_INIT_) || !defined (_UART_RESET_AT_INIT_) || !defined (_TIMER_RESET_AT_INIT_)
	volatile mml_gcr_regs_t		*mml_gcr = (volatile mml_gcr_regs_t*)MML_GCR_IOBASE;
#endif
	/** Main interfaces used by Secure ROM code, let's reset'em */
#ifndef _GPIO_RESET_AT_INIT_
	/** GPIOS */
	/** Reset all GPIOs' interface */
	mml_gcr->rstr |= MML_GCR_RSTR_GPIO_MASK;
	/** Wait for GPIOs reset to be done */
	while( MML_GCR_RSTR_GPIO_MASK & mml_gcr->rstr );
	/** No clock for GPIOs */
#endif /* _GPIO_RESET_AT_INIT_ */
#ifndef _UART_RESET_AT_INIT_
	/** UARTS */
	/** Reset all UARTs' interface */
	mml_gcr->rstr |= MML_GCR_RSTR_UARTS_MASK;
	/** Wait for UARTs reset to be done */
	while( MML_GCR_RSTR_UARTS_MASK & mml_gcr->rstr );
	/** Stop clock of each UART. It'll be set independently */
	mml_gcr->perckcn |= ( 1 << MML_PERCKCN_DEV_UART0 );
	mml_gcr->perckcn |= ( 1 << MML_PERCKCN_DEV_UART1 );
#endif /* _UART_RESET_AT_INIT_ */
#ifndef _TIMER_RESET_AT_INIT_
	/** TIMERS */
	/** Reset all TIMERs' interface */
	mml_gcr->rstr |= MML_GCR_RSTR_TIMERS_MASK;
	/** Wait for TIMERs reset to be done */
	while( MML_GCR_RSTR_TIMERS_MASK & mml_gcr->rstr );
	/** Stop clock of each Timer. It'll be set independently */
	mml_gcr->perckcn |= ( 1 << MML_PERCKCN_DEV_T0 );
	mml_gcr->perckcn |= ( 1 << MML_PERCKCN_DEV_T1 );
	mml_gcr->perckcn |= ( 1 << MML_PERCKCN_DEV_T2 );
	mml_gcr->perckcn |= ( 1 << MML_PERCKCN_DEV_T3 );
#endif /* _TIMER_RESET_AT_INIT_ */
#else
	/** Nothing to do right now */
#endif /* _WITH_DEVICES_ */
	/** We're done */
	return;
}

#ifdef _WITH_DEVICES_
/******************************************************************************/
int config_uart(void)
{
	mml_uart_config_t						uart_conf;

	uart_conf.baudrate = K_LITE_UART0_DEFAULT_BAUDRATE;
	uart_conf.data_bits = MML_UART_DATA_TRANSFER_SIZE_8_BITS;
	uart_conf.flwctrl = MML_UART_HW_FLOW_CTL_DISABLE;
	uart_conf.parity = MML_UART_PARITY_NONE;
	uart_conf.parity_mode = MML_UART_PARITY_MODE_ONES;
	uart_conf.rts_ctl = MML_UART_RTS_IO_LEVEL_LOW;
	uart_conf.stop_bits = MML_UART_STOPBITS_ONE;
	uart_conf.handler = (mml_uart_handler_t)config_uart;
	/** We're done */
	return  mml_uart_init(MML_UART_DEV0, uart_conf);
}

/******************************************************************************/
void save_all_regsiters(void)
{
	register unsigned int					i;

	/** GPIO */
	for( i = MML_GPIO_DEV_MIN;i < MML_GPIO_DEV_COUNT;i++ )
	{
		/**  */
		switch( i )
		{
			case MML_GPIO_DEV0:
				memcpy_int((unsigned int*)&dev_gpio[i], (unsigned int*)MML_GPIO0_IOBASE, ( sizeof(mml_gpio_regs_t) / 4 ));
				break;
			case MML_GPIO_DEV1:
				memcpy_int((unsigned int*)&dev_gpio[i], (unsigned int*)MML_GPIO1_IOBASE, ( sizeof(mml_gpio_regs_t) / 4 ));
				break;
			case MML_GPIO_DEV2:
				memcpy_int((unsigned int*)&dev_gpio[i], (unsigned int*)MML_GPIO2_IOBASE, ( sizeof(mml_gpio_regs_t) / 4 ));
				break;
			default:
				goto save_all_regsiters_out;
		}
	}
	/** UART */
	for( i = MML_UART_DEV_MIN;i <= MML_UART_DEV_MAX;i++ )
	{
		/**  */
		switch( i )
		{
			case MML_UART_DEV0:
				memcpy_int((unsigned int*)&dev_uart[i], (unsigned int*)MML_UART0_IOBASE, ( sizeof(mml_uart_regs_t) / 4 ));
				break;
			case MML_UART_DEV1:
				memcpy_int((unsigned int*)&dev_uart[i], (unsigned int*)MML_UART1_IOBASE, ( sizeof(mml_uart_regs_t) / 4 ));
				break;
			default:
				goto save_all_regsiters_out;
		}
	}
	/** SPI */
	for( i = MML_SPI_DEV_MIN;i <= MML_SPI_DEV_MAX;i++ )
	{
		/**  */
		switch( i )
		{
			case MML_SPI_DEV0:
				memcpy_int((unsigned int*)&dev_spi[i], (unsigned int*)MML_SPI0_IOBASE, ( sizeof(mml_spi_regs_t) / 4 ));
				break;
			case MML_SPI_DEV1:
				memcpy_int((unsigned int*)&dev_spi[i], (unsigned int*)MML_SPI1_IOBASE, ( sizeof(mml_spi_regs_t) / 4 ));
				break;
			case MML_SPI_DEV2:
				memcpy_int((unsigned int*)&dev_spi[i], (unsigned int*)MML_SPI2_IOBASE, ( sizeof(mml_spi_regs_t) / 4 ));
				break;
			default:
				goto save_all_regsiters_out;
		}
	}
	/** We're done */
save_all_regsiters_out:
	return;
}

/******************************************************************************/
int display_gpio_registers(void)
{
#ifdef ORWL_DEBUG
	register unsigned int					i;
#endif
	/**  */
	lite_printf("\n\tGPIOs registers:\n");
	/**  */
#if ORWL_DEBUG
	for( i = MML_GPIO_DEV_MIN;i < MML_GPIO_DEV_COUNT;i++ )
	{
		/**  */
		lite_printf("\t\tGPIO%d\n",i);
		/**  */
		lite_printf("\t\t\tEN: 0x%08x\n",dev_gpio[i].en);
		lite_printf("\t\t\tEN_SET: 0x%08x\n",dev_gpio[i].en_set);
		lite_printf("\t\t\tEN_CLR: 0x%08x\n",dev_gpio[i].en_clr);
		lite_printf("\t\t\tOUT_EN: 0x%08x\n",dev_gpio[i].out_en);
		lite_printf("\t\t\tOUT_EN_SET: 0x%08x\n",dev_gpio[i].out_en_set);
		lite_printf("\t\t\tOUT_EN_CLR: 0x%08x\n",dev_gpio[i].out_en_clr);
		lite_printf("\t\t\tOUT: 0x%08x\n",dev_gpio[i].out);
		lite_printf("\t\t\tOUT_SET: 0x%08x\n",dev_gpio[i].out_set);
		lite_printf("\t\t\tOUT_CLR: 0x%08x\n",dev_gpio[i].out_clr);
		lite_printf("\t\t\tIN: 0x%08x\n",dev_gpio[i].in);
		lite_printf("\t\t\tINT_MOD: 0x%08x\n",dev_gpio[i].int_mod);
		lite_printf("\t\t\tINT_POL: 0x%08x\n",dev_gpio[i].int_pol);
		lite_printf("\t\t\tINT_EN: 0x%08x\n",dev_gpio[i].int_en);
		lite_printf("\t\t\tINT_EN_SET: 0x%08x\n",dev_gpio[i].int_en_set);
		lite_printf("\t\t\tINT_EN_CLR: 0x%08x\n",dev_gpio[i].int_en_clr);
		lite_printf("\t\t\tINT_STAT: 0x%08x\n",dev_gpio[i].int_stat);
		lite_printf("\t\t\tINT_CLR: 0x%08x\n",dev_gpio[i].int_clr);
		lite_printf("\t\t\tWAKE_EN: 0x%08x\n",dev_gpio[i].wake_en);
		lite_printf("\t\t\tOPEN_DRAIN_EN: 0x%08x\n",dev_gpio[i].open_drain_en);
		lite_printf("\t\t\tINT_DUAL_EDGE: 0x%08x\n",dev_gpio[i].int_dual_edge);
		lite_printf("\t\t\tPAD_CFG1: 0x%08x\n",dev_gpio[i].pad_cfg1);
		lite_printf("\t\t\tPAS_CFG2: 0x%08x\n",dev_gpio[i].pad_cfg2);
		lite_printf("\t\t\tEN1: 0x%08x\n",dev_gpio[i].en1);
		lite_printf("\t\t\tEN1_SET: 0x%08x\n",dev_gpio[i].en1_set);
		lite_printf("\t\t\tEN1_CLR: 0x%08x\n",dev_gpio[i].en1_clr);
	}
#endif
	/** We're done */
	return NO_ERROR;
}

/******************************************************************************/
int display_uart_registers(void)
{
	register unsigned int					i;

	/**  */
	lite_printf("\n\tUARTs registers:\n");
	/**  */
	for( i = MML_UART_DEV_MIN;i <= MML_UART_DEV_MAX;i++ )
	{
		/**  */
		lite_printf("\t\tUART%d\n",i);
		lite_printf("\t\t\tCR: 0x%08x\n", dev_uart[i].cr);
		lite_printf("\t\t\tSR: 0x%08x\n", dev_uart[i].sr);
		lite_printf("\t\t\tIER: 0x%08x\n", dev_uart[i].ier);
		lite_printf("\t\t\tISR: 0x%08x\n", dev_uart[i].isr);
		lite_printf("\t\t\tBRR: 0x%08x\n", dev_uart[i].brr);
		lite_printf("\t\t\tRCR: 0x%08x\n", dev_uart[i].rcr);
		lite_printf("\t\t\tTXR: 0x%08x\n", dev_uart[i].txr);
		lite_printf("\t\t\tPNR: 0x%08x\n", dev_uart[i].pnr);
		lite_printf("\t\t\tDR: 0x%08x\n", dev_uart[i].dr);
	}
	/** We're done */
	return NO_ERROR;
}

/******************************************************************************/
int display_spi_registers(void)
{
	register unsigned int					i;

	/**  */
	lite_printf("\n\tSPIs registers:\n");
	/**  */
	for( i = MML_SPI_DEV_MIN;i <= MML_SPI_DEV_MAX;i++ )
	{
		/**  */
		lite_printf("\t\tSPI%d\n",i);
		lite_printf("\t\t\tDR: 0x%08x\n", dev_spi[i].dr);
		lite_printf("\t\t\tCR: 0x%08x\n", dev_spi[i].cr);
		lite_printf("\t\t\tSR: 0x%08x\n", dev_spi[i].sr);
		lite_printf("\t\t\tMR: 0x%08x\n", dev_spi[i].mr);
		lite_printf("\t\t\tDSR: 0x%08x\n", dev_spi[i].dsr);
		lite_printf("\t\t\tBRR: 0x%08x\n", dev_spi[i].brr);
		lite_printf("\t\t\tDMAR: 0x%08x\n", dev_spi[i].dmar);
		lite_printf("\t\t\tI2S_CR: 0x%08x\n", dev_spi[i].i2s_cr);
	}
	/** We're done */
	return NO_ERROR;
}

/******************************************************************************/
void memcpy_int(unsigned int *p_dst, unsigned int *p_src, unsigned int size)
{
	register unsigned int					i;

	for( i = 0;i < size;i++ )
	{
		p_dst[i] = p_src[i];
	}
	/** We're done */
	return;
}
/******************************************************************************/
int reset_all_interfaces(void)
{
#ifdef _DUMMY_CODE_
	int										result = COMMON_ERR_UNKNOWN;

	/** GPIOs */
	result = mml_gpio_reset_interface();
	if ( result )
	{
		goto reset_all_interfaces_out;
	}
	/** UARTs */
	result = mml_uart_reset_interface();
	if ( result )
	{
		goto reset_all_interfaces_out;
	}
	/** TIMERs */
	result = mml_tmr_reset_interface();
	if ( result )
	{
		goto reset_all_interfaces_out;
	}
	/** We're done */
reset_all_interfaces_out:
	return result;
#else
	return NO_ERROR;
#endif /* _DUMMY_CODE_ */
}
#endif /* _WITH_DEVICES_ */
/******************************************************************************/
/* EOF */
