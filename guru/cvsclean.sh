#!/bin/sh
#	$Id: cvsclean.sh,v 1.1 2004-05-12 19:11:27 pwessel Exp $

# create the killfile
cat /dev/null >killfiles

# all the files in .cvsignore
for x in `find . -name .cvsignore`
do
  $AWK '{printf "%s/%s\n","'`dirname $x`'",$0}' $x >>killfiles
done

# remove backup files
find -name "*~" >>killfiles
find -name ".*~" >>killfiles
find -name "*.bak" >>killfiles
find -name ".*.bak" >>killfiles
find -name ".#*" >>killfiles

# remove the exceptions from the list
# these are non-cvs files, we nevertheless want to keep
#
# the result of grep is redirected to rm command
cat killfiles | awk '{printf "ls -d -1 %s 2>/dev/null\n",$0}' |sh | grep -v \
-e "guru/gmtguru.macros$" \
-e "share/binned_border_c.cdf$" \
-e "share/binned_border_f.cdf$" \
-e "share/binned_border_h.cdf$" \
-e "share/binned_border_i.cdf$" \
-e "share/binned_border_l.cdf$" \
-e "share/binned_GSHHS_c.cdf$" \
-e "share/binned_GSHHS_f.cdf$" \
-e "share/binned_GSHHS_h.cdf$" \
-e "share/binned_GSHHS_i.cdf$" \
-e "share/binned_GSHHS_l.cdf$" \
-e "share/binned_river_c.cdf$" \
-e "share/binned_river_f.cdf$" \
-e "share/binned_river_h.cdf$" \
-e "share/binned_river_i.cdf$" \
-e "share/binned_river_l.cdf$" \
| $AWK '{printf "rm -rvf \"%s\"\n",$0}' |sh

# delete the killfile
rm killfiles
