#!/bin/bash
#-----------------------------------------------------------------------------
#	 $Id: webman.sh,v 1.1 2011-03-25 15:55:02 remko Exp $
#
#	webman.sh - Automatic generation of the GMT web manual pages
#
#	Author:	Paul Wessel and Remko Scharroo
#	Date:	22-JUN-2007
#
#	Uses groff -T html + alot of sed and awk...
#	Assumes a cvs update has occured so files are fresh.
#
#	Must be run from the main GMT directory after man pages have
#	been made.
#
#-----------------------------------------------------------------------------

tmp=${TMPDIR:-/tmp}/gmt.$$

trap 'rm -rf $tmp;exit 1' 1 2 3 15

if [ $# = 1 ]; then	# If -s is given we run silently with defaults
	gush=0
else			# else we make alot of noise
	gush=1
fi

echo "Creating HTML man pages ..."

mkdir -p html/man $tmp

cd ..

# Make a list of all manpages
grep -h ".[135]\$" guru/GMT_progs_files_ascii.lis guru/GMT_suppl.lis > $tmp/pages.lis

# Gurus who have their own supplemental packages can have them processed too by
# defining an environmental parameter MY_GMT_SUPPL which contains a list of these
# supplements.  They must all be in src of course

for package in ${MY_GMT_SUPPL}; do
	ls src/$package/*.[135] >> $tmp/pages.lis
done

# Now make sed script that will replace the bold and italic versions of GMT program names with
# similarly formatted links.

while read f; do
	prog=`basename $f|cut -d. -f1`
	echo $prog | awk '{printf "s%%<b>%s</b>%%<b><A HREF=%c%s.html%c>%s</A></b>%%g\n", $1, 34, $1, 34, $1}' >> $tmp/pages.w0.sed
	echo $prog | awk '{printf "s%%<i>%s</i>%%<i><A HREF=%c%s.html%c>%s</A></i>%%g\n", $1, 34, $1, 34, $1}' >> $tmp/pages.w0.sed
done < $tmp/pages.lis

# Make sed script that adds active links to gmtdefault for all GMT defaults parameters.
# all.sed is run on all programs except gmtdefaults and add links to the relevant anchors in gmtdefaults.html
# def.sed adds the anchors needed in gmtdefaults.html

cat > $tmp/pages.all.sed <<END
s%<body>%<body bgcolor="#ffffff">%g
END
grep -v '^#' src/gmt_keywords.d | awk '{printf "s%%<b>%s</b>%%<b><A HREF=%cgmtdefaults.html#%s%c>%s</A></b>%%g\n", $1, 34, $1, 34, $1}' >> $tmp/pages.all.sed
grep -v '^#' src/gmt_keywords.d | awk '{printf "s%%><b>%s</b>%%><b><A NAME=%c%s%c>%s</A></b>%%g\n", $1, 34, $1, 34, $1}' > $tmp/pages.def.sed

# Do all the manpage conversions

while read f; do
	prog=`basename $f|cut -d. -f1`
	html=doc/html/man/$prog.html
	[ $gush = 1 ] && echo "Making $prog.html"
	grep -v "${prog}<" $tmp/pages.w0.sed > $tmp/pages.t0.sed
	if [ "X$prog" = "Xgmtdefaults" ]; then
		groff -man -T html $f | sed -f $tmp/pages.t0.sed -f $tmp/pages.def.sed -f $tmp/pages.all.sed > $html
	else
		groff -man -T html $f | sed -f $tmp/pages.t0.sed -f $tmp/pages.all.sed > $html
	fi
done < $tmp/pages.lis

# Clean up

rm -rf $tmp
