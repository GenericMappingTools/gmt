#!/bin/csh -f
#
#	$Id: do_examples.csh,v 1.15 2008-04-15 19:08:50 remko Exp $
#
# csh script to test all GMT examples (csh versions).
# If one argument is passed it is assumed to be the
# bin directory where the executables are.  If a second (and/or 3rd)
# argument is passed it is assumed to dir(s) of shared libs

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
	if ($#argv >= 2) then
		if ($?LD_LIBRARY_PATH) then
			setenv LD_LIBRARY_PATH ${2}:$LD_LIBRARY_PATH
		else
			setenv LD_LIBRARY_PATH ${2}:/usr/lib
		endif
	endif
	if ($#argv == 3) then
		setenv LD_LIBRARY_PATH ${3}:$LD_LIBRARY_PATH
	endif
endif

set dir = `which psxy`
if ($status) exit 1
echo "Running examples with executables from $dir" | sed s:/psxy\$::

# Modify the system gmtdefaults file a bit

gmtdefaults -Du > .gmtdefaults4
gmtset PS_IMAGE_COMPRESS lzw

# Loop over all examples and run each job

foreach ex (ex??/job*.csh)
	echo -n "Doing example $ex ... "
	cd `dirname $ex`
	\cp -f ../.gmtdefaults4 .
	csh -f `basename $ex`
	\rm -f .gmtdefaults4 .gmtcommands4
	cd ..
	echo "done"
end

\rm -f .gmtdefaults4

echo "Completed all examples"
