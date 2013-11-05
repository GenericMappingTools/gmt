#!/bin/sh
#	$Id$
#
# These functions can be used from any sh/bash script by specifying
# . gmt_shell_functions.sh
# in your script.  See our script template gmt_script.sh for usage.
#
#----GMT SHELL FUNCTIONS--------------------
#	Creates a unique temp directory and points GMT_TMPDIR to it
gmt_init_tmpdir () {
	export GMT_TMPDIR=`mktemp -d ${TMPDIR:-/tmp}/gmt.XXXXXX`
}

#	Remove the temp directory created by gmt_init_tmpdir
gmt_remove_tmpdir () {
	rm -rf $GMT_TMPDIR
	unset GMT_TMPDIR
}

#	Remove all files and directories in which the current process number is part of the file name
gmt_cleanup() {
	rm -rf *$$*
	if [ $# -eq 1 ]; then
		rm -rf ${1}*
	fi
}

#	Send a message to stderr
gmt_message() {
	echo "$*" >&2
}

#	Print a message to stderr and exit
gmt_abort() {
	echo "$*" >&2
	exit
}

#	Return integer total number of lines in the file(s)
gmt_nrecords() {
	cat $* | wc -l | awk '{print $1}'
}

#	Returns the number of fields or arguments
gmt_nfields() {
	echo $* | awk '{print NF}'
}

#	Returns the given field (arg 1) in current record (arg 2)
#	Must pass arg 2 inside double quotes to preserve it as one item
gmt_get_field() {
	echo $2 | cut -f$1 -d ' '
}

#	Return w/e/s/n from given table file(s)
#	May also add -Idx/dy to round off answer
gmt_get_region() {
	printf "%s/%s/%s/%s\n" `minmax -C $* | cut -d'	' -f1-4`
}

#	Return the w/e/s/n from the header in grd file
gmt_get_gridregion() {
	printf "%s/%s/%s/%s\n" `grdinfo -C $* | cut -d'	' -f2-5`
}

#	Return the current map width (expects -R and -J settings)
gmt_map_width() {
	mapproject $* /dev/null -V 2>&1 | grep Transform | awk -F/ '{print $5}'
}

#	Return the current map height (expects -R and -J settings)
gmt_map_height() {
	mapproject $* /dev/null -V 2>&1 | grep Transform | awk -F/ '{print $7}' | cut -f1 -d' '
}

# Make output PostScript file name based on script base name

gmt_set_psfile() {
	echo `basename $1 '.sh'`.ps
}

# For animations: Create a lexically increasing file namestem (no extension) based on prefix and frame number
# i.e., prefix_######
gmt_set_framename() {
	echo $1 $2 | awk '{printf "%s_%6.6d\n", $1, $2}'
}

# For animations: Increment frame counter by one

gmt_set_framenext() {
	gmtmath -Q $1 1 ADD =
}