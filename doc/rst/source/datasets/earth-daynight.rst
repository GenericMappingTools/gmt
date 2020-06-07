Global Earth Day/Night Images
=============================

.. figure:: /_images/daynight.jpg
   :height: 888 px
   :width: 1774 px
   :align: center
   :scale: 40 %

We serve two NASA image products: Their Blue and Black marbles.
We have filtered and downsampled those as well at the same resolutions that are
available for the Earth DEMs.  However, all images are pixel-registered only.
These images may be plotted with :doc:`/grdimage` or :doc:`/grdview` and manipulated
by :doc:`/grdmix`.  The above example mixes both images according to a day-night
mask and adds illumination from a corresponding Earth DEM.

Usage
-----

We have processed and reformatted this publicly available global imagery
and standardized their file names.  In GMT, you may access a global daytime or
nighttime image by specifying the special names

   @earth_day_\ *rr*\ *u
   @earth_night_\ *rr*\ *u

where *rr* is a 2-digit integer specifying the image resolution in the unit *u*, where
*u* is either **d**, **m** or **s** for arc degree, arc minute or arc second, respectively.
Note there is no file extension specified.
The following codes for *rr*\ *u* are supported:

.. _tbl-earth_relief:

==== ================= =======  =====================================================
Code Dimensions        Size     Description
==== ================= =======  =====================================================
01d       360 x    180  128 KB  1 arc degree image view (Blue|Black Marble @ 111 km)
30m       720 x    360  435 KB  30 arc minute image view (Blue|Black Marble @ 55 km)
20m      1080 x    540  918 KB  20 arc minute image view (Blue|Black Marble @ 37 km)
15m      1440 x    720  1.6 MB  15 arc minute image view (Blue|Black Marble @ 28 km)
10m      2160 x   1080  3.4 MB  10 arc minute image view (Blue|Black Marble @ 18 km)
06m      3600 x   1800  8.8 MB  6 arc minute image view (Blue|Black Marble @ 10 km)
05m      4320 x   2160   13 MB  5 arc minute image view (Blue|Black Marble @ 9 km)
04m      5400 x   2700   19 MB  4 arc minute image view (Blue|Black Marble @ 7.5 km)
03m      7200 x   3600   33 MB  3 arc minute image view (Blue|Black Marble @ 5.6 km)
02m     10800 x   5400   71 MB  2 arc minute image view (Blue|Black Marble @ 3.7 km)
01m     21600 x  10800  258 MB  1 arc minute image view (Blue|Black Marble @ 1.9 km)
30s     43200 x  21600  935 MB  30 arc second image view (Blue|Black Marble original)
==== ================= =======  =====================================================

All of these images will, when downloaded, be placed in your ~/.gmt/server director under
the ``earth/earth_day`` and ``earth/earth_night`` sub-directories.

Technical Information
---------------------

The 01m and lower resolution images are derivatives of NASA's Blue and Black marble images.
We have downsampled them via Cartesian Gaussian filtering to prevent aliasing while preserving
the latitude-dependent resolution in the original images. The full (6 sigma) filter-widths are
indicated in parenthesis.

The dimensions above reflect the number of nodes covered by the global images and the sizes refer
to the geotiff files on the remote server.
