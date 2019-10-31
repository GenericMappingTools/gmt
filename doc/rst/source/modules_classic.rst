GMT Modules (Classic Mode)
==========================

.. note::

   Looking for the *modern mode* modules like ``begin`` and ``figure``? See the
   :doc:`equivalent page for modern mode <modules>`.

This is a list of all GMT "classic mode" core and supplemental modules and their uses,
as well as some utility scripts.
These modules are fully compatible with GMT 4 and 5.
All modules are requested via a call to the :doc:`gmt` program.

.. Add core and supplemental modules to hidden toctrees to
.. suppress "document isn't included in any toctree" warnings

.. toctree::
   :hidden:

   gmt
   gmt-config
   gmt5syntax
   gmt_shell_functions.sh
   gmtswitch
   isogmt
   supplements/img/img2google

.. toctree::
    :hidden:

    blockmean
    blockmedian
    blockmode
    dimfilter
    docs
    filter1d
    fitcircle
    gmt2kml
    gmtconnect
    gmtconvert
    gmtdefaults
    gmtget
    gmtinfo
    gmtlogo_classic
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
    grdcontour_classic
    grdconvert
    grdcut
    grdedit
    grdfft
    grdfill
    grdfilter
    grdgradient
    grdhisteq
    grdimage_classic
    grdinfo
    grdinterpolate
    grdlandmask
    grdmask
    grdmath
    grdpaste
    grdproject
    grdsample
    grdtrack
    grdtrend
    grdvector_classic
    grdview_classic
    grdvolume
    greenspline
    kml2gmt
    makecpt
    mapproject
    nearneighbor
    project
    psbasemap
    psclip
    pscoast
    pscontour
    psconvert
    psevents
    pshistogram
    psimage
    pslegend
    psmask
    psrose
    psscale
    pssolar
    psternary
    pstext
    pswiggle
    psxy
    psxyz
    sample1d
    spectrum1d
    sph2grd
    sphdistance
    sphinterpolate
    sphtriangulate
    splitxyz
    surface
    trend1d
    trend2d
    triangulate
    xyz2grd

.. toctree::
    :hidden:

    supplements/geodesy/earthtide
    supplements/geodesy/gpsgridder
    supplements/geodesy/psvelo
    supplements/gshhg/gshhg
    supplements/img/img2grd
    supplements/mgd77/mgd77convert
    supplements/mgd77/mgd77header
    supplements/mgd77/mgd77info
    supplements/mgd77/mgd77list
    supplements/mgd77/mgd77magref
    supplements/mgd77/mgd77manage
    supplements/mgd77/mgd77path
    supplements/mgd77/mgd77sniffer
    supplements/mgd77/mgd77track_classic
    supplements/potential/gmtflexure
    supplements/potential/gmtgravmag3d
    supplements/potential/gravfft
    supplements/potential/grdflexure
    supplements/potential/grdgravmag3d
    supplements/potential/grdredpol
    supplements/potential/grdseamount
    supplements/potential/talwani2d
    supplements/potential/talwani3d
    supplements/segy/pssegyz
    supplements/segy/pssegy
    supplements/segy/segy2grd
    supplements/seis/pscoupe
    supplements/seis/psmeca
    supplements/seis/pspolar
    supplements/seis/pssac
    supplements/spotter/backtracker
    supplements/spotter/gmtpmodeler
    supplements/spotter/grdpmodeler
    supplements/spotter/grdrotater
    supplements/spotter/grdspotter
    supplements/spotter/hotspotter
    supplements/spotter/originater
    supplements/spotter/polespotter
    supplements/spotter/rotconverter
    supplements/spotter/rotsmoother
    supplements/x2sys/x2sys_binlist
    supplements/x2sys/x2sys_cross
    supplements/x2sys/x2sys_datalist
    supplements/x2sys/x2sys_get
    supplements/x2sys/x2sys_init
    supplements/x2sys/x2sys_list
    supplements/x2sys/x2sys_merge
    supplements/x2sys/x2sys_put
    supplements/x2sys/x2sys_report
    supplements/x2sys/x2sys_solve

Program
-------

- :doc:`gmt`

Core Modules
------------

.. hlist::
    :columns: 6

    - :doc:`blockmean`
    - :doc:`blockmedian`
    - :doc:`blockmode`
    - :doc:`dimfilter`
    - :doc:`docs`
    - :doc:`filter1d`
    - :doc:`fitcircle`
    - :doc:`gmt2kml`
    - :doc:`gmtconnect`
    - :doc:`gmtconvert`
    - :doc:`gmtdefaults`
    - :doc:`gmtget`
    - :doc:`gmtinfo`
    - :doc:`gmtlogo_classic`
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
    - :doc:`grdcontour_classic`
    - :doc:`grdconvert`
    - :doc:`grdcut`
    - :doc:`grdedit`
    - :doc:`grdfft`
    - :doc:`grdfill`
    - :doc:`grdfilter`
    - :doc:`grdgradient`
    - :doc:`grdhisteq`
    - :doc:`grdimage_classic`
    - :doc:`grdinfo`
    - :doc:`grdinterpolate`
    - :doc:`grdlandmask`
    - :doc:`grdmask`
    - :doc:`grdmath`
    - :doc:`grdpaste`
    - :doc:`grdproject`
    - :doc:`grdsample`
    - :doc:`grdtrack`
    - :doc:`grdtrend`
    - :doc:`grdvector_classic`
    - :doc:`grdview_classic`
    - :doc:`grdvolume`
    - :doc:`greenspline`
    - :doc:`kml2gmt`
    - :doc:`makecpt`
    - :doc:`mapproject`
    - :doc:`nearneighbor`
    - :doc:`project`
    - :doc:`psbasemap`
    - :doc:`psclip`
    - :doc:`pscoast`
    - :doc:`pscontour`
    - :doc:`psconvert`
    - :doc:`psevents`
    - :doc:`pshistogram`
    - :doc:`psimage`
    - :doc:`pslegend`
    - :doc:`psmask`
    - :doc:`psrose`
    - :doc:`psscale`
    - :doc:`pssolar`
    - :doc:`psternary`
    - :doc:`pstext`
    - :doc:`pswiggle`
    - :doc:`psxy`
    - :doc:`psxyz`
    - :doc:`sample1d`
    - :doc:`spectrum1d`
    - :doc:`sph2grd`
    - :doc:`sphdistance`
    - :doc:`sphinterpolate`
    - :doc:`sphtriangulate`
    - :doc:`splitxyz`
    - :doc:`surface`
    - :doc:`trend1d`
    - :doc:`trend2d`
    - :doc:`triangulate`
    - :doc:`xyz2grd`

Supplemental Modules
--------------------

.. hlist::
    :columns: 6

    - :doc:`/supplements/geodesy/earthtide`
    - :doc:`/supplements/geodesy/gpsgridder`
    - :doc:`/supplements/geodesy/psvelo`
    - :doc:`/supplements/gshhg/gshhg`
    - :doc:`/supplements/img/img2grd`
    - :doc:`/supplements/mgd77/mgd77convert`
    - :doc:`/supplements/mgd77/mgd77header`
    - :doc:`/supplements/mgd77/mgd77info`
    - :doc:`/supplements/mgd77/mgd77list`
    - :doc:`/supplements/mgd77/mgd77magref`
    - :doc:`/supplements/mgd77/mgd77manage`
    - :doc:`/supplements/mgd77/mgd77path`
    - :doc:`/supplements/mgd77/mgd77sniffer`
    - :doc:`/supplements/mgd77/mgd77track_classic`
    - :doc:`/supplements/potential/gmtflexure`
    - :doc:`/supplements/potential/gmtgravmag3d`
    - :doc:`/supplements/potential/gravfft`
    - :doc:`/supplements/potential/grdflexure`
    - :doc:`/supplements/potential/grdgravmag3d`
    - :doc:`/supplements/potential/grdredpol`
    - :doc:`/supplements/potential/grdseamount`
    - :doc:`/supplements/potential/talwani2d`
    - :doc:`/supplements/potential/talwani3d`
    - :doc:`/supplements/segy/pssegy`
    - :doc:`/supplements/segy/pssegyz`
    - :doc:`/supplements/segy/segy2grd`
    - :doc:`/supplements/seis/pscoupe`
    - :doc:`/supplements/seis/psmeca`
    - :doc:`/supplements/seis/pspolar`
    - :doc:`/supplements/seis/pssac`
    - :doc:`/supplements/spotter/backtracker`
    - :doc:`/supplements/spotter/gmtpmodeler`
    - :doc:`/supplements/spotter/grdpmodeler`
    - :doc:`/supplements/spotter/grdrotater`
    - :doc:`/supplements/spotter/grdspotter`
    - :doc:`/supplements/spotter/hotspotter`
    - :doc:`/supplements/spotter/originater`
    - :doc:`/supplements/spotter/polespotter`
    - :doc:`/supplements/spotter/rotconverter`
    - :doc:`/supplements/spotter/rotsmoother`
    - :doc:`/supplements/x2sys/x2sys_binlist`
    - :doc:`/supplements/x2sys/x2sys_cross`
    - :doc:`/supplements/x2sys/x2sys_datalist`
    - :doc:`/supplements/x2sys/x2sys_get`
    - :doc:`/supplements/x2sys/x2sys_init`
    - :doc:`/supplements/x2sys/x2sys_list`
    - :doc:`/supplements/x2sys/x2sys_merge`
    - :doc:`/supplements/x2sys/x2sys_put`
    - :doc:`/supplements/x2sys/x2sys_report`
    - :doc:`/supplements/x2sys/x2sys_solve`

Utility Scripts
---------------

.. hlist::
    :columns: 6

    - :doc:`gmt-config`
    - :doc:`gmt5syntax`
    - :doc:`gmt_shell_functions.sh`
    - :doc:`gmtswitch`
    - :doc:`isogmt`
    - :doc:`supplements/img/img2google`

Plotting
--------

+-------------------------------+---------------------------------------------------------------------+
| :doc:`gmtlogo_classic`        | Plot the GMT logo on maps                                           |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`grdcontour_classic`     | Contouring of 2-D gridded data sets                                 |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`grdimage_classic`       | Produce images from 2-D gridded data sets                           |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`grdvector_classic`      | Plotting of 2-D gridded vector fields                               |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`grdview_classic`        | 3-D perspective imaging of 2-D gridded data sets                    |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`psbasemap`              | Create a basemap plot                                               |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`psclip`                 | Use polygon files to define clipping paths                          |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`pscoast`                | Plot (and fill) coastlines, borders, and rivers on maps             |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`pscontour`              | Contour or image raw table data by triangulation                    |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`psevents`               | Plot event symbols and labels for a moment in time                  |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`pshistogram`            | Plot a histogram                                                    |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`psimage`                | Plot Sun raster files on a map                                      |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`pslegend`               | Plot a legend on a map                                              |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`psmask`                 | Create overlay to mask out regions on maps                          |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`psrose`                 | Plot sector or rose diagrams                                        |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`psscale`                | Plot gray scale or color scale on maps                              |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`pssolar`                | Plot day-light terminators and other sunlight parameters            |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`psternary`              | Plot data on ternary diagrams                                       |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`pstext`                 | Plot text strings on maps                                           |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`pswiggle`               | Draw table data time-series along track on maps                     |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`psxy`                   | Plot symbols, polygons, and lines on maps                           |
+-------------------------------+---------------------------------------------------------------------+
| :doc:`psxyz`                  | Plot symbols, polygons, and lines in 3-D                            |
+-------------------------------+---------------------------------------------------------------------+

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
| :doc:`grdinterpolate` | Interpolate new layer(s) from a 3-D netCDF data cube                |
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
| :doc:`docs`           | Show HTML documentation of specified module                         |
+-----------------------+---------------------------------------------------------------------+
| :doc:`gmt2kml`        | Like :doc:`plot` but writes KML for use in Google Earth             |
+-----------------------+---------------------------------------------------------------------+
| :doc:`gmtwhich`       | Find full path to specified files                                   |
+-----------------------+---------------------------------------------------------------------+
| :doc:`kml2gmt`        | Extracts coordinates from Google Earth KML files                    |
+-----------------------+---------------------------------------------------------------------+
| :doc:`psconvert`      | Crop and convert PostScript files to raster images, EPS, and PDF    |
+-----------------------+---------------------------------------------------------------------+

geodesy
-------

+--------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/geodesy/earthtide`      | Compute grids or time-series of solid Earth tides                                 |
+--------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/geodesy/gpsgridder`     | Interpolate GPS velocity vectors using Green's functions for a thin elastic sheet |
+--------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/geodesy/psvelo`         | Plot velocity vectors, crosses, and wedges on maps                                |
+--------------------------------------------+-----------------------------------------------------------------------------------+

GSHHG
-----

+---------------------------------+-----------------------------------------------------------+
| :doc:`/supplements/gshhg/gshhg` | Extract data tables from binary GSHHS or WDBII data files |
+---------------------------------+-----------------------------------------------------------+

IMG
---

+----------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/img/img2grd`        | Extract a subset from an img file in Mercator or Geographic format                |
+----------------------------------------+-----------------------------------------------------------------------------------+

MGD77
-----

+-----------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77convert`        | Convert MGD77 data to other file formats                                          |
+-----------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77header`         | Create MGD77 headers from A77 files                                               |
+-----------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77info`           | Extract information about MGD77 files                                             |
+-----------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77list`           | Extract data from MGD77 files                                                     |
+-----------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77magref`         | Evaluate the IGRF or CM4 magnetic field models                                    |
+-----------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77manage`         | Manage the content of MGD77+ files                                                |
+-----------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77path`           | Return paths to MGD77 cruises and directories                                     |
+-----------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77sniffer`        | Along-track quality control of MGD77 cruises                                      |
+-----------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77track_classic`  | Plot track-line map of MGD77 cruises                                              |
+-----------------------------------------------+-----------------------------------------------------------------------------------+

potential
---------

+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/gmtflexure`   | Compute flexural deformation of 2-D loads, forces, and bending moments                 |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/gmtgravmag3d` | Compute the gravity/magnetic anomaly of a 3-D body by the method of Okabe              |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/gravfft`      | Spectral calculations of gravity, isostasy, admittance, and coherence for grids        |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/grdflexure`   | Compute flexural deformation of 3-D surfaces for various rheologies                    |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/grdgravmag3d` | Computes the gravity effect of one (or two) grids by the method of Okabe               |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/grdredpol`    | Compute the Continuous Reduction To the Pole, AKA differential RTP                     |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/grdseamount`  | Create synthetic seamounts (Gaussian, parabolic, cone or disc, circular or elliptical) |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/talwani2d`    | Compute geopotential anomalies over 2-D bodies by the method of Talwani                |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/talwani3d`    | Compute geopotential anomalies over 3-D bodies by the method of Talwani                |
+--------------------------------------------+----------------------------------------------------------------------------------------+

SEGY
----

+-----------------------------------+------------------------------------+
| :doc:`/supplements/segy/pssegyz`  | Plot a SEGY file on a map in 3-D   |
+-----------------------------------+------------------------------------+
| :doc:`/supplements/segy/pssegy`   | Plot a SEGY file on a map          |
+-----------------------------------+------------------------------------+
| :doc:`/supplements/segy/segy2grd` | Converting SEGY data to a GMT grid |
+-----------------------------------+------------------------------------+

seis
----

+----------------------------------+-----------------------------------------------------------+
| :doc:`/supplements/seis/pscoupe` | Plot cross-sections of focal mechanisms                   |
+----------------------------------+-----------------------------------------------------------+
| :doc:`/supplements/seis/psmeca`  | Plot focal mechanisms on maps                             |
+----------------------------------+-----------------------------------------------------------+
| :doc:`/supplements/seis/pspolar` | Plot polarities on the inferior focal half-sphere on maps |
+----------------------------------+-----------------------------------------------------------+
| :doc:`/supplements/seis/pssac`   | Plot seismograms in SAC format on maps                    |
+----------------------------------+-----------------------------------------------------------+

spotter
-------

+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/backtracker`  | Generate forward and backward flowlines and hotspot tracks              |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/gmtpmodeler`  | Evaluate a plate motion model at given locations                        |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/grdpmodeler`  | Evaluate a plate motion model on a geographic grid                      |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/grdrotater`   | Finite rotation reconstruction of geographic grid                       |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/grdspotter`   | Create CVA image from a gravity or topography grid                      |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/hotspotter`   | Create CVA image from seamount locations                                |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/originater`   | Associate seamounts with nearest hotspot point sources                  |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/polespotter`  | Find stage poles given fracture zones and abyssal hills                 |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/rotconverter` | Manipulate total reconstruction and stage rotations                     |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/rotsmoother`  | Get mean rotations and covariance matrices from set of finite rotations |
+------------------------------------------+-------------------------------------------------------------------------+

x2sys
-----

+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_binlist`  | Create bin index listing from track data files                     |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_cross`    | Calculate crossovers between track data files                      |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_datalist` | Extract content of track data files                                |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_get`      | Get track listing from track index database                        |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_init`     | Initialize a new x2sys track database                              |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_list`     | Extract subset from crossover data base                            |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_merge`    | Merge an updated COEs table (smaller) into the main table (bigger) |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_put`      | Update track index database from track bin file                    |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_report`   | Report statistics from crossover data base                         |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_solve`    | Determine least-squares systematic correction from crossovers      |
+------------------------------------------+--------------------------------------------------------------------+
