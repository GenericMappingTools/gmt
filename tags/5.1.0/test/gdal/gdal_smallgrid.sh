#!/bin/bash
#
# $Id$
#
# test reading and writing a very small grid via gdal

# create pixel test grid
gmt grdmath -r -R-0.5/1.5/-0.5/1.5 -I1 X 1 NAN = pixel.nc=nb
gmt grd2xyz pixel.nc > pixel.xyz
diff -q --strip-trailing-cr ${src:-.}/smallgrid.xyz pixel.xyz

# create gridline test grid
gmt grdmath -R0/1/0/1 -I1 X 1 NAN = gridline.nc=nb
gmt grd2xyz gridline.nc > gridline.xyz
diff -q --strip-trailing-cr ${src:-.}/smallgrid.xyz gridline.xyz

gdal_types="u8 u16 i16 u32 i32 float32"

for type in ${gdal_types}; do
  # write pixel grid and read back
  gmt grdreformat pixel.nc pixel_${type}.tif=gd///99:gtiff/${type}
  gmt grdreformat pixel_${type}.tif pixel_${type}.nc=nb
  gmt grd2xyz pixel_${type}.tif > pixel_${type}.tif.xyz
  gmt grd2xyz pixel_${type}.nc  > pixel_${type}.nc.xyz
  diff -q --strip-trailing-cr ${src:-.}/smallgrid.xyz pixel_${type}.tif.xyz
  diff -q --strip-trailing-cr ${src:-.}/smallgrid.xyz pixel_${type}.nc.xyz

  # write gridline grid and read back
  gmt grdreformat gridline.nc gridline_${type}.tif=gd///99:gtiff/${type}
  gmt grdreformat gridline_${type}.tif gridline_${type}.nc=nb
  gmt grd2xyz gridline_${type}.tif > gridline_${type}.tif.xyz
  gmt grd2xyz gridline_${type}.nc  > gridline_${type}.nc.xyz
  diff -q --strip-trailing-cr ${src:-.}/smallgrid.xyz gridline_${type}.tif.xyz
  diff -q --strip-trailing-cr ${src:-.}/smallgrid.xyz gridline_${type}.nc.xyz
done
