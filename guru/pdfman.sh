#!/bin/sh
#-----------------------------------------------------------------------------
#	 $Id: pdfman.sh,v 1.13 2007-06-29 18:40:30 remko Exp $
#
#	pdfman.sh - Automatic generation of the GMT ps and pdf manual pages
#
#	Author:	Paul Wessel and Remko Scharroo
#	Date:	29-JUN-2007
#	Version: 1.2 Bourne shell
#
#	Uses groff -man
#	Assumes a cvs update has occured so files are fresh.
#
#	Must be run from the main GMT directory after man pages have
#	been made.
#
#-----------------------------------------------------------------------------

trap 'rm -f $$.*;exit 1' 1 2 3 15

if [ $# = 1 ]; then	# If -s is given we run silently with defaults
	gush=0
else			# else we make alot of noise
	gush=1
fi

mkdir -p www/gmt/doc/ps
mkdir -p www/gmt/doc/pdf

man2pdf () {
	rm -f www/gmt/doc/ps/$1.ps
	add=0
	echo "Creating $1.ps ..."
	while read f; do
		[ $gush = 1 ] && echo "Appending $f"
		[ $add = 1 ] && echo "false 0 startjob pop" >> www/gmt/doc/ps/$1.ps
		add=1
		sed s/GMTMANSECTION/l/g $f | groff -man >> www/gmt/doc/ps/$1.ps
	done
	echo "Converting $1.ps to $1.pdf"
	ps2pdf www/gmt/doc/ps/$1.ps www/gmt/doc/pdf/$1.pdf
}

# Convert all program manuals to PS and PDF
grep -h .man\$ guru/GMT_progs_files_ascii.lis | man2pdf GMT_Manpages

# Do the supplemental packages
grep -h .man\$ guru/GMT_suppl.lis | man2pdf GMT_Manpages_suppl

# Gurus who have their own supplemental packages can have them processed too by
# defining an environmental parameter MY_GMT_SUPPL which contains a list of these
# supplements.  They must all be in src of course

rm -f $$.lis
for package in ${MY_GMT_SUPPL}; do
	ls src/$package/*.man >> $$.lis
done
[ -f $$.lis ] && man2pdf GMT_My_Manpages_suppl < $$.lis

rm -f $$.*
