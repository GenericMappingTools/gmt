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
[ |-D|\ [*dx*\ [/*dy*]] ]
[ |-G| ]
[ |-I|\ [**dnrwz**] ]
[ |-M|\ *margins* ]
[ |-N|\ **l**\|\ **h**\ [*n*] ]
[ |-Q| ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*wmin*\ /*wmax*] ]
[ |-Z|\ [*zmin*\ /*zmax*]] ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdselect** reads several grids (or cubes) and skips those that fail any tests that
have been specified.  It then either reports the names of the sources that passed the tests
or determines the intersection or union of the regions of those sources.

Required Arguments
------------------

.. |Add_ingrid| replace:: The name of one or several 2-D grid or 3-D cube files. 
.. include:: explain_grd_inout.rst_
    :start-after: ingrid-syntax-begins
    :end-before: ingrid-syntax-ends

Optional Arguments
------------------

.. _-A:

**-A**
    Append either directive **i** for intersection or **u** for union.  We
    then report either the common region for all the grids (or cubes) that passes the
    tests (if any) or the maximum extent of all the sources that passed the tests.
    If **-A** is not used then we just list the names of the sources that passed
    the test.

. _-C:

**-C**
    Formats the report using tab-separated fields on a single line. The
    output is *w e s n {b t} v0 v1*. The data in braces only apply if **-Q** is
    used with 3-D data cubes.  **Note**: The *v0 v1* range reflects the full range of the
    data values and not just for the nodes inside the determined region.

.. _-D:

**-D**\ *xinc*\ [**+e**\|\ **n**][/\ *yinc*\ [**+e**\|\ **n**]]
    Only pass grids (or cubes) whose spacings match the given spacing.
    Optionally append a suffix modifier.
    **Geographical (degrees) coordinates**: Append
    **m** to indicate arc minutes or **s** to indicate arc seconds. If one
    of the units **e**, **f**, **k**, **M**, **n** or **u** is appended
    instead, the increment is assumed to be given in meter, foot, km, Mile,
    nautical mile or US survey foot, respectively, and will be converted to
    the equivalent degrees longitude at the middle latitude of the region
    (the conversion depends on :term:`PROJ_ELLIPSOID`). If *y_inc* is given
    but set to 0 it will be reset equal to *x_inc*; otherwise it will be
    converted to degrees latitude. **All coordinates**: If **+e** is appended
    then the corresponding max *x* (*east*) or *y* (*north*) may be slightly
    adjusted to fit exactly the given increment [by default the increment
    may be adjusted slightly to fit the given domain]. Finally, instead of
    giving an increment you may specify the *number of nodes* desired by
    appending **+n** to the supplied integer argument; the increment is then
    recalculated from the number of nodes and the domain. The resulting
    increment value depends on whether you have selected a
    gridline-registered or pixel-registered grid; see :ref:`GMT File Formats` for
    details.

.. _-G:

**-G**
    Force (possible) download of all tiles of tiled global remote grids in order
    to report the requested information [refuse to give the information for tiled grids].

.. _-I:

**-I**\ [**dnrwz**]
    Reverses the sense of the test for each of the items specified:

    - **d** - select data sources NOT having the specified increment in **-D**.
    - **n** - select data sources failing the NaN criterion in **-N**.
    - **r** - select data sources NOT having the specified registration in **-r**.
    - **w** - select data sources whose data are NOT within the range specified by **-W**.
    - **z** - select cubes whose z-levels are NOT within the range specified by **-Z** (requires **-Q**).

.. _-M:

**-M**\ *margins*
    Extend the region determined via **-A** by the *margins*.  The *margins* can be specified as
    a single value (the same margin on all sides), a pair of values separated by slashes
    (setting separate *x* and *y* margins), or the full set of four slash-separated margins
    (for setting separate west, east, south, and north margins) [no region extension].

.. _-N:

**-N**\ **l**\|\ **h**\ [*n*]
    Only pass data sources that have a total number of NaNs that is either **l**\ ower or **h**\ igher than *n* [0].

.. _-Q:

**-Q**
    All input files must be data 3-D netCDF data cube files [all files are 2-D grids].

.. |Add_-R| replace:: Using the **-R** option will select a subsection of the input grid(s). If this subsection
    exceeds the boundaries of the grid, only the common region will be extracted. If **-Q** is used you must also
    append limits in the *z* dimension. |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [*wmin*\ /*wmax*]
    Only pass data sources whose data range overlaps with the stated range.  If *wmin* is not
    given it defaults to -infinity, while if *wmax* is not given it defaults to +infinity.

.. _-Z:

**-Z**\ [*zmin*\ /*zmax*]
    Requires **-Q**. Only pass cubes whose z-range overlaps with the stated range.  If *zmin* is not
    given it defaults to -infinity, while if *zmax* is not given it defaults to +infinity.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-ocols.rst_

.. _-reg:

**-r**\ [**g**\|\ **p**]
    Only pass data sources that have the same registration as that specified [no registration requirement].

.. include:: explain_help.rst_

Examples
--------

To find the extended region (union) of all the grids given, try::

    gmt grdselect *.grd -Au

To find the common region (intersection) that all the grids share, try::

    gmt grdselect *.grd -Ai

To list all the files that have more than 10 NaN nodes and are pixel registered, try::

    gmt grdselect *.grd -Nh10 -rp

See Also
--------

:doc:`gmt`, :doc:`grd2cpt`,
:doc:`grd2xyz`, :doc:`grdedit`, :doc:`grdinfo`
