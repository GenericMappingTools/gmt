.. _modules:

GMT Core Modules
================

This is a list of all GMT core modules and their uses.

.. note::

   Looking for the *classic mode* modules like ``psxy`` and ``pscoast``? See the
   :ref:`equivalent page for classic mode <modules_classic>`.

.. hlist::
    :columns: 6

    - :doc:`basemap`
    - :doc:`begin`
    - :doc:`blockmean`
    - :doc:`blockmedian`
    - :doc:`blockmode`
    - :doc:`clear`
    - :doc:`clip`
    - :doc:`coast`
    - :doc:`colorbar`
    - :doc:`contour`
    - :doc:`dimfilter`
    - :doc:`docs`
    - :doc:`end`
    - :doc:`events`
    - :doc:`figure`
    - :doc:`filter1d`
    - :doc:`fitcircle`
    - :doc:`gmt2kml`
    - :doc:`gmtconnect`
    - :doc:`gmtconvert`
    - :doc:`gmtdefaults`
    - :doc:`gmtget`
    - :doc:`gmtinfo`
    - :doc:`gmtlogo`
    - :doc:`gmtmath`
    - :doc:`gmtregress`
    - :doc:`gmtselect`
    - :doc:`gmtset`
    - :doc:`gmtsimplify`
    - :doc:`gmtspatial`
    - :doc:`gmtvector`
    - :doc:`gmtwhich`
    - :doc:`grd2cpt`
    - :doc:`grd2kml`
    - :doc:`grd2xyz`
    - :doc:`grdblend`
    - :doc:`grdclip`
    - :doc:`grdcontour`
    - :doc:`grdconvert`
    - :doc:`grdcut`
    - :doc:`grdedit`
    - :doc:`grdfft`
    - :doc:`grdfill`
    - :doc:`grdfilter`
    - :doc:`grdgradient`
    - :doc:`grdhisteq`
    - :doc:`grdimage`
    - :doc:`grdinfo`
    - :doc:`grdlandmask`
    - :doc:`grdmask`
    - :doc:`grdmath`
    - :doc:`grdpaste`
    - :doc:`grdproject`
    - :doc:`grdsample`
    - :doc:`grdtrack`
    - :doc:`grdtrend`
    - :doc:`grdvector`
    - :doc:`grdview`
    - :doc:`grdvolume`
    - :doc:`greenspline`
    - :doc:`histogram`
    - :doc:`image`
    - :doc:`inset`
    - :doc:`kml2gmt`
    - :doc:`legend`
    - :doc:`makecpt`
    - :doc:`mapproject`
    - :doc:`mask`
    - :doc:`movie`
    - :doc:`nearneighbor`
    - :doc:`plot`
    - :doc:`plot3d`
    - :doc:`project`
    - :doc:`psconvert`
    - :doc:`rose`
    - :doc:`sample1d`
    - :doc:`solar`
    - :doc:`spectrum1d`
    - :doc:`sph2grd`
    - :doc:`sphdistance`
    - :doc:`sphinterpolate`
    - :doc:`sphtriangulate`
    - :doc:`splitxyz`
    - :doc:`subplot`
    - :doc:`surface`
    - :doc:`ternary`
    - :doc:`text`
    - :doc:`trend1d`
    - :doc:`trend2d`
    - :doc:`triangulate`
    - :doc:`wiggle`
    - :doc:`xyz2grd`

Session management
------------------

+-----------------------+---------------------------------------------------------------------+
| :doc:`begin`          | Initiate a new GMT session using modern mode                        |
+-----------------------+---------------------------------------------------------------------+
| :doc:`clear`          | Delete current history, conf, cpt, sessions, data or cache          |
+-----------------------+---------------------------------------------------------------------+
| :doc:`docs`           | Show HTML documentation of specified module or display graphics     |
+-----------------------+---------------------------------------------------------------------+
| :doc:`end`            | Terminate GMT modern mode session and produce optional graphics     |
+-----------------------+---------------------------------------------------------------------+
| :doc:`figure`         | Set attributes for the current figure                               |
+-----------------------+---------------------------------------------------------------------+
| :doc:`inset`          | Manage figure inset setup and completion                            |
+-----------------------+---------------------------------------------------------------------+
| :doc:`subplot`        | Manage figure subplot configuration and selection                   |
+-----------------------+---------------------------------------------------------------------+

Plotting
--------

+-----------------------+---------------------------------------------------------------------+
| :doc:`basemap`        | Create a basemap plot                                               |
+-----------------------+---------------------------------------------------------------------+
| :doc:`clip`           | Use polygon files to define clipping paths                          |
+-----------------------+---------------------------------------------------------------------+
| :doc:`coast`          | Plot (and fill) coastlines, borders, and rivers on maps             |
+-----------------------+---------------------------------------------------------------------+
| :doc:`colorbar`       | Plot gray scale or color scale on maps                              |
+-----------------------+---------------------------------------------------------------------+
| :doc:`contour`        | Contour or image raw table data by triangulation                    |
+-----------------------+---------------------------------------------------------------------+
| :doc:`events`         | Plot event symbols and labels for a moment in time                  |
+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtlogo`        | Plot the GMT logo on maps                                           |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdcontour`     | Contouring of 2-D gridded data sets                                 |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdimage`       | Produce images from 2-D gridded data sets                           |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdvector`      | Plotting of 2-D gridded vector fields                               |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdview`        | 3-D perspective imaging of 2-D gridded data sets                    |
+-----------------------+---------------------------------------------------------------------+
| :doc:`histogram`      | Plot a histogram                                                    |
+-----------------------+---------------------------------------------------------------------+
| :doc:`image`          | Plot Sun raster files on a map                                      |
+-----------------------+---------------------------------------------------------------------+
| :doc:`legend`         | Plot a legend on a map                                              |
+-----------------------+---------------------------------------------------------------------+
| :doc:`mask`           | Create overlay to mask out regions on maps                          |
+-----------------------+---------------------------------------------------------------------+
| :doc:`plot`           | Plot symbols, polygons, and lines on maps                           |
+-----------------------+---------------------------------------------------------------------+
| :doc:`plot3d`         | Plot symbols, polygons, and lines in 3-D                            |
+-----------------------+---------------------------------------------------------------------+
| :doc:`rose`           | Plot sector or rose diagrams                                        |
+-----------------------+---------------------------------------------------------------------+
| :doc:`solar`          | Plot day-light terminators and other sunlight parameters            |
+-----------------------+---------------------------------------------------------------------+
| :doc:`ternary`        | Plot data on ternary diagrams                                       |
+-----------------------+---------------------------------------------------------------------+
| :doc:`text`           | Plot text strings on maps                                           |
+-----------------------+---------------------------------------------------------------------+
| :doc:`wiggle`         | Draw table data time-series along track on maps                     |
+-----------------------+---------------------------------------------------------------------+

Filtering
---------

+-----------------------+---------------------------------------------------------------------+
| :doc:`blockmean`      | L\ :math:`_2` (*x*,\ *y*,\ *z*) table data filter/decimator         |
+-----------------------+---------------------------------------------------------------------+
| :doc:`blockmedian`    | L\ :math:`_1` (*x*,\ *y*,\ *z*) table data filter/decimator         |
+-----------------------+---------------------------------------------------------------------+
| :doc:`blockmode`      | Mode estimate (*x*,\ *y*,\ *z*) table data filter/decimator         |
+-----------------------+---------------------------------------------------------------------+
| :doc:`dimfilter`      | Directional filtering of 2-D gridded files in the space/time domain |
+-----------------------+---------------------------------------------------------------------+
| :doc:`filter1d`       | Time domain filtering of 1-D data tables                            |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdfilter`      | Filter 2-D gridded data sets in the space domain                    |
+-----------------------+---------------------------------------------------------------------+

Gridding
--------

+-----------------------+---------------------------------------------------------------------+
| :doc:`greenspline`    | Interpolation with Green's functions for splines in 1–3 D           |
+-----------------------+---------------------------------------------------------------------+
| :doc:`nearneighbor`   | Nearest-neighbor gridding scheme                                    |
+-----------------------+---------------------------------------------------------------------+
| :doc:`sphinterpolate` | Spherical gridding in tension of data on a sphere                   |
+-----------------------+---------------------------------------------------------------------+
| :doc:`surface`        | A continuous curvature gridding algorithm                           |
+-----------------------+---------------------------------------------------------------------+
| :doc:`triangulate`    | Perform optimal Delauney triangulation and gridding                 |
+-----------------------+---------------------------------------------------------------------+

Sampling of 1-D and 2-D data
----------------------------

+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtsimplify`    | Line reduction using the Douglas-Peucker algorithm                  |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdsample`      | Resample a 2-D gridded data set onto a new grid                     |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdtrack`       | Sample 2-D gridded data sets at specified (*x*,\ *y*) locations     |
+-----------------------+---------------------------------------------------------------------+
| :doc:`sample1d`       | Resampling of 1-D table data sets                                   |
+-----------------------+---------------------------------------------------------------------+

Projection and map-transformation
---------------------------------

+-----------------------+---------------------------------------------------------------------+
| :doc:`grdproject`     | Project gridded data sets onto a new coordinate system              |
+-----------------------+---------------------------------------------------------------------+
| :doc:`mapproject`     | Transformation of coordinate systems for table data                 |
+-----------------------+---------------------------------------------------------------------+
| :doc:`project`        | Project table data onto lines or great circles                      |
+-----------------------+---------------------------------------------------------------------+

Information retrieval
---------------------

+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtdefaults`    | List the current default settings                                   |
+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtget`         | Retrieve selected parameters in current file                        |
+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtinfo`        | Get information about table data files                              |
+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtset`         | Change selected parameters in current file                          |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdinfo`        | Get information about grid files                                    |
+-----------------------+---------------------------------------------------------------------+

Mathematical operations on tables or grids
------------------------------------------

+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtmath`        | Mathematical operations on table data                               |
+-----------------------+---------------------------------------------------------------------+
| :doc:`makecpt`        | Make color palette tables                                           |
+-----------------------+---------------------------------------------------------------------+
| :doc:`spectrum1d`     | Compute various spectral estimates from time-series                 |
+-----------------------+---------------------------------------------------------------------+
| :doc:`sph2grd`        | Compute grid from spherical harmonic coefficients                   |
+-----------------------+---------------------------------------------------------------------+
| :doc:`sphdistance`    | Create grid of NN or distances to nearest points on a sphere        |
+-----------------------+---------------------------------------------------------------------+
| :doc:`sphtriangulate` | Delaunay or Voronoi construction of spherical (*lon*,\ *lat*) data  |
+-----------------------+---------------------------------------------------------------------+

Convert or extract subsets of data
----------------------------------

+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtconnect`     | Connect segments into more complete lines or polygons               |
+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtconvert`     | Convert data tables from one format to another                      |
+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtselect`      | Select subsets of table data based on multiple spatial criteria     |
+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtspatial`     | Geospatial operations on lines and polygons                         |
+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtvector`      | Operations on Cartesian vectors in 2-D and 3-D                      |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grd2kml`        | Create KML image quadtree from single grid                          |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grd2xyz`        | Conversion from 2-D grid file to table data                         |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdblend`       | Blend several partially over-lapping grid files onto one grid       |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdconvert`     | Converts grid files into other grid formats                         |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdcut`         | Cut a sub-region from a grid file                                   |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdpaste`       | Paste together grid files along a common edge                       |
+-----------------------+---------------------------------------------------------------------+
| :doc:`splitxyz`       | Split *xyz* files into several segments                             |
+-----------------------+---------------------------------------------------------------------+
| :doc:`xyz2grd`        | Convert an equidistant table *xyz* file to a 2-D grid file          |
+-----------------------+---------------------------------------------------------------------+

Trends in 1-D and 2-D data
--------------------------

+-----------------------+---------------------------------------------------------------------+
| :doc:`fitcircle`      | Finds the best-fitting great or small circle for a set of points    |
+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtregress`     | Linear regression of 1-D data sets                                  |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdtrend`       | Fit trend surface to grids and compute residuals                    |
+-----------------------+---------------------------------------------------------------------+
| :doc:`trend1d`        | Fits polynomial or Fourier trends to :math:`y = f(x)` series        |
+-----------------------+---------------------------------------------------------------------+
| :doc:`trend2d`        | Fits polynomial trends to :math:`z = f(x,y)` series                 |
+-----------------------+---------------------------------------------------------------------+

Grid operations
---------------

+-----------------------+---------------------------------------------------------------------+
| :doc:`grd2cpt`        | Make color palette table from a grid files                          |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdclip`        | Limit the *z*-range in gridded data sets                            |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdedit`        | Modify header information in a 2-D grid file                        |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdfft`         | Perform operations on grid files in the frequency domain            |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdfill`        | Interpolate across holes in a grid                                  |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdgradient`    | Compute directional gradient from grid files                        |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdhisteq`      | Histogram equalization for grid files                               |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdlandmask`    | Create masking grid files from shoreline data base                  |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdmask`        | Reset grid nodes in/outside a clip path to constants                |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdmath`        | Mathematical operations on grid files                               |
+-----------------------+---------------------------------------------------------------------+
| :doc:`grdvolume`      | Calculate volumes under a surface within specified contour          |
+-----------------------+---------------------------------------------------------------------+

Miscellaneous
-------------

+-----------------------+---------------------------------------------------------------------+
| :doc:`gmt2kml`        | Like :doc:`plot` but writes KML for use in Google Earth             |
+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtwhich`       | Find full path to specified files                                   |
+-----------------------+---------------------------------------------------------------------+
| :doc:`kml2gmt`        | Extracts coordinates from Google Earth KML files                    |
+-----------------------+---------------------------------------------------------------------+
| :doc:`movie`          | Create animation sequences and movies                               |
+-----------------------+---------------------------------------------------------------------+
| :doc:`psconvert`      | Crop and convert PostScript files to raster images, EPS, and PDF    |
+-----------------------+---------------------------------------------------------------------+

.. Add a hidden toctree to suppress "document isn't included in any toctree" warnings
.. toctree::
   :hidden:

   basemap
   begin
   blockmean
   blockmedian
   blockmode
   clear
   clip
   coast
   colorbar
   contour
   dimfilter
   docs
   end
   events
   figure
   filter1d
   fitcircle
   gmt2kml
   gmtconnect
   gmtconvert
   gmtdefaults
   gmtget
   gmtinfo
   gmtlogo
   gmtmath
   gmtregress
   gmtselect
   gmtset
   gmtsimplify
   gmtspatial
   gmtvector
   gmtwhich
   grd2cpt
   grd2kml
   grd2xyz
   grdblend
   grdclip
   grdcontour
   grdconvert
   grdcut
   grdedit
   grdfft
   grdfill
   grdfilter
   grdgradient
   grdhisteq
   grdimage
   grdinfo
   grdlandmask
   grdmask
   grdmath
   grdpaste
   grdproject
   grdsample
   grdtrack
   grdtrend
   grdvector
   grdview
   grdvolume
   greenspline
   histogram
   image
   inset
   kml2gmt
   legend
   makecpt
   mapproject
   mask
   movie
   nearneighbor
   plot
   plot3d
   project
   psconvert
   rose
   sample1d
   solar
   spectrum1d
   sph2grd
   sphdistance
   sphinterpolate
   sphtriangulate
   splitxyz
   subplot
   surface
   ternary
   text
   trend1d
   trend2d
   triangulate
   wiggle
   xyz2grd

