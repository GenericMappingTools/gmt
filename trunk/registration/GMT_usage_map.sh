#!/bin/sh
#
#	$Id: GMT_usage_map.sh,v 1.6 2001-03-08 17:18:51 pwessel Exp $
#
# This script creates a fresh gmt_usage.jpg plot for the web page
# The coordinates passed have been checked for range etc
# It is run from inside the registration directory and will
# collect new registrations from my mailbox Registrations.
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
#	05-FEB-2001

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

MAIL=/home/aa1/wessel/nsmail/GMT.sbd/Registrations	# Where incoming registrations reside
REGHOME=/home/aa/wessel/GMTdev/GMT/registration	# Where to do the work
CVSROOT=":pserver:pwessel@gmt.soest.hawaii.edu:/home/gmt/gmt/cvs"

cd $REGHOME
if [ "X$GMTHOME" = "X" ]; then	# Must set environment
	export GMTHOME=/opt/gmt
	export PATH=$GMTHOME/bin:$PATH
fi

jd=`date +%j`
yr=`date +%Y`
if [ ! -d RegArchive/$yr ]; then
	mkdir -p RegArchive/$yr
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


# Get a copy if not already there

	SAVE="RegArchive/$yr/Registrations.$jd"
	if [ ! -e $SAVE ]; then
		cp $MAIL $SAVE
		cp -f /dev/null $MAIL
	else
		echo "GMT_usage_map.x: Can only be run once a day" >&2
		exit
	fi

# OK, go ahead and process the new data

	grep Longitude $SAVE > $$.lon
	grep Latitude $SAVE > $$.lat
	grep City $SAVE > $$.city
	grep Country $SAVE > $$.country

	nx=`cat $$.lon | wc -l`
	ny=`cat $$.lat | wc -l`

	if [ $nx -ne $ny ]; then
		echo "GMT_usage_map.x: Different number of lons and lats\!" >&2
		rm -f $$.lon $$.lat $$.city $$.country
		exit
	fi

	paste $$.lon $$.lat $$.city $$.country | awk '{ if (!($3 == 0 && $6 == 0) && ($3 >= -360.0 && $3 <= 360.0 && $6 >= -90.0 && $6 <= 90.0)) print $3, $6, $9, $12}' > $$.d
	awk '{if ($1 < 0.0) {printf "%lg\t%lg\t%s\t%s\n", $1+360.0, $2, $3, $4} else {printf "%lg\t%lg\t%s\t%s\n", $1, $2, $3, $4}}' $$.d | sort -u > $$.new
#
#	Then only keep ones over land with 4 columns
#
	awk '{if (NF == 4) print $0}' $$.new | gmtselect -R0/360/-60/72 -Jx1d -Ns/k -Dl > new_sites_land.d
	n=`cat new_sites_land.d | wc  -l`
	echo "GMT_usage_map.x: Found $n new sites" >&2

	rm -f $$.*
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
	echo "GMT_usage_map.x: Added $delta new sites" >&2
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
	/usr/X11R6/bin/convert -density 100x100 -crop 0x0 gmt_usage.ps gmt_usage.jpg
	gmtset DOTS_PR_INCH 300 PAPER_MEDIA Letter
	rm -f gmt_usage.ps
	echo "GMT_usage_map.x: Created new map (gmt_usage.jpg)" >&2
fi
