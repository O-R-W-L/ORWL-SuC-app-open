#===========================================================================
# @file build.mk
#
# @brief This file contains base directories of external stack build
#
# @author ravikiran@design-shift.com
#
#============================================================================
#
# Copyright � Design SHIFT, 2017-2018
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

## header include
include source/ext_stack/NFC/build.mk
include source/ext_stack/CryptoEngine/build.mk
include source/ext_stack/MEMS/build.mk
include source/ext_stack/CyaSSL/build.mk

##All the include directories must go here
ExtStackIncludeDir	+=	$(NFCIncludeDir) \
				$(CRYPTOHeaders) \
				$(MPUIncludeDir) \
				$(CYASSLIncludeDir)

##All the application src directories must go heere
ExtStackSourceDir	+=	$(NFCSourceDir) \
				$(CRYPTOSource) \
				$(MPUSourceDir) \
				$(CYASSLSourceDir)
