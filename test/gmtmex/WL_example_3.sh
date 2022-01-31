#!/usr/bin/env bash
#
# Bourne shell replica of GMT/MEX example 3 (HDF5 file layers)
# This is Figure 4 in Wessel & Luis, 2017
#
# GMT_KNOWN_FAILURE_LINUX
# GMT_KNOWN_FAILURE_WINDOWS
# DVC_TEST
gmt begin WL_example_3 ps
  # Import sea surface temperature grids from several HDF5 layers (lon, lat, sst, sst_qual)
  # Speed up processing by using native binary intermediary files
  file="A2016152023000.L2_LAC_SST.nc"
  args="=gd?HDF5:A2016152023000.L2_LAC_SST.nc"
  gmt which -Gc @${file}
  gmt grd2xyz @${file}${args}://geophysical_data/qual_sst -ZTLf > qual_sst.b
  gmt grd2xyz @${file}${args}://geophysical_data/sst -ZTLf > sst.b
  gmt grd2xyz @${file}${args}://navigation_data/longitude -ZTLf > lon.b
  gmt grd2xyz @${file}${args}://navigation_data/latitude -ZTLf > lat.b
  # Merge columns and skip record whose sst_qual > 0
  gmt convert -A lon.b lat.b sst.b qual_sst.b -bi1f -bo4d | gmt select -Z-/0+c3 -bi4d -bo3d -o0-2 > input.b
  # Perform nearest neighbor gridding
  gmt nearneighbor -R-12/-1/33/43 -I0.01 -S0.05 -bi3d input.b -GG.grd
  # Create a mask that is NaN where no data exist
  gmt grdmask -NNaN/1/1 -S0.02 input.b -Gmask.grd -bi3d
  # Apply the mask to limit the plot that follows
  gmt grdmath G.grd mask.grd MUL = new.grd
  # Select color tables
  gmt grd2cpt -Cjet -E -M --COLOR_NAN=255 new.grd
  gmt grdimage -JM15c -Ba -BWSne new.grd
  gmt colorbar -DJTC+w14c/0.25c+o0/1c+h -Baf+u@.
 # Clip topography to only plot over land, using shading
  gmt makecpt -Celevation -T0/3000
  gmt coast -Di -Gc -A1000
  gmt grdimage @earth_relief_01m -t50 -I+a0+nt0.7
  gmt coast -Q
  gmt coast -Di -W0.5p -A1000 -N1/0.5p -TdjTR+w3c+o1c+f3+l --FONT_TITLE=12 --MAP_TITLE_OFFSET=5p
  gmt colorbar -DJBC+w14c/0.25c+o0/1c+h+ef -Baf+u" m"
gmt end show
