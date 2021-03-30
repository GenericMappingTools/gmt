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

**grdcut** will produce a new *outgrid* file which is a subregion of
*ingrid*. The subregion is specified with **-R** as in other programs;
the specified range must not exceed the range of *ingrid* (but see **-N**).
If in doubt, run :doc:`grdinfo` to check range. Alternatively, define the subregion
indirectly via a range check on the node values or via distances from a
given point. Finally, you can use **-J** for oblique projections to determine
the corresponding rectangular **-R** setting that will give a grid that fully
covers the oblique domain.
Complementary to **grdcut** there is :doc:`grdpaste`, which
will join together two grid files along a common edge.

Required Arguments
------------------

*ingrid*
    This is the input grid file (see :ref:`Grid File Formats <grd_inout_full>`).

.. _-G:

**-G**\ *outgrid*
    This is the output grid file.

Optional Arguments
------------------

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
    Allow grid to be extended if new **-R** exceeds existing boundaries.
    Append *nodata* value to initialize nodes outside current region [Default is NaN].

.. |Add_-R| replace:: This defines the subregion to be cut out. |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

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
    
See Also
--------

:doc:`gmt`,
:doc:`grdclip`,
:doc:`grdfill`,
:doc:`grdinfo`,
:doc:`grdpaste`,
:doc:`surface`
