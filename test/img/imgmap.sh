#!/usr/bin/env bash
# Testing various ways of extracting and plotting IMG grids

ps=imgmap.ps
IMG=@topo.8.4.img
# Get subset of original merc grid and plot as Cartesian
# This requires a spherical Mercator projection overlay
gmt img2grd $IMG -R180/200/-5/5 -T1 -S1 -Gimg_m.nc -M
gmt makecpt -Crainbow -T-8000/0 > t.cpt
gmt grdimage img_m.nc -Jx0.25i -Ct.cpt -P -K -Xc > $ps
# Overlay geographic basemap using the same scale in spherical Mercator
gmt psbasemap -R -Jm0.25i -Ba -BWSne -O -K --PROJ_ELLIPSOID=sphere >> $ps
# Get a geographic grid by automatically undoing the spherical Mercator
# This yields a grid with an exact region that differs from given -R
gmt img2grd $IMG -R180/200/-5/5 -T1 -S1 -Gimg_g1.nc
# Now we can plot this geographic grid using any projection, here Mercator
gmt grdimage img_g1.nc -Jm -Ct.cpt -O -K -Ba -BWSne -Y3.25i >> $ps
# Get resampled geo grid with a domain exactly matching the requested -R
gmt img2grd $IMG -R180/200/-5/5 -T1 -S1 -Gimg_g2.nc -E
# Now we can plot this geographic grid using any projection, here Mercator
gmt grdimage img_g2.nc -Jm -Ct.cpt -O -K -Ba -BWSne -Y3.25i >> $ps
gmt psxy -R -J -O -T >> $ps
