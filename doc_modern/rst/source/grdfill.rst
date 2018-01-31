.. index:: ! grdfill

*******
grdfill
*******

.. only:: not man

    Interpolate across holes in a grid

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdfill** *ingrid* |-A|\ *algorithm*
|-G|\ *outgrid*
[ |-L|\ [**p**] ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]

|No-spaces|

Description
-----------

**grdfill** will read a grid with one or more holes that are
identified by NaNs.  It offers several algorithms for how those
holes can be replaced with actual data values. 

Required Arguments
------------------

*ingrid*
    The grid file with the holes. (See GRID FILE FORMATS below).

.. _-A:

**-A**\ *algorithm*
    Specify which algorithm should be used to fill in the holes:

    **-Ac**\ *constant*: Replace all NaNs with *constant*.

    **-An**\ *radius*: Replace all NaNs with the nearest neighbor
    node value, within the distance *radius* from each NaN node.
    Default radius is the diagonal of the grid.

.. _-G:

**-G**\ *outgrid*
    *outgrid* is the output grid file after filling the holes. (See GRID FILE FORMATS below).

Optional Arguments
------------------

.. _-L:

**-L**\ [**p**]
    Just list the subregions w/e/s/n of each hole detected.
    No grid fill takes place and **-G** is ignored.
    Append **p** to write polygons corresponding to these regions instead.

.. _-R:

.. |Add_-R| replace:: This defines the subregion of the grid to be read.
.. include:: explain_-R.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_coord.rst_

Examples
--------

Toreplace all NaNs in the grid rough.nc with the value -1, try:

   ::

    gmt grdfill rough.nc -Grough_filled.nc -Ac-1

If we instead wanted to replace the values with estimates of the
nearest neighbors, but only up to a radius of 10 nodes, try

   ::

    gmt grdfill rough.nc -Grough_filled.nc -An10

See Also
--------

:doc:`gmt`, :doc:`grdfilter`
