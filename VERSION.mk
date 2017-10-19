#
# VERSION.mk -- Project version number
#
# Copyright (c) 2014, Maxim Integrated Products
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the Maxim Integrated nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY MAXIM INTEGRATED ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# [INTERNAL] ===================================================================
#
# Created on: Apr 28, 2014
#     Author: Yann G. <yann.gaude@maxim-ic.com>
#
# ---- Subversion keywords
# $Rev: 4371 $: Revision of last commit
# $Author: yann.gaude $: Author of last commit
# $Date: 2014-11-13 10:06:17 +0100 (Thu, 13 Nov 2014) $: Date of last commit
#
# [/INTERNAL] ==================================================================

VERSION_MAJOR = 9
VERSION_MINOR = 18
VERSION_PATCH = 0

## -----------------------------------------------------------------------------
# !!DO NOT MODIFY!!
# ------------------------------------------------------------------------------
# This revision number is not accurate it is overwritten by using 'svn info' 
SVN_REVISION  := $(strip $(subst ",,$(subst \ev: ,,"\$Rev: $(Nothing)")))
-include $(toolsDir)/VERSION.mk
-include $(buildDir)/SVN-INFO.mk
