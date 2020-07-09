Global Earth Relief Grids
=========================

.. figure:: /_images/dem.jpg
   :height: 888 px
   :width: 1774 px
   :align: center
   :scale: 40 %

In addition to the GSHHG coastlines, rivers, and borders data built into some
modules (:doc:`/coast`, :doc:`/gmtselect`, :doc:`/grdlandmask`), we offer several
remote global data grids that you can access via our *remote file* mechanism.
The first time you access one of these files we will download the file from
the GMT server and save it to the *server* directory under your GMT user directory (~/.gmt).
We then read the file from there.  Should you have limited space then there
are ways to control how much is downloaded (see below).

Usage
-----

We have processed and reformatted publicly available global relief
grids and standardized their file names.  In GMT, you may access a global relief grid
(or a subset only by using the **-R** option) by specifying the special name

   @earth_relief_\ *rr*\ *u*\ [_\ *reg*\ ]

where *rr* is a 2-digit integer specifying the grid resolution in the unit *u*, where
*u* is either **d**, **m** or **s** for arc degree, arc minute or arc second, respectively.
Optionally, you can append _\ **g** or _\ **p** to specifically get the gridline-registered or
pixel-registered version (if they both exist).  If *reg* is not specified we will return
the pixel-registered version unless only the gridline-registered file is available.
The following codes for *rr*\ *u* and the optional *reg* are supported (dimensions are listed
for pixel-registered grids; gridline-registered grids increment dimensions by one):

.. _tbl-earth_relief:

==== ================= === =======  ==================================================
Code Dimensions        Reg Size     Description
==== ================= === =======  ==================================================
01d       360 x    180 g,p  128 KB  1 arc degree global relief (SRTM15+V2.1 @ 111 km)
30m       720 x    360 g,p  435 KB  30 arc minute global relief (SRTM15+V2.1 @ 55 km)
20m      1080 x    540 g,p  918 KB  20 arc minute global relief (SRTM15+V2.1 @ 37 km)
15m      1440 x    720 g,p  1.6 MB  15 arc minute global relief (SRTM15+V2.1 @ 28 km)
10m      2160 x   1080 g,p  3.4 MB  10 arc minute global relief (SRTM15+V2.1 @ 18 km)
06m      3600 x   1800 g,p  8.8 MB  6 arc minute global relief (SRTM15+V2.1 @ 10 km)
05m      4320 x   2160 g,p   13 MB  5 arc minute global relief (SRTM15+V2.1 @ 9 km)
04m      5400 x   2700 g,p   19 MB  4 arc minute global relief (SRTM15+V2.1 @ 7.5 km)
03m      7200 x   3600 g,p   33 MB  3 arc minute global relief (SRTM15+V2.1 @ 5.6 km)
02m     10800 x   5400 g,p   71 MB  2 arc minute global relief (SRTM15+V2.1 @ 3.7 km)
01m     21600 x  10800 g,p  258 MB  1 arc minute global relief (SRTM15+V2.1 @ 1.9 km)
30s     43200 x  21600 g,p  935 MB  30 arc second global relief (SRTM15+V2.1 @ 1.0 km)
15s     86400 x  43200 p    3.2 GB  15 arc second global relief (SRTM15+V2.1)
03s    432000 x 216000 g    6.8 GB  3 arc second global relief (SRTM3S)
01s   1296000 x 432000 g     41 GB  1 arc second global relief (SRTM1S)
==== ================= === =======  ==================================================

All of these data will, when downloaded, be placed in your ~/.gmt/server directory, with
the earth_relief files being placed in an ``earth/earth_relief`` sub-directory.  If you
do not specify a CPT then this dataset default to the GMT master *geo*.

Technical Information
---------------------

As you see, the 30s and lower resolutions are all derivatives of Scripps' SRTM15+V2.1 grid
(Tozer et al., 2019).  We have downsampled it via Cartesian Gaussian filtering to prevent
aliasing while preserving the latitude-dependent resolution in the original 15 arc sec grid.
The full (6 sigma) filter-widths are indicated in parenthesis. The 3 and 1 arc second data
are the SRTM 1x1 degree tiles from NASA.  To improve responsiveness, the larger files (i.e.,
for grid spacings 05m and smaller) have been tiled as well.  When the 06m or lower resolution
grids are accessed the first time we download the entire file, regardless of your selected region (**-R**).
However, for the tiled data sets we only download the tiles that are inside your selected region
the first time they are referenced. **Note**: The 3 and 1 arc second grids only extend
to latitudes ±60˚ and are only available over land.  When these grids are accessed as
@earth_relief_01s or @earth_relief_03s we will automatically up-sample the relevant @earth_relief_15s
tiles to fill in the missing ocean values. If you just want the original land-only SRTM tiles
you may use the special names @srtm_relief_03s or @srtm_relief_01s instead. Almost all grids
are available in both gridline- and pixel-registered formats except the original pixel-registered
SRTM15+V2.1 (here called @earth_relief_15s) and the gridline-registered SRTM tiles.

The dimensions above reflect the number of nodes covered by the global grids and the sizes refer
to the files on the remote server.  For single grids, these are already in the final netCDF-4
compressed short int grids, making the files much smaller than their original source files without
any loss of precision.  To minimize download speed, the dataset tiles are all stored as JPEG2000
images on the GMT server due to superior compression, but once downloaded to your server directory 
they are converted to the same short int compressed netCDF4 format for easier access. This step
uses our GDAL bridge and thus requires that you have built GMT with GDAL support
*and* that your GDAL distribution was built with openjpeg support.


.. _jp2_compression:

.. figure:: /_images/srtm1.*
   :width: 500 px
   :align: center

   Histogram of compression rates for the SRTM 1x1 arc second tiles.  100% reflects the full short integer
   size of an uncompressed tile (~25 Mb).  As can be seen, on average a JPEG2000 tile is only half the
   size of the corresponding fully compressed (level 9) netCDF short int grid.  This is why we
   have chosen the JP2 format for tiles on the server.

Data Space Concerns
-------------------

There are several ways you can control the amount of space taken up by your own server directory:

#. You can set an upper file size limit for download via the GMT default setting
   :term:`GMT_DATA_SERVER_LIMIT`; the default is unlimited.
#. You can remove the entire server directory via gmt :doc:`/clear` data.
#. You can be clever and set up a crontab job that deletes data files you have not
   touched in, say, 6 months (or some other interval).

Offline Usage
-------------

If you anticipate to be without an Internet connection (or a very slow one), you can download
all the remote files prior to losing connection, using the module :doc:`/gmtget`. You can choose
which data to download and limit it to grid spacings larger or equal to a limit, and you can
minimize space on your computer by requesting the JPEG2000 tiles not be converted until GMT
is accessing them.  Here are some examples of usage.  Download the entire cache directory used
in examples and tests::

    gmt get -Dcache

To get all the data for Earth but only for 30 arc sec and coarser, and leave as JPEG2000 tiles::

    gmt get -Ddata=earth -I30s -N

As shown in the table, the largest dataset may take some time to download the data from GMT server. Be patient!

.. include:: ../data-updating.rst_

Data References
---------------

#. SRTM15+V2.1 [http://dx.doi.org/10.1029/2019EA000658].
#. SRTMGL3 tiles: [https://lpdaac.usgs.gov/products/srtmgl3v003].
#. SRTMGL1 tiles: [https://lpdaac.usgs.gov/products/srtmgl1v003].
