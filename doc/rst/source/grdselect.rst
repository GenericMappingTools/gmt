.. index:: ! grdselect
.. include:: module_core_purpose.rst_

*********
grdselect
*********

|grdselect_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdselect** *source1 source2 ...*
[ |-A|\ **i**\|\ **u**\ [**+il**\|\ **h**\|\ *inc*] ]
[ |-C|\ *pointfile* ]
[ |-D|\ *inc* ]
[ |-E|\ [**b**] ]
[ |-F|\ *polygonfile*\ [**+i**\|\ **o**] ]
[ |-G| ]
[ |-I|\ [**C**][**D**][**F**][**L**][**N**][**R**][**W**][**Z**][**r**] ]
[ |-L|\ *linefile* ]
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

**grdselect** reads several images or grids (or cubes) and skips those that fail any tests that may
have been specified.  It then either reports the names of the sources that passed the tests
or determines the intersection or union of the regions of those sources and the range of
data values inside that region. The region may be rounded and padded outwards, then reported
either by its coordinates, a bounding polygon, or a -Rw/e/s/n string.  **Note**: This module
is new in GMT 6.3.0 and is considered *experimental*.

Required Arguments
------------------

.. |Add_ingrid| replace:: The name of one or several 2-D grid, image or 3-D cube files. 
.. include:: explain_grd_inout.rst_
    :start-after: ingrid-syntax-begins
    :end-before: ingrid-syntax-ends

Optional Arguments
------------------

.. _-A:

**-A**\ **i**\|\ **u**\ [**+il**\|\ **h**\|\ *inc*]
    Append either directive **i** for intersection or **u** for union.  We
    then report either the common region for all the data sources that passed the
    tests (if any) or the maximum extent of all the sources that passed the tests.
    Optionally, append modifier **+i** and specify a rounding increment to be applied
    to the final region: Specify **l** for the lowest increment of all passed data
    sources, **h** for the highest increment, or *inc* to set a specific increment.
    If **-A** is not used then we just list the names of the sources that passed
    (or not; see |-I|) any tests selected from |-C| |-D| |-F| |-L| |-N| |-R| |-W| |-Z| |-r|.

.. _-C:

**-C**\ *pointfile*
    Specify a multisegment point file. A data source must contain at least one point
    from *pointfile* to pass [Default is to not consider point inclusions].  **Note**: If data source
    is a 3-D cube then we also check if *pointfile* has *z*-coordinates and if the 3-D
    point is inside the cube [Default is to only check in map view].

.. _-D:

**-D**\ *inc*
    Only pass data sources whose increments match the given increments [Default does not
    consider the increments when passing or skipping data sources].

.. _-E:

**-E**\ [**b**]
    Formats an output data record using tab-separated fields on a single line. The
    columns are *w e s n {b t} v0 v1*. The data in braces only apply if |-Q| is
    used with 3-D data cubes [Default outputs a single -Rw/e/s/n{/b/t} string].
    Alternatively, append directive **b** to output the region's closed polygon.
    **Note**: The *v0 v1* range reflects the full range of all data nodes that lie
    inside the final region after any rounding or padding have taken place.  Also,
    if the grids have different increments then a rounding increment is required
    to be set via **-A+i** unless **-Eb** is used.

.. _-F:

**-F**\ *polygonfile*\ [**+i**\|\ **o**]
    Specify a multisegment closed polygon file. A data source must partially or
    fully overlap with at least one polygon in *polygonfile* to pass [Default is to not consider
    polygonal areas]. To only find data sources that are fully inside or outside the polygon
    append **+i** or **+o**, respectively. **Note**: If data source is a cube then we ignore
    the *z*-dimension (i.e., we only check for overlap in map view).

.. _-G:

**-G**
    Force (possible) download of all tiles of any tiled global remote grids given in order
    to report the requested information [Default is to refuse to give the information for tiled grids].

.. _-I:

**-I**\ [**C**][**D**][**F**][**L**][**N**][**R**][**W**][**Z**][**r**]
    Reverses the sense of the test for each of the options that match the specified code(s):

    - **C** - select data sources *not* containing any of the points specified in |-C|.
    - **D** - select data sources *not* having the specified increment in |-D|.
    - **F** - select data sources *not* overlapping with any of the polygons specified in |-F|.
    - **L** - select data sources *not* traversed by any of the lines specified in |-L|.
    - **N** - select data sources *failing* the NaN criterion in |-N|.
    - **R** - select data sources *not* having any overlap with the region set in |-R|.
    - **W** - select data sources whose data range do *not* overlap the range specified by |-W|.
    - **Z** - select cubes whose z-dimension range do *not* overlap the range specified by |-Z| (requires |-Q|).
    - **r** - select data sources *not* having the specified registration specified by |-r|.

   If no argument is given then we reverse all the tests, i.e, the same as **-ICDFLNRWZr**.

.. _-L:

**-L**\ *linefile*
    Specify a multisegment line file. A data source must be traversed by at least one line
    in *linefile* to pass [Default is no line traversing considered]. **Note**: If data source is a
    cube then we ignore the *z*-dimension (i.e., we only check for crossings in map view).

.. _-M:

**-M**\ *margins*
    Extend the region determined via |-A| by the given *margins*.  These can be specified as
    a single value (use the same margin on all sides), a pair of values separated by slashes
    (set separate *x* and *y* margins), or the full set of four slash-separated margins
    (set separate west, east, south, and north margins) [no region padding]. For geographic
    (lon/lat) grids you may use units **d**, **m**, or **s** as needed.

.. _-N:

**-N**\ **l**\|\ **h**\ [*n*]
    Only pass data sources that have a total number of NaNs that is either **l**\ ower or **h**\ igher than *n* [Default is 0].
    **Note**: Cannot be used with images.

.. _-Q:

**-Q**
    All input files must be data 3-D netCDF data cube files [Default is all files are 2-D grids or images].

.. |Add_-R| replace:: Using the **-R** option will in essence supply another region that will be included in the computation via |-A|, as well as limit the reading to that subset. If |-Q| is used you must also
    append limits in the *z* dimension. |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [*wmin*]\ /[*wmax*]
    Only pass data sources whose data range overlaps with the specified range.  If *wmin* is not
    given it defaults to -infinity, while if *wmax* is not given it defaults to +infinity.
    **Note**: Cannot be used with images.

.. _-Z:

**-Z**\ [*zmin*]\ /[*zmax*]
    Requires |-Q|. Only pass cubes whose *z*-dimension range overlaps with the specified range.  If *zmin* is not
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
    Only pass data sources that have the same registration as that specified [Ignore registration when
    determining which data sources to pass or skip].

.. include:: explain_help.rst_

Examples
--------

To find the extended region (union) of all the grids given, plus a padding of 2 degrees, try::

    gmt grdcut @earth_relief_01d -R1/3/1/3 -Gsubset1.nc
    gmt grdcut @earth_relief_01d -R2/5/2/5 -Gsubset2.nc
    gmt grdselect *.nc -Au -M2

To find the common region (intersection) that all the grids share, try::

    gmt grdselect *.nc -Ai

To find the common region (intersection) that all the grids share but extend it by 2 degrees and then write the bounding polygon, try::

    gmt grdselect *.nc -Ai -M2 -Eb > wesn_polygon.txt

To list all the data sources that have more than 10 NaN nodes and are pixel registered, try::

    gmt grdselect *.nc -Nh10 -rp

To list all the grids that are entirely included by the polygon in wesn_polygon.txt, try::

    gmt grdselect *.nc -Fwesn_polygon.txt+i

See Also
--------

:doc:`gmt`,
:doc:`gmtselect`,
:doc:`grd2xyz`,
:doc:`grdedit`,
:doc:`grdinfo`
