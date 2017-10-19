#===========================================================================
# @file build.mk
#
# @brief This file contains base directories of application that is required
#	for build
#
# @author ravikiran@design-shift.com
#
#============================================================================
#
# Copyright © Design SHIFT, 2017-2018
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#	Redistributions of source code must retain the above copyright.
#	Neither the name of the [ORWL] nor the
#	names of its contributors may be used to endorse or promote products
#	derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY DESIGN SHIFT ''AS IS'' AND ANY
#  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL DESIGN SHIFT BE LIABLE FOR ANY
#  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#============================================================================

##All the include directories must go here
HeadrAppDir		= $(srcbase)/application/system/include
HeadrSecMonDir 	= $(srcbase)/application/secure_monitor/include
NfcHeadrDir 	= $(srcbase)/application/nfc_interface/include
TrngHeadrDir 	= $(srcbase)/application/trng/include
SuCAppHeadrDir 	= $(srcbase)/application/app/include
SuCAppIncRot 		= $(srcbase)/application/app/include/rot
SuCAppIncOob 		= $(srcbase)/application/app/include/oob
SuCAppIncUser 		= $(srcbase)/application/app/include/user
SuCAppIncTamp       = $(srcbase)/application/app/include/tamper
PinHandlingHeadrDir = $(srcbase)/application/app/include/pinhandling
HeadrDisplay 		= $(srcbase)/application/display/include
MpuHeadDir		= $(srcbase)/application/mpuapp/include
MemHeadDir		= $(srcbase)/application/mem/include
PowerBtnHdrDir	= $(srcbase)/application/powerbtn/include
HeadrPRNG 		= $(srcbase)/application/prng/include
LeHeadrDir		= $(srcbase)/application/le_integration/include
IntelSucCommHeadrDir 	= $(srcbase)/application/intel_suc_comm/include

##All the application src directories must go here
SrcAppDir		= $(srcbase)/application/system/src
SrcSecMonDir		= $(srcbase)/application/secure_monitor/src
NfcSrcDir		= $(srcbase)/application/nfc_interface/src
TrngSrcDir 		= $(srcbase)/application/trng/src
SuCAppSrcDir 	= $(srcbase)/application/app/src
SuCAppSrcRot 		= $(srcbase)/application/app/src/rot
SuCAppSrcOob 		= $(srcbase)/application/app/src/oob
SuCAppSrcUser 		= $(srcbase)/application/app/src/user
SuCAppSrcTamp       = $(srcbase)/application/app/src/tamper
SuCAppSrcPinHandling = $(srcbase)/application/app/src/pinhandling
SrcDisplay		= $(srcbase)/application/display/src
MpuSrcDir		= $(srcbase)/application/mpuapp/src
MemSrcDir		= $(srcbase)/application/mem/src
PowerBtnSrcDir		= $(srcbase)/application/powerbtn/src
SrcPRNG		= $(srcbase)/application/prng/src
LeSrcDir		= $(srcbase)/application/le_integration/src
IntelSucCommSrcDir 	= $(srcbase)/application/intel_suc_comm/src

