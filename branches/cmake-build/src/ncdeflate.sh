#!/bin/bash

#--------------------------------------------------------------------
# $Id$
#
# Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo,
# J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; version 2 of the
# License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Contact info: gmt.soest.hawaii.edu
#--------------------------------------------------------------------
#
# Compresses all variables in the given netcdf files with the
# specified deflation level.

function usage () {
  echo "usage: $(basename $0) [-d n] [-s] file [file ...]"  >&2
  echo "  [-d n]    set compression level (0=min, 9=max, default=3)" >&2
  echo "  [-s]      add shuffle option to deflation compression" >&2
  echo "  [-v]      be verbose" >&2
  echo "  [-x s]    add suffix \"s\" to output filename" >&2
  exit 1
}

# set defaults
NC_DEFLATE=3
NC_SHUFFLE=
NC_SUFFIX=""
NC_VERBOSE=0

# handle filenames with spaces
IFS=$'\n'

# test if nccopy in PATH
if ! which nccopy >/dev/null 2>&1; then
  echo "$(basename $0): cannot find nccopy." >&2
  exit 1
fi

# find compatible stat command
if stat -f%z "$0" >/dev/null 2>&1; then
  # BSD
  STAT=stat
  STATOPT="-f%z"
elif stat -c%s "$0" >/dev/null 2>&1; then
  # GNU
  STAT=stat
  STATOPT="-c%s"
elif gstat -c%s "$0" >/dev/null 2>&1; then
  # GNU
  STAT=gstat
else
  echo "$(basename $0): no compatible stat command found." >&2
  exit 1
fi

function file1_greater_file2 () {
  # return TRUE if 1st file is larger than the 2nd
  size1=$($STAT $STATOPT "$1")
  size2=$($STAT $STATOPT "$2")
  if [ "$size1" -gt "$size2" ]; then
    # compression ratio
    NC_RATIO=$((100 - ${size2} * 100 / ${size1}))
    return 0
  fi
  NC_RATIO="NA"
  return 1
}

# parse command line args
while getopts ":d:svx:" opt; do
  case $opt in
    d)
    NC_DEFLATE="$OPTARG"
    ;;
    s)
    NC_SHUFFLE="-s"
    ;;
    v)
    NC_VERBOSE=TRUE
    ;;
    x)
    NC_SUFFIX="$OPTARG"
    ;;
    \?)
    echo "Invalid option: -$OPTARG" >&2
    usage
    ;;
    :)
    echo "Option -$OPTARG requires an argument." >&2
    usage
    ;;
  esac
done

# test if 0 <= NC_DEFLATE <= 9
test "$NC_DEFLATE" -ge 0 -a "$NC_DEFLATE" -le 9 || usage

# remove options from $@
shift $((OPTIND-1))

test -z "$@" && usage

for file in $@; do
  if ! [ -f "$file" ]; then
    echo "$file: not found" >&2
    continue
  fi

  # compress file
  if [ "$NC_VERBOSE" = "TRUE" ]; then
    echo -n "$(basename "${file}")... " >&2
  fi
  nccopy -d $NC_DEFLATE $NC_SHUFFLE "$file" "${file}.$$.tmp" || continue

  # check compressed file size
  if [ "$NC_DEFLATE" -eq 0 ] || file1_greater_file2 "$file" "${file}.$$.tmp" ; then
    # if no compression or if compressed file is smaller than original file:
    # replace original file with compressed/decompressed file
    mv -f "${file}.$$.tmp" "${file}${NC_SUFFIX}"

    if [ "$NC_VERBOSE" = "TRUE" ]; then
      if [ "$NC_DEFLATE" -eq 0 ]; then
        echo "done." >&2
      else
        echo "ratio: ${NC_RATIO}%." >&2
      fi
    fi
  else
    # keep original file and remove compressed file
    rm -f "${file}.$$.tmp"

    if [ "$NC_VERBOSE" = "TRUE" ]; then
      echo "could not reduce file size. already compressed?" >&2
    fi
  fi
done

exit 0
