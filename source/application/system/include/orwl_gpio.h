 /**===========================================================================
 * @file orwl_gpio.h
 *
 * @brief This file contains common data types and macros used for the
 * initialization of GPIO pins.
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
 ============================================================================
 *
 */

#ifndef _INCLUDE_ORWL_GPIO_H_
#define _INCLUDE_ORWL_GPIO_H_
/** driver includes */
#include <mml_gpio_regs.h>
#include <mml_gpio.h>
#include <mml_gpio_regs.h>


typedef struct
{
   mml_gpio_id_t dev_id;      		  /* The GPIO device identifier */
   int offset;               		  /* Pin offset in range of 0to31 */
   int bits_count;            	          /* Number of consecutive pins */
   mml_gpio_direction_t config;  	  /* GPIO Direction configuration */
   unsigned int data;         		  /* Default value of pin */
}gpioORWLGioArg;

/** We needed 5 seconds delay to make sure Intel has powered off. 5*10^6 micro
 * seconds make 5 seconds, but its not working as expected it is taking some 65
 * seconds. Based on trial and error method we have found that 10^4 some how get
 * 3-4 seconds which solves our purpose.
 */
#define gpioCPU_SUTDOWN_PULSE_US    50000

#define gpioGIO_HIGH                0x01  /** Default value of pin is One */
#define gpioGIO_LOW                 0x00  /** Default value of pin is One */
#define gpioNO_OF_PINS                 1  /** No.of pins to be initialized */

/* ORWL GIO PORT definition */
#define gpioGIO_PORT0		MML_GPIO_DEV0	/**< SuC GPIO Port 0 Id */
#define gpioGIO_PORT1		MML_GPIO_DEV1	/**< SuC GPIO Port 1 Id */
#define gpioGIO_PORT2		MML_GPIO_DEV2	/**< SuC GPIO Port 2 Id */

#ifdef  ORWL_EVT2
#define gpioORWL_MAX_ARRAY_SIZE_INIT  40  /**< Array size */
#define gpioEN_VDD_BL        	      14  /**gpioEN_VDD_BL - P0.14 */
#define gpioOLED_VCI_EN      	      15  /** gpioOLED_VCI_EN - P0.15 */
#endif  /* ORWL_EVT2 */

#ifdef  ORWL_EVT3
#define gpioEN_VDD_BL       	      10  /** gpioEN_VDD_BL - P0.10 */
#define gpioOLED_VCI_EN     	      11  /** gpioOLED_VCI_EN - P0.11 */
#define gpioCPU_RTS1                  15  /** gpioCPU_RTS1 - P0.15 */
#define gpioSuC_ID0        	      	  24  /** gpioSuC_ID0 - P1.24 */
#define gpioSuC_ID1                   25  /** gpioSuC_ID1 - P1.25 */
#define gpioSuC_ID2                   26  /** gpioSuC_ID2 - P1.26 */
#endif /* ORWL_EVT3 */

/**
 *  GPIO's of Port0
 */

#define gpioUC_EC_GPIO0               0   /** gpioUC_EC_GPIO0 - P0.0 */
#define gpioUC_EC_GPIO1               1   /** gpioUC_EC_GPIO1 - P0.1 */
#define gpioUC_EC_GPIO2               2   /** gpioUC_EC_GPIO2 - P0.2 */
#define gpioUC_EC_IRQ_ACK             3   /** gpioUC_EC_IRQ_ACK - P0.3 */
#define gpioUC_EC_IRQ_ACK_uC          4   /** gpioUC_EC_IRQ_ACK_uC - P0.4 */
#define gpioUC_SYSTEM_GPIO0           5   /** gpioUC_SYSTEM_GPIO0 - P0.5 */
#define gpioUC_SYSTEM_GPIO2           6   /** gpioUC_SYSTEM_GPIO2 - P0.6 */
#define gpioUC_SYSTEM_GPIO3           7   /** gpioUC_SYSTEM_GPIO3 - P0.7 */
#define gpioUC_PCH_SPI_IO2_EN        22   /** gpioUC_PCH_SPI_IO2_EN - P0.22 */
#define gpioOLED_DC_3V3              29   /** gpioOLED_DC_3V3 - P0.29 */
#define gpioUC_SPI_WP		     30   /** gpioUC_SPI_WP - P0.30 */
#define gpioHDMI_DDC_EN              31   /** gpioHDMI_DDC_EN - P0.31 */

/**
 * GPIO's of Port1
 */

#define gpioID_PINS1                  0   /** gpioID_PINS1 - P1.0 */
#define gpioID_PINS2                  1   /** gpioID_PINS2 - P1.1 */
#define gpioID_PINS3                  2   /** gpioID_PINS3 - P1.2 */
#define gpioUC_PWRBTNIN               3   /** gpioUC_PWRBTNIN - P1.3 */
#define gpioPWRBTIN                   4   /** gpioPWRBTIN - P1.4 */
#define gpioCLRC_TXVDD_EN             5   /** gpioCLRC_TXVDD_EN - P1.5 */
#define gpioCLRC_IRQ_N                6   /** gpioCLRC_IRQ_N - P1.6 */
#define gpioCLRC_PDWN                 7   /** gpioCLRC_PDWN - P1.7 */
#define gpioUC_TYPEC2_USB_S           8   /** gpioUC_TYPEC2_USB_S - P1.8 */
#define gpioUC_VIN_EN                 9   /** gpioUC_VIN_EN - P1.9 */
#define gpioINT_MPU6500              10   /** gpioINT_MPU6500 - P1.10 */
#define gpioCCG_I2C_INT_uC           11   /** gpioCCG_I2C_INT_uC - P1.11 */
#define gpioUC_CCG_RST               12   /** gpioUC_CCG_RST - P1.12 */
#define gpioBT_IRQ		     14   /** gpioBT_IRQ - P1.14 */
#define gpioUC_nOE		     15   /** gpioUC_nOE - P1.15 */
#define gpioUC_TYPEC1_USB_EN         16   /** gpioUC_TYPEC1_USB_EN - P1.16 */
#define gpioUC_TYPEC1_USB_S          17   /** gpioUC_TYPEC1_USB_S - P1.17 */
#define gpioUC_TYPEC2_USB_EN         18   /** gpioUC_TYPEC2_USB_EN - P1.18 */
#define gpioUC_MUX_EN                22   /** gpioUC_MUX_EN - P1.22 */
#define gpioUC_MUX1_EN               23   /** gpioUC_MUX1_EN - P1.23 */
#define gpioHW_SUC_ID0               24   /** gpioHW_SUC_ID0 - P1.24 */
#define gpioHW_SUC_ID1               25   /** gpioHW_SUC_ID0 - P1.25 */
#define gpioHW_SUC_ID2               26   /** gpioHW_SUC_ID0 - P1.26 */
#define gpioUC_SPI_CS_1		     27  /** gpioUC_SPI_CS_1 - P1.27 - NOT USED. OUTPUT LOGIC 1 */
#define gpioUC_SPI_CS_2		     28  /** gpioUC_SPI_CS_1 - P1.28 - NOT USED. OUTPUT LOGIC 1 */

#define gpioOLED_RESET_L_3V3         31   /** gpioOLED_RESET_L_3V3 - P1.31 */



#ifdef INTEL_DEBUG_SPI_ELIMINATE

#define gpio_Dev1_SUC_SPI2_1              20	//CPU_uC_SPI_CS_3V3
#define gpio_Dev1_SUC_SPI2_2              21	//CPU_uC_SPI_MOSI_3V3
#define gpio_Dev1_SUC_SPI2_3              29	//CPU_uC_SPI_SCK_3V3
#define gpio_Dev1_SUC_SPI2_4              19	//uC_CE#
#define gpio_Dev0_SUC_SPI2_5              30	//uC_SPI_WP#

#endif // INTEL_DEBUG_SPI_ELIMINATE

/**
 *  GPIO's of Port2
 */
#define gpioUC_VBUS_DET               0   /** gpioUC_VBUS_DET - P2.0 */

/* function declaration */
void vOrwlGpioInit( void );

/** @brief Function to shutdown Intel CPU by cutting of the power supply
 *
 * return void
 */
void gpioIntelCpuShutdown( void );

#endif //_INCLUDE_ORWL_GPIO_H_
