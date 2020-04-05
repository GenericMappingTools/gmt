#!/usr/bin/env bash
# count_function_usage.sh
#
# Fine all functions exported in gmt source codes and rank them based on how
# many files they appear in.  The idea is to identify functions that should be
# made static, perhaps by moving from where they are declared to where they are
# used.

if [ ! -d src ]; then
        echo "count_function_usage.sh: Must be run from top-level gmt directory" >&2
        exit 1
fi
# Use subdir in /tmp as working directory
rm -rf /tmp/gmt
mkdir -p /tmp/gmt
# Exclude these files from participating since they are not relevant to the problem
ls src/*test*.c */*test*.c src/triangle.c src/s_rint.c src/psldemo.c src/script2verbatim.c src/example1.c src/gmtprogram.c src/gshhg_version.c | sort -u > /tmp/gmt/exclude_files.lis
# Exclude these functions from reporting since the modules wont be moved anyway (an no need to see main)
(gmt --show-modules; gmt --show-classic) | sort -u | awk '{printf "GMT_%s\n", $1}' > /tmp/gmt/exclude_funcs.lis
echo main >> /tmp/gmt/exclude_funcs.lis
# Find all C source files, then remove the ones in /tmp/gmt/exclude_files.lis
find src -name '*.c' | sort -u > /tmp/gmt/all_files.lis
comm -23  /tmp/gmt/all_files.lis /tmp/gmt/exclude_files.lis |\
	# Run ctags to output exported functions in each C file
	# Options:
	#   --filter=yes: let ctags read c source file name from stdin
	#	--file-scope=no:  exclude all "local" tags (i.e. local variables and static functions)
	# Output format:
	#	tag_name tag_type line_no source_filename the_source_line
	# Example output:
	#   shore_res_to_int function    386 src/gmt_shore.c  GMT_LOCAL int shore_res_to_int (char res) {
	ctags --filter=yes --file-scope=no -x | grep -v GMT_LOCAL |\
	# Further exclude some global variables
	gawk '$2=="function" {print $1, $4}' > /tmp/gmt/tmp.list
# Make list of functions and how many files each appear in, then sort that file
# Make a subdirectory per function in /tmp/gmt so we can print out the final sorted
# output at the end
rm -f /tmp/gmt/counts.lis
while read func file; do
	found=$(egrep -c "^${func}\$" /tmp/gmt/exclude_funcs.lis)
	if [ ${found} -eq 0 ]; then
	   echo $func "[declared in $file]" >&2	# Reporting progress
	   mkdir -p /tmp/gmt/${func}
	   find src -name '*.c' ! -path "$file" -exec grep $func {} \+ | awk -F':' '{print "\t", $1}' | sort -u > /tmp/gmt/${func}/t.lis
	   N=$(wc -l < /tmp/gmt/${func}/t.lis)
	   echo ${func} $file ${N} >> /tmp/gmt/counts.lis
	fi
done < /tmp/gmt/tmp.list
sort -k 3 -n /tmp/gmt/counts.lis > /tmp/gmt/sorted.lis
while read func file N; do
	echo $func "[declared in $file]"
	cat /tmp/gmt/${func}/t.lis
done < /tmp/gmt/sorted.lis
# rm -rf /tmp/gmt
