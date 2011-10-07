#
# $Id$
#
# Functions to be used with test scripts

# Print the shell script name and purpose and fill out to 72 characters
# and make sure to use US system defaults
header () {
  printf "%-72s" "$0: $1"
}

# Compare the ps file with its original. Check $1.ps (if $1 given) or $ps
pscmp () {
  f=${1:-`basename $ps .ps`}
  d=`basename $PWD`
  rms=`gm compare -density 100 -metric rmse -file $f.png $f.ps orig/$f.ps|grep Total|cut -c23-`
  if test $? -ne 0; then
    echo "[FAIL]"
    echo $d/$f: $rms >> ../fail_count.d
        ((ERROR++))
  elif test `echo 200 \> $rms|bc` -eq 1; then
    echo "[PASS]"
    rm -f $f.png $f.ps
  else
    echo "[FAIL]"
    echo $d/$f: RMS Error = $rms >> ../fail_count.d
        ((ERROR++))
  fi
}

passfail () {
  if [ -s fail ]; then
    echo "[FAIL]"
    echo $d/$1: `wc -l fail`ed lines >> ../fail_count.d
    mv -f fail $1.log
    ((ERROR++))
  else
    echo "[PASS]"
    rm -f fail $1.log $1.png
  fi
}

# Temporary change LANG to C
LANG=C

# Use executables from GMT_BINARY_DIR
BIN_DIR=@GMT_BINARY_DIR@/src
SRC_DIR=@GMT_SOURCE_DIR@/src
SUPPLEMENTS_DIR=$(find ${BIN_DIR}/* -maxdepth 0 -type d -print0 | sed -e 's/\o0/:/g')
export PATH="${BIN_DIR}:${SUPPLEMENTS_DIE}:${SRC_DIR}:${PATH}"
export GMT_SHAREDIR="@GMT_SOURCE_DIR@/share"
export GMT_USERDIR="@GMT_BINARY_DIR@/share"

# Reset error count
ERROR=0

# Convert PS to PDF
function make_pdf()
{
  test -f ${ps} || return
  test -f ${ps%.ps}.pdf && return
  ps2raster -Tf -A -P ${ps} || ((ERROR++))
  for ps in *.ps; do
    test -f ${ps} || continue
    test -f ${ps%.ps}.pdf && continue
    ps2raster -Tf -A -P ${ps} || ((ERROR++))
  done
}

# Make sure to cleanup at end
function on_exit()
{
  make_pdf
  rm -f .gmt* gmt.conf
  echo "exit status: ${ERROR}"
  exit ${ERROR}
}
trap on_exit EXIT

# Catch other errors
set -e
function on_err()
{
  ((ERROR++))
  on_exit
}
trap on_err ERR SIGSEGV SIGTRAP SIGBUS

# Start with proper GMT defaults
gmtset -Du

# vim: ft=sh
