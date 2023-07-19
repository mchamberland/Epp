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
include $(SPEC_DIR)egspp_$(my_machine).conf


USER_CODE = Epp
user_files = Epp
user_dependencies = Epp.cpp

# optimization level 1 is required to avoid an odd artifact that would otherwise
# occur in the propagation
#opt = -O1 -ffast-math

# The directory of the egs++ libraries relative to HEN_HOUSE
egspp = egs++
EGSPP = egs++$(DSEP)

# Relative path from the egs++ directory to the directory where object 
# files and DSOs (DLLs) get put
dso = dso$(DSEP)$(my_machine)

# The absolute path to the egs++ libraries
abs_dso = $(HEN_HOUSE)$(EGSPP)$(dso)
ABS_DSO = $(abs_dso)$(DSEP)

# The absolute path to the main egs++ library directory
abs_egspp = $(HEN_HOUSE)$(egspp)
ABS_EGSPP = $(abs_egspp)$(DSEP)

# The absolute path to the C/C++ interface directory
egs_interface = $(HEN_HOUSE)interface
EGS_INTERFACE = $(egs_interface)$(DSEP)

# Include directories to look for include files
INCL = -I. -I$(HEN_HOUSE)lib$(DSEP)$(my_machine) -I$(abs_egspp) -I$(egs_interface)

CPP_SOURCES = $(EGS_SOURCEDIR)egsnrc.macros $(MACHINE_MACROS) \
            $(EGS_INTERFACE)egs_c_interface2.macros \
            $(EGSPP_USER_MACROS) \
            $(MACHINE_MORTRAN) $(EGS_SOURCEDIR)egs_utilities.mortran \
            $(EGS_INTERFACE)egs_c_interface2.mortran \
            $(EGS_SOURCEDIR)transportp.macros \
            $(EGS_SOURCEDIR)get_inputs.mortran\
            $(EGS_SOURCEDIR)egsnrc.mortran

# The standard set of object files needed
egs_files = $(EGS_BASE_APPLICATION) egsnrc egs_interface2
egs_objects = $(addsuffix _$(my_machine).$(obje),$(egs_files))

# The user objects
user_objects = $(addsuffix _$(my_machine).$(obje),$(user_files))

# The target (the executable)
target = $(USER_BINDIR)$(USER_CODE)$(EXE)

# A standard rule for compiling C++ files
object_rule = $(CXX) $(INCL) $(DEF1) $(DEF_USER) $(opt) -c $(COUT)$@ $<

# Files that a lot of stuff depends upon
common_h_files1 = $(EGS_INTERFACE)egs_interface2.h $(HEN_HOUSE)lib$(DSEP)$(my_machine)$(DSEP)egs_config1.h

common_h_files2 = $(ABS_EGSPP)egs_libconfig.h \
                  $(ABS_EGSPP)egs_base_geometry.h $(ABS_EGSPP)egs_vector.h \
                  $(ABS_EGSPP)egs_math.h $(ABS_EGSPP)egs_functions.h \
                  $(ABS_EGSPP)egs_base_source.h $(ABS_EGSPP)egs_input.h \
                  $(ABS_EGSPP)egs_object_factory.h

# Dependencies
dep_Epp_advanced_application = array_sizes.h \
        $(common_h_files1) $(common_h_files2) \
        $(ABS_EGSPP)egs_input.h $(ABS_EGSPP)egs_base_source.h \
        $(ABS_EGSPP)egs_object_factory.h $(ABS_EGSPP)egs_timer.h \
        Epp_application.h $(ABS_EGSPP)egs_run_control.h

dep_user_code = $(user_dependencies) array_sizes.h $(common_h_files1) \
        $(common_h_files2)

dep_egs_interface = $(EGS_INTERFACE)egs_interface2.c $(common_h_files1)


$(target): $(user_objects) $(egs_objects)
	$(F77) $(opt) $(EOUT)$@ $^ $(lib_link1) $(user_lib_path) $(link2_prefix)egspp$(link2_suffix) $(fortran_libs)


$(USER_CODE)_$(my_machine).$(obje): $(dep_user_code)

egs_interface2_$(my_machine).$(obje): $(dep_egs_interface)
	$(object_rule)

egsnrc_$(my_machine).$(obje): egsnrc_$(my_machine).F array_sizes.h
	$(F77) $(FC_FLAGS) $(FDEFS) -c $(FOUT)$@ $<

egsnrc_$(my_machine).F: $(CPP_SOURCES)
	@echo Mortran compiling EGSnrc sources ...
	@$(MORTRAN_EXE) -s -i -d $(MORTRAN_DATA) -f $(CPP_SOURCES) -o7 $@ \
           -o8 $(@:.F=.mortlst)

$(user_objects):
	$(object_rule)

clean:
	$(REMOVE) mortjob.mortran egsnrc_$(my_machine).F egsnrc_$(my_machine).mortlst
	$(REMOVE) $(target) $(user_objects) $(egs_objects)

.PHONY: clean library
