#!/bin/sh
#
#	$Id: GMT_usage_map.sh,v 1.22 2003-09-23 18:21:37 pwessel Exp $
#
# This script creates a fresh gmt_usage.jpg plot for the web page
# The coordinates passed have been checked for range etc
# It is run from inside the registration directory and will
# collect new lon/lat locations from /tmp/gmtregistration.
# This script performs any of three operations; by default they
# are all done unless you specify one of them:
#
#	get	Get fresh registrations and compile locations
#	update	Update the CVS version of the complete list
#	map	Generate a new usage map with the latest data
#	all	Do all of the above [Default]
#	help	Give a brief help message
#
#	Paul Wessel
#	30-SEPT-2002
#
# Typicall this script is run by cron on gmt:
#
# 1 0 * * * /home/aa/pwessel/UH/RESEARCH/PROJECTS/GMTdev/GMT/registration/GMT_usage_map.sh
#
# Then, to remove the gmtregistration file (which has permission that this user
# cannot delete), a root cron script
#
# 5 0 * * * rm -f /tmp/gmtregistrations
#
# Related info:  The registration form on gmt.soest.hawaii.edu collects
# a lon,lat location of the users site.  The submitted data form
# is processed by gmt_form.pl in the cgi-bin directory on the gmt
# server (currently /var/www/cgi-bin on gmt) which will write the lon/lat
# to /tmp/gmtregistration on gmt.  This script
# then acts on these records as described above.

if [ $# = 1 ] && [ $1 = "help" ]; then
	cat << EOF >&2
usage: GMT_usage_map.sh [all | get | update | map | help]

get	Get fresh registrations and compile locations
update	Update the CVS version of the complete list
map	Generate a new usage map with the latest data
all	Do all of the above [Default]
help	Give a brief help message
EOF
	exit
fi
GS_LIB=/usr/share/ghostscript/7.05/lib
GMTHOME=/home/aa/pwessel/UH/RESEARCH/PROJECTS/GMTdev/GMT
PATH=$GMTHOME/bin:$PATH
export PATH
export GS_LIB
REGHOME=$GMTHOME/registration	# Where to do the work
CVSROOT=":pserver:pwessel@gmt.soest.hawaii.edu:/home/gmt/gmt/cvs"

cd $REGHOME
if [ "X$GMTHOME" = "X" ]; then	# Must set environment
	export GMTHOME=/opt/gmt
	export PATH=$GMTHOME/bin:$PATH
fi

if [ $# = 1 ]; then	# Only wanted some tasks done
	key=$1
else				# Default is all tasks
	key="all"
fi
if [ $key = "all" ] || [ $key = "get" ]; then
#	Extracts new sites from my mail folder and only returns
#	those over land.  To be run from the GMT/registration
#	directory.


# Check if there is new data there

	FILE=/tmp/gmtregistration
	if [ ! -e $FILE ]; then
		echo "GMT_usage_map.x: No new registrations to process" >&2
		exit
	fi

# OK, go ahead and process the new data

#
#	Only keep ones over land
#
	gmtselect -R0/360/-60/72 -Jx1d -Ns/k -Dl $FILE > new_sites_land.d
	n=`cat new_sites_land.d | wc  -l`
	if [ $n -gt 0 ]; then
		echo "GMT_usage_map.x: Found $n new sites" >&2
	fi

	rm -f $FILE
fi

if [ $key = "all" ] || [ $key = "update" ]; then

#	Gets the previous GMT_old_unique_sites.d file,
#	add in the new_sites_land.d data, and runs blockmean
#	on it again to remove duplicates

	cvs update GMT_old_unique_sites.d
	egrep '^#' GMT_old_unique_sites.d > $$.d
	n_old=`grep -v '^#' GMT_old_unique_sites.d | wc -l`
	egrep -v '^#' GMT_old_unique_sites.d > $$.add
	awk '{print $1, $2, 1}' new_sites_land.d >> $$.add
	blockmean -R0/360/-72/72 -I15m $$.add -S >> $$.d
	mv -f $$.d GMT_old_unique_sites.d
	cvs commit -m "Automatic update" -n GMT_old_unique_sites.d
	rm -f $$.add new_sites_land.d
	n_new=`grep -v '^#' GMT_old_unique_sites.d | wc -l`
	delta=`expr $n_new - $n_old`
	if [ $delta -gt 0 ]; then
		echo "GMT_usage_map.x: Added $delta new sites" >&2
	fi
fi

if [ $key = "all" ] || [ $key = "map" ]; then

	gmtset DOTS_PR_INCH 100 FRAME_WIDTH 0.04i PAPER_MEDIA Letter+
	cvs update GMT_old_unique_sites.d
	psxy -R0/5.75/0/3 -Jx1i -P -X0.0133i -Y0.0133i -K -L -W2p << EOF > gmt_usage.ps
0 0
5.75 0
5.75 3
0 3
EOF
	pscoast -R-175/185/-60/72 -JM5.0i -G25/140/25 -S0/30/120 -Dc -A2000 -Ba60f30/30WSne -K -O -X0.6i -Y0.35i >> gmt_usage.ps
	grep -v '^#' GMT_old_unique_sites.d | psxy -R -JM -O -K -Sc0.02 -G255/255/0 >> gmt_usage.ps
	date +%x | awk '{print 0.1, 0.1, 10, 0, 0, "LB", $1}' | pstext -R0/5/0/5 -Jx1i -O -W255/255/255o >> gmt_usage.ps
	echo "quit" >> gmt_usage.ps
	convert -density 100x100 -crop 0x0 gmt_usage.ps gmt_usage.jpg
	gmtset DOTS_PR_INCH 300 PAPER_MEDIA Letter
	rm -f gmt_usage.ps
	install -m 644 gmt_usage.jpg /home/gmt/gmt/www/gmt/images
fi
