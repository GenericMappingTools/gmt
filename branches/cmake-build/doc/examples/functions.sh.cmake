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
BIN_DIR=@GMT_BINARY_DIR@/src
SRC_DIR=@GMT_SOURCE_DIR@/src
SUPPLEMENTS_DIR=$(find ${BIN_DIR}/* -maxdepth 0 -type d -and -not -name '*.dSYM' -print0 | tr '\0' ':')
export PATH="${BIN_DIR}:${SUPPLEMENTS_DIR}:${SRC_DIR}:${PATH}"
export GMT_SHAREDIR="@GMT_SOURCE_DIR@/share"
export GMT_USERDIR="@GMT_BINARY_DIR@/share"

# Reset error count
ERROR=0

# Convert PS to PDF
function make_pdf()
{
  test -f ${ps} || return
  #test -f ${ps%.ps}.pdf && return # do not replace existing pdf
  ps2raster -Tf -A -P ${ps} || ((ERROR++))
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
gmtset -Du FORMAT_TIME_LOGO "Version 5"

# vim: ft=sh
