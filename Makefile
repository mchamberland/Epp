###############################################################################
#
# Makefile, Version 1.4.3 Sat 09 Jul 2011
#
# -----------------------------------------------------------------------------
#
# Epp (Easy particle propagation)
# Copyright (C) 2011 CancerCare Manitoba
#
# The latest version of Epp and additional information are available online at 
# http://www.physics.umanitoba.ca/~elbakri/epp/
#
# Epp is based on the EGSnrc C++ class library which is available online at
# http://www.irs.inms.nrc.ca/EGSnrc/pirs898/
#
# Epp is free software; you can redistribute it and/or modify it under the 
# terms of the GNU General Public License as published by the Free Software 
# Foundation; either version 2 of the License, or (at your option) any later
# version.                                       
#                                                                           
# Epp is distributed in the hope that it will be useful, but WITHOUT ANY 
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
# details.                              
#                                                                           
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# -----------------------------------------------------------------------------
#
#   Contact:
#
#   Jonas Lippuner
#   CancerCare Manitoba, Department of Medical Physics
#   675 McDermot Ave, Winnipeg, Manitoba, R3E 0V9, Canada
#   Email: jonas@lippuner.ca 
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)egspp1.spec
include $(SPEC_DIR)egspp_$(my_machine).conf


USER_CODE = Epp
user_files = Epp
user_dependencies = Epp.cpp

# optimization level 1 is required to avoid an odd artifact that would otherwise
# occur in the propagation
#opt = -O1 -ffast-math

# Specify from which base class this application is being derived.
# This has the effect of automatically compiling the base application
# class and including it into the list of object files.
#
EGS_BASE_APPLICATION = egs_advanced_application

CPP_SOURCES = $(C_ADVANCED_SOURCES)

include $(HEN_HOUSE)makefiles$(DSEP)cpp_makefile

# Dependencies
dep_Epp_advanced_application = array_sizes.h \
        $(common_h_files1) $(common_h_files2) \
        $(ABS_EGSPP)egs_input.h $(ABS_EGSPP)egs_base_source.h \
        $(ABS_EGSPP)egs_object_factory.h $(ABS_EGSPP)egs_timer.h \
        Epp_application.h $(ABS_EGSPP)egs_run_control.h

dep_user_code = $(user_dependencies) array_sizes.h $(common_h_files1) \
        $(common_h_files2)

.PHONY: clean library
