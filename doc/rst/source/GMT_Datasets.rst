Global Grids
============

In addition to the GSHHS coastlines, rivers, and borders data built into some
modules (:doc:`coast`, :doc:`gmtselect`, :doc:`grdlandmask`), we offer several
remote global data grids that you can access via our *remote file* mechanism.
The first time you access one of these files we will download the file from
the GMT server and save it to the *server* directory under your GMT user directory (~/.gmt).
We then read the file from there.  Should you have limited space then there
are ways to control how much is downloaded (see below).

Global Earth Relief Grids
-------------------------

We have processed and reformatted various publicly available global relief
grids and standardized their file names.  In GMT, you may access a global relief grid
(or a subset only by using the **-R** option) by specifying the special name

   @earth_relief_\ *rr*\ *u*

where *rr* is a 2-digit integer specifying the grid resolution in the unit
*u*, where *u* is either **m** or **s** for arc minute or arc second, respectively.
The following codes for *rr*\ *u* are supported:

.. _tbl-remotefiles:

    +------+------------------+--------+-----------------------------------------------+
    | Code | Dimensions       | Size   | Description                                   |
    +======+==================+========+===============================================+
    | 60m  |     361 x    181 | 111 KB | 60 arc minute global relief (ETOPO1 @ 111 km) |
    +------+------------------+--------+-----------------------------------------------+
    | 30m  |     721 x    361 | 376 KB | 30 arc minute global relief (ETOPO1 @ 55 km)  |
    +------+------------------+--------+-----------------------------------------------+
    | 20m  |    1081 x    541 | 782 KB | 20 arc minute global relief (ETOPO1 @ 37 km)  |
    +------+------------------+--------+-----------------------------------------------+
    | 15m  |    1441 x    721 | 1.3 MB | 15 arc minute global relief (ETOPO1 @ 28 km)  |
    +------+------------------+--------+-----------------------------------------------+
    | 10m  |    2161 x   1081 | 2.8 MB | 30 arc minute global relief (ETOPO1 @ 18 km)  |
    +------+------------------+--------+-----------------------------------------------+
    | 06m  |    3601 x   1801 | 7.4 MB | 6 arc minute global relief (ETOPO1 @ 10 km)   |
    +------+------------------+--------+-----------------------------------------------+
    | 05m  |    4321 x   2161 |  11 MB | 5 arc minute global relief (ETOPO1 @ 9 km)    |
    +------+------------------+--------+-----------------------------------------------+
    | 04m  |    5401 x   2701 |  16 MB | 4 arc minute global relief (ETOPO1 @ 7.5 km)  |
    +------+------------------+--------+-----------------------------------------------+
    | 03m  |    7201 x   3601 |  27 MB | 3 arc minute global relief (ETOPO1 @ 5.6 km)  |
    +------+------------------+--------+-----------------------------------------------+
    | 02m  |   10801 x   5401 |  58 MB | 2 arc minute global relief (ETOPO2v2)         |
    +------+------------------+--------+-----------------------------------------------+
    | 01m  |   21601 x  10801 | 214 MB | 1 arc minute global relief (ETOPO1)           |
    +------+------------------+--------+-----------------------------------------------+
    | 30s  |   43201 x  21601 | 778 MB | 30 arc second global relief (SRTM30+)         |
    +------+------------------+--------+-----------------------------------------------+
    | 15s  |   86401 x  43201 | 2.6 GB | 15 arc second global relief (SRTM15+)         |
    +------+------------------+--------+-----------------------------------------------+
    | 03s  |  432001 x 216001 | 6.8 GB | 3 arc second global relief (SRTM3S)           |
    +------+------------------+--------+-----------------------------------------------+
    | 01s  | 1296001 x 432001 |  41 GB | 1 arc second global relief (SRTM1S)           |
    +------+------------------+--------+-----------------------------------------------+

All of these data will, when downloaded, be placed in your ~/.gmt/server directory, with
the SRTM data organized in sub-directories srtm1 and srtm3 within the server directory.

Technical Information
~~~~~~~~~~~~~~~~~~~~~

As you see, the 03m and lower resolutions are all derivatives of NOAA's ETOPO1 grid.  We have
downsampled it via spherical Gaussian filtering to prevent aliasing.  The full (6 sigma)
filter-widths are indicated in parenthesis. For 2 arc minute we use the original ETOPO2v2
file from NOAA (with ice_surface).  The 30 and 15 arc second grids are the global SRTM30+
and SRTM15+ products from Dave Sandwell (Scripps), while the 3 and 1 arc second data are
the SRTM 1x1 degree tiles from NASA.  When the 15s or lower resolution grids are accessed
the first time we download the entire file, regardless of your selected region (**-R**).
However, for the SRTM tiles we only download the tiles that are inside your selected region
the first time they are referenced. Also note that the 3 and 1 arc second grids only extend
to latitudes ±60˚. The SRTM tiles are only valid over land.  However, when these grids
are accessed as @earth_relief_01s or @earth_relief_03s we will automatically upsample the
@earth_relief_15s grid to fill in the missing ocean values (but only if the region includes oceanic areas).
If you just want the original land-only
SRTM tiles you may use @srtm_relief_03s or @srtm_relief_01s instead. All grids are gridline-registered.

The dimensions above reflect the number of nodes covered by the global grids and the sizes are
the file sizes of the netCDF-4 compressed short int grids, making the files much smaller
than their original source files without any loss of precision.  To improve download speed,
the SRTM tiles are stored as JPEG2000 images on the GMT server due to superior compression,
but once downloaded to your server directory they are converted to short int compressed netCDF4 grids.
This step uses our GDAL bridge and thus requires that you have built GMT with GDAL support
*and* that your GDAL distribution was built with openjpeg support.

Data Space Concerns
~~~~~~~~~~~~~~~~~~~

There are several ways you can control the amount of space taken up by your server directory:

#. You can set an upper file size limit for download via the GMT default setting
   :ref:`GMT_DATA_URL_LIMIT <GMT_DATA_URL_LIMIT>`; the default is unlimited.
#. You can remove the entire server directory via gmt :doc:`clear`.
#. You can be clever and set up a crontab job that deletes data files you have not
   touched in, say, 6 months (or some other interval).

Data References
~~~~~~~~~~~~~~~

#. ETOPO2v2 [https://dx.doi.org/10.7289/V5J1012Q].
#. ETOPO1: Amante, C., and B. W. Eakins (2008), ETOPO1 1 arc-minute global relief model: Procedures, data sources and analysisRep., National Geophysical Data Center, Boulder, CO [https://www.ngdc.noaa.gov/mgg/global/relief/ETOPO1/data/ice_surface/grid_registered/netcdf/ETOPO1_Ice_g_gmt4.grd.gz].
#. SRTM30+: Becker, J. J., et al. (2009), Global Bathymetry and Elevation Data at 30 Arc Seconds Resolution: SRTM30_PLUS, Marine Geodesy, 32, 355–371 [ftp://topex.ucsd.edu/pub/srtm30_plus/topo30/topo30.grd].
#. SRTM15+: Olson, C. L., J. J. Becker, and D. T. Sandwell (2014), A new global bathymetry map at 15 arcsecond resolution for resolving seafloor fabric: SRTM15_PLUS, in Eos Trans. AGU, edited, pp. Abstract OS34A-03 [ftp://topex.ucsd.edu/pub/srtm15_plus/topo15.grd].
#. SRTMGL3 tiles: [https://lpdaac.usgs.gov/dataset_discovery/measures/measures_products_table/srtmgl3_v003].
#. SRTMGL1 tiles: [https://lpdaac.usgs.gov/dataset_discovery/measures/measures_products_table/srtmgl1_v003].
