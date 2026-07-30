.. index:: ! grdcut
.. include:: module_core_purpose.rst_

******
grdcut
******

|grdcut_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdcut**
*ingrid*
|-G|\ *outgrid*
|SYN_OPT-R|
[ |-D|\ [**+t**] ]
[ |-E|\ **x**\|\ **y**\ *coord* ]
[ |-F|\ *polygonfile*\ [**+c**][**+i**] ]
[ |-J|\ *parameters* ]
[ |-N|\ [*nodata*] ]
[ |-S|\ *lon/lat/radius*\ [**+n**] ]
[ |SYN_OPT-V| ]
[ |-Z|\ [*min/max*]\ [**+n**\|\ **N**\|\ **r**] ]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdcut** will produce a new *outgrid* file which is a sub-region of
*ingrid*. The sub-region may be specified with |-R| as in other programs;
the specified range must not exceed the range of *ingrid* (but see |-N|).
If in doubt, run :doc:`grdinfo` to check range. Alternatively, define the
sub-region indirectly via a range check on the node values or via distances from
a fixed point. Furthermore, you can use |-J| for oblique projections to determine
the corresponding rectangular |-R| setting that will give a sub-region that fully
covers the oblique domain.  You can use |-F| to specify a polygon and either use
its bounding box for sub-region or set grid nodes inside or outside the polygon
to NaN. Finally, if the input is a 3-D netCDF cube then you can make a vertical
slice through existing nodes. **Note**: If the input grid is actually an image (gray-scale,
RGB, or RGBA), then options |-N| and |-Z| are unavailable, while for multi-layer
GeoTIFF files only options |-R|, |-S| and |-G| are supported, i.e., you can cut out
a sub-region only (which we do via *gdal_translate* if you have multiple bands).
Complementary to **grdcut** there is :doc:`grdpaste`, which will join together
two grid files (not images) along a common edge.

Required Arguments
------------------

.. |Add_ingrid| replace:: Input grid file.
.. include:: explain_grd_inout.rst_
    :start-after: ingrid-syntax-begins
    :end-before: ingrid-syntax-ends

.. _-G:

.. |Add_outgrid| replace:: Give the name of the output grid file.
.. include:: /explain_grd_inout.rst_
    :start-after: outgrid-syntax-begins
    :end-before: outgrid-syntax-ends

.. |Add_-R| replace:: This defines the subregion to be cut out. |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

Optional Arguments
------------------

.. _-D:

**-D**\ [**+t**]
    A "dry run": Simply report the region and increment of what would be the
    extracted grid given the selected options.  No grid is created (|-G| is disallowed)
    and instead we write a single data record with *west east south north xinc yinc*
    to standard output. The increments will reflect the input grid unless it is a
    remote gridded data set without implied resolution. Append **+t** to instead receive
    the information as the trailing string "-Rwest/east/south/north -Ixinc/yinc".

.. _-E:

**-E**\ **x**\|\ **y**\ *coord*
    We extract a vertical slice going along the **x**\ -column *coord* or along the
    **y**\ -row *coord*, depending on the given directive.
    **Note**: (1) Input file must be a 3-D netCDF cube, and |-E| can only be used with
    option |-G|. (2) *coord* must exactly match the coordinates given by th cube. We are
    not interpolating between nodes and only do a clean slice through existing cube nodes.

.. _-F:

**-F**\ *polygonfile*\ [**+c**][**+i**]
    Specify a multisegment closed polygon file.  All grid nodes outside the
    polygon will be set to NaN.  Append **+i** to invert that and set all
    nodes inside the polygon to NaN instead. Optionally, append **+c** to
    crop the grid region to reflect the bounding box of the polygon.

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-N:

**-N**\ [*nodata*]
    Allow grid to be extended if new |-R| exceeds existing boundaries.
    Append *nodata* value to initialize nodes outside current region [Default is NaN].

.. _-S:

**-S**\ *lon/lat/radius*\ [**+n**]
    Specify an origin and radius; append a distance unit (see `Units`_) and
    we determine the corresponding rectangular region so that all grid
    nodes on or inside the circle are contained in the subset. If
    **+n** is appended we set all nodes outside the circle to NaN.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**\ [*min/max*]\ [**+n**\|\ **N**\|\ **r**]
    Determine a new rectangular region so that all nodes *outside* this
    region are also outside the given *z*-range [-inf/+inf]. To indicate
    no limit on min or max only, specify a hyphen (-). Normally, any NaNs
    encountered are simply skipped and not considered in the range-decision.
    Append **+n** to consider a NaN to be outside the given *z*-range. This means
    the new subset will be NaN-free. Alternatively, append **+r** to
    consider NaNs to be within the data range. In this case we stop
    shrinking the boundaries once a NaN is found [Default simply skips NaNs
    when making the range decision].  Finally, if your core subset grid is
    surrounded by rows and/or columns that are all NaNs, append **+N** to
    strip off such columns before (optionally) considering the range of the
    core subset for further reduction of the area.

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_grd_coord.rst_

Examples
--------

.. include:: explain_example.rst_

To obtain data for an oblique Mercator projection map we need to extract
more data that is actually used. This is necessary because the output of
**grdcut** has edges defined by parallels and meridians, while the
oblique map in general does not. Hence, to get all the data from the
ETOPO2 data needed to make a contour map for the region defined by its
lower left and upper right corners and the desired projection, use::

    gmt grdcut @earth_relief_02m -R160/20/220/30+r -Joc190/25.5/292/69/1 -Gdata.nc

Suppose you have used :doc:`surface` to grid ship gravity in the region
between 148E - 162E and 8N - 32N, and you do not trust the gridding near
the edges, so you want to keep only the area between 150E - 160E and 10N - 30N, then::

    gmt grdcut grav_148_162_8_32.nc -Ggrav_150_160_10_30.nc -R150/160/10/30 -V

To return the subregion of a grid such that any boundary strips where
all values are entirely above 0 are excluded, try::

    gmt grdcut bathy.nc -Gtrimmed_bathy.nc -Z-/0 -V

To return the subregion of a grid such that any boundary rows or columns
that are all NaNs, try::

    gmt grdcut bathy.nc -Gtrimmed_bathy.nc -Z+N -V

To return the subregion of a grid that contains all nodes within a
distance of 500 km from the point 45,30 try::

    gmt grdcut bathy.nc -Gsubset_bathy.nc -S45/30/500k -V

To create a topography grid with data only inside France and set it
to NaN outside France, based on the 10x10 minute DEM, try::

    gmt coast -EFR -M > FR.txt
    gmt grdcut @earth_relief_10m -FFR.txt+c -GFR_only.grd
    gmt grdimage FR_only.grd -B -pdf map

To determine what grid region and resolution (in text format) most suitable for a 24 cm wide map
that is using an oblique projection to display the remote Earth Relief data grid, try::

    gmt grdcut @earth_relief -R270/20/305/25+r -JOc280/25.5/22/69/24c -D+t -V

To extract a vertical grid slice at *x = 35* and parallel to the *y-z* plane
from the 3-D model seis3D.nc, try::

    gmt grdcut seis3D.nc -Gslice_x35.nc -Ex35 -V

Notes
-----

If the input file is a GeoTIFF with multiple data bands then the output format will
depend on your selection (if any) of the bands to keep: If you do not specify
any bands (which means we consider all the available bands) or you select more
than one band, then the output file can either be another GeoTIFF (if you give
a .tif[f] extension) or it can be a multiband netCDF file (if you give a .nc or .grd
extension). If you select a single band from the input GeoTIFF then GMT will
normally read that in as a single grid layer and thus write a netCDF grid (unless
you append another grid format specifier). However, if your output filename has
a .tif[f] extension then we will instead write it as a one-band GeoTIFF.
All GeoTIFF output operations are done via GDAL's *gdal_translate*.

See Also
--------

:doc:`gmt`,
:doc:`grdclip`,
:doc:`grdfill`,
:doc:`grdinfo`,
:doc:`grdpaste`,
:doc:`surface`
