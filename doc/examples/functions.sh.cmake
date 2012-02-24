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
export GMT_SOURCE_DIR="@GMT_SOURCE_DIR@"
export PATH="@GMT_BINARY_DIR_PATH@:@GMT_SOURCE_DIR@/src:${PATH}"
export GMT_SHAREDIR="@GMT_SOURCE_DIR@/share"
export GMT_USERDIR="@GMT_BINARY_DIR@/share"
export EXTRA_FONTS_DIR="@CMAKE_CURRENT_SOURCE_DIR@/ex31/fonts"
export CMP_FIG_PATH="@GMT_SOURCE_DIR@/doc/fig"
export GRAPHICSMAGICK="@GRAPHICSMAGICK@"

# Reset error count
ERROR=0

# Convert PS to PDF
function make_pdf()
{
  psfile="${1:-$ps}"
  pdfile="${psfile%.ps}.pdf"
  test -f "${psfile}" || return 1
  ps2raster -Tf -A -P -Ggs -C-sFONTPATH=${EXTRA_FONTS_DIR} ${psfile} || ((++ERROR))
  test -f "${pdfile}" || ((++ERROR))
}

# Compare the ps file with its original.
pscmp () {
  test -f "${1:-$ps}" || return 1
  f=${1:-$(basename $ps .ps)}
  d=$(basename $PWD)
  if ! [ -x "$GRAPHICSMAGICK" ]; then
    echo "[PASS] (without comparison)"
    return
  fi
  # syntax: gm compare [ options ... ] reference-image [ options ... ] compare-image [ options ... ]
  rms=$(${GRAPHICSMAGICK} compare -density 200 -maximum-error 0.001 -highlight-color magenta -highlight-style assign -metric rmse -file ${f}.png ${CMP_FIG_PATH}/${f}.ps ${1:-$ps}) || pscmpfailed="yes"
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
    ((++ERROR))
  else
    test -z "$rms" && rms=NA
    echo "RMS Error = $rms [PASS]"
    rm -f ${f}.png
  fi
}

# Make sure to cleanup at end
function cleanup()
{
  rm -f .gmt* gmt.conf
  lockfile.sh remove example
  echo "exit status: ${ERROR}"
  exit ${ERROR}
}

# Test the output image before exiting
function on_exit()
{
  set +e
  trap - EXIT # Restore EXIT trap
  make_pdf
  pscmp
  cleanup
}
trap on_exit EXIT

# Catch other errors
set -e
function on_err()
{
  set +e
  trap - EXIT ERR SIGSEGV SIGTRAP SIGBUS # Restore trap
  ((++ERROR))
  cleanup
}
trap on_err ERR SIGSEGV SIGTRAP SIGBUS

# Create lockfile (needed for running parallel tasks in the same directory).
# Timeout after 240 seconds.
lockfile.sh example 48 5

# Start with proper GMT defaults
gmtset -Du FORMAT_TIME_LOGO "Version 5"

# vim: ft=sh
