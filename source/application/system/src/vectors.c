/******************************************************************************
 * Copyright (C) 2012 Maxim Integrated Products, All Rights Reserved.
 *
 * This software is protected by copyright laws of the United States and
 * of foreign countries. This material may also be protected by patent laws
 * and technology transfer regulations of the United States and of foreign
 * countries. This software is furnished under a license agreement and/or a
 * nondisclosure agreement and may only be used or reproduced in accordance
 * with the terms of those agreements. Dissemination of this information to
 * any party or parties not specified in the license agreement and/or
 * nondisclosure agreement is expressly prohibited.
 *
 *     Description: AM62 Startup Code
 *             $Id: vectors.c.rca 1.3 Thu Jun  7 13:56:41 2012 jerbrodt Experimental $
 *
 ******************************************************************************
 */

/** Global includes */
#include <config.h>
/** Other includes */
#include <arch/cortex-m3/nvic.h>
/** Local includes */

/* FreeRTOS includes. */
extern void xPortPendSVHandler( void ) __attribute__ (( naked ));
extern void xPortSysTickHandler( void );
extern void vPortSVCHandler( void ) __attribute__ (( naked ));

//*****************************************************************************
extern void __reset(void);

//*****************************************************************************
// The following is a construct created by the linker, indicating where the
// the stack resides in memory.
//*****************************************************************************
extern void __section_end_stack;

__attribute__ ((section (".text.init"))) void handler_fault(void)
{
	/** We're done */
	while(1);
}

//*****************************************************************************
// The vector table.  Note that the proper constructs must be placed on this to
// ensure that it ends up at physical address 0x0000.0000.
//*****************************************************************************
__attribute__ ((section(".nvic_section")))
void (* const rom_vectors[])(void) =
{
    &__section_end_stack,                   // The initial stack pointer
    __reset,                                // The reset handler
    handler_fault,                          // The NMI handler
    handler_fault,                        // The hard fault handler
    handler_fault,                    // The MPU fault handler
    handler_fault,                    // The bus fault handler
    handler_fault,                    // The usage fault handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    vPortSVCHandler,                    // SVCall handler
    handler_default_debug_monitor_,                    // Debug monitor handler
    0,                                      // Reserved
    xPortPendSVHandler,                    // The PendSV handler
    xPortSysTickHandler,                    // The SysTick handler
    handler_default_undefined_,                    // Interrupt #16
    handler_default_undefined_,                    // Interrupt #17
    handler_default_undefined_,                    // Interrupt #18
    handler_default_undefined_,                    // Interrupt #19
    handler_default_undefined_,                    // Interrupt #20
    handler_default_undefined_,                    // Interrupt #21
    handler_default_undefined_,                    // Interrupt #22
    handler_default_undefined_,                    // Interrupt #23
    handler_default_undefined_,                    // Interrupt #24
    handler_default_undefined_,                    // Interrupt #25
    handler_default_undefined_,                    // Interrupt #26
    handler_default_undefined_,                    // Interrupt #27
    handler_default_undefined_,                    // Interrupt #28
    handler_default_undefined_,                    // Interrupt #29
    handler_default_undefined_,                    // Interrupt #30
    handler_default_undefined_,                    // Interrupt #31
    handler_default_undefined_,                    // Interrupt #32
    handler_default_undefined_,                    // Interrupt #33
    handler_default_undefined_,                    // Interrupt #34
    handler_default_undefined_,                    // Interrupt #35
    handler_default_undefined_,                    // Interrupt #36
    handler_default_undefined_,                    // Interrupt #37
    handler_default_undefined_,                    // Interrupt #38
    handler_default_undefined_,                    // Interrupt #39
    handler_default_undefined_,                    // Interrupt #40
    handler_default_undefined_,                    // Interrupt #41
    handler_default_undefined_,                    // Interrupt #42
    handler_default_undefined_,                    // Interrupt #43
    handler_default_undefined_,                    // Interrupt #44
    handler_default_undefined_,                    // Interrupt #45
    handler_default_undefined_,                    // Interrupt #46
    handler_default_undefined_,                    // Interrupt #47
    handler_default_undefined_,                    // Interrupt #48
    handler_default_undefined_,                    // Interrupt #49
    handler_default_undefined_,                    // Interrupt #50
    handler_default_undefined_,                    // Interrupt #51
    handler_default_undefined_,                    // Interrupt #52
    handler_default_undefined_,                    // Interrupt #53
    handler_default_undefined_,                    // Interrupt #54
    handler_default_undefined_,                    // Interrupt #55
    handler_default_undefined_,                    // Interrupt #56
    handler_default_undefined_,                    // Interrupt #57
    handler_default_undefined_,                    // Interrupt #58
    handler_default_undefined_,                    // Interrupt #59
    handler_default_undefined_,                    // Interrupt #60
    handler_default_undefined_,                    // Interrupt #61
    handler_default_undefined_,                    // Interrupt #62
    handler_default_undefined_,                    // Interrupt #63
};

/******************************************************************************/
/* EOF */
