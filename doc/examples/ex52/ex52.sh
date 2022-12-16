#!/usr/bin/env bash
# GMT EXAMPLE 52
#
# Purpose:     Mixing images and overlaying them on the Earth
# GMT modules: grdgradient, grdimage, grdmath, grdmix, grdsample, solar

res=20m # Change to 15m, 10m, 06m, 05m, 04m, 03m, 02m, 01m, 30s for higher resolutions

gmt begin ex52

  # Use the location of the Sun at noon on Jun 22, 2000, Hawaii Standard Time
  # Make a global grid with a smooth 2-degree transition across day/night boundary.
  gmt grdmath -Rd -I${res} -r $(gmt solar -C -o0:1 -I+d2000-06-22T24:00+z-10) 2 DAYNIGHT = w.grd

  # We will create an intensity grid based on a pixel-registered DEM so that we can see structures in the oceans
  gmt grdgradient @earth_relief_${res}_p -Nt0.5 -A45 -Gintens.grd
  
  # Blend the earth_day and earth_night geotiffs using the weights, so that when w is 1
  # we get the earth_day, and then adjust colors based on the intensity.

  gmt grdmix @earth_day_${res} @earth_night_${res} -Ww.grd -Iintens.grd -Gview.tif

  # Plot this image on an Earth with view from over Mexico
  gmt grdimage view.tif -JG80W/30N/18c -Bafg
  # Clean up after use
  rm -f w.grd intens.grd view.tif
gmt end show
