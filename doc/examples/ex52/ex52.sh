#!/usr/bin/env bash
# GMT EXAMPLE 52
#
# Purpose:     Mixing images and overlaying on the Earth
# GMT modules: grdgradient, grdimage, grdmath, grdmix, grdsample, solar

gmt begin ex52 ps

  # Use the location of the Sun at noon on Jun 22, 2000, Hawaii Standard Time
  # Make a global 6m grid with a smooth transition across day/night boundary.
  # We take advantage of the ATAN function to relatively quickly ramp from 0 to 1
  gmt grdmath -Rd -I20m -r --PROJ_ELLIPSOID=sphere $(gmt solar -C -o0:1 -I+d2000-06-22T24:00+z-10) SDIST KM2DEG 90 SUB NEG 2 DIV ATAN DUP UPPER DIV 1 ADD 2 DIV = w.grd

  # We will create an intensity grid based on the 6m DEM so that we can see structures in the oceans
  gmt grdgradient @earth_relief_20m -Nt0.5 -A45 -Gintens.grd
  # Need to make this pixel-registered to match Geotiffs
  gmt grdsample intens.grd -T -Gintens.grd

  # Blend the Blue and Black Marble geotiffs using the weights, so that when w is 1 we get the Blue marble
  # and adjust colors based on the intensity.

  gmt grdmix @BlueMarble_20m.tif @BlackMarble_20m.tif -Ww.grd -Iintens.grd -Gnewmap.tif

  # Plot this image on an Earth with view from over Mexico
  gmt grdimage newmap.tif -JG80W/30N/18c -Bafg
gmt end show
