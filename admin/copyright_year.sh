#!/usr/bin/env bash
#
# Script to update copyright year of all GMT files:
#
#   bash admin/copyright_year.sh
#

lastyear=2019
newyear=2020

if [ ! -d cmake ]; then
    echo "Must be run from top-level gmt directory"
    exit 1
fi

# 1. Find all files with "Copyright"
find -E . \
    -regex '.*\.(md|c|h|in|rst|bash|csh|sh|bat|m|cmake|txt|TXT)' \
    ! -path "./share/spotter/*" \
    -exec grep -H Copyright {} + | \
    grep -v ${newyear} | \
    awk -F: '{print $1}' > ${TMPDIR}/$$.tmp.lis

# 2. Add extra files not found by 'find'
cat >> ${TMPDIR}/$$.tmp.lis << EOF
./src/gmtswitch
./src/grd2sph.c.template
./src/img/img2google
./share/tools/ncdeflate
EOF

# 3. Update the files
while read f; do
    sed -E -i.bak "s/Copyright \(c\) ([0-9]+)-${lastyear}/Copyright \(c\) \1-${newyear}/" $f
    rm -f $f.bak
done < ${TMPDIR}/$$.tmp.lis

# 4. Update GMT_VERSION_YEAR in cmake/ConfigDefault.cmake
sed -i.bak "s/set (GMT_VERSION_YEAR \"${lastyear}\")/set (GMT_VERSION_YEAR \"${newyear}\")/" cmake/ConfigDefault.cmake
rm -f cmake/ConfigDefault.cmake.bak

# 5. Clean up
rm -f ${TMPDIR}/$$.tmp.lis
