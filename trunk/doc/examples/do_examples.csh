#!/bin/csh -f
#
#	$Id: do_examples.csh,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
#
# csh script to test all GMT examples (csh versions).
# If one argument is passed it is assumed to be the
# bin directory where the executables are.  If a second
# argument is passed it is assumed to be dir of shared libs

unalias cd

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

if ($#argv >= 1) then
	cd ..
	set top = `pwd`
	cd examples
	set oldpath = ($path)
	set path = ($1 $oldpath)
	if ($#argv == 2) then
		if ($?LD_LIBRARY_PATH) then
			set oldld = $LD_LIBRARY_PATH
		else
			set oldld = "/usr/lib"
		endif
		setenv LD_LIBRARY_PATH ${2}:${oldld}
	endif
	if ($?GMTHOME) then
		set old_GMTHOME = $GMTHOME
	else
		set old_GMTHOME = $top
	endif
	setenv GMTHOME $top
endif

# Loop over all examples and run each job

foreach dir (01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20)
	if (-e ex${dir}/job${dir}.csh ) then
		echo -n "Doing example ${dir}..."
		cd ex${dir}
		csh -f job${dir}.csh
		echo "done"
		cd ..
	endif
end

if ($#argv >= 1) then
	set path = ($oldpath)
	if ($#argv == 2) then
		setenv LD_LIBRARY_PATH $oldld
	endif
	setenv GMTHOME $old_GMTHOME
endif
echo "Completed all examples"
