.. index:: ! grdfill

*******
grdfill
*******

.. only:: not man

    Interpolate across holes in a grid

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdfill** *ingrid*
|-A|\ *mode*\ [*arg*]
|-G|\ *outgrid*
[ |SYN_OPT-R| ]
[ |-L|\ [**p**] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdfill** reads a grid that presumably has unfilled holes that the user
wants to fill in some fashion.  Holes are identified by NaN values but
this criteria can be changed.  There are several different algorithms that
can be used to replace the hole values.

Required Arguments
------------------

*ingrid*
    This is the input grid file.

.. _-A:

**-A**\ *mode*\ [*arg*]
    Specify the hole-filling algorithm to use.  Choose from **c** for constant
    fill and append the constant value, **n** for nearest neighbor and optionally
    append a search radius in pixels, or **s** for bicubic spline [NOT IMPLEMENTED YET].

.. _-G:

**-G**\ *outgrid*
    This is the output grid file.

Optional Arguments
------------------

.. _-N:

**-N**\ [*nodata*]
    Sets the node value that identifies a point as a member of a hole [Default is NaN].

.. _-R:

.. |Add_-R| replace:: This defines the subregion to be cut out.
.. include:: explain_-R.rst_

.. _-L:

**-L**\ [**p**]
    Just list the rectangular subregions west east south north of each hole.
    No grid fill takes place and **-G** is ignored. Optionally, append **p**
    to instead write closed polygons for all subregions.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_coord.rst_

Examples
--------

To identify all regions with NaNs in the grid data.grd and create a listing of the
bounding coordinates of rectangular regions that would cover these NaN areas, try

   ::

    gmt grdfill data.grd -L > west_listing.txt

To identify the same areas but this time write a multisegment file with polygons
corresponding to the rectangular subregions, use

   ::

    gmt grdfill data.grd -Lp > NaN_regions.txt

To replace all NaN values in the file data.grd with the value 999.0, use

   ::

    gmt grdfill data.grd -Ac999 -Gno_NaNs_data.grd


To replace all NaN values in the file data.grd with the values at the
nearest non-NaN neighbor, try

   ::

    gmt grdfill data.grd -An -Gno_NaNs_NN_data.grd


See Also
--------

:doc:`gmt`,
:doc:`grdcut`,
:doc:`grdclip`,
:doc:`grdedit`,
:doc:`grdinfo`
