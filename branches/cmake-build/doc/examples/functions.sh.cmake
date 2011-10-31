# $Id$
#
# Bash script inclusion to test all GMT examples.
# It sets the environment such that the examples are created as in the manual.

# Determine if awk is buggy
result=`echo 1 | awk '{print sin($1)}'`
if [ $result = 1 ]; then # awk is rotten
  if [ `type nawk | grep "not found" | wc -l` -eq 1 ]; then
    export AWK=gawk
  else
    export AWK=nawk
  fi
else
  export AWK=awk
fi

# Temporary change LANG to C
LANG=C

# Use executables from GMT_BINARY_DIR
export PATH="@GMT_BINARY_DIR_PATH@:${PATH}"
export GMT_SHAREDIR="@GMT_SOURCE_DIR@/share"
export GMT_USERDIR="@GMT_BINARY_DIR@/share"
export EXTRA_FONTS_DIR="@CMAKE_CURRENT_SOURCE_DIR@/ex31/fonts"

# Reset error count
ERROR=0

# Convert PS to PDF
function make_pdf()
{
  test -f ${ps} || return
  #test -f ${ps%.ps}.pdf && return # do not replace existing pdf
  ps2raster -Tf -A -P -C-sFONTPATH=${EXTRA_FONTS_DIR} ${ps} || ((ERROR++))
}

# Make sure to cleanup at end
function on_exit()
{
  make_pdf
  rm -f .gmt* gmt.conf example.lock
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

# Create lockfile (needed for running parallel tasks in the same directory).
# Timeout and remove lockfile after 240 seconds.
lockfile -5 -l 240 example.lock

# Start with proper GMT defaults
gmtset -Du FORMAT_TIME_LOGO "Version 5"

# vim: ft=sh
