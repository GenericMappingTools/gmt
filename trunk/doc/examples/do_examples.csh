#!/bin/csh -f
#
#	$Id: do_examples.csh,v 1.9 2007-03-28 03:02:12 pwessel Exp $
#
# csh script to test all GMT examples (csh versions).
# If one argument is passed it is assumed to be the
# bin directory where the executables are.  If a second
# argument is passed it is assumed to be dir of shared libs

unalias cd

# Temporary change LANG to C
setenv LANG C

# First find the right awk tool:

gawk 'END {print 1}' /dev/null >& $$
set n = `cat $$`

if ($#n == 1 && $n[1] == 1) then
	setenv AWK gawk
else
	\rm -f $$
	nawk 'END {print 1}' /dev/null >& $$
	set n = `cat $$`
	if ($#n == 1 && $n[1] == 1) then
		setenv AWK nawk
	else
		setenv AWK awk
	endif
endif
\rm -f $$

# Extend executable and library path if requested

if ($#argv >= 1) then
	cd ..
	set top = `pwd`
	cd examples
	set path = ($1 $path)
	if ($#argv == 2) then
		if ($?LD_LIBRARY_PATH) then
			setenv LD_LIBRARY_PATH ${2}:$LD_LIBRARY_PATH
		else
			setenv LD_LIBRARY_PATH ${2}:/usr/lib
		endif
	endif
endif

# Loop over all examples and run each job

foreach dir (01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26)
	if (-e ex${dir}/job${dir}.csh ) then
		echo -n "Doing example ${dir}..."
		cd ex${dir}
		\cp -f ../.gmtdefaults4.doc .gmtdefaults4
		csh -f job${dir}.csh
		\rm -f .gmtdefaults4
		cd ..
		echo "done"
	endif
end

echo "Completed all examples"
