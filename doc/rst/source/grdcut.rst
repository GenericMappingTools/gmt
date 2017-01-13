.. index:: ! grdcut

******
grdcut
******

.. only:: not man

    grdcut - Extract subregion from a grid

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**grdcut** *ingrid* |-G|\ *outgrid*
|SYN_OPT-R|
[ |-N|\ [*nodata*] ]
[ |-S|\ [**n**]\ *lon/lat/radius*\ [*unit*] ]
[ |SYN_OPT-V| ]
[ |-Z|\ [\ **n**\ \|\ **r**]\ *min/max* ]
[ |SYN_OPT-f| ]

|No-spaces|

Description
-----------

**grdcut** will produce a new *outgrid* file which is a subregion of
*ingrid*. The subregion is specified with **-R** as in other programs;
the specified range must not exceed the range of *ingrid* (but see **-N**).
If in doubt, run :doc:`grdinfo` to check range. Alternatively, define the subregion
indirectly via a range check on the node values or via distances from a
given point. Complementary to **grdcut** there is :doc:`grdpaste`, which
will join together two grid files along a common edge. 

Required Arguments
------------------

*ingrid*
    This is the input grid file.

.. _-G:

**-G**\ *outgrid*
    This is the output grid file.

Optional Arguments
------------------

.. _-N:

**-N**\ [*nodata*]
    Allow grid to be extended if new **-R** exceeds existing boundaries.
    Append *nodata* value to initialize nodes outside current region [Default is NaN].

.. _-R:

.. |Add_-R| replace:: This defines the subregion to be cut out.
.. include:: explain_-R.rst_

.. _-S:

**-S**\ [**n**]\ *lon/lat/radius*\ [*unit*]
    Specify an origin and radius; append a distance unit (see UNITS) and
    we determine the corresponding rectangular region so that all grid
    nodes on or inside the circle are contained in the subset. If
    **-Sn** is used we set all nodes outside the circle to NaN. 

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-Z:

**-Z**\ [\ **n**\ \|\ **r**]\ *min/max*
    Determine the new rectangular region so that all nodes *outside* this
    region are also outside the given *z*-range [-inf/+inf]. To indicate
    no limit on min or max, specify a hyphen (-). Normally, any NaNs
    encountered are simply skipped and not considered in the decision.
    Use **-Zn** to consider a NaN to be outside the *z*-range. This means
    the new subset will be NaN-free. Alternatively, use **-Zr** to
    consider NaNs to be within the data range. In this case we stop
    shrinking the boundaries once a NaN is found [Default simply skips NaNs
    when making the range decision].

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_grd_inout_short.rst_

.. include:: explain_grd_coord.rst_

Examples
--------

Suppose you have used :doc:`surface` to grid ship gravity in the region
between 148E - 162E and 8N - 32N, and you do not trust the gridding near
the edges, so you want to keep only the area between 150E - 160E and 10N - 30N, then:

   ::

    gmt grdcut grav_148_162_8_32.nc -Ggrav_150_160_10_30.nc -R150/160/10/30 -V

To return the subregion of a grid such that any boundary strips where
all values are entirely above 0 are excluded, try

   ::

    gmt grdcut bathy.nc -Gtrimmed_bathy.nc -Z-/0 -V

To return the subregion of a grid that contains all nodes within a
distance of 500 km from the point 45,30 try

   ::

    gmt grdcut bathy.nc -Gsubset_bathy.nc -S45/30/500k -V

See Also
--------

:doc:`gmt`,
:doc:`grdclip`,
:doc:`grdinfo`,
:doc:`grdpaste`,
:doc:`surface`
