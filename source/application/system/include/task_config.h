/**===========================================================================
 * @file task_config.h
 *
 * @brief This file contains the parameters used to configure
 * various task in ORWL. For example, it includes task priority,
 * delay, stack size etc for each task
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

#ifndef INCLUDE_TASK_CONFIG_H_
#define INCLUDE_TASK_CONFIG_H_

/*---------------------------------------------------------------------------*/

/**
 * task priority enum
 *
 * Task priority level definitions used to create the tasks
 */
enum taskPriority
    {
    ePRIORITY_DEFUALT = 0, 	/** default priority level */
    ePRIORITY_APPLICATION = 1, 	/** All tasks in application go with this priority, unless specified otherwise */
    ePRIORITY_IDLE_TASK = 2, 	/** idle task */
    };

/*---------------------------------------------------------------------------*/

/* Stack reservation for each task in application */
#define configSTACK_SIZE_HEART_BEAT		(1024)	/**< HeartBeat task stack size */
#define configSTACK_SIZE_USER_MODE_TSK		(1024)	/**< User mode task stack size */
#define configSTACK_SIZE_OOB_MODE_TSK  		(1024)	/**< Oob mode task stack size */
#define configSTACK_SIZE_PIN_ENTRY_TSK		(1024)	/**< Pin entry task stack size */
#define configSTACK_SIZE_MPU_FIFO_READ_TSK  	(256)	/**< Oob mode task stack size */
#define configSTACK_SIZE_ROT_TSK		(1024)	/**< Rot Mode Task */
#define configSTACK_SIZE_POWER_BTN_TSK		(256)	/**< Power Btn Task */
#define configSTACK_SIZE_NFC_PROD_TEST		(1024)	/**< Nfc Production test task stack size*/
#define configSTACK_SIZE_INTEL_SUC_MANAGE_DATA  (512)	/**< Managing received data task */
#define configSTACK_SIZE_TAMPER_MODE_TASK       (512)  /**< Managing received data task */
/*---------------------------------------------------------------------------*/
#endif /* INCLUDE_TASK_CONFIG_H_ */
