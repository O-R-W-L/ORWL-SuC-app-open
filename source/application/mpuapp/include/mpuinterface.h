/**===========================================================================
 * @file mpuinterface.h
 *
 * @brief This file contains function prototype for mpu operation.
 *
 * @author vikram.k@design-shift.com
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
 ============================================================================
 *
 */

#ifndef MPU_INTERFACE_H
#define MPU_INTERFACE_H
#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <event_groups.h>

#ifndef ENABLE_MPU_GIO_INTERRUPT
#define mpuinterfaceFIFOREAD_DELAY 	(100)	/**< Task Delay for Read MPU FIFO */
#endif

/** @brief lIntializeMpu initializes the mpu.
 *
 * @return 0 on success and error code on failure.
 */
int32_t lMpuinterfaceIntializeMpu(void);

/** @brief lEnableDmp enables the dmp.
 *
 * @return 0 on success and error code on failure.
 */
int32_t lMpuinterfaceEnableDmp(void);

/** @brief lEnableDmp disables dmp.
 *
 * @return 0 on success and error code on failure.
 */
int32_t lMpuinterfaceDisableDmp(void);

/** @brief Enables Motion detection
 *
 * @return 0 on success and error code on failure.
 */
int32_t lMpuinterfaceEnableMotionDetection(void);

/** @brief disables motion detection
 *
 * @return 0 on success and error code on failure.
 */
int32_t lMpuinterfaceDisableMotionDetection(void);

/** @brief This function reads the motion interrupt register and checks
 * if device motion detected. This function need to be called by the
 * application whenever there is a need to check for motion.
 *
 * @return pdTRUE on motion detection and pdFALSE if no motion detected
 */
BaseType_t xMpuinterfaceIsMotionDetected( void );

#ifndef ENABLE_MPU_GIO_INTERRUPT
/* @brief This task polls for the MPU FIFO data for every 100 ms.
 *
 * It Waits for the on the start command (event) from the supervisor.
 * On receiving the START event, it will start polling for FIFO data.
 * On receiving the STOP event, it will stop the polling.
 * This is a workaround to interrupt mode problem in freeRTOS.
 * after fixing the interrupt mode problem this thread is no more required.
 *
 * @return void
 */
void vMpuinterfaceReadFifoTsk(void *vArgs);

#endif
#endif /* MPU_INTERFACE_H */
