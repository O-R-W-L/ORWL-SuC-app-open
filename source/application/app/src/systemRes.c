/**===========================================================================
 * @file systemRes.c
 *
 * @brief This file contains definition of common api's used across various modes including
 * common resource access for SuC.
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ============================================================================
 *
 */

/* FreeRTOS includes */
#include <FreeRTOS.h>
#include <stdlib.h>
#include <task_config.h>
#include <task.h>
#include <portmacro.h>

/* local include */
#include <orwl_gpio.h>
#include <systemRes.h>


/* system resource */
static xgSysRes_t prvSystemResHandle;

uint32_t ulSystemResInitGlobalSystemResources( void )
{
	/* GIO handler assignment */
	prvSystemResHandle.xUCSysGio0.ulGropuId = gpioGIO_PORT0;
	prvSystemResHandle.xUCSysGio0.ulOPinId = gpioUC_SYSTEM_GPIO0;

	prvSystemResHandle.xUCSysGio3.ulGropuId = gpioGIO_PORT0;
	prvSystemResHandle.xUCSysGio3.ulOPinId = gpioUC_SYSTEM_GPIO3;

	prvSystemResHandle.xUCSysGio2.ulGropuId = gpioGIO_PORT0;
	prvSystemResHandle.xUCSysGio2.ulOPinId = gpioUC_SYSTEM_GPIO2;

	// Initialize the resource for the other Intel Control interfaces
	prvSystemResHandle.xUCSysUSBPort2Sel.ulGropuId = gpioGIO_PORT1;
	prvSystemResHandle.xUCSysUSBPort2Sel.ulOPinId = gpioUC_TYPEC2_USB_S ;

	prvSystemResHandle.xUCSysUSBPort1Sel.ulGropuId =gpioGIO_PORT1;
	prvSystemResHandle.xUCSysUSBPort1Sel.ulOPinId = gpioUC_TYPEC1_USB_S ;

	prvSystemResHandle.xUCSysUSBPort2En.ulGropuId =gpioGIO_PORT1;
	prvSystemResHandle.xUCSysUSBPort2En.ulOPinId = gpioUC_TYPEC2_USB_EN ;

	prvSystemResHandle.xUCSysUSBPort1En.ulGropuId =gpioGIO_PORT1;
	prvSystemResHandle.xUCSysUSBPort1En.ulOPinId = gpioUC_TYPEC1_USB_EN ;

	prvSystemResHandle.xUCSysECPwnBtn.ulGropuId = gpioGIO_PORT1;
	prvSystemResHandle.xUCSysECPwnBtn.ulOPinId = gpioUC_PWRBTNIN ;

#ifdef ORWL_EVT2
	/* No dedicated GIO for BLE monitor */
#else //EVT3
	prvSystemResHandle.xProximityCheck.ulGropuId = gpioGIO_PORT1;
	prvSystemResHandle.xProximityCheck.ulOPinId = gpioBT_IRQ;
#endif
	return NO_ERROR;
}
/*----------------------------------------------------------------------------*/

void vSystemResPowerOn_IntelCpu( void )
{
      TickType_t xDelay30ms = 30 / portTICK_PERIOD_MS;

   /* Let initialize the Power and HW IPC with Intel */
    mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysGio0.ulGropuId ,
	    prvSystemResHandle.xUCSysGio0.ulOPinId , 1 , gpioGIO_HIGH );

    mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysGio3.ulGropuId ,
	    prvSystemResHandle.xUCSysGio3.ulOPinId , 1 , gpioGIO_HIGH );

    mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysECPwnBtn.ulGropuId ,
	    prvSystemResHandle.xUCSysECPwnBtn.ulOPinId , 1 , gpioGIO_HIGH );

    /* Mimic power button  */
    vSystemResIntel_ShortPress();
}
/*----------------------------------------------------------------------------*/

void vSystemResPowerOff_IntelCpu( void )
{
    /* Mimic Low Bat signal for System to shutdown  */
    mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysGio0.ulGropuId ,
	    prvSystemResHandle.xUCSysGio0.ulOPinId , 1 , gpioGIO_LOW );
}
/*----------------------------------------------------------------------------*/

void vSystemResForcePowerOff_IntelCpu( void )
{
    /* Force shutdown the system */
    gpioIntelCpuShutdown();
}
/*----------------------------------------------------------------------------*/

uint32_t ulSystemResPowerState_Intel( void )
{
    uint32_t gpioVal ;
    mml_gpio_read_bit_pattern(prvSystemResHandle.xUCSysGio2.ulGropuId,
	    prvSystemResHandle.xUCSysGio2.ulOPinId, 1, &gpioVal);
    return gpioVal ;
}
/*----------------------------------------------------------------------------*/

void vSystemResProxi_On( void )
{
    /* switch off the display */
    vSystemResPowerOff_IntelDisp() ;

    /* Disable the USB Ports */
    mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysUSBPort1En.ulGropuId ,
	    prvSystemResHandle.xUCSysUSBPort1En.ulOPinId , 1 , gpioGIO_HIGH );

    mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysUSBPort2En.ulGropuId ,
	    prvSystemResHandle.xUCSysUSBPort2En.ulOPinId , 1 , gpioGIO_HIGH );
}
/*----------------------------------------------------------------------------*/

void vSystemResProxi_Off( void )
{
    /* Enable the USB Ports */
    mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysUSBPort1En.ulGropuId ,
	    prvSystemResHandle.xUCSysUSBPort1En.ulOPinId , 1 , gpioGIO_LOW );

    mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysUSBPort2En.ulGropuId ,
	    prvSystemResHandle.xUCSysUSBPort2En.ulOPinId , 1 , gpioGIO_LOW );

    /* switch ON the display */
    vSystemResPowerOn_IntelDisp() ;
}
/*----------------------------------------------------------------------------*/

void vSystemResIntel_ShortPress( void )
{
    TickType_t xDelay = (SystemResINTEL_POWER_SEQ_DELAY_MS / portTICK_PERIOD_MS);

    /* Mimic power button  */
    mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysECPwnBtn.ulGropuId ,
	    prvSystemResHandle.xUCSysECPwnBtn.ulOPinId , 1 , gpioGIO_LOW );

    /* Generate short key press */
    vTaskDelay(xDelay) ;

    mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysECPwnBtn.ulGropuId ,
	    prvSystemResHandle.xUCSysECPwnBtn.ulOPinId , 1 , gpioGIO_HIGH );
}
/*----------------------------------------------------------------------------*/

void vSystemResIntel_LongPress( void )
{
    TickType_t xDelay = (SystemResINTEL_LONG_PRESS_DELAY / portTICK_PERIOD_MS);

    /* Mimic power button  */
    mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysECPwnBtn.ulGropuId ,
	    prvSystemResHandle.xUCSysECPwnBtn.ulOPinId , 1 , gpioGIO_LOW );

    /* Generate Long key press */
    vTaskDelay(xDelay) ;

    mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysECPwnBtn.ulGropuId ,
	    prvSystemResHandle.xUCSysECPwnBtn.ulOPinId , 1 , gpioGIO_HIGH );
}
/*----------------------------------------------------------------------------*/

void vSystemIntelMotion_ShutDown( void )
{
    uint8_t delayCount=0 ;
    uint32_t gpioVal ;
    TickType_t xDelay1s=(SystemResINTEL_SHUTDOWN_WAIT_PERIOD/portTICK_PERIOD_MS);

    /* Assert low bat for shutdown to initiate */
    vSystemResPowerOff_IntelCpu();

    do
    {
	/* wait for 3 seconds for system to shutdown */
	vTaskDelay(xDelay1s) ;
	delayCount++ ;
	gpioVal =  ulSystemResPowerState_Intel() ;
    }while (delayCount<SystemResDELAY_INC_1S_30S_COUNT && gpioVal) ;

    /* Shutdown the Intel Power */
    gpioIntelCpuShutdown();
}
/*----------------------------------------------------------------------------*/

void vSystemResPowerOn_IntelDisp( void )
{
	/* Enable GIO to power on display */
	mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysGio3.ulGropuId ,
		prvSystemResHandle.xUCSysGio3.ulOPinId , 1 , gpioGIO_HIGH );
}
/*----------------------------------------------------------------------------*/

void vSystemResPowerOff_IntelDisp( void )
{
	/* Disable GIO to power off display */
	mml_gpio_write_bit_pattern( prvSystemResHandle.xUCSysGio3.ulGropuId ,
		prvSystemResHandle.xUCSysGio3.ulOPinId , 1 , gpioGIO_LOW );
}
/*----------------------------------------------------------------------------*/

void vSystemResBT_GPIO_Config( void )
{
	mml_gpio_config_t xconfig;
	xconfig.gpio_direction = MML_GPIO_DIR_IN;
	xconfig.gpio_function = MML_GPIO_NORMAL_FUNCTION;
	xconfig.gpio_intr_mode = 0;
	xconfig.gpio_intr_polarity = 0;
	xconfig.gpio_pad_config = MML_GPIO_PAD_NORMAL;
	/* Configuring as input */
	mml_gpio_set_config(prvSystemResHandle.xProximityCheck.ulGropuId,
		prvSystemResHandle.xProximityCheck.ulOPinId, 1, xconfig);
}
/*----------------------------------------------------------------------------*/

void vSystemResBT_GPIO_Status( uint8_t *pucStatus )
{
	/* Get LE GPIO status */
	mml_gpio_pin_input(prvSystemResHandle.xProximityCheck.ulGropuId,
		prvSystemResHandle.xProximityCheck.ulOPinId, pucStatus);
}
/*----------------------------------------------------------------------------*/

void vEnableUSBToIntel( void )
{
    mml_gpio_write_bit_pattern(prvSystemResHandle.xUCSysUSBPort2Sel.ulGropuId,
	    prvSystemResHandle.xUCSysUSBPort2Sel.ulOPinId, 1, gpioGIO_HIGH);

    mml_gpio_write_bit_pattern(prvSystemResHandle.xUCSysUSBPort1Sel.ulGropuId,
	    prvSystemResHandle.xUCSysUSBPort1Sel.ulOPinId, 1, gpioGIO_HIGH);
}
/*----------------------------------------------------------------------------*/
void vDisableUSBToIntel( void )
{
    mml_gpio_write_bit_pattern(prvSystemResHandle.xUCSysUSBPort2Sel.ulGropuId,
	    prvSystemResHandle.xUCSysUSBPort2Sel.ulOPinId, 1, gpioGIO_LOW);

    mml_gpio_write_bit_pattern(prvSystemResHandle.xUCSysUSBPort1Sel.ulGropuId,
	    prvSystemResHandle.xUCSysUSBPort1Sel.ulOPinId, 1, gpioGIO_LOW);
}
/*----------------------------------------------------------------------------*/
