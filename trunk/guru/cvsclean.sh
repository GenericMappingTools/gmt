#!/bin/sh
#	$Id: cvsclean.sh,v 1.3 2004-08-18 23:19:44 pwessel Exp $

trap 'rm -f $$.*; exit 1' 1 2 3 15

# create the killfile
cat /dev/null >$$.lis

# all the files in .cvsignore
for x in `find . -name .cvsignore`
do
  $AWK '{printf "%s/%s\n","'`dirname $x`'",$0}' $x >>$$.lis
done

# remove backup files
find . -name "*~" >>$$.lis
find . -name ".*~" >>$$.lis
find . -name "*.bak" >>$$.lis
find . -name ".*.bak" >>$$.lis
find . -name ".#*" >>$$.lis

# remove the exceptions from the list
# these are non-cvs files, we nevertheless want to keep
#
# the result of grep is redirected to rm command
cat $$.lis | awk '{printf "ls -d -1 %s 2>/dev/null\n",$0}' |sh | grep -v \
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
rm $$.lis
