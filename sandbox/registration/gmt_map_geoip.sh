#!/bin/bash
#	$Id$
#
# This script creates a fresh map_geoip_all.png plot for the wiki page
# The backgorund map is stored in svn in gmt5/sandbox but copied to
# here for now.
#

renice 17 -p $$ > /dev/null # be nice

# redmine public folder
MAP_INSTALL_PREFIX=/opt/bitnami/rubystack-3.2.3-0/apps/redmine/public/gmt

export PATH=$PATH:/opt/gmt/gmt5/bin

set -E # Shell functions and subshells need to inherit ERR trap

function on_err()
{
  trap - EXIT ERR SIGSEGV SIGTRAP SIGBUS # Restore trap
  echo "ERROR: ${1}:${2}" >&2 # Report error line
  exit 1
}

# Trap errors
trap 'on_err "${BASH_SOURCE}" "${LINENO}"' ERR SIGSEGV SIGTRAP SIGBUS

function add_locations()
{
  local resultvar=$1
  local outfile=$2
  local segment_header=$3
  shift 3
  local files="$@"

  echo "> $segment_header" >> $outfile
  # sum of d/l for each block in 3rd col
  gawk '{print $1, $2, 1}' $files | gmt blockmean -fg -Rd -I15m -Ss > locations.tmp
  # categorize d/l sums and reverse sort so that big circles are below small ones in the map
  gawk '{c=0.20} $3<100 {c=0.14} $3<10 {c=0.08} {print $1, $2, c}' locations.tmp | sort -nrk3 >> $outfile
  rm -f locations.tmp
  # update locations count
  n_added=$(cat $files | wc -l)
  eval $resultvar="'$n_added'"
  (( n_locations += n_added )) || : # second assignment in case return code != 0
 }

function plot_locations()
{
  local ps=$1
  background_ps=wiki_background_map.ps
  shift
  local files="$@"

  rm -f gmt.conf
  gmt set FONT_LABEL 7p,AvantGarde-BookOblique,black \
  FONT_ANNOT_PRIMARY 7p,AvantGarde-BookOblique,black \
  MAP_FRAME_PEN 1p
  cp -f $background_ps $ps
#  gmt pscoast -Rd -JN10c -Bg45 -A10000+l -Dc -Wthinnest -Givory -Saliceblue -K > ${ps}
#  gmt psxy -fg -R -J -Sc -Gpink -Wthinnest,black -O -K $files >> ${ps}
  gmt psxy -R190/330/-90/90 -Ji0.028c -Sc -Gpink -Wthinnest,black -O -K $files >> ${ps}
  gmt psxy -R-30/60/-90/90  -Ji0.028c -Sc -Gpink -Wthinnest,black -O -K $files -X3.92c >> ${ps}
  gmt psxy -R60/190/-90/90  -Ji0.028c -Sc -Gpink -Wthinnest,black -O -K $files -X2.52c >> ${ps}
  gmt psxy -R -J -O -K -T -X-6.44c >> ${ps}
  gmt pslegend -Dx0.25c/-0.1c/9c/TL --MAP_FRAME_AXES="" -O << EOF >> ${ps}
N 3
S 0.07c c 0.14c orange    thinnest 0.25c HTTP downloads (${n_dl_red:-0})
S 0.07c c 0.14c orangered thinnest 0.25c FTP downloads (${n_dl_ftp:-0})
S 0.07c c 0.14c maroon    thinnest 0.25c Subversion checkouts (${n_dl_svn:-0})
N 3
G 0.07c
S 0.07c c 0.08c - 0.25p 0.25c 1-9
S 0.07c c 0.14c - 0.25p 0.25c 10-99
S 0.07c c 0.20c - 0.25p 0.25c 100+
N 3
G 0.12c
L - - L Total: ${n_locations:-0}
L - - L \040
L - - L $(date -u "+%F %T %Z")
EOF
  gmt ps2raster -TG -A -P -E150 -Qt4 -Qg4 -C-dDOINTERPOLATE ${ps}
}

# change into output dir
cd ${HOME}/gmt_maps

# location of database
FILES_DB=/opt/bitnami/rubystack-3.2.3-0/apps/files/db/production.sqlite3

# get list of downloaded files' names
files_http=$(sqlite3 ${FILES_DB} "select distinct file from request_ips where file not like '%md5'")

# foreach http filename
for file in $files_http ; do
  # get long, lat, and time for each access
  sqlite3 ${FILES_DB} << EOF > geoip_red_${file}.xy
.separator " "
select longitude, latitude, created_at from request_ips where file = "${file}";
EOF
done

# Test for changes since last run
md5sum geoip_*.xy > geoip_current.md5
if [ "$1" != "-f" ] && diff -q geoip_current.md5 geoip_last.md5; then
  # no update of map necessary
  rm -f geoip_current.md5 geoip_red_*.xy
  exit 0
fi
mv geoip_current.md5 geoip_last.md5

# reset locations and count
n_locations=0
rm -f locations_all.xy

# plot geo-ip map of all downloaded files
ps=map_geoip_all.ps
add_locations n_dl_svn locations_all.xy -Gmaroon geoip_svn*.xy
#add_locations n_dl_ftp locations_all.xy -Gorangered geoip_ftp*.xy
add_locations n_dl_red locations_all.xy -Gorange geoip_red*.xy
plot_locations $ps locations_all.xy

# copy file to redmine public folder
rsync -a ${ps%ps}png ${MAP_INSTALL_PREFIX}

# clean up
rm -f gmt.conf gmt.history locations_all.xy geoip_red_*.xy

exit 0

# Add this to wiki: Geolocation mapping is based on <a href="http://www.maxmind.com">MaxMind's</a> freely available GeoLite data.

# foreach filename make map
for file in $files_http; do
  # do gmt plot
  ps=map_geoip_${file}.ps
  n_locations=$(wc -l geoip_red_${file}.xy)
  # todo: blockmean geoip_${file}.xy
  gmt psxy -Rd -JI10c -Bxg30 -Byg15 -Sc0.1c -Gyellow -Wyellow geoip_red_${file}.xy > ${ps}
  # todo: print total number of dots
  # gmt pstext << EOF
  #   n = $n_locations
  # EOF
  gmt ps2raster -TG -A -P ${ps}

  # clean up
  rm -f ${ps} geoip_red_${file}.xy
done

rm -f map_geoip_all.ps

exit 0
