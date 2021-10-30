.. index:: ! grdselect
.. include:: module_core_purpose.rst_

*******
grdselect
*******

|grdselect_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdselect** *grid1 grid2 ...*
[ |-A|\ **i**\|\ **u**\ [**+il**\|\ **h**\|\ *incs*] ]
[ |-C| ]
[ |-G| ]
|SYN_OPT-I|
[ |-I|\ [*dx*\ [/*dy*]\|\ **b**\|\ **i**\|\ **r**] ]
[ |-M|\ *margins* ]
[ |-N|\ **l**\|\ **h**\ [*n*] ]
[ |-Q| ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |-Z|\ [*zmin*\ /*zmax*][**+i**] ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdselect** reads several grids and skips those that fail any tests that
have been specified.  It then either reports the names of the grids that pass
or determines the intersection or union of the regions of those grids.

Required Arguments
------------------

.. |Add_ingrid| replace:: The name of one or several 2-D grid files. 
.. include:: explain_grd_inout.rst_
    :start-after: ingrid-syntax-begins
    :end-before: ingrid-syntax-ends

Optional Arguments
------------------

.. _-A:

**-A**
    Append either directive **i** for intersection or **u** for union.  We
    then report either the common region for all the grids that passes the
    tests (if any) or the maximum extent of all the grids that passed the tests.
    If **-A** is not used then we just list the names of the grids that passed
    the test.

. _-C:

**-C**
    Formats the report using tab-separated fields on a single line. The
    output is *w e s n {b t} v0 v1*. The data in braces only apply if **-Q** is
    used with 3-D data cubes.

.. _-I:

.. include:: explain_-I.rst_

.. _-G:

**-G**
    Force (possible) download of all tiles of tiled global remote grids in order
    to report the requested information [refuse to give the information for tiled grids].

.. _-M:

**-M**\ *margins*
    This is additional space that is added to the region determined via **-A**.  The margins can be specified as
    a single value (for same margin on all sides), a pair of values separated by slashes
    (for setting separate *x* and *y* margins), or the full set of four slash-separated margins
    (for setting separate west, east, south, and north margins) [no region extension].

.. _-N:

**-N**\ **l**\|\ **h**\ [*n*]
    Only pass grids that have a total number of NaNs that is either **l**\ ower or **h**\ igher than *n* [0].

.. _-Q:

**-Q**
    All input files must be data 3-D netCDF data cube files [all files are 2-D grids].

.. |Add_-R| replace:: Using the **-R** option will select a subsection of the input grid(s). If this subsection
    exceeds the boundaries of the grid, only the common region will be extracted. If **-Q** is used you must also
    append limits in the *z* dimension. |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**\ [*zmin*\ /*zmax*][**+i**]
    Only pass grids whose data range overlaps with the stated range. You can invert the
    test by appending **+i** so that only grids outside the stated range are found.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-ocols.rst_

.. _-regg:

**-r**\ [**g**\|\ **p**]
    Onpy pass grids that have the same registration as that specified [no registration requirement].

.. include:: explain_help.rst_

Examples
--------

To find the common region to all the grids given, try::

    gmt grdselect *.grd -Au

To find the extended region that fits all the grids given, try::

    gmt grdselect *.grd -Ai

To list all the files that more than 10 NaN and are pixel registered, try::

    gmt grdselect *.grd -Nh10 -rp

See Also
--------

:doc:`gmt`, :doc:`grd2cpt`,
:doc:`grd2xyz`, :doc:`grdedit`, :doc:`grdinfo`
