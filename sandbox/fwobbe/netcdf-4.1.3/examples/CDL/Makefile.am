## This is a automake file, part of Unidata's netCDF package.
# Copyright 2006, see the COPYRIGHT file for more information.  

# This file builds CDL examples.

# $Id$

# All we do is run two shell scripts, which create the sample files
# and then compare them with the C versions of the examples.
if !BUILD_DLL
TESTS = create_sample_files.sh do_comps.sh
endif

# Ship the scripts needed to create the sample files and compare them.
EXTRA_DIST = do_comps.sh create_sample_files.sh simple_xy.cdl	\
sfc_pres_temp.cdl pres_temp_4D.cdl

# Clean up files created during the process.
CLEANFILES = simple_xy.nc sfc_pres_temp.nc pres_temp_4D.nc

