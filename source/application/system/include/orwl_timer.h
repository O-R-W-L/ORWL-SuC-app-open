/**===========================================================================
 * @file orwl_timer.h
 *
 * @brief This file contains macros and decleration related to timer used by
 * application.
 *
 * @author viplav.roy@design-shift.com
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
#ifndef _TIMER_H_
#define _TIMER_H_

#include <FreeRTOS.h>
#include <mml_tmr.h>
#include <event_groups.h>

#include <events.h>
#include <usermode.h>

/* Structure to hold data regarding initially used timer id */
typedef struct
{
	uint32_t ulUsedTimerList;
}timerUsedList_t;

#define timerLE_TIMER_ID			(MML_TMR_DEV0)	/**< Le seed update timer id */
#define timerPROXIMITY_TIMER_ID			(MML_TMR_DEV1)	/**< 24 hrs proximity timer id */
#define timerONE_SEC_TIMEOUT			(54000000)	/**< Timer compare value for one second timeout */
/* NOTE: clock = MML_TMR_PRES_DIV_64 and timeout = 1 * 54000000 gives 64 sec timer */
/**< 15 mins Le timeout value = (15mins * 60secs)/64 = 14.0625 */
#define timerLE_TIMOUT_VALUE			(14)
#define timerLE_TIMEOUT				(timerLE_TIMOUT_VALUE *\
						timerONE_SEC_TIMEOUT )
/**< 23:59:00 proximity timeout value using 4096 prescaler = 86340secs/4096 = 21.079101 */
#define timerPROXIMITY_TIMOUT_VALUE		(21)
#define timerPROXIMITY_TIMEOUT			(timerPROXIMITY_TIMOUT_VALUE *\
						timerONE_SEC_TIMEOUT )

/**
 * @brief This function is the wrapper function used for initializing Le timer.
 *
 * @param None
 *
 * @return NO_ERROR on success or error code
 */
int32_t lTimerLeTmrInit( void );

/**
 * @brief This function is the wrapper function used for initializing 24 hr
 * proximity timeout timer.
 *
 * @param None
 *
 * @return NO_ERROR on success or error code
 */
int32_t lTimerProximityTimeoutTmrInit( void );

/**
 * @brief This function is the wrapper function used for reading the count value
 * of given timer.
 *
 * @param eTimer_id     Timer Id
 * @param pulCountValue Pointer to get timer id count value
 *
 * @return NO_ERROR on success or error code
 */
int32_t lTimerRead( mml_tmr_id_t eTimer_id, uint32_t * pulCountValue );

/**
 * @brief This function is for initializing the timer.
 *
 * @param pxConfig pointer to timer configuration structure
 * @param eTimerId LE timer id
 *
 * @return NO_ERROR on success or error code
 */
int32_t lTimerInit( mml_tmr_config_t *pxConfig, mml_tmr_id_t eTimerId );

/**
 * @brief This function is used to disable the timer.
 *
 * @param eTimerId Le timer id
 *
 * @return NO_ERROR on success or error code
 */
int32_t lTimerClose( mml_tmr_id_t eTimerId );
#endif /* _TIMER_H_ */
