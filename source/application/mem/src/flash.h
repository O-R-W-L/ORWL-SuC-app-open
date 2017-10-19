 /**===========================================================================
 * @file flash.h
 *
 * @brief This file contains all the data structures and macros to define ORWL
 * flash layout
 *
 * @author ravikiran.hv@design-shift.com
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

#ifndef falshINCLUDE_FLASH_H_
#define flashINCLUDE_FLASH_H_

/* Global includes */
#include <mq55/arch/mml.h>

/*---------------------------------------------------------------------------*/
/** ORWL SuC SoC flash size */
#define flashPAGE_SIZE			(4096)	/** 4KB page size */

/** ORWL page allocation in flash partition-1*/
/** Flash region to store firmware */
#define flashFIRMWARE_NUM_PAGES		(160)
/** Reserved */
#define flashRESERVED0_NUM_PAGES	(18)
/** manufacturer data */
#define flashMANUFACT_DATA_NUM_PAGES	(2)
/** access keys & pins */
#define flashACCESS_KEY_NUM_PAGES	(2)
/** encryption keys */
#define flashENC_KEY_NUM_PAGES		(2)
/** keyFob ID */
#define flashKEYFOB_ID_NUM_PAGES	(4)
/** tamper history */
#define flashTAMP_HISTORY_NUM_PAGES	(2)
/** user config */
#define flashUSER_CONFIG_NUM_PAGES	(2)
/** Reserved */
#define flashRESERVED1_NUM_PAGES	(2)

/** ORWL partition size of each individual partition */

/** define application firmware size */
#define flashFIRMWARE_SIZE		(flashFIRMWARE_NUM_PAGES * flashPAGE_SIZE)
/** defines reserved region0 */
#define flashRESERVED0_SIZE		(flashRESERVED0_NUM_PAGES * flashPAGE_SIZE)
/** defines manufact. data size */
#define flashMANUFACT_DATA_SIZE		(flashMANUFACT_DATA_NUM_PAGES * flashPAGE_SIZE)
/** defines acess pin and key area size */
#define flashACCESS_KEY_SIZE		(flashACCESS_KEY_NUM_PAGES * flashPAGE_SIZE)
/** defines SHIFT a=encryption key area */
#define flashENC_KEY_SIZE		(flashENC_KEY_NUM_PAGES * flashPAGE_SIZE)
/** defines keyfob id allocated size */
#define flashKEYFOB_ID_SIZE		(flashKEYFOB_ID_NUM_PAGES * flashPAGE_SIZE)
/** defines tamper history allocated size */
#define flashTAMP_HIST_SIZE		(flashTAMP_HISTORY_NUM_PAGES * flashPAGE_SIZE)
/** defines user config allocated size */
#define flashUSER_CONFIG_SIZE		(flashUSER_CONFIG_NUM_PAGES * flashPAGE_SIZE)
/** defines reserved region1 size */
#define flashRESERVED1_SIZE		(flashRESERVED1_NUM_PAGES * flashPAGE_SIZE)

/** OFFSET calculation for each section of FIRST partition [first 512KB] */

/** flash start offset, this needs to be added with MML_MEM_FLASH_BASE to get
 * actual start address where data can be written. This applies for all
 * the offset
 */
#define flashFIRMWARE_START_ADDR	(MML_MEM_FLASH_BASE)
/** start of reserved0 region */
#define flashRESERVED0_START_ADDR	(flashFIRMWARE_START_ADDR  + flashFIRMWARE_SIZE)
/** manufacturer data store offset */
#define flashMANUFACT_DATA_START_ADDR	(flashRESERVED0_START_ADDR + flashRESERVED0_SIZE)
/** access key store offset */
#define flashACCESS_KEY_START_ADDR	(flashMANUFACT_DATA_START_ADDR + flashMANUFACT_DATA_SIZE)
/** encryption key store offset */
#define flashENC_KEY_START_ADDR		(flashACCESS_KEY_START_ADDR + flashACCESS_KEY_SIZE)
/** keyfobID store offset */
#define flashKEYFOB_ID_START_ADDR	(flashENC_KEY_START_ADDR + flashENC_KEY_SIZE)
/** tamper history storing offset */
#define flashTAMP_HIST_START_ADDR	(flashKEYFOB_ID_START_ADDR + flashKEYFOB_ID_SIZE)
/** user config offset */
#define flashUSER_CONFIG_START_ADDR	(flashTAMP_HIST_START_ADDR + flashTAMP_HIST_SIZE)
/** reserved1 region offset */
#define flashRESERVED1_START_ADDR	(flashUSER_CONFIG_START_ADDR + flashUSER_CONFIG_SIZE)
/** second partition layout [512KB to 1MB offset] */

#endif /* flashINCLUDE_FLASH_H_ */
