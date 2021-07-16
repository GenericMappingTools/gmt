.. index:: ! grdfill
.. include:: module_core_purpose.rst_

*******
grdfill
*******

|grdfill_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdfill** *ingrid*
[ |-A|\ **c**\|\ **n**\|\ **s**\ [*arg*] ]
[ |-G|\ *outgrid* ]
[ |-L|\ [**p**] ]
[ |-N|\ *value* ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdfill** reads a grid that presumably has unfilled holes that the user
wants to fill in some fashion.  Holes are identified by NaN values but
this criteria can be changed via **-N**.  There are several different algorithms that
can be used to replace the hole values.  **Note**: One of **-A** or **-L** is required.

Required Arguments
------------------

*ingrid*
    This is the input grid file.

Optional Arguments
------------------

.. _-A:

**-A**\ **c**\|\ **n**\|\ **s**\ [*arg*]
    Specify the hole-filling algorithm to use.  Choose among **c** for constant
    fill (and append the constant fill *value*), **n** for nearest neighbor (and optionally
    append a search *radius* in pixels [default radius is :math:`r = \sqrt{X^2 + Y^2}`,
    where (*X,Y*) are the node dimensions of the grid]), or
    **s** for bicubic spline (optionally append a *tension* parameter [no tension]).

.. _-G:

**-G**\ *outgrid*
    This is the output grid file.

.. _-N:

**-N**\ [*nodata*]
    Sets the node value used to identify a point as a member of a hole [Default is NaN].

.. |Add_-R| replace:: This defines the subregion to be cut out. |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-L:

**-L**\ [**p**]
    Just list the rectangular subregions west east south north of each hole.
    No grid fill takes place and **-G** is ignored. Optionally, append **p**
    to instead write closed polygons for all subregions.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_coord.rst_

Examples
--------

.. include:: explain_example.rst_

To identify all regions with NaNs in the grid data.grd and create a listing of the
bounding coordinates of rectangular regions that would cover these NaN areas, try::

    gmt grdfill data.grd -L > wesn_listing.txt

To identify the same areas but this time write a multisegment file with polygons
corresponding to the rectangular subregions, use::

    gmt grdfill data.grd -Lp > NaN_regions.txt

To replace all NaN values in the file data.grd with the value 999.0, use::

    gmt grdfill data.grd -Ac999 -Gno_NaNs_data.grd


To replace all NaN values in the file data.grd with the values at the
nearest non-NaN neighbor, try::

    gmt grdfill data.grd -An -Gno_NaNs_NN_data.grd

To replace all NaN values in the file data.grd with a spline interpolation using a tension of 0.2, try::

    gmt grdfill data.grd -As0.2 -Gno_NaNs_spline_data.grd


See Also
--------

:doc:`gmt`,
:doc:`grdcut`,
:doc:`grdclip`,
:doc:`grdedit`,
:doc:`grdinfo`
