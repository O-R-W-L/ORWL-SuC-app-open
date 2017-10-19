/**===========================================================================
 * @file powerbtn.h
 *
 * @brief This file contains prototype power button management in ORWL
 *
 * @author ravikiran@design-shift.com
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

#ifndef _INCLUDE_POWERBTN_H_
#define _INCLUDE_POWERBTN_H_

#include <irq.h>
#include <orwl_gpio.h>
#include <mml_gpio.h>

#define powerbtnPWR_BTN_GIO	( gpioPWRBTIN )   /**< ORWL Power Button GIO */
#define powerbtnGIO_GROUP	( MML_GPIO_DEV1 ) /**< Power btn GPIO group identifier */

/**
 * @brief This function configures the ORWL power button
 *
 * @return void
 */
void vPowerbtnConfig( void );

/**
 * @brief Power button handler task
 *
 * @return void
 */
void vPowerbtnTsk( void *pvArgs );
#endif /* _INCLUDE_POWERBTN_H_ */
