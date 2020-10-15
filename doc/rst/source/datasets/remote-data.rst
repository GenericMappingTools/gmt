Remote Data Sets
================

A *remote data set* is a data set that is stored on one or more remote servers. It may
be a single grid file or a collection of subset tiles making up a larger grid. They
are not distributed with GMT or installed during the installation procedures.
GMT offers several remote global data grids that you can access via our *remote file* mechanism.
The first time you access one of these files, GMT will download the file (or a subset tile) from
the selected GMT server and save it to the *server* directory under your GMT user directory [~/.gmt].
From then on we read the local file from there.

By using the remote file mechanism you should know that these files, on the server, will change
from time to time (i.e., new versions are released, a problem in one file is fixed, or a dataset
becomes obsolete), and GMT will take actions accordingly.  It is our policy to only supply the *latest*
version of any dataset that undergoes revisions.  If you require previous versions for your work you
will need to get those data from the data provider separately.  Unless you deactivate the remote data service,
GMT will do the following when you request a remote file in a GMT command:

#. We check if the locally cached catalog with information about the data available from the server
   is up-to-date or if it needs to be refreshed.  If the file is older that the :term:`GMT_DATA_UPDATE_INTERVAL`
   limit then we refresh the catalog.
#. When the catalog is refreshed, we determine the publication date for each dataset on the server,
   and if any local copies you may have are now obsolete we will remove them to force a re-download from the server.

Usage
-----

We have processed and reformatted publicly available global data sets (grids and images)
and standardized their file names.  In GMT, you may access such data
(or a subset only by using the **-R** option) by specifying the special name

   @remote_name_\ *rr*\ *u*\ [_\ *reg*\ ]

where the leading @ symbol identifies the file as a remote data set, the *remote_name_* is specific
to the dataset and the *rr* code is a 2-digit integer specifying the grid/image
resolution in the unit *u*, where *u* is either **d**, **m** or **s** for arc degree, arc minute or
arc second, respectively. Optionally, you can append _\ **g** or _\ **p** to specifically get the
gridline-registered or pixel-registered version (if they both exist).  If *reg* is not specified we
will return the pixel-registered version unless only the gridline-registered file is available.  If you
do specify a specific registration and that version is not available you will get an error message.
The codes for *rr*\ *u* and the optional *reg* that are supported will be listed in the sections
below describing each of the available data sets.


Currently, GMT provides the following datasets (with their special names in parentheses)

- `Global Earth Relief Grids`_ (``earth_relief``)
- `Global Earth Seafloor Crustal Age Grids`_ (``earth_age``)
- `Global Earth Day/Night Images`_ (``earth_day`` and ``earth_night``)
- `Global Earth Mask Grids`_ (``earth_mask``)

Many of the remote datasets have a preferred, default color table that will be used unless you
override that default by giving your desired CPT information.

Controlling the Process
-----------------------

There are several ways you can control the remote data process and the amount of space taken up by your
own server directory:

#. You can select the GMT data server closest to you to minimize download time [:term:`GMT_DATA_SERVER`].
#. You can set an upper limit on the file sizes that may be downloaded [:term:`GMT_DATA_SERVER_LIMIT`].
#. You can turn off the automatic download temporarily [:term:`GMT_AUTO_DOWNLOAD`].
#. You can control how often GMT will refresh the catalog of information on your computer
   [:term:`GMT_DATA_UPDATE_INTERVAL`]
#. You can clear the *server* directory, or perhaps just some subsets, any time via gmt :doc:`/clear`.

Offline Usage
-------------

If you anticipate to be without an Internet connection (or have a very slow one), you can download
all (or some) of the remote files prior to losing connection with the module :doc:`/gmtget`. You
can choose which data to download and limit it to node spacings larger or equal to a limit, and you
can minimize space on your computer by requesting that any JPEG2000 tiles *not* be converted until GMT
is accessing them.  Here are some examples of usage.  Download the entire cache directory used
in examples and tests::

    gmt get -Dcache

Get all the data for Earth but only for 1 arc minute and coarser, and leave tiles in JPEG2000 format::

    gmt get -Ddata=earth -I1m -N

As shown in the tables below, the largest datasets may take some time to download the data from GMT
server, so be patient!

File Compression
----------------

Typically, a dataset is released by the data provider in a single, high-resolution format.
To optimize use of these data in GMT and to prevent download bottlenecks we have downsampled
them via Cartesian Gaussian filtering to prevent aliasing while preserving the latitude-dependent
resolution in the original grid or image. To improve responsiveness, the larger files (i.e., currently
for node spacings 05m and smaller) have been split into smaller tiles.  When the 06m or lower resolution
files are accessed the first time we download the entire file, regardless of your selected region (**-R**).
However, for the tiled data sets we only download the tiles that intersect your selected region
the first time they are referenced.

Single grids are provided as netCDF-4 maximum-lossless compressed short int grids, making the files
much smaller than their original source files without any loss of precision.  To minimize download
speed, the dataset tiles are all stored as JPEG2000 images on the GMT server due to superior compression,
but once downloaded to your server directory they are converted to the same short int compressed netCDF4
format for easier access. This step uses our GDAL bridge and thus requires that you have built GMT with
GDAL support *and* that your GDAL distribution was built with *openjpeg* support.


.. _jp2_compression:

.. figure:: /_images/srtm1.*
   :width: 500 px
   :align: center

   Histogram of compression rates for the SRTM 1x1 arc second tiles.  100% reflects the full short integer
   size of an uncompressed tile (~25 Mb).  As can be seen, on average a JPEG2000 tile is only half the
   size of the corresponding fully compressed (level 9) netCDF short int grid.  This is why we
   have chosen the JP2 format for tiles on the server.

Cache File Updates
------------------

Remote cache files are our collection of miscellaneous files that are used throughout the GMT examples,
man pages, and test suite.  There is no system nor catalog and files come and go as we need them. The cache
files are subject to similar rules as the remote data set when it comes to refreshing or deleting them.
If any of these files is precious to you we suggest you make a copy somewhere.

Getting a single grid
---------------------

Should you need a single grid from any of our tiled dataset, e.g., to feed into other programs that do
not depend on GMT, you can create that via :doc:`/grdcut`.  For instance, to make a global grid from the
eight tiles that make up the 2m x 2m gridline-registered data, try::

    gmt grdcut @earth_relief_02m_g -Gearth_at_2m.grd

----

.. include:: earth-relief.rst_

----

.. include:: earth-age.rst_

----

.. include:: earth-daynight.rst_

----

.. include:: earth-masks.rst_
