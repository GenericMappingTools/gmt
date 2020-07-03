Global Earth Seafloor Crustal Age Grids
=======================================

.. figure:: /_images/agemap.jpg
   :height: 888 px
   :width: 1774 px
   :align: center
   :scale: 40 %

Numerous geodynamic and geophysical studies consider both the seafloor depth and
the seafloor crustal age. `EarthByte <https://www.earthbyte.org/>`_ has pioneered
the creation of crustal age grids since 1997 and we offer their latest version for
remote use in GMT.

Usage
-----

We have processed and reformatted the EarthByte crustal age grids
and standardized their file names.  In GMT, you may access a global crustal age grid
(or a subset only by using the **-R** option) by specifying the special name

   @earth_age_\ *rr*\ *u*\ [_\ *reg*\ ]

where *rr* is a 2-digit integer specifying the grid resolution in the unit *u*, where
*u* is either **d**, **m** or **s** for arc degree, arc minute or arc second, respectively.
Optionally, you can append _\ **g** or _\ **p** to specifically get the gridline-registered or
pixel-registered version (if they both exist).  If *reg* is not specified we will return
the pixel-registered version unless only the gridline-registered file is available.
The following codes for *rr*\ *u* and the optional *reg* are supported (dimensions are listed
for pixel-registered grids; gridline-registered grids increment dimensions by one):

.. _tbl-earth_age:

==== ================= === =======  ======================================================
Code Dimensions        Reg Size     Description
==== ================= === =======  ======================================================
01d       360 x    180 g,p  128 KB  1 arc degree global seafloor ages (EarthByte @ 111 km)
30m       720 x    360 g,p  435 KB  30 arc minute global relief (EarthByte @ 55 km)
20m      1080 x    540 g,p  918 KB  20 arc minute global relief (EarthByte @ 37 km)
15m      1440 x    720 g,p  1.6 MB  15 arc minute global relief (EarthByte @ 28 km)
10m      2160 x   1080 g,p  3.4 MB  10 arc minute global relief (EarthByte @ 18 km)
06m      3600 x   1800 g,p  8.8 MB  6 arc minute global relief (EarthByte @ 10 km)
05m      4320 x   2160 g,p   13 MB  5 arc minute global relief (EarthByte @ 9 km)
04m      5400 x   2700 g,p   19 MB  4 arc minute global relief (EarthByte @ 7.5 km)
03m      7200 x   3600 g,p   33 MB  3 arc minute global relief (EarthByte @ 5.6 km)
02m     10800 x   5400 g,p   71 MB  2 arc minute global relief (EarthByte @ 3.7 km)
01m     21600 x  10800 g,p  258 MB  1 arc minute global relief (EarthByte original)
==== ================= === =======  ======================================================

All of these data will, when downloaded, be placed in your ~/.gmt/server directory, with
the earth_age files being placed in an ``earth/earth_age`` sub-directory.

Technical Information
---------------------

We scale and reformat the original data to take up very little space so that downloads
from the servers are as fast as possible.  For the seafloor crustal age grid this means
we chose 0.01 My as the smallest data unit, which is well below the uncertainties in the
model.  Data are scaled and shifted to fit in a short integer grid that is highly compressed
by netCDF lossless compression and chunking.

Data References
---------------

#. Seton et al. [2020, in review] [https://www.earthbyte.org/category/resources/data-models/seafloor-age/].
