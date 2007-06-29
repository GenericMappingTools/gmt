#!/bin/sh
#-----------------------------------------------------------------------------
#	 $Id: webman.sh,v 1.43 2007-06-29 18:40:30 remko Exp $
#
#	webman.sh - Automatic generation of the GMT web manual pages
#
#	Author:	Paul Wessel and Remko Scharroo
#	Date:	22-JUN-2007
#	Version: 1.3 Bourne shell
#
#	Uses groff -T html + alot of sed and awk...
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

echo "Creating HTML man pages ..."
mkdir -p www/gmt/doc/html

# Make a list of all manpages
grep -h .man\$ guru/GMT_progs_files_ascii.lis guru/GMT_suppl.lis > $$.lis

# Gurus who have their own supplemental packages can have them processed too by
# defining an environmental parameter MY_GMT_SUPPL which contains a list of these
# supplements.  They must all be in src of course

for package in ${MY_GMT_SUPPL}; do
	ls src/$package/*.man >> $$.lis
done

# Now make sed script that will replace the bold and italic versions of GMT program names with
# similarly formatted links.

while read f; do
	prog=`basename $f .man`
	echo $prog | awk '{printf "s%%<b>%s</b>%%<A HREF=%c%s.html%c><b>%s</b></A>%%g\n", $1, 34, $1, 34, $1}' >> $$.w0.sed
	echo $prog | awk '{printf "s%%<i>%s</i>%%<A HREF=%c%s.html%c><i>%s</i></A>%%g\n", $1, 34, $1, 34, $1}' >> $$.w0.sed
done < $$.lis

# Make sed script that adds active links to gmtdefault for all GMT defaults parameters.
# all.sed is run on all programs except gmtdefaults and add links to the relevant anchors in gmtdefaults.html
# def.sed adds the anchors needed in gmtdefaults.html

cat > $$.all.sed <<END
s%GMTMANSECTION%l%g
s%<body>%<body bgcolor="#ffffff">%g
END
grep -v '^#' src/gmt_keywords.d | awk '{printf "s%%<b>%s</b>%%<A HREF=%cgmtdefaults.html#%s%c><b>%s</b></A>%%g\n", $1, 34, $1, 34, $1}' >> $$.all.sed
grep -v '^#' src/gmt_keywords.d | awk '{printf "s%%<p><b>%s</b></p></td>%%<A NAME=%c%s%c><p><b>%s</b></p></td>%%g\n", $1, 34, $1, 34, $1}' > $$.def.sed

# Do all the manpage conversions

while read f; do
	prog=`basename $f .man`
	rm -f www/gmt/doc/html/$prog.html
	[ $gush = 1 ] && echo "Making $prog.html"
	grep -v "${prog}<" $$.w0.sed > $$.t0.sed
	if [ "X$prog" = "Xgmtdefaults" ]; then
		groff -man -T html $f | sed -f $$.t0.sed -f $$.def.sed -f $$.all.sed > www/gmt/doc/html/$prog.html
	else
		groff -man -T html $f | sed -f $$.t0.sed -f $$.all.sed > www/gmt/doc/html/$prog.html
	fi
done < $$.lis

# Clean up

rm -f $$.*
