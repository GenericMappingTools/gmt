#!/usr/bin/env bash
# GDAL writes no y:actual_range for a netCDF grid with a single row, so GMT reads
# its y range as 0/0 and y_inc as 0 (issue #9261). The read must fail, not crash.
# The 1-row input is a native binary grid so that no netCDF 1-row grid is read before that.

gmt grdmath -R-1.2/1.2/-0.2/0.2 -I0.4 -r X = row.grd=bf
gmt grdconvert row.grd=bf row_gdal.nc=gd:netCDF
gmt grdtrend row_gdal.nc -N4+x -Ttrend.nc 2> err.txt || true

# Write out anything else than a clean read error as failure
grep -q "GMT_GRID_READ_ERROR" err.txt || echo "no clean read error" > fail
