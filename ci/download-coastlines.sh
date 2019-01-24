#!/usr/bin/env bash
# Download and install the coastlines and boundaries datasets

# To return a failure if any commands inside fail
#set -e
set -x

# gshhg and dcw tarballs are cached here:
test -d $HOME/pkg-gshhg-dcw || mkdir $HOME/pkg-gshhg-dcw
cd $HOME/pkg-gshhg-dcw

# Get the coastlines and country polygons
EXT="tar.gz"
GSHHG="gshhg-gmt-2.3.7"
DCW="dcw-gmt-1.1.4"
MD5_GSHHG=8ee2653f9daf84d49fefbf990bbfa1e7
MD5_DCW=4f30857a8b12af0f910222fceb591538

# GSHHG (coastlines, rivers, and political boundaries):
echo ""
echo "Downloading and unpacking GSHHG"
echo "================================================================================"
curl -L -O -C - --retry 10 "https://mirrors.ustc.edu.cn/gmt/$GSHHG.$EXT" || true
md5=$(md5sum $GSHHG.$EXT | cut -d ' ' -f 1)
test "$md5" = "$MD5_GSHHG"
tar xzf $GSHHG.$EXT
mv $GSHHG/* $COASTLINEDIR/

# DCW (country polygons):
echo ""
echo "Downloading and unpacking DCW"
echo "================================================================================"
curl -L -O -C - --retry 10 "https://mirrors.ustc.edu.cn/gmt/$DCW.$EXT" || true
md5=$(md5sum $DCW.$EXT | cut -d ' ' -f 1)
test "$md5" = "$MD5_DCW"
tar xzf $DCW.$EXT
mv $DCW/* $COASTLINEDIR

ls $COASTLINEDIR

# Turn off exit on failure.
set +e
