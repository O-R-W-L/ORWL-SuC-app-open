/*
 * private.h --
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
 *     * Neither the name of the <organization> nor the
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
 * Created on: Feb 06, 2012
 * Author: Yann G. (yann.gaude@maximintegrated.com)
 *
 * ---- Subversion keywords (need to set the keyword property)
 * $Revision:: $: Revision of last commit
 * $Author:: $: Author of last commit
 * $Date:: $: Date of last commit
 * [/INTERNAL] -------------------------------------------------------------- */

#ifndef _PRIVATE_H_
#define _PRIVATE_H_

/** Global includes */
#include <config.h>
/** Other includes */
#include <mml.h>
/** Local includes */

/** Maxim system frequency for platform */
#define	K_ANGELA_FREQ_SYSTEM_MAX		MML_SYS_FREQ

 #ifdef _WITH_DEVICES_
#define	K_BLINK_LED_MAX_COUNT			0xfffff
#define	K_BLINK_LED_ON_MAX_COUNT		( 3 * K_BLINK_LED_MAX_COUNT )
#define	K_BLINK_LED_OFF_MAX_COUNT		( 2 * K_BLINK_LED_MAX_COUNT )

#define	MML_GPIO2_SDHC_LED_PIN			16
 #endif /* _WITH_DEVICES_ */

#define	K_DEFAULT_TIMEOUT_VALUE			0xbb8
/** Maximum timeout value */
#define	K_COBRA_TIMEOUT_MAX			0xfffe
/** Minimum timeout value */
#define	K_COBRA_TIMEOUT_MIN			0x1

/******************************************************************************/
int uart_write_char(const char c);
void init_interfaces(void);
 #ifdef _WITH_DEVICES_
int config_uart(void);
int config_gpio(void);
int blink_led(void);
void save_all_regsiters(void);
int display_gpio_registers(void);
int display_uart_registers(void);
int display_tmr_registers(void);
int display_spi_registers(void);
int reset_all_interfaces(void);
 #endif /* _WITH_DEVICES_ */
void memcpy_int(unsigned int *p_dst, unsigned int *p_src, unsigned int size);
void usecase_timeout_handler(void);

#endif /* _PRIVATE_H_ */

/******************************************************************************/
/* EOF */
