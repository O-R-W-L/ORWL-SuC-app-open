#===========================================================================
# @file build.mk
#
# @brief This file contains base directories of NXP NFC stack
#
# @author vikram.k@design-shift.com
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

#MPU6500 stack path [used locally]
MPU_PATH = $(srcbase)/ext_stack/MEMS/MPU6500

##All the include directories must go here
MPUIncludeDir=$(MPU_PATH)/arm/MPU6500/Projects/eMD6/core/driver/eMPL \
			$(MPU_PATH)/arm/MPU6500/Projects/eMD6/core/driver/include \
			$(MPU_PATH)/arm/MPU6500/Projects/eMD6/core/driver/orwl \
			$(MPU_PATH)/arm/MPU6500/Projects/eMD6/core/eMPL-hal \
			$(MPU_PATH)/arm/MPU6500/Projects/eMD6/core/mllite \
			$(MPU_PATH)/arm/MPU6500/Projects/eMD6/core/mpl

##All the application core directories must go here
MPUSourceDir=$(MPU_PATH)/arm/MPU6500/Projects/eMD6/core/driver/eMPL \
			$(MPU_PATH)/arm/MPU6500/Projects/eMD6/core/driver/include \
			$(MPU_PATH)/arm/MPU6500/Projects/eMD6/core/driver/orwl \
			$(MPU_PATH)/arm/MPU6500/Projects/eMD6/core/eMPL-hal \
			$(MPU_PATH)/arm/MPU6500/Projects/eMD6/core/mllite \
			$(MPU_PATH)/arm/MPU6500/Projects/eMD6/core/mpl

LIBS_MPU += mplmpu m
MPU_LIB_PATH = $(srcbase)/ext_stack/MEMS/MPU6500/mpllib/arm/gcc/4.9.3
