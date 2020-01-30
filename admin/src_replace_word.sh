#!/usr/bin/env bash
#
# Script to do a simple source-code substitution where we
# wish to replace one word with another word throughout all
# C source and include files.  E.g.,
#
# admin/replace.sh GMT_MSG_LONG_VERBOSE GMT_MSG_INFORMATION
#

if [ ! -d cmake ]; then
	echo "Must be run from top-level gmt directory"
	exit 1
fi
if [ ! $# -eq 2 ]; then
	echo "usage: replace oldword newword"
	exit 1
fi
# 1. Find all source files with word $1 in them
find -E src \
	-regex '.*\.(c|h|in)' \
	-exec grep -H $1 {} \; | \
	awk -F: '{print $1}' | sort -u > /tmp/$$.tmp.lis

# 2. Update the files and replace $1 by $2
while read f; do
	sed -E -i.bak "s/$1/$2/" $f
	rm -f $f.bak
done < /tmp/$$.tmp.lis

# 3. Clean up
rm -f /tmp/$$.tmp.lis
