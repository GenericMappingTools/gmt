#!/bin/csh
#
#	$Id$
#
#	Environmental variables needed by GMT gurus
#	Stick these in your environment before making GMT
#
# 1. Using awk
#   Different platforms have different version of awk.  While GMT configure
#   will determine this, you need $AWK to be set to run make_examples.csh etc
#   so you might as well set it to nawk, gawk, mawk, or awk.  Note old awk is
#   not smart enough so try the others first.

setenv AWK gawk								# nawk, gawk, or compatible

#
# 2. Enable Matlab
#   Some of the gurus may have Matlab installed on their system, and thus can
#   compile the mex files for the src/mex supplement.  Give the path to the
#   Matlab directory or set it to NONE if you dont have it

setenv MATLAB /usr/local/matlab						# Set to NONE if you do not have Matlab

#
# 3. The GMT Environment
#   Before you start issuing make commands, you should have these set properly
#
setenv GMTROOT <fullpathtoyour>/GMTdev/GMT
setenv MANPATH $GMTROOT/man						# Or add this part if MANPATH exists for other reasons
setenv NETCDFHOME  /usr/local						# Set this to where netcdf lives

#
# 4. Searchable path
#   Make sure you add $GMTROOT/bin to your path.
