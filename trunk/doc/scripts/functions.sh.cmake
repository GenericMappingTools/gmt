#
# $Id$
#
# Functions to be used with test scripts

scriptname=$(basename "$0")

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

getbox () {
  # Expects -Joption and distance in km from map center
  range=$((echo -$2 -$2; echo $2 $2) | mapproject $1 -R0/360/-90/90 -I -Fk -C)
  printf " -R%f/%f/%f/%fr\n" $range
}

getrect () {
  # Expects xmin xmax ymin ymax in km relative to map center
  # -R and -J are set by preceding GMT commands
  (echo -$1 -$1; echo -$1 $1; echo $1 $1; echo $1 -$1) | mapproject -R -J -I -Fk -C
}

# Temporary change LANG to C
LANG=C

# Use executables from GMT_BINARY_DIR, fallback to CMAKE_INSTALL_PREFIX/GMT_BINDIR
export PATH="@GMT_BINARY_DIR_PATH@:@GMT_SOURCE_DIR@/src:@CMAKE_INSTALL_PREFIX@/@GMT_BINDIR@:${PATH}"
export GMT_SHAREDIR="@GMT_SOURCE_DIR@/share"
export GMT_USERDIR="@GMT_BINARY_DIR@/share"

# Define variables that are needed *within* test scripts
GMT_SOURCE_DIR="@GMT_SOURCE_DIR@"
EXTRA_FONTS_DIR="@CMAKE_CURRENT_SOURCE_DIR@/ex31/fonts"
GRAPHICSMAGICK="@GRAPHICSMAGICK@"

# Reset error count
ERROR=0

# Where the current script resides (need absolute path)
cd "$(dirname "$0")"
src="${PWD}"

# Convert PS to PDF
function make_pdf()
{
  pdf="${ps%.ps}.pdf"
  test -f "$ps" || return 1
  ps2raster -Tf -A -P -Ggs -C-sFONTPATH=${EXTRA_FONTS_DIR} "$ps" || ((++ERROR))
  test -f "$pdf" || ((++ERROR))
}

# Compare the ps file with its original. Check $ps against original $ps or against $1.ps (if $1 given)
pscmp () {
  test -f "$ps" || return 1
  if ! [ -x "$GRAPHICSMAGICK" ]; then
    echo "[PASS] (without comparison)"
    return
  fi
  # syntax: gm compare [ options ... ] reference-image [ options ... ] compare-image [ options ... ]
  rms=$(${GRAPHICSMAGICK} compare -density 200 -maximum-error 0.001 -highlight-color magenta -highlight-style assign -metric rmse -file "${ps%.ps}.png" "$ps" "$src/../fig/${1:-$ps}") || pscmpfailed="yes"
  rms=$(sed -nE '/Total:/s/ +Total: ([0-9.]+) .+/\1/p' <<< "$rms")
  if [ -z "$rms" ]; then
    rms="NA"
  else
    rms=$(printf "%.3f\n" $rms)
  fi
  if [ "$pscmpfailed" ]; then
    now=$(date "+%F %T")
    echo "RMS Error = $rms [FAIL]"
    echo "$now ${src##*/}/${ps%.ps}: RMS Error = $rms" >> ../fail_count.d
    make_pdf "$ps" # try to make pdf file
    ((++ERROR))
  else
    test -z "$rms" && rms=NA
    echo "RMS Error = $rms [PASS]"
  fi
}

# Make sure to cleanup at end
function cleanup()
{
  cd "@GMT_BINARY_DIR@" # get out of exec_dir before removing it
  test "${ERROR}" -eq 0 && rm -rf "${exec_dir}"
  echo "exit status: ${ERROR}"
  exit ${ERROR}
}

# Test the output image before exiting
function on_exit()
{
  set +e
  trap - EXIT # Restore EXIT trap
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

# Create a temporary directory exec_dir in the build dir
# and run remainder of this GMT script there
exec_dir="@CMAKE_CURRENT_BINARY_DIR@/${scriptname%.sh}"
mkdir -p "${exec_dir}"
cd "${exec_dir}"

# Start with proper GMT defaults
gmtset -Du PS_CHAR_ENCODING ISOLatin1+

# Convert script name to PS filename
ps="${scriptname%.sh}.ps"

# vim: ft=sh
