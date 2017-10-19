/**===========================================================================
 * @file check_tamper.h
 *
 * @brief This file contains routines for determining if device tampered in
 * previously (either in last boot or when device power was off)
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
#ifndef _INCLUDE_CHECK_TAMPER_H_
#define _INCLUDE_CHECK_TAMPER_H_
/*
 * @brief this enum defines tamper check values to identify whether device
 * tampered or not
 */
typedef enum xDEV_TAMPERE_STATE
{
	eDEV_NORMAL	= 0x0,
	eDEV_TAMPERED,
}xDevtamperState;

/**
 * @brief This function checks if the device was tampered in last boot or when
 * ORWL power supply is OFF
 *
 * @return xDevtamperState enum
 * 			returns eDEV_NORMAL if device untampered
 * 			returns eDEV_TAMPERED if device tampered
 */
xDevtamperState xTamperIsDevTampered( void );
#endif //_INCLUDE_CHECK_TAMPER_H_
