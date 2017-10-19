#===========================================================================
# @file build.mk
#
# @brief This file contains base directories of each module src and header.
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

## inlcude all the build configs
include source/application/build.mk
include source/ext_stack/build.mk

#Display Library
LIBS_DISPLAY += ramtexlib

##ORWL Configuration
ORWL_CONFIGS    =	-DORWL_EVT3 \
			-DGHW_SINGLE_CHIP \
			-DINTEL_DEBUG_SPI_ELIMINATE \
			-DENABLE_EXTERNAL_SENSOR \
			-DENABLE_INTERNAL_SENSOR
#			-DBLE_SEED_KEYFOB_SERIAL
#			-DROT_PRINT_DEF_PIN
#			-DENABLE_BLE
#			-DORWL_PRODUCTION_KEYFOB_SERIAL \
#			-DDEBUG_TAMPER

##All the include directories must go here
AppIncludeDir	+= 	$(HeadrAppDir) \
                   	$(ExtStackIncludeDir) \
                   	$(HeadrSecMonDir) \
			$(NfcHeadrDir)	\
			$(TrngHeadrDir) \
			$(SuCAppHeadrDir) \
			$(SuCAppIncRot)\
			$(SuCAppIncOob) \
			$(SuCAppIncUser) \
			$(SuCAppIncTamp)\
			$(PinHandlingHeadrDir)\
			$(HeadrDisplay) \
			$(MpuHeadDir) \
			$(MemHeadDir) \
			$(PowerBtnHdrDir) \
			$(HeadrPRNG) \
			$(LeHeadrDir) \
			$(IntelSucCommHeadrDir)

##All the application src directories must go here
AppSourceDir	+=	$(SrcAppDir) \
                  	$(ExtStackSourceDir)\
                  	$(SrcSecMonDir) \
			$(NfcSrcDir) \
			$(TrngSrcDir) \
			$(SuCAppSrcDir) \
			$(SuCAppSrcRot) \
			$(SuCAppSrcOob) \
			$(SuCAppSrcUser) \
			$(SuCAppSrcTamp) \
			$(SuCAppSrcPinHandling) \
			$(SrcDisplay) \
			$(MpuSrcDir) \
			$(MemSrcDir) \
			$(PowerBtnSrcDir) \
			$(SrcPRNG) \
			$(LeSrcDir) \
			$(IntelSucCommSrcDir)
