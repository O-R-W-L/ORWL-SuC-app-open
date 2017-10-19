#===========================================================================
# @file build.mk
#
# @brief This file contains base directories of Ramtex graphics library.
#
# @author aakash.sarkar@design-shift.com
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
CYASSL_LIB_PATH = $(srcbase)/ext_stack/CyaSSL

##All the include directories must go here
CYASSLIncludeDir	+=	$(CYASSL_LIB_PATH)

CYASSLSourceDir		+=	$(CYASSL_LIB_PATH)/ctaocrypt/src