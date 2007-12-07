#!/bin/sh
#	$Id: gmt_shell_functions.sh,v 1.1 2007-12-07 19:16:37 guru Exp $
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
	answer=`minmax -C $*`
	x0=`echo $answer | cut -f1 -d ' '`
	x1=`echo $answer | cut -f2 -d ' '`
	y0=`echo $answer | cut -f3 -d ' '`
	y1=`echo $answer | cut -f4 -d ' '`
	echo "$x0/$x1/$y0/$y1"
}

#	Return the w/e/s/n from the header in grd file
gmt_get_gridregion() {
	answer=`grdinfo -C $1`
	x0=`echo $answer | cut -f2 -d ' '`
	x1=`echo $answer | cut -f3 -d ' '`
	y0=`echo $answer | cut -f4 -d ' '`
	y1=`echo $answer | cut -f5 -d ' '`
	echo "$x0/$x1/$y0/$y1"
}

# Make output PostScript file name based on script base name

gmt_set_psfile() {
	echo `basename $1 '.sh'`.ps
}
