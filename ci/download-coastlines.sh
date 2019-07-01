#!/usr/bin/env bash
# Download and install the coastlines and boundaries datasets

# General settings:
EXT="tar.gz"
GSHHG="gshhg-gmt-2.3.7"
DCW="dcw-gmt-1.1.4"
MD5_GSHHG=8ee2653f9daf84d49fefbf990bbfa1e7
MD5_DCW=4f30857a8b12af0f910222fceb591538

# Used for checking the downloaded files:
check_md5 ()
{
  md5_ref=$1
  file=$2

  test -f "$file" || return 1
  md5=$(openssl dgst -md5 $file | cut -d ' ' -f 2)
  test "$md5" = "$md5_ref"
}

# To return a failure if any commands inside fail
set -e

# Create the directory for coastline data
mkdir -p $COASTLINEDIR

# GSHHG and DCW tarballs are cached here:
test -d $HOME/cache-gshhg-dcw || mkdir $HOME/cache-gshhg-dcw
cd $HOME/cache-gshhg-dcw

# GSHHG (coastlines, rivers, and political boundaries):
echo ""
echo "Downloading and unpacking GSHHG"
echo "================================================================================"
# download when md5sums don't match:
check_md5 $MD5_GSHHG $GSHHG.$EXT || curl -L -O --retry 10 "https://mirrors.ustc.edu.cn/gmt/$GSHHG.$EXT"
check_md5 $MD5_GSHHG $GSHHG.$EXT
tar xzf $GSHHG.$EXT
mv $GSHHG $COASTLINEDIR/gshhg

# DCW (country polygons):
echo ""
echo "Downloading and unpacking DCW"
echo "================================================================================"
# download when md5sums don't match:
check_md5 $MD5_DCW $DCW.$EXT || curl -L -O --retry 10 "https://mirrors.ustc.edu.cn/gmt/$DCW.$EXT"
check_md5 $MD5_DCW $DCW.$EXT
tar xzf $DCW.$EXT
mv $DCW $COASTLINEDIR/dcw

ls $COASTLINEDIR/gshhg $COASTLINEDIR/dcw

# Turn off exit on failure.
set +e
