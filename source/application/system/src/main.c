/**===========================================================================
 * @file main.c
 *
 * @brief This file contains ORWL hardware setup, peripheral configuration
 * and application initializations.
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
 ============================================================================
 *
 */

/*---------------------------------------------------------------------------*/

/* Global includes */
#include <errors.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* driver includes */
#include <cobra_defines.h>
#include <mml_nvic_regs.h>
#include <mml_gcr_regs.h>
#include <mml_gcr.h>
#include <mml_gpio.h>
#include <mml_uart_regs.h>
#include <mml_uart.h>
#include <mml_sflc.h>

/* application includes */
#include <printf_lite.h>
#include <private.h>
#include <debug.h>
#include <task_config.h>
#include <uart_config.h>
#include <orwl_gpio.h>
#include <orwl_secmon.h>
#include <delay.h>
#include <orwl_oled.h>
#include <orwl_disp_interface.h>
#include <irq.h>
#include <i2c.h>
#include <mpuinterface.h>
#include <Init.h>
#include <mem_common.h>
#include <pinentry.h>
#include <orwl_err.h>

/* RTOS includes */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

/* NFC application header */
#include <nfc_app.h>
#include <nfc_common.h>

/* application header */
#include <systemRes.h>
#include <oob.h>
#include <usermode.h>
#include <rot.h>
#include <tamper.h>

#include "../src/flash.h"
#include <powerbtn.h>
#include <orwl_disp_interface.h>

#if ORWL_PRODUCTION_KEYFOB_SERIAL
#include <nfcprod_test.h>
#endif

/*---------------------------------------------------------------------------*/

/* TODO: remove this or modify this based on use case */
#define	mainHEART_BEAT_TICKS	(10000)	/** system heart beat interval (ticks) */

/*---------------------------------------------------------------------------*/

/** global resource handler for user mode */
xSMAppResources_t xgResource;
/* Flag to check if display init is success */
uint8_t ucGPIODisplayInitDone = 0;

/* variables for to copy NVIC */
extern uint32_t __section_nvic_start;
extern uint32_t __section_nmi_code_start;
extern uint32_t __section_nmi_code_end;
extern uint32_t __section_nmi_code_load_start;

/**
 * Pointer of array string that needs to printed on console for every boot at
 * start.
 */
const char *ccWelcomeStr[] = {
	"     #####      #######      ##     ###     ##   ##",
	"   #########    ########     ##     ###     ##   ##",
	"  ###     ###   ##    ###     ##    # #    ##    ##",
	" ##         ##  ##     ##     ##    # #    ##    ##",
	" ##         ##  ##     ##     ##    # ##   ##    ##",
	" ##         ##  ##    ###     ##   ## ##   ##    ##",
	" ##         ##  ########       ##  #   #  ##     ##",
	" ##         ##  ######         ##  #   #  ##     ##",
	" ##         ##  ##   ##        ##  #   ## ##     ##",
	" ##         ##  ##   ###       ## ##   ## ##     ##",
	"  ##       ##   ##    ##        ####    # #      ##",
	"  ###     ###   ##     ##       ###     ###      ##",
	"   #########    ##      ##      ###     ###      #########",
	"     #####      ##      ###      ##     ###      #########",
	" ==========================================================",
	" ==========================================================",
	"NULL"
};

/**
 * Function declaration
 */
static void prvMainSetupORWL( void );
/*---------------------------------------------------------------------------*/

/** @brief ORWL SuC setup and interface initialization
 *
 * Configures the system clock
 * Resets the interfaces and enables UART
 *
 * @return void
 * 	Never returns if setup fails
 *
 */
static void prvMainSetupORWL( void ) {
	int iresult = COMMON_ERR_UNKNOWN;

       /* Set the system frequency */
       iresult = mml_set_system_frequency(MML_GCR_SYSFREQ_108MHZ);
       if ( iresult )
       {
   	/* Oops, I did it again ... */
	   while(1);
       }

       /* Reset interfaces */
       init_interfaces();

       /* init flash */
       mml_sflc_init();

       /* Initialize UART0 port with default configurations */
       iresult = config_uart();
       if ( iresult )
       {
   	/* Oops, I did it again ... */
	   while(1);
       }

       /* Wait for system to stabilize */
       delayMICRO_SEC(10000);

      /* All is OK. Program will continue to setup tasks */
       return;
}

/*---------------------------------------------------------------------------*/
/** @brief ORWL main function entry point
 *
 * Setup the hardware, initializes the peripherals,
 * creates the necessary threads for the application
 * and starts the RTOS scheduler
 *
 * @return Never returns if freeRTOS scheduler invoked successfully.
 * 		Returns "COMMON_ERR_FATAL_ERROR" on insufficient heap to start the scheduler.
 */
int main( void ) {

	int ierr = NO_ERROR;
	/* Index to display welcome console */
	uint8_t ucIndex = 0;
	volatile uint32_t *p_my_rom_vectors  = (volatile uint32_t*)&__section_nvic_start;
	volatile uint32_t *pSrc;
	volatile uint32_t *pDst;
	/* boot mode */
	uint32_t ulProdCycle = eBOOTMODE_IVALID;

	/** Copy NMI function(s) to internal RAM */
	pSrc = (volatile uint32_t*)&__section_nmi_code_load_start;
	for ( pDst = &__section_nmi_code_start; pDst < &__section_nmi_code_end; )
	{
		*pDst++ = *pSrc++;
	}

	/* ORWL initial setup */
	prvMainSetupORWL( );

	/** Copy the reset vectors from ROM to SRAM. */
	p_my_rom_vectors[2] = (volatile uint32_t)vSecmonDefaultTamperDetectNmi;

	/* Display boot console on boot */
	while(1)
	{
	    /* Compare is string is equal to NULL, if NULL break the while */
	    if (!strcmp((const char *) ccWelcomeStr[ucIndex],(const char *) "NULL"))
	    {
                break;
	    }
	    /* Display on console */
	    debugPLAIN_PRINT("%s\n",ccWelcomeStr[ucIndex]);
	    /* increment count to print next string */
	    ucIndex++;
	}

	/* Display ORWL Software version on console */
	debugPLAIN_PRINT("ORWL Software Version Major:Minor %d:%d\n\r",VERSION_MAJOR,VERSION_MINOR);

	/* Initialization of GPIO */
	vOrwlGpioInit( );
	/* Update the flag, saying GPIO init has been done. This will be
	 * useful in case of tamper NMI where NMI might occur before the GPIO
	 * init has been done. In such cases display should be used only if the
	 * GPIO init has been done. Set BIT 1 for GPIO init done.
	 */
	ucGPIODisplayInitDone = ucGPIODisplayInitDone | initGPIO_INIT_DONE;

	/* Initializing HW */
	vDisplayInterfaceEnable( );

	/* Update the flag, saying display init has been done. This will be
	 * useful in case of tamper NMI where NMI might occur before the display
	 * init has been done. In such cases display should be used only if the
	 * display init has been done. Set BIT 2 for Display init done.
	 */
	ucGPIODisplayInitDone = ucGPIODisplayInitDone | initDISPLAY_INIT_DONE;

	/* Initializing the crypto library */
	vNfcCommonCryptoInit( );

	/* Initialize I2C */
	ierr = lI2cInit();
	if (ierr)
	{
	    debugERROR_PRINT("I2C initialization failed\n");
	    while(1);
	}

	/* Register the interrupt handler */
	ierr = lIrqInit();
	if(ierr)
	{
	    debugERROR_PRINT("Interrupt handler registeration failed\n");
	    while(1);
	}

	//while(1);
	/* Initialize system global resources */
	if( ulSystemResInitGlobalSystemResources( ) != NO_ERROR ) {
		debugERROR_PRINT( "\n Failed to initialize System Resources \n" );
		return COMMON_ERR_FATAL_ERROR;
	}

	/* initialize all user mode global resources */
	if( ulUserModeInitGlobalApplicationResources( &xgResource ) != NO_ERROR ) {
		debugERROR_PRINT(
			"\n Failed to initialize user mode global resources \n" );
		return COMMON_ERR_FATAL_ERROR;
	}
#ifdef ORWL_PRODUCTION_KEYFOB_SERIAL
	if( ulNfcInterafceInit( &xgResource ) != 0 )
	{
		debugERROR_PRINT( "\n Failed to initialize the NFC stack \n" );
		debugERROR_PRINT( "\n It should never happen. Aborting \n" );
		return COMMON_ERR_FATAL_ERROR;
	}

	ierr = xTaskCreate( vNfcProdTestTask , "NFC_test_production" ,
				configSTACK_SIZE_NFC_PROD_TEST , &xgResource,
				ePRIORITY_IDLE_TASK , NULL );
	if( ierr != pdPASS )
	{
		while ( 1 );
	}
#else
	ierr = lInitGetSuCBootMode( &ulProdCycle );
	if(ierr != NO_ERROR)
	{
	    int32_t lRes;
	    debugERROR_PRINT( "\n Failed to read current mode \n" );
	    lRes = lcommonEraseEncyKey();
	    lRes |= lcommonEraseAccessKey();

	    /* erase may or may not pass - ignore return status */
	    /*TODO: update OLED - Error state message */
	    while(1);
	}

	debugPRINT( "\n Boot mode selected %d \n ",ulProdCycle);
	vEnableUSBToIntel();

	switch(ulProdCycle)
	{
	    case eBOOTMODE_DEV_ROT:
		/* Perform system state check */
		ierr = lInitSystemStateCheck(ulProdCycle);
		if(ierr)
		{
		    debugERROR_PRINT("lInitSystemStateCheck Failed");
		    return COMMON_ERR_FATAL_ERROR;
		}

		/* Initiate BIOS Communication and Session Management */
		ierr = lIntelSucComInit();
		if(ierr)
		{
		    debugERROR_PRINT("lIntelSucComInit Failed");
		    return COMMON_ERR_FATAL_ERROR;
		}

		/* GIO PWR BTN */
		vPowerbtnConfig();

		ierr = xTaskCreate(vPowerbtnTsk, "PowerBtnTask",
			configSTACK_SIZE_POWER_BTN_TSK, &xgResource,
			ePRIORITY_IDLE_TASK, NULL);
		if (ierr != pdPASS)
		{
		    debugERROR_PRINT("Failed to create vPowerbtnTsk");
		    while (1);
		}

		ierr = xTaskCreate( vRotModeTask, "RotModetask",
			configSTACK_SIZE_ROT_TSK,
			&xgResource,ePRIORITY_IDLE_TASK , NULL );
		if(ierr != pdPASS)
		{
		    debugPRINT("Failed to create task");
		    while(1);
		}
	    break;
	    case eBOOTMODE_DEV_OOB:
		ierr = lInitSystemStateCheck(ulProdCycle);
		if(ierr)
		{
		    debugERROR_PRINT("lInitSystemStateCheck Failed");
		    return COMMON_ERR_FATAL_ERROR;
		}

		/* GIO PWR BTN */
		vPowerbtnConfig();

		if( ulNfcInterafceInit( &xgResource ) != 0 )
		{
			debugERROR_PRINT( "\n Failed to initialize the NFC stack \n" );
			debugERROR_PRINT( "\n It should never happen. Aborting \n" );
			return COMMON_ERR_FATAL_ERROR;
		}

		ierr = lMpuinterfaceIntializeMpu();
		if (ierr)
		{
		    debugERROR_PRINT("MPU initialization failed");
		    return COMMON_ERR_FATAL_ERROR;
		}
		debugPRINT("MPU initialized\n");

		ierr = xTaskCreate(vPowerbtnTsk, "PowerBtnTask",
			configSTACK_SIZE_POWER_BTN_TSK, &xgResource,
			ePRIORITY_IDLE_TASK, NULL);
		if (ierr != pdPASS)
		{
		    debugERROR_PRINT("Failed to create vPowerbtnTsk");
		    while (1);
		}

		ierr = xTaskCreate( vPinentryTask, "PinentryTask",
			configSTACK_SIZE_PIN_ENTRY_TSK,
			&xgResource,ePRIORITY_IDLE_TASK , NULL );
		if(ierr != pdPASS)
		{
		    debugERROR_PRINT("Failed to create task");
		    while(1);
		}

		ierr = xTaskCreate(vOobModeTask, "oobModeTask",
			configSTACK_SIZE_OOB_MODE_TSK,
			&xgResource,
			ePRIORITY_IDLE_TASK, NULL );
		if(ierr != pdPASS)
		{
		    debugERROR_PRINT("Failed to create task");
		    while(1);
		}

		#ifndef ENABLE_MPU_GIO_INTERRUPT
		    ierr = xTaskCreate(vMpuinterfaceReadFifoTsk, "MpuReadFifoTask",
			    configSTACK_SIZE_MPU_FIFO_READ_TSK,
			    &xgResource, ePRIORITY_IDLE_TASK,
			    &xgResource.xMpuFifoTskHandle);
		    if (ierr != pdPASS)
		    {
			debugPRINT("Failed to create task");
			while (1);
		    }

		    /* keep this task in suspended mode */
		    vTaskSuspend(xgResource.xMpuFifoTskHandle);
		#endif
	    break;
	    case eBOOTMODE_DEV_USER:

		/* TODO: This should be handled in different way in next version
		 * USER mode to be enabled with tamper recovery in future. */
		/* Initiate BIOS Communication and Session Management */
		ierr = lIntelSucComInit();
		if(ierr)
		{
		    debugERROR_PRINT("lIntelSucComInit Failed");
		    return COMMON_ERR_FATAL_ERROR;
		}
		/* Check for device tamper in previous boot. This should be called
		 * after lIntelSucComInit.
		 */
		ierr = lInitSystemStateCheck(ulProdCycle);
		if(ierr)
		{
		    if(ierr == eORWL_ERROR_DEVICE_TAMPERED)
	            {
			 /* We just start Intel subsystem and wait for sharing
			  * the log interface. Intel SuC Communication task is
			  * sufficient for updating Coreboot with required logs.
			  */
			ierr = xTaskCreate( vTamperTamperModeTask , "TamperMode" ,
				configSTACK_SIZE_TAMPER_MODE_TASK , &xgResource,
				ePRIORITY_IDLE_TASK , NULL );
			if( ierr != pdPASS )
			{
			    while ( 1 );
			}
	            }
		    else
		    {
			debugERROR_PRINT("lInitSystemStateCheck Failed");
			return COMMON_ERR_FATAL_ERROR;
		    }
		}
		/* if lInitSystemStateCheck is success then create user mode */
		else
		{
		    /* continue if check succeeds */
		    /*vInitDisplayUserModeWelcomeScreen();*/

		    /* GIO PWR BTN */
		    vPowerbtnConfig();

		    if( ulNfcInterafceInit( &xgResource ) != 0 )
		    {
			    debugERROR_PRINT( "\n Failed to initialize the NFC stack \n" );
			    debugERROR_PRINT( "\n It should never happen. Aborting \n" );
			    return COMMON_ERR_FATAL_ERROR;
		    }

		    ierr = lMpuinterfaceIntializeMpu();
		    if (ierr)
		    {
			debugPRINT("MPU initialization failed");
			return COMMON_ERR_FATAL_ERROR;
		    }

		    debugPRINT("MPU initialized\n");

		    ierr = xTaskCreate( vPinentryTask, "PinentryTask",
			    configSTACK_SIZE_PIN_ENTRY_TSK,
			    &xgResource,ePRIORITY_IDLE_TASK , NULL );
		    if(ierr != pdPASS)
		    {
			debugERROR_PRINT("Failed to create task");
			while(1);
		    }

		    #ifndef ENABLE_MPU_GIO_INTERRUPT
			ierr = xTaskCreate(vMpuinterfaceReadFifoTsk, "MpuReadFifoTask",
				configSTACK_SIZE_MPU_FIFO_READ_TSK,
				&xgResource, ePRIORITY_IDLE_TASK,
				&xgResource.xMpuFifoTskHandle);
			if (ierr != pdPASS)
			{
			    debugERROR_PRINT("Failed to create task");
			    while (1);
			}

			/* keep this task in suspended mode */
			vTaskSuspend(xgResource.xMpuFifoTskHandle);
		    #endif

		    ierr = xTaskCreate(vPowerbtnTsk, "PowerBtnTask",
			    configSTACK_SIZE_POWER_BTN_TSK, &xgResource,
			    ePRIORITY_IDLE_TASK, NULL);
		    if (ierr != pdPASS)
		    {
			debugERROR_PRINT("Failed to create vPowerbtnTsk");
			while (1);
		    }

		    ierr = xTaskCreate( vUserModeUserModeTask , "UserMode" ,
			    configSTACK_SIZE_USER_MODE_TSK , &xgResource,
			    ePRIORITY_IDLE_TASK , NULL );
		    if( ierr != pdPASS )
		    {
			    while ( 1 );
		    }
		}
	    break;
	    case eBOOTMODE_DEV_USER_TAMPER:
		/* Initiate BIOS Communication and Session Management */
		ierr = lIntelSucComInit();
		if(ierr)
		{
		    debugERROR_PRINT("lIntelSucComInit Failed");
		    return COMMON_ERR_FATAL_ERROR;
		}
		/* We just start Intel subsystem and wait for sharing
		 * the log interface. Intel SuC Communication task is
		 * sufficient for updating Coreboot with required logs.
		 */
		ierr = xTaskCreate( vTamperTamperModeTask , "TamperMode" ,
			configSTACK_SIZE_TAMPER_MODE_TASK , &xgResource,
			ePRIORITY_IDLE_TASK , NULL );
		if( ierr != pdPASS )
		{
		    while ( 1 );
		}
		break;
	    case eBOOTMODE_DEV_ERROR:
		/* Display tampered Logo */
		delayMICRO_SEC(100000);
		/* For displaying image on OLED */
		/*vInitDisplayTamperScreen();*/
	    /* break; */
	    default:
		while(1);
	    break;
	}
#endif
	/* Start the scheduler. */
	vTaskStartScheduler( );

	/* Will only get here if there was insufficient heap to start the
	 scheduler or vTaskEndScheduler() is called*/
	return COMMON_ERR_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*/
/* EOF */
