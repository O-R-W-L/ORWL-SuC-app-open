#===========================================================================
# @file build.mk
#
# @brief This file contains base directories of NXP NFC stack
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

#NFC stack path [used locally]
NFC_PATH = $(srcbase)/ext_stack/NFC

##All the include directories must go here
NFCIncludeDir	=	$(NFC_PATH)/NxpNfcRdLib/comps/phacDiscLoop/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phalMfdf/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phalMfdf/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phalTop/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phceCidManager/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phhalHw/src/Rc663 \
					$(NFC_PATH)/NxpNfcRdLib/comps/phLog/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phNfcLib/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phnpSnep/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phOsal/src/Freertos \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p3a/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p3b/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p4/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p4a/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p4mC/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalMifare/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalMifare/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phPlatform/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phPlatform/src/Port/max32550 \
					$(NFC_PATH)/NxpNfcRdLib/intfs \
					$(NFC_PATH)/NxpNfcRdLib/types \
					$(NFC_PATH)/NxpNfcRdLib/comps/phbalReg/src/MaxSPI

##All the application src directories must go heere
NFCSourceDir	=	$(NFC_PATH)/NxpNfcRdLib/comps/phacDiscLoop/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phacDiscLoop/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phalMfdf/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phalMfdf/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phCidManager/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phCidManager/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phhalHw/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phhalHw/src/Rc663 \
					$(NFC_PATH)/NxpNfcRdLib/comps/phLog/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phnpSnep/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phnpSnep/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phOsal/src/Freertos \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p3a/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p3a/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p3b/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p3b/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p4/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p4/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p4a/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p4a/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p4mC/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalI14443p4mC/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalMifare/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phpalMifare/src/Sw \
					$(NFC_PATH)/NxpNfcRdLib/comps/phPlatform/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phPlatform/src/Port/max32550 \
					$(NFC_PATH)/NxpNfcRdLib/comps/phTools/src \
					$(NFC_PATH)/NxpNfcRdLib/comps/phbalReg/src/MaxSPI \
					$(NFC_PATH)/NxpNfcRdLib/comps/phNfcLib/src
