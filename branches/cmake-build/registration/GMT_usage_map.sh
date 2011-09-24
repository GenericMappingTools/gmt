#!/bin/sh
#
#	$Id$
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
#	30-SEPT-2008
#
# Typicall this script is run by cron on Paul's computer since
# SOEST does not want jobs to run on the web server.  These are
# the crontab entries on macnut right now:
#
# Run 1 min past midnight, every day [Creates updated hit map for GMT main page]
# 1 0 * * * /Users/pwessel/UH/RESEARCH/CVSPROJECTS/GMTdev/GMT/registration/GMT_usage_map.sh > $HOME/macnut_cron1.log 2>&1
# Run 1 am, every night [Makes sure my local GMT tree is up-to-date with the latest changes]
# 0 1 * * *       /Users/pwessel/UH/RESEARCH/CVSPROJECTS/GMTdev/GMT/guru/nightly_gmt_cvsupdate.sh > $HOME/cron.log 2>&1
# Run 2 am, every day [Place the latest ChangeLog file on the SOEST web server]
# 0 2 * * *	scp /Users/pwessel/UH/RESEARCH/CVSPROJECTS/GMTdev/GMT/ChangeLog imina:/export/imina2/httpd/htdocs/gmt/gmt >> $HOME/cron.log 2>&1

#
# The first cmd will scp the file /tmp/gmtregistrations from the SOEST web server and
# process the data, produce an updated JPG image, and scp the file to the
# proper GMT directory on the web server
#
# 5 0 * * * rm -f /tmp/gmtregistrations
#
# Related info:  The registration form on gmt.soest.hawaii.edu collects
# a lon,lat location of the users site.  The submitted data form
# is processed by gmt_form.pl in the cgi-bin directory on the gmt
# server (currently /var/www/cgi-bin on gmt) which will write the lon/lat
# to /tmp/gmtregistration on gmt.  This script
# then acts on these records as described above.
# Note: This script uses the GMT4 registration dir since that part is in GMT 4 subversion

GMT4=/Users/pwessel/UH/RESEARCH/CVSPROJECTS/GMTdev/GMT4

if [ "X$GMTHOME" = "X" ]; then	# Running crontab and environment is not set
	. /sw/bin/init.sh
	GS_LIB=/sw/share/ghostscript/8.61/lib
	GMTHOME4=/Users/pwessel/UH/RESEARCH/CVSPROJECTS/GMTdev/gmt4
	GMTHOME5=/Users/pwessel/UH/RESEARCH/CVSPROJECTS/GMTdev/gmt5
	PATH=$GMTHOME5/bin:$PATH
	export PATH
	export GS_LIB
	CVSROOT=":pserver:guru@pohaku.soest.hawaii.edu:/usr/local/cvs"
fi
if [ $# = 1 ] && [ $1 = "help" ]; then
	cat << EOF >&2
usage: GMT_usage_map.sh [-v] [all | get | update | map | help]

get	Get fresh registrations and compile locations
update	Update the Subversion version of the complete list
map	Generate a new usage map with the latest data
all	Do all of the above [Default]
help	Give a brief help message
EOF
	exit
fi
REGHOME4=$GMTHOME4/registration	# Where to do the work
REGHOME5=$GMTHOME5/registration	# Where to do the work
gmtswitch gmt5

cd $REGHOME5
verbose=0
if [ $# -ge 1 ]; then	# Check for verbose first
	if [ "X$1" = "X-v" ]; then
		verbose=1
		shift
	fi
fi
if [ $# = 1 ]; then	# Only wanted some tasks done
	key=$1
else				# Default is all tasks
	key="all"
fi
#if [ $key = "all" ] || [ $key = "get" ]; then
#	Extracts new sites from teh web server's tmp dir and only returns
#	those over land.  To be run from the GMT/registration directory.

# Check if there is new data there
#
#	scp imina.soest.hawaii.edu:/tmp/gmtregistration /tmp
#	FILE=/tmp/gmtregistration
#	if [ ! -e $FILE ] & [ $verbose -eq 1 ]; then
#		echo "GMT_usage_map.sh: No new registrations to process" >&2
#		exit
#	fi
#
## OK, go ahead and process the new data

#
#	Only keep ones over land
#
#	gmtselect -R0/360/-60/72 -Jm1 -Ns/k -Dl $FILE > new_sites_land.d
#	n=`cat new_sites_land.d | wc  -l`
#	if [ $n -gt 0 ] & [ $verbose -eq 1 ]; then
#		echo "GMT_usage_map.sh: Found $n new sites" >&2
#	fi
#	rm -f $FILE
#fi

#if [ $key = "all" ] || [ $key = "update" ]; then
#
#	Gets the previous GMT_old_unique_sites.d file,
#	add in the new_sites_land.d data, and runs blockmean
#	on it again to remove duplicates
#
#	svn -q update
#	egrep '^#' GMT_old_unique_sites.d > $$.d
#	n_old=`grep -v '^#' GMT_old_unique_sites.d | wc -l`
#	egrep -v '^#' GMT_old_unique_sites.d > $$.add
#	awk '{print $1, $2, 1}' new_sites_land.d >> $$.add
#	blockmean -R0/360/-72/72 -I15m $$.add -S >> $$.d
#	mv -f $$.d GMT_old_unique_sites.d
#	svn -q commit -m "Automatic update" GMT_old_unique_sites.d
#	rm -f $$.add new_sites_land.d
#	n_new=`grep -v '^#' GMT_old_unique_sites.d | wc -l`
#	delta=`expr $n_new - $n_old`
#	if [ $delta -gt 0 ] & [ $verbose -eq 1 ]; then
#		echo "GMT_usage_map.sh: Added $delta new sites" >&2
#	fi
#fi

if [ $key = "all" ] || [ $key = "map" ]; then

#	svn -q update
#	cd $REGHOME5
	cp $REGHOME4/GMT_old_unique_sites.d .
#	pscoast -R-175/185/-60/72 -JKf0/5.0i -Gburlywood -Sazure -Dc -A2000 -Ba60f30/30WSne -K -P -X0.6i -Y0.35i --MAP_FRAME_WIDTH=0.04i --FONT_ANNOT_PRIMARY=12p > gmt_usage.ps
#	pscoast -Rg -JKf10/5.0i -Gspringgreen -Sazure -Dc -Wfaint -A2000 -B0 -K -P -X0.6i -Y0.35i --MAP_FRAME_PEN=1p --FONT_ANNOT_PRIMARY=12p > gmt_usage.ps
#	grep -v '^#' GMT_old_unique_sites.d | psxy -R -J -O -K -Sc0.02i -Gred >> gmt_usage.ps
#	date +%x | awk '{print 0.1, 0.1, $1}' | pstext -R0/5/0/5 -Jx1i -F+f10p,Helvetica+jLB -O -Gcornsilk -To -W0.25p >> gmt_usage.ps
#	ps2raster -E150 -A -TG gmt_usage.ps
#	rm -f gmt_usage.ps
	#scp gmt_usage.png imina.soest.hawaii.edu:/export/imina2/httpd/htdocs/gmt5/gmt
	pscoast -R200/340/-90/90 -Ji0.014i -Bg -A10000 -Dc -Gdarkred -Sazure -K -P > gmt_usage.ps
	grep -v '^#' GMT_old_unique_sites.d | psxy -R -J -O -K -Sc0.02i -Gyellow >> gmt_usage.ps
	pscoast -R-20/60/-90/90 -J -B -Dc -A10000 -Gdarkgreen -Sazure -X1.96i -O -K >> gmt_usage.ps
	grep -v '^#' GMT_old_unique_sites.d | psxy -R -J -O -K -Sc0.02i -Gyellow >> gmt_usage.ps
	pscoast -R60/200/-90/90 -J -B -Dc -A10000 -Gdarkblue -Sazure -X1.12i -O -K >> gmt_usage.ps
	grep -v '^#' GMT_old_unique_sites.d | psxy -R -J -O -K -Sc0.02i -Gyellow >> gmt_usage.ps
	date +%x | awk '{print 0.05, 0.05, $1}' | pstext -R0/5/0/5 -Jx1i -F+f10p,Helvetica+jLB -O -Gcornsilk -TO -W0.25p -X-3.08i >> gmt_usage.ps
	ps2raster -E150 -A -TG gmt_usage.ps
	rm -f gmt_usage.ps
	gmtswitch gmt4
fi
