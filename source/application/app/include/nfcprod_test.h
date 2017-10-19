/**===========================================================================
 * @file nfcprod_test.h
 *
 * @brief This header file contains function prototypes used in nfcprod_test.c
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

#if ORWL_PRODUCTION_KEYFOB_SERIAL

#ifndef _NFCPROD_TEST_H_
#define _NFCPROD_TEST_H_

#include <FreeRTOS.h>
#include <usermode.h>
#include <events.h>

/**
 * @brief This is a test thread. This thread is used in the production build for
 * getting the Keyfob/peer's serial key over NFC.
 *
 * @param pvArg void pointer containing resource data.
 *
 * @return void
 */
void vNfcProdTestTask(void *pvArg);
#endif /* _NFCPROD_TEST_H_ */
#endif /* ORWL_PRODUCTION_KEYFOB_SERIAL */
