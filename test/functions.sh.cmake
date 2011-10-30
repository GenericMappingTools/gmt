#
# $Id$
#
# Functions to be used with test scripts

# Print the shell script name and purpose and fill out to 72 characters
# and make sure to use US system defaults
header () {
  printf "%-72s\n" "$0: $1"
}

# Convert PS to PDF
function make_pdf()
{
  test -f ${1} || return 1
  #test -f ${ps%.ps}.pdf && return # do not replace existing PDF file
  ps2raster -Tf -A -P ${1} || ((ERROR++))
}

# Compare the ps file with its original. Check $1.ps (if $1 given) or $ps
pscmp () {
  make_pdf $ps # make pdf file
  f=${1:-$(basename $ps .ps)}
  d=$(basename $PWD)
  # syntax: gm compare [ options ... ] reference-image [ options ... ] compare-image [ options ... ]
  rms=$(gm compare -density 300 -maximum-error 0.057 -highlight-color magenta -highlight-style assign -metric rmse -file ${f}.png orig/${f}.ps ${f}.ps) || pscmpfailed="yes"
  rms=$(sed -nE '/Total:/s/ +Total: ([0-9.]+) .+/\1/p' <<< "$rms")
  if [ -z "$rms" ]; then
    rms="NA"
  else
    rms=$(printf "%.3f\n" $rms)
  fi
  if [ "$pscmpfailed" ]; then
    now=$(date "+%F %T")
    echo "RMS Error = $rms [FAIL]"
    echo "$now ${d}/${f}: RMS Error = $rms" >> ../fail_count.d
    ((ERROR++))
  else
    test -z "$rms" && rms=NA
    echo "RMS Error = $rms [PASS]"
    rm -f ${f}.png ${f}.ps
  fi
}

passfail () {
  if [ -s fail ]; then
    now=$(date "+%F %T")
    echo "[FAIL]"
    echo "$now $d/$1: $(wc -l fail)ed lines" >> ../fail_count.d
    mv -f fail $1.log
    ((ERROR++))
  else
    echo "[PASS]"
    rm -f fail ${1}.log ${1}.png
  fi
}

# Temporary change LANG to C
LANG=C

# Use executables from GMT_BINARY_DIR
GMT_BINARY_DIR=@GMT_BINARY_DIR@
GMT_SOURCE_DIR=@GMT_SOURCE_DIR@
SUPPLEMENTS_DIR=$(find ${GMT_BINARY_DIR}/src/* -maxdepth 0 -type d -and -not -name '*.dSYM' -print0 | tr '\0' ':')
export PATH="${GMT_BINARY_DIR}/src:${SUPPLEMENTS_DIR}:${GMT_SOURCE_DIR}/src:${PATH}"
export GMT_SHAREDIR="${GMT_SOURCE_DIR}/share"
export GMT_USERDIR="${GMT_BINARY_DIR}/share"
export HAVE_GMT_DEBUG_SYMBOLS="@HAVE_GMT_DEBUG_SYMBOLS@"
export HAVE_OPENMP="@HAVE_OPENMP@"

# Reset error count
ERROR=0

# Make sure to cleanup at end
function on_exit()
{
  rm -f .gmt* gmt.conf test.lock
  echo "exit status: ${ERROR}"
  trap - EXIT # Restore ERR trap
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

# Create lockfile (needed for running parallel tests).
# Timeout and remove lockfile after 180 seconds.
lockfile -5 -l 180 test.lock

# Start with proper GMT defaults
gmtset -Du
gmtset PS_MEDIA letter PROJ_LENGTH_UNIT inch

# vim: ft=sh
