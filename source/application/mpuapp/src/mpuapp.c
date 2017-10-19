/**===========================================================================
 * @file mpuapp.c
 *
 * @brief This application is used for tap and motion detection.
 *
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ============================================================================
 *
 */

/*---------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <mml_i2c.h>
#include <mml_intc.h>
#include <orwl_gpio.h>
#include <mml_gpio.h>
#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>
#include <invensense.h>
#include <invensense_adv.h>
#include <eMPL_outputs.h>
#include <mltypes.h>
#include <mpu.h>
#include <log.h>
#include <packet.h>
#include <mpuapp.h>
#include <mpuinterface.h>
#include <irq.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <event_groups.h>
#include <portable.h>
#include <errors.h>

#include <oob.h>
#include <pinentry.h>
#include <usermode.h>

/**Global variable for pin entry event group.*/
extern xSMAppResources_t xgResource;

/** Hal layer structure*/
struct hal_s {
    uint8_t ucLpAccelMode;	/**<Low power accel mode*/
    uint8_t ucSensors;		/**<Sensor turned on*/
    uint8_t ucDmpOn;		/**<Set to 1 on enabling DMP*/
    volatile uint8_t ucNewGyro;	/**<Updated on new gyro value*/
    uint8_t ucMotionIntMode;	/**<Set 1 to enable motion interrupt mode*/
    uint16_t ucDmpFeatures;	/**<DMP feature*/
};
static struct hal_s xHal = {0}; /**<Hal structure variable*/
/* Don't change the variable name it is used by library*/
/**Key used by MPL library*/
unsigned char *mpl_key = (unsigned char*)"eMPL 5.1";

/**Platform-specific information*/
struct platform_data_s {
    int8_t cOrientation[9]; /**<Gyro orientation matrix*/
};

/* The sensors can be mounted onto the board in any orientation. The mounting
 * matrix seen below tells the MPL how to rotate the raw data from the
 * driver(s).
 * TODO: The following matrices refer to the configuration on internal test
 * boards at Invensense. If needed, please modify the matrices to match the
 * chip-to-body matrix for your particular set up.
 */
static struct platform_data_s xGyroPdata = {
    .cOrientation = { 1, 0, 0,
                     0, 1, 0,
                     0, 0, 1}
};

/* ---------------------------------------------------------------------------*/

/** @brief  prvTapCb interrupt call back handler.
 *
 * @return void.
 */
static void prvMpuappTapCb(void);

/** @brief iMpuMotionGpioConfig configures gpio pin as interrupt line.
 *
 * @return 0 on success and -1 on failure.
 */
static int32_t lMpuappGpioConfig(void);

static int32_t lMpuappGpioConfig(void)
{
    mml_gpio_config_t xGpioConfig;

    xGpioConfig.gpio_direction = MML_GPIO_DIR_IN;
    xGpioConfig.gpio_function = MML_GPIO_NORMAL_FUNCTION;
    xGpioConfig.gpio_pad_config = MML_GPIO_PAD_PULLUP;
    xGpioConfig.gpio_intr_mode = MML_GPIO_INT_MODE_EDGE_TRIGGERED;
    xGpioConfig.gpio_intr_polarity = MML_GPIO_INT_POL_FALLING;

    return mml_gpio_set_config(MML_GPIO_DEV1, gpioINT_MPU6500, 1,
	    xGpioConfig);

}
/*---------------------------------------------------------------------------*/

static void prvMpuappTapCb(void)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;

    /* TODO: check if it is called from thread context or ISR context
     * Using ARM cortex register
     * REF: NFC stack for more details
     * Note:
     * We can have a generic API in future which decides the caller's context
     * dynamically. So both interrupt and thread can call this api's without
     * any change
     */
#ifdef ENABLE_MPU_GIO_INTERRUPT
     /* xHigherPriorityTaskWoken must be initialized to pdFALSE. */
     xHigherPriorityTaskWoken = pdFALSE;

     xResult = xEventGroupSetBitsFromISR(xgResource.xEventGroupPinEntry,
	     pinentryGET_TAP_EVENT,&xHigherPriorityTaskWoken );

     if( xResult != pdFAIL )
       {
   	/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
   	switch should be requested. The macro used is port specific and will
   	be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
   	the documentation page for the port being used. */
   	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
       }
#else	/* polling mode */
	 do
	 {
	     xResult = xEventGroupSetBits( xgResource.xEventGroupPinEntry, pinentryGET_TAP_EVENT );
	 }while(0);
#endif
}
/*---------------------------------------------------------------------------*/

void vMpuappGyroDataReadyCb(void)
{
    int16_t sGyro[3], sAccelShort[3], sSensors;
    uint8_t ucMore;
    int32_t lQuat[4];
    uint32_t ulSensorTimestamp;

    dmp_read_fifo(sGyro, sAccelShort, lQuat, &ulSensorTimestamp, &sSensors, &ucMore);
}
/*---------------------------------------------------------------------------*/

int32_t lMpuappDisableMotion(void)
{
    return mpu_lp_motion_interrupt(0, 0, 0);
}
/*---------------------------------------------------------------------------*/

int32_t lMpuappEnableMotion(void) {
       int32_t lRetVal;

    lRetVal = mpu_lp_motion_interrupt(800, 1, 5);
    if (lRetVal)
    {
	return lRetVal;
    }
    /* Notify the MPL that contiguity was broken. */
    inv_accel_was_turned_off();
    inv_gyro_was_turned_off();
    inv_compass_was_turned_off();
    inv_quaternion_sensor_was_turned_off();
    debugPRINT("Enabled motion configuration\n");
    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/

int32_t lMpuappmain(void)
{
    inv_error_t xResult;
    uint8_t ucAccelFsr;
    uint16_t usGyroRate, usGyroFsr;
    struct int_param_s xIntParam;

    lMpuappGpioConfig();
    xResult = mpu_init(&xIntParam);
    if (xResult)
    {
	return mpuappMPLINITFAILURE;
    }

#ifdef ENABLE_MPU_GIO_INTERRUPT
    /* configure the interrupt*/
    xResult = lIrqDisable(MML_GPIO_DEV1, gpioINT_MPU6500);
    xResult |= lIrqClearInterrupt(MML_GPIO_DEV1, gpioINT_MPU6500);
    xResult |= lIrqSetup(MML_GPIO_DEV1, gpioINT_MPU6500,
 	  MML_GPIO_INT_MODE_EDGE_TRIGGERED, MML_GPIO_INT_POL_FALLING,
	  vMpuappGyroDataReadyCb);
#endif
    if (xResult !=  NO_ERROR)
    {
	return xResult;
    }
    xResult = inv_init_mpl();
    if (xResult)
    {
	return mpuappMPLLIBFAILURE;
    }

    /* Compute 6-axis and 9-axis quaternions. */
    inv_enable_quaternion();
    inv_enable_9x_sensor_fusion();

    inv_enable_fast_nomot();

    /* Update gyro biases when temperature changes. */
    inv_enable_gyro_tc();

    /* Allows use of the MPL APIs in read_from_mpl. */
    inv_enable_eMPL_outputs();

    xResult = inv_start_mpl();
    if (xResult == INV_ERROR_NOT_AUTHORIZED)
    {
	return mpuappNOTAUTH;
    }
    if (xResult)
    {
	return mpuappMPLSTATLIBFAIL;
    }

    /* Get/set hardware configuration. Start gyro. */
    /* Wake up all sensors. */

    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);

    /* Push both gyro and accel data into the FIFO. */
    mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    mpu_set_sample_rate(mpuappDEFAULT_MPU_HZ);

    /* Read back configuration in case it was set improperly. */
    mpu_get_sample_rate(&usGyroRate);
    mpu_get_gyro_fsr(&usGyroFsr);
    mpu_get_accel_fsr(&ucAccelFsr);

    /* Sync driver configuration with MPL. */
    /* Sample rate expected in microseconds. */
    inv_set_gyro_sample_rate(1000000L / usGyroRate);
    inv_set_accel_sample_rate(1000000L / usGyroRate);
    inv_set_gyro_sample_rate(1000000L / usGyroRate);
    inv_set_accel_sample_rate(1000000L / usGyroRate);

    /* Set chip-to-body orientation matrix.
     * Set hardware units to dps/g's/degrees scaling factor.
     */
    inv_set_gyro_orientation_and_scale(
            inv_orientation_matrix_to_scalar(xGyroPdata.cOrientation),
            (long)usGyroFsr<<15);
    inv_set_accel_orientation_and_scale(
            inv_orientation_matrix_to_scalar(xGyroPdata.cOrientation),
            (long)ucAccelFsr<<15);

    /* To initialize the DMP:
     * 1. Call dmp_load_motion_driver_firmware(). This pushes the DMP image in
     *    inv_mpu_dmp_motion_driver.h into the MPU memory.
     * 2. Push the gyro and accel orientation matrix to the DMP.
     * 3. Register gesture callbacks. Don't worry, these callbacks won't be
     *    executed unless the corresponding feature is enabled.
     * 4. Call dmp_enable_feature(mask) to enable different features.
     * 5. Call dmp_set_fifo_rate(freq) to select a DMP output rate.
     * 6. Call any feature-specific control functions.
     *
     * To enable the DMP, just call mpu_set_dmp_state(1). This function can
     * be called repeatedly to enable and disable the DMP at runtime.
     *
     * The following is a short summary of the features supported in the DMP
     * image provided in inv_mpu_dmp_motion_driver.c:
     * DMP_FEATURE_LP_QUAT: Generate a gyro-only quaternion on the DMP at
     * 200Hz. Integrating the gyro data at higher rates reduces numerical
     * errors (compared to integration on the MCU at a lower sampling rate).
     * DMP_FEATURE_6X_LP_QUAT: Generate a gyro/accel quaternion on the DMP at
     * 200Hz. Cannot be used in combination with DMP_FEATURE_LP_QUAT.
     * DMP_FEATURE_TAP: Detect taps along the X, Y, and Z axes.
     * DMP_FEATURE_ANDROID_ORIENT: Google's screen rotation algorithm. Triggers
     * an event at the four orientations where the screen should rotate.
     * DMP_FEATURE_GYRO_CAL: Calibrates the gyro data after eight seconds of
     * no motion.
     * DMP_FEATURE_SEND_RAW_ACCEL: Add raw accelerometer data to the FIFO.
     * DMP_FEATURE_SEND_RAW_GYRO: Add raw gyro data to the FIFO.
     * DMP_FEATURE_SEND_CAL_GYRO: Add calibrated gyro data to the FIFO. Cannot
     * be used in combination with DMP_FEATURE_SEND_RAW_GYRO.
     */
    debugPRINT("loading firmware\n");
    if ( dmp_load_motion_driver_firmware() )
	debugERROR_PRINT("Failed to load firmware\n");
    dmp_set_orientation(
        inv_orientation_matrix_to_scalar(xGyroPdata.cOrientation));
    dmp_register_tap_cb(prvMpuappTapCb);
    /*
     * Known Bug -
     * DMP when enabled will sample sensor data at 200Hz and output to FIFO at the rate
     * specified in the dmp_set_fifo_rate API. The DMP will then sent an interrupt once
     * a sample has been put into the FIFO. Therefore if the dmp_set_fifo_rate is at 25Hz
     * there will be a 25Hz interrupt from the MPU device.
     *
     * There is a known issue in which if you do not enable DMP_FEATURE_TAP
     * then the interrupts will be at 200Hz even if fifo rate
     * is set at a different rate. To avoid this issue include the DMP_FEATURE_TAP
     *
     * DMP sensor fusion works only with gyro at +-2000dps and accel +-2G
     */
    xHal.ucDmpFeatures = DMP_FEATURE_TAP | DMP_FEATURE_GYRO_CAL;
    dmp_set_tap_thresh(TAP_XYZ, mpuappTap_TRESH);
    dmp_set_interrupt_mode(DMP_INT_GESTURE);
    dmp_enable_feature(xHal.ucDmpFeatures);
    dmp_set_fifo_rate(mpuappDEFAULT_MPU_HZ);
#ifdef ENABLE_MPU_GIO_INTERRUPT
    xResult = lIrqEnableInterrupt(MML_GPIO_DEV1, gpioINT_MPU6500);
    if (xResult != NO_ERROR)
    {
	return xResult;
    }
#endif
    mpu_set_dmp_state(1);

    return NO_ERROR;
}
/*---------------------------------------------------------------------------*/
