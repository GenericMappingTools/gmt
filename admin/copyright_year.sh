#!/usr/bin/env bash
#
# Script to update copyright year
#

if [ ! -d cmake ]; then
    echo "Must be run from top-level gmt directory"
    exit 1
fi

lastyear=2019
newyear=2020

# 1. Find all files with "Copyright"
find -E . \
    -regex '.*\.(md|c|h|in|rst|bash|csh|sh|bat|m|cmake|txt)' \
    ! -path "./share/spotter/*" \
    -exec grep -H Copyright {} + | \
    grep -v ${newyear} | \
    awk -F: '{print $1}' > $$.tmp.lis

cat >> $$.tmp.lis << EOF
./src/gmtswitch
./src/grd2sph.c.template
./src/gmtswitch
./share/tools/ncdeflate
./src/img/img2google
EOF

# 2. Update the files
while read f; do
    sed -E -i.bak "s/Copyright \(c\) ([0-9]+)-${lastyear}/Copyright \(c\) \1-${newyear}/" $f
    rm -f $f.bak
done < $$.tmp.lis

# 3. Update GMT_VERSION_YEAR in cmake/ConfigDefault.cmake
sed -i.bak "s/set (GMT_VERSION_YEAR \"${lastyear}\")/set (GMT_VERSION_YEAR \"${newyear}\")/" cmake/ConfigDefault.cmake
rm -f cmake/ConfigDefault.cmake.bak

# 3. Clean up
rm -f $$.tmp.lis
