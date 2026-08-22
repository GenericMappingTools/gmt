#!/usr/bin/env bash
# GMT EXAMPLE 51
#
# Purpose:     Illustrate usage of OpenStreetMap coastlines in GMT
# GMT modules: convert, select, coast, plot
# GDAL progs:  ogr2ogr


# First we convert the shapefile to something more GMT friendly. Here we make it
# into a large ASCII file. Human-readable but not very efficient. GDAL's ogr2ogr
# is used with the "-f OGR_GMT" option indicating that the output should have
# the native GMT format. Input is the downloaded land_polygons.shp, and the
# output is named land_polygons_osm_planet.gmt. The resulting file is very
# large (>1 GB) but already usable. But wait, there is more ...

ogr2ogr -f OGR_GMT land_polygons_osm_planet.gmt land_polygons.shp

# Resorting to GMT's convert program we take our very large ASCII file and
# reduce it to about a third of its original size by converting it to a binary
# file. Lets have a closer look at -bo2f: 
# -bo selects native binary output
# 2 makes it two data columns
# f indicates 4-byte single-precision for the data

gmt convert land_polygons_osm_planet.gmt -bo2f > land_polygons_osm_planet.bf2

# Now the heavylifting is done and we have a reusable global coastline file
# with higher precision than the GSHHG coastlines.
# Again, we could use the file as is, but there are more performance gains to
# collect. We extract just the area we need for our plot. Here the island of
# Sao Vicente, Cape Verde. select to the rescue! -bo2f is nothing new and -bi2f
# looks very similar: the -bi is indicating native binary input. -R...+r gives
# the area which we are interested in.

gmt select land_polygons_osm_planet.bf2 -bi2f -bo2f -R-25.14/16.75/-24.8/16.95+r > land_polygons_osm_caboverde.bf2

# Finally it is time to do the thing we are here for: Plot some OSM coastlines
# and compare them to the GSHHG coastlines.

gmt begin ex51
  
  # To make the lines a bit nicer we set the line endcaps and the way lines are
  # joined to round
  
  gmt set PS_LINE_CAP round
  gmt set PS_LINE_JOIN round
  
  # To get an idea where the GSHHG coastlines are we lay them down first with a
  # thin red pen. -JL defines a Lambert conic conformal projection and -R...+r
  # is the area of the plot. -W defines the pen and -B the style of the plot
  # borders, gridlines and annotations. 
  
  gmt coast -JL-24.9/16.55/16.3/16.7/15c -R-25.14/16.75/-24.8/16.95+r -Wred -Ba10mg10m
  
  # Time to use the OSM coastlines we prepared earlier. Straightforward we
  # supply the extracted area, tell plot about the binary format (-bi), define
  # pen (-W) and fill (-G)
  
  gmt plot land_polygons_osm_caboverde.bf2 -bi2f -Wthinnest,black -Ggrey
  
  # Here we plot the GSHHG coastlines a second time but now with a dashed pen
  # to highlight the areas where they would otherwise be hidden behind the grey
  # OSM landmass. 
  
  gmt coast -Wred,dashed
  
gmt end show
