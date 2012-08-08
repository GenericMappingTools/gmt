#!/bin/sh
# This shell script runs the ncdump tests.
# $Id$

set -e
echo ""
#!/bin/sh

# This shell file runs benchmarks on the 2D radar data on parallel platforms.

# $Id$

set -e
echo ""
echo "Getting GRIDSPEC test input data from Unidata FTP site..."

file_list='19800101.atmos_daily.tile1.nc  19800101.atmos_daily.tile4.nc  get_data.sh OCCAM_p5degree.nc 19800101.atmos_daily.tile2.nc  19800101.atmos_daily.tile5.nc 19800101.atmos_daily.tile3.nc 19800101.atmos_daily.tile6.nc'
for file in $file_list; do
    if test -f $file; then
	echo "we already have $file..."
    else
	wget ftp://ftp.unidata.ucar.edu/pub/libcf/sample_data/gridspec/$file
    fi
done

exit 0

