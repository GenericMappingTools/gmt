.. index:: ! xyz2grd

*******
xyz2grd
*******

.. only:: not man

    Convert data table to a grid file

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt xyz2grd** [ *table* ] |-G|\ *grdfile*
|SYN_OPT-I|
|SYN_OPT-R|
[ |-A|\ [**d**\ \|\ **f**\ \|\ **l**\ \|\ **m**\ \|\ **n**\ \|\ **r**\ \|\ **S**\ \|\ **s**\ \|\ **u**\ \|\ **z**] ]
[ |-D|\ [**+x**\ *xname*][**+y**\ *yname*][**+z**\ *zname*][**+s**\ *scale*][**+o**\ *offset*][**+n**\ *invalid*][**+t**\ *title*][**+r**\ *remark*] ]
[ |-J|\ *parameters* ]
[ |-S|\ [*zfile*] ]
[ |SYN_OPT-V| ]
[ |-Z|\ [*flags*\ ] ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**xyz2grd** reads one or more z or xyz tables and creates a binary grid
file. **xyz2grd** will report if some of the nodes are not filled in
with data. Such unconstrained nodes are set to a value specified by the
user [Default is NaN]. Nodes with more than one value will be set to the
mean value. As an option (using **-Z**), a 1-column z-table may be read
assuming all nodes are present (z-tables can be in organized in a number
of formats, see **-Z** below.)  Note: **xyz2grd** does not grid the data,
it simply reformats existing data to a grid structure.  For gridding,
see **surface**, **greenspline**, **nearneighbor**, or **triangulate**.

Required Arguments
------------------

.. _-G:

**-G**\ *grdfile*
    *grdfile* is the name of the binary output grid file. (See GRID FILE
    FORMAT below.)

.. _-I:

.. include:: explain_-I.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

Optional Arguments
------------------

*table*
    One or more ASCII [or binary, see **-bi**]
    files holding z or (x,y,z) values. The xyz triplets do not have to
    be sorted. One-column z tables must be sorted and the **-Z** must be set.

.. _-A:

**-A**\ [**d**\ \|\ **f**\ \|\ **l**\ \|\ **m**\ \|\ **n**\ \|\ **r**\ \|\ **S**\ \|\ **s**\ \|\ **u**\ \|\ **z**]
    By default we will calculate mean values if multiple entries fall on
    the same node. Use **-A** to change this behavior, except it is
    ignored if **-Z** is given. Append **f** or **s** to simply keep the
    first or last data point that was assigned to each node. Append
    **l** or **u** or **d** to find the lowest (minimum) or upper (maximum) value
    or the difference between the maximum and miminum value
    at each node, respectively. Append **m** or **r** or **S** to compute mean or
    RMS value or standard deviation at each node, respectively. Append **n** to simply count
    the number of data points that were assigned to each node (this only
    requires two input columns *x* and *y* as *z* is not consulted). Append
    **z** to sum multiple values that belong to the same node.

.. _-D:

.. include:: explain_-D_cap.rst_

.. _-J:

.. |Add_-J| replace:: Use the **-J** syntax to save the georeferencing info as CF-1 compliant metadata in
    netCDF grids. Remember also that PROJ.4 syntax can be used directly in **-J**. This referencing will be
    recognized by GDAL and increasingly also by GMT.
.. include:: explain_-J.rst_

.. _-S:

**-S**\ [*zfile*]
    Swap the byte-order of the input only. No grid file is produced. You
    must also supply the **-Z** option. The output is written to *zfile*
    (or stdout if not supplied).

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-Z:

**-Z**\ [*flags*]
    Read a 1-column ASCII [or binary] table. This assumes that all the
    nodes are present and sorted according to specified ordering
    convention contained in *flags*. If incoming data represents rows,
    make *flags* start with **T**\ (op) if first row is y
    = ymax or **B**\ (ottom) if first row is y = ymin.
    Then, append **L** or **R** to indicate that first element is at
    left or right end of row. Likewise for column formats: start with
    **L** or **R** to position first column, and then append **T** or
    **B** to position first element in a row. Note: These two row/column
    indicators are only required for grids; for other tables they do not
    apply. For gridline registered grids: If data are periodic in x but
    the incoming data do not contain the (redundant) column at x = xmax,
    append **x**. For data periodic in y without redundant row at y =
    ymax, append **y**. Append **s**\ *n* to skip the first *n* number
    of bytes (probably a header). If the byte-order or the words needs
    to be swapped, append **w**. Select one of several data types (all
    binary except **a**):

    **A** ASCII representation of one or more floating point values per record

    **a** ASCII representation of a single item per record

    **c** int8_t, signed 1-byte character

    **u** uint8_t, unsigned 1-byte character

    **h** int16_t, signed 2-byte integer

    **H** uint16_t, unsigned 2-byte integer

    **i** int32_t, signed 4-byte integer

    **I** uint32_t, unsigned 4-byte integer

    **l** int64_t, long (8-byte) integer

    **L** uint64_t, unsigned long (8-byte) integer

    **f** 4-byte floating point single precision

    **d** 8-byte floating point double precision

    Default format is scanline orientation of ASCII numbers: **-ZTLa**.
    Note that **-Z** only applies to 1-column input. The difference
    between **A** and **a** is that the latter can decode both
    *date*\ **T**\ *clock* and *ddd:mm:ss[.xx]* formats while the former
    is strictly for regular floating point values.

.. |Add_-bi| replace:: [Default is 3 input columns]. This option only applies
    to xyz input files; see **-Z** for z tables.
.. include:: explain_-bi.rst_

.. |Add_-di| replace:: Also sets nodes with no input xyz triplet to this value
    [Default is NaN].
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| replace:: Not used with binary data.
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_float.rst_

.. include:: explain_grd_output.rst_

.. include:: explain_grd_coord.rst_

Swapping Limitations
--------------------

All data types can be read, even 64-bit integers, but internally grids
are stored using floats. Hence, integer values exceeding the float
type's 23-bit mantissa may not be represented exactly. When **-S** is
used no grids are implied and we read data into an intermediate double
container. This means all but 64-bit integers can be represented using
the double type's 53-bit mantissa.

Examples
--------

To create a grid file from the ASCII data in hawaii\_grv.xyz, use

   ::

    gmt xyz2grd hawaii_grv.xyz -D+xdegree+ydegree+zGal+t"Hawaiian Gravity"+r"GRS-80 Ellipsoid used"
                -Ghawaii_grv_new.nc -R198/208/18/25 -I5m -V

To create a grid file from the raw binary (3-column, single-precision
scanline-oriented data raw.b, use

   ::

    gmt xyz2grd raw.b -D+xm+ym+zm -Graw.nc -R0/100/0/100 -I1 -V -Z -bi3f

To make a grid file from the raw binary USGS DEM (short integer
scanline-oriented data topo30.b on the NGDC global relief Data CD-ROM,
with values of -9999 indicate missing data, one must on some machine
reverse the byte-order. On such machines (like Sun), use

   ::

    gmt xyz2grd topo30.b -D+xm+ym+zm -Gustopo.nc -R234/294/24/50 -I30s -di-9999 -ZTLhw

Say you have received a binary file with 4-byte floating points that
were written on a machine of different byte-order than yours. You can
swap the byte-order with

   ::

    gmt xyz2grd floats.bin -Snew_floats.bin -V -Zf

To make a pixel node registrated tiff of the number of data points
that is assigned to each node in a cartesian data set, use

   ::

    gmt xyz2grd data.txt -R0/100/0/100 -r -I10 -An -Gnumber_of_points.tif=gd:GTiff

See Also
--------

:doc:`gmt`,
:doc:`grd2xyz`,
:doc:`grdedit`,
:doc:`grdconvert`,
:doc:`greenspline`,
:doc:`nearneighbor`,
:doc:`surface`,
:doc:`triangulate`
