/**===========================================================================
 * @file orwl_gpio.c
 *
 * @brief This file is used for initialization and management
 * of GPIO lines for SuC.
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

/*---------------------------------------------------------------------------*/

/** Global includes */
#include <stdint.h>
#include <errors.h>

/** driver includes */
#include <mml_gpio.h>
#include <mml_gpio_regs.h>

/** application includes */
#include <orwl_gpio.h>
#include <printf_lite.h>
#include <debug.h>
#include <delay.h>


#ifdef ORWL_EVT3
/* Array of the arguments for mml_gpio_init function for EVT3 */

static gpioORWLGioArg orwlArg[]={
	{ MML_GPIO_DEV0 ,gpioUC_EC_GPIO0 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,			/* P0.0 */
	{ MML_GPIO_DEV0 ,gpioUC_EC_GPIO1 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,			/* P0.1 */
	{ MML_GPIO_DEV0 ,gpioUC_EC_GPIO2 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,			/* P0.2 */
	{ MML_GPIO_DEV0 ,gpioUC_EC_IRQ_ACK ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,			/* P0.3 */
	{ MML_GPIO_DEV0 ,gpioUC_EC_IRQ_ACK_uC ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,			/* P0.4 */
	{ MML_GPIO_DEV0 ,gpioUC_SYSTEM_GPIO0 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH } ,			/* P0.5 */
	{ MML_GPIO_DEV0 ,gpioUC_SYSTEM_GPIO2 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,			/* P0.6 */
	{ MML_GPIO_DEV0 ,gpioUC_SYSTEM_GPIO3 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH } ,			/* P0.7 */
	{ MML_GPIO_DEV0 ,gpioEN_VDD_BL ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW } ,			/* P0.10 */
	{ MML_GPIO_DEV0 ,gpioOLED_VCI_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW } ,			/* P0.11 */
	{ MML_GPIO_DEV0 ,gpioUC_PCH_SPI_IO2_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW } ,			/* P0.22 */
	{ MML_GPIO_DEV0 ,gpioOLED_DC_3V3 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW } ,			/* P0.29 */
	{ MML_GPIO_DEV0 ,gpioHDMI_DDC_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH } ,			/* P0.31 */
	{ MML_GPIO_DEV1 ,gpioID_PINS1 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_LOW } ,			/* P1.0 */
	{ MML_GPIO_DEV1 ,gpioID_PINS2 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,			/* P1.1 */
	{ MML_GPIO_DEV1 ,gpioID_PINS3 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,			/* P1.2 */
	{ MML_GPIO_DEV1 ,gpioUC_PWRBTNIN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH } ,			/* P1.3 */
	{ MML_GPIO_DEV1 ,gpioPWRBTIN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,			/* P1.4 */
	{ MML_GPIO_DEV1 ,gpioCLRC_TXVDD_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW } ,			/* P1.5 */
	{ MML_GPIO_DEV1 ,gpioCLRC_IRQ_N ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,			/* P1.6 */
	{ MML_GPIO_DEV1 ,gpioCLRC_PDWN , gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH } ,			/* P1.7 */
	{ MML_GPIO_DEV1 ,gpioUC_TYPEC2_USB_S ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW } ,			/* P1.8 */
	{ MML_GPIO_DEV1 ,gpioUC_VIN_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_LOW } ,			/* P1.9 */
	{ MML_GPIO_DEV1 ,gpioINT_MPU6500 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN , gpioGIO_HIGH } ,			/* P1.10 */
	{ MML_GPIO_DEV1 ,gpioCCG_I2C_INT_uC ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN , gpioGIO_HIGH } ,			/* P1.11 */
	{ MML_GPIO_DEV1 ,gpioUC_CCG_RST ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT , gpioGIO_HIGH } ,			/* P1.12 */
	{ MML_GPIO_DEV1 ,gpioBT_IRQ ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN , gpioGIO_HIGH },			/* P1.14 */
	{ MML_GPIO_DEV1 ,gpioUC_nOE ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT , gpioGIO_LOW },			/* P1.15 */
	{ MML_GPIO_DEV1 ,gpioUC_TYPEC1_USB_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW } ,			/* P1.16 */
	{ MML_GPIO_DEV1 ,gpioUC_TYPEC1_USB_S ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW } ,			/* P1.17 */
	{ MML_GPIO_DEV1 ,gpioUC_TYPEC2_USB_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW },			/* P1.18 */
	{ MML_GPIO_DEV1 ,gpioUC_MUX_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH } ,			/* P1.22 */
	{ MML_GPIO_DEV1 ,gpioUC_MUX1_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW } ,			/* P1.23 */
	{ MML_GPIO_DEV1 ,gpioHW_SUC_ID0 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH  },			/* P1.24 */
	{ MML_GPIO_DEV1 ,gpioHW_SUC_ID1 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH  },			/* P1.25 */
	{ MML_GPIO_DEV1 ,gpioHW_SUC_ID2 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH  },			/* P1.26 */
	{ MML_GPIO_DEV1 ,gpioUC_SPI_CS_1 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH  },			/* P1.27 */
	{ MML_GPIO_DEV1 ,gpioUC_SPI_CS_2 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH  },			/* P1.28 */
	{ MML_GPIO_DEV1 ,gpioOLED_RESET_L_3V3 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH } ,			/* P1.31 */
	{ MML_GPIO_DEV2 ,gpioUC_VBUS_DET ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH }				/* P2.0 */
    };
#endif //ORWL_EVT3
#ifdef INTEL_DEBUG_SPI_ELIMINATE

static gpioORWLGioArg orwlIntelSpi[]={
	{ MML_GPIO_DEV1 ,gpio_Dev1_SUC_SPI2_1 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,
	{ MML_GPIO_DEV1 ,gpio_Dev1_SUC_SPI2_2 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,
	{ MML_GPIO_DEV1 ,gpio_Dev1_SUC_SPI2_3 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,
	{ MML_GPIO_DEV1 ,gpio_Dev1_SUC_SPI2_4 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH } ,
	{ MML_GPIO_DEV0 ,gpio_Dev0_SUC_SPI2_5 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH }
};
#endif // INTEL_DEBUG_SPI_ELIMINATE

#ifdef ORWL_EVT2
/* Array of the arguments for mml_gpio_init function for EVT2 */

	gpioORWLGioArg orwlArg[gpioORWL_MAX_ARRAY_SIZE_INIT]={
	{ MML_GPIO_DEV0 ,gpioUC_EC_GPIO0 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH },			/* P0.0 */
	{ MML_GPIO_DEV0 ,gpioUC_EC_GPIO1 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH },			/* P0.1 */
	{ MML_GPIO_DEV0 ,gpioUC_EC_GPIO2 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH },			/* P0.2 */
	{ MML_GPIO_DEV0 ,gpioUC_EC_IRQ_ACK ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH },			/* P0.3 */
	{ MML_GPIO_DEV0 ,gpioUC_EC_IRQ_ACK_uC ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH },			/* P0.4 */
	{ MML_GPIO_DEV0 ,gpioUC_SYSTEM_GPIO0 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH },			/* P0.5->Out*/
	{ MML_GPIO_DEV0 ,gpioUC_SYSTEM_GPIO2 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH },			/* P0.6->Out*/
	{ MML_GPIO_DEV0 ,gpioUC_SYSTEM_GPIO3 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH },			/* P0.7->In*/
	{ MML_GPIO_DEV0 ,gpioEN_VDD_BL ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW },			/* P0.14 */
	{ MML_GPIO_DEV0 ,gpioOLED_VCI_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW },			/* P0.15 */
	{ MML_GPIO_DEV0 ,gpioUC_PCH_SPI_IO2_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW },			/* P0.22 */
	{ MML_GPIO_DEV0 ,gpioOLED_DC_3V3 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW },			/* P0.29 */
	{ MML_GPIO_DEV0 ,gpioUC_SPI_WP ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW },			/* P0.30 */
	{ MML_GPIO_DEV0 ,gpioHDMI_DDC_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH },			/* P0.31 */
	{ MML_GPIO_DEV1 ,gpioID_PINS1 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_LOW },				/* P1.0 */
	{ MML_GPIO_DEV1 ,gpioID_PINS2 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH },			/* P1.1 */
	{ MML_GPIO_DEV1 ,gpioID_PINS3 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH },			/* P1.2 */
	{ MML_GPIO_DEV1 ,gpioUC_PWRBTNIN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH },			/* P1.3 */
	{ MML_GPIO_DEV1 ,gpioPWRBTIN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH },			/* P1.4 */
	{ MML_GPIO_DEV1 ,gpioCLRC_TXVDD_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW },			/* P1.5 */
	{ MML_GPIO_DEV1 ,gpioCLRC_IRQ_N ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH },			/* P1.6 */
	{ MML_GPIO_DEV1 ,gpioCLRC_PDWN , gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH },			/* P1.7 */
	{ MML_GPIO_DEV1 ,gpioUC_VIN_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN , gpioGIO_LOW },			/* P1.9 */
	{ MML_GPIO_DEV1 ,gpioINT_MPU6500 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN , gpioGIO_HIGH },			/* P1.10 */
	{ MML_GPIO_DEV1 ,gpioCCG_I2C_INT_uC ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN , gpioGIO_HIGH },			/* P1.11 */
	{ MML_GPIO_DEV1 ,gpioUC_CCG_RST ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT , gpioGIO_HIGH },			/* P1.12 */
	{ MML_GPIO_DEV1 ,gpioUC_TYPEC1_USB_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW },			/* P1.16 */
	{ MML_GPIO_DEV1 ,gpioUC_TYPEC1_USB_S ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH },			/* P1.17 */
	{ MML_GPIO_DEV1 ,gpioUC_TYPEC2_USB_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW },			/* P1.18 */
	{ MML_GPIO_DEV1 ,gpioUC_TYPEC2_USB_S ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW },			/* P1.19 */
	{ MML_GPIO_DEV1 ,gpioUC_MUX_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH },			/* P1.22 */
	{ MML_GPIO_DEV1 ,gpioUC_MUX1_EN ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_LOW },			/* P1.23 */
	{ MML_GPIO_DEV1 ,gpioHW_SUC_ID0 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH  },			/* P1.24 */
	{ MML_GPIO_DEV1 ,gpioHW_SUC_ID1 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH  },			/* P1.25 */
	{ MML_GPIO_DEV1 ,gpioHW_SUC_ID2 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH  },			/* P1.26 */
	{ MML_GPIO_DEV1 ,gpioOLED_RESET_L_3V3 ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_OUT ,gpioGIO_HIGH },			/* P1.31 */
	{ MML_GPIO_DEV2 ,gpioUC_VBUS_DET ,gpioNO_OF_PINS ,
		MML_GPIO_DIR_IN ,gpioGIO_HIGH  }			/* P2.0 */
	};
#endif // ORWL_EVT2

/**
* @func vOrwlGpioInit(void)
*
* @brief <The function is used to initialize and configure
* the GPIO pins individually>
*
* <This function initialize GPIO pins and sets its default value>
*
* @return <returning void>
*/

void vOrwlGpioInit( void )
{
	uint32_t	ulResult = COMMON_ERR_UNKNOWN;
	uint8_t		ucRow;
	uint32_t	ulArraySize ;

	ulArraySize = sizeof(orwlArg)/sizeof(gpioORWLGioArg) ;

        mml_gpio_config_t configBank =
				{	.gpio_direction = MML_GPIO_DIR_IN,
					.gpio_function = MML_GPIO_NORMAL_FUNCTION,
					.gpio_pad_config = MML_GPIO_PAD_NORMAL,
					.gpio_intr_mode = 0,
					.gpio_intr_polarity = 0
				};

        mml_gpio_config_t configBankInWithPadPullUp =
        			{	.gpio_direction = MML_GPIO_DIR_IN,
        				.gpio_function = MML_GPIO_NORMAL_FUNCTION,
					.gpio_pad_config = MML_GPIO_PAD_PULLUP,
					.gpio_intr_mode = 0,
					.gpio_intr_polarity = 0
        			};

/**
 * initialization of all GPIO pins
 */
	configBank.gpio_direction = MML_GPIO_DIR_IN ;
	ulResult = mml_gpio_init(MML_GPIO_DEV0 ,gpioUC_EC_GPIO0 ,
		gpioNO_OF_PINS ,configBank );
	ulResult = mml_gpio_init(MML_GPIO_DEV1 ,gpioID_PINS1 ,
		gpioNO_OF_PINS ,configBank);
	ulResult = mml_gpio_init(MML_GPIO_DEV2 ,gpioUC_VBUS_DET ,
		gpioNO_OF_PINS ,configBank);
	if(ulResult)
	{
	    /* if initialization fails */
	    return;
	}

	for(ucRow=0; ucRow < ulArraySize; ucRow++)
	{
	    configBank.gpio_direction = orwlArg[ucRow].config ;

	    if(orwlArg[ucRow].dev_id == gpioGIO_PORT1 && orwlArg[ucRow].offset == gpioUC_VIN_EN)
	    {
		ulResult = mml_gpio_set_config(orwlArg[ucRow].dev_id,
				    orwlArg[ucRow].offset,orwlArg[ucRow].bits_count,
				    configBankInWithPadPullUp);
	    }
	    else
	    {

		ulResult = mml_gpio_set_config(orwlArg[ucRow].dev_id,
		    orwlArg[ucRow].offset,orwlArg[ucRow].bits_count,
		    configBank);
	    }
	    if ( ulResult )
   	    {
		/* if initialization fails */
		debugPRINT("port-%d pin_no %d initialization failed\n",
			orwlArg[ucRow].dev_id,orwlArg[ucRow].offset);
   	    }
	}

#ifdef INTEL_DEBUG_SPI_ELIMINATE

	for(ucRow=0; ucRow < (sizeof(orwlIntelSpi)/sizeof(gpioORWLGioArg)); ucRow++)
	{
	    configBank.gpio_direction = orwlIntelSpi[ucRow].config ;
	    configBank.gpio_pad_config = MML_GPIO_PAD_PULLDOWN;

	    ulResult = mml_gpio_set_config(orwlIntelSpi[ucRow].dev_id,
		    orwlIntelSpi[ucRow].offset,orwlIntelSpi[ucRow].bits_count,
		    configBank);
	    if ( ulResult )
   	    {
		/* if initialization fails */
		debugPRINT("port-%d pin_no %d initialization failed\n",
			orwlIntelSpi[ucRow].dev_id,orwlIntelSpi[ucRow].offset);
   	    }
	}
#endif /*INTEL_DEBUG_SPI_ELIMINATE*/
/**
 *  set default value to GPIO
 */
	for(ucRow=0; ucRow < ulArraySize; ucRow++)
	{
	    if(orwlArg[ucRow].config == MML_GPIO_DIR_OUT)
	    {
		ulResult = mml_gpio_write_bit_pattern(orwlArg[ucRow].dev_id,
			orwlArg[ucRow].offset,orwlArg[ucRow].bits_count,
			    orwlArg[ucRow].data);
		if ( ulResult )
		{
		    /* if initialization fails */
		    debugPRINT("port-%d pin_no %d initialization failed\n",
			    orwlArg[ucRow].dev_id,orwlArg[ucRow].offset);
		}
	    }
	}
/* We're done */
}

/*---------------------------------------------------------------------------*/

void gpioIntelCpuShutdown( void )
{
    uint32_t	ulResult = COMMON_ERR_UNKNOWN;
    mml_gpio_config_t configBankIn = {	.gpio_direction = MML_GPIO_DIR_IN,
					.gpio_function = MML_GPIO_NORMAL_FUNCTION,
					.gpio_pad_config = MML_GPIO_PAD_PULLUP,
					.gpio_intr_mode = 0,
					.gpio_intr_polarity = 0
				    };

    mml_gpio_config_t configBankOut = {	.gpio_direction = MML_GPIO_DIR_OUT,
					.gpio_function = MML_GPIO_NORMAL_FUNCTION,
					.gpio_pad_config = MML_GPIO_PAD_NORMAL,
					.gpio_intr_mode = 0,
					.gpio_intr_polarity = 0
				    };

    /* Configure gpioUC_VIN_EN as output */
    ulResult = mml_gpio_set_config(gpioGIO_PORT1,
		    gpioUC_VIN_EN, gpioNO_OF_PINS, configBankOut);
    if (ulResult)
    {
	/* if initialization fails */
	debugPRINT("\n failed to initialize gpioUC_VIN_EN \n");
    }

    /* generate 1 ms pulse */
    mml_gpio_write_bit_pattern(gpioGIO_PORT1, gpioUC_VIN_EN, gpioNO_OF_PINS, gpioGIO_LOW);
    delayuSEC(gpioCPU_SUTDOWN_PULSE_US);

    /* Configure gpioUC_VIN_EN as input again */
    ulResult = mml_gpio_set_config(gpioGIO_PORT1,
		    gpioUC_VIN_EN, gpioNO_OF_PINS, configBankIn);
    if (ulResult)
    {
	/* if initialization fails */
	debugPRINT("\n failed to initialize gpioUC_VIN_EN \n");
    }
}
/*---------------------------------------------------------------------------*/
