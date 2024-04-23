.. index:: ! grd2xyz
.. include:: module_core_purpose.rst_

*******
grd2xyz
*******

|grd2xyz_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grd2xyz** *ingrid*
[ |-C|\ [*section*/]\ *master*\|\ *cpt*\|\ *color*\ :math:`_1`,\ *color*\ :math:`_2`\ [,\ *color*\ :math:`_3`\ ,...]\ [**+h**\ [*hinge*]][**+i**\ *dz*][**+u**\|\ **U**\ *unit*][**+s**\ *fname*] ]
[ |-F|\ [**f**\|\ **i**] ]
[ |-L|\ [**c**\|\ **r**\|\ **x**\|\ **y**]\ *value* ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |-T|\ [**a**\|\ **b**][*base*] ]
[ |-W|\ [**a**\ [**+u**\ *unit*]\|\ *weight*] ] [ |-Z|\ [*flags*] ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-f| ]
[ **-ho**\ [*n*] ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-qo| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grd2xyz** reads one or more binary 2-D grid files and writes out
xyz-triplets in ASCII [or binary] format to standard output. Modify the
precision of the ASCII output format by editing the
:term:`FORMAT_FLOAT_OUT` parameter in your :doc:`gmt.conf` file or use
**--FORMAT_FLOAT_OUT**\ =\ *format* on the command line, or choose binary
output using single or double precision storage. As an option you may
output z-values without the (*x, y*) coordinates (see |-Z| below) or you can
save the grid in the STL format for 3-D printers with |-T|. Also, by giving a CPT via
|-C| we will add *r*, *g*, *b*, *a* columns to the output based on *z* values.

Required Arguments
------------------

.. |Add_ingrid| replace:: Names of 2-D binary grid files to be converted.
.. include:: explain_grd_inout.rst_
    :start-after: ingrid-syntax-begins
    :end-before: ingrid-syntax-ends

Optional Arguments
------------------

.. _-C:

.. include:: dump_rgb.rst_

.. _-F:

**-F**\ [**f**\|\ **i**]
    Replace the x- and y-coordinates on output with the corresponding
    column and row numbers. These start at 0 (C-style counting); append
    **f** to start at 1 (FORTRAN-style counting). Alternatively, append
    **i** to write just the two columns *index* and *z*, where *index*
    is the 1-D indexing that GMT uses when referring to grid nodes.

.. _-L:

**-L**\ **c**\|\ **r**\|\ **x**\|\ **y**]\ *value*
    Limit the output of records to a single row or column.  Identify the desired
    vector either by *row* or *column* number (via directives **c** or **r**), or by the
    constant *x* or *y* value (via directives **x** or **y**).  If your selection is outside
    the valid range then no output will result and a warning is issued.  **Note**: For
    directives **x** and **y** we find the nearest column or row, respectively.

.. |Add_-R| replace:: Using the |-R| option will select a subsection of the grid. If this subsection exceeds the
    boundaries of the grid, only the common region will be output. |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-T:

**-T**\ [**a**\|\ **b**][*base*]
    Write STL triangulation for 3-D printing to standard output.  By default (or via **-Ta**) we write an STL ASCII file.
    Append **b** to instead write the STL binary (little-endian) format. For more information on STL, see the
    `STL overview on Wikipedia <https://en.wikipedia.org/wiki/STL_(file_format)>`_.  **Note**: All coordinates are
    adjusted so that *xmin = ymin = zmin = 0*.  For other adjustments, see :doc:`grdedit`, :doc:`grdproject` and :doc:`grdmath`.
    Optionally, append a lower *base* other than the grid's minimum value [Default]. **Note**: The grid must be free
    of NaN values.  If your grid contains NaNs then we automatically replace these with the minimum value in the grid;
    use :doc:`grdmath` to pre-process the grid if you wish to select another value.

.. figure:: /_images/GMT_STL.jpg
   :width: 600 px
   :align: center

   3-D print of Vailulu’u crater multibeam data (2006, R/V Kilo Moana off Samoa) via a GMT STL file.  Original
   multibeam data processed with `MB-System <https://www.mbari.org/technology/mb-system/>`_ seen
   on the right. Photos courtesy of Jasper Konter, U of Hawaii at Manoa.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [**a**\ [**+u**\ *unit*]\|\ *weight*]
    Write out *x, y, z, w*\ , where *w* is the supplied *weight* (or 1 if not
    supplied) [Default writes *x, y, z* only].  Choose **-Wa** to compute
    weights equal to the area each node represents.  For Cartesian grids this
    is simply the product of the *x* and *y* increments (except for
    gridline-registered grids at all sides [half] and corners [quarter]).
    For geographic grids we default to a length unit of **k** (hence area is in km\ :sup:`2`). Change
    this by appending **+u**\ *unit* (see `Units`_). For such grids, the area
    varies with latitude and also sees special cases for gridline-registered layouts
    at sides, corners, and poles.

.. _-Z:

**-Z**\ [*flags*]
    Write a 1-column ASCII [or binary] table. Output will be organized
    according to the specified ordering convention contained in *flags*.
    If data should be written by rows, make *flags* start with
    **T** (op) if first row is y = ymax or
    **B** (ottom) if first row is y = ymin. Then,
    append **L** or **R** to indicate that first element should start at
    left or right end of row. Likewise for column formats: start with
    **L** or **R** to position first column, and then append **T** or
    **B** to position first element in a row. For gridline registered
    grids: If grid is periodic in x but the written data should not
    contain the (redundant) column at x = xmax, append **x**. For grid
    periodic in y, skip writing the redundant row at y = ymax by
    appending **y**. If the byte-order needs to be swapped, append
    **w**. Select one of several data types (all binary except **a**):

    * **a** ASCII representation of a single item per record
    * **c** int8_t, signed 1-byte character
    * **u** uint8_t, unsigned 1-byte character
    * **h** int16_t, short 2-byte integer
    * **H** uint16_t, unsigned short 2-byte integer
    * **i** int32_t, 4-byte integer
    * **I** uint32_t, unsigned 4-byte integer
    * **l** int64_t, long (8-byte) integer
    * **L** uint64_t, unsigned long (8-byte) integer
    * **f** 4-byte floating point single precision
    * **d** 8-byte floating point double precision

    Default format is scanline orientation of ASCII numbers: **-ZTLa**.

.. |Add_-bo| replace:: [Default is 3]. This option
    only applies to xyz output; see |-Z| for z table output.
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-f| replace:: See also **TIME
    COORDINATES** below. **-h** Output 1 header record based on
    information in the first grid file header. Ignored if binary output
    is selected. [Default is no header].
..  include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-qo.rst_

.. include:: explain_-s.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_

Non-equidistant x/y Coordinates
-------------------------------

In general, GMT modules cannot accept grids with variable *x* and/or *y* coordinates as most
algorithms and plotting options expect equidistant grids.  However, you can use **grd2xyz**
to dump the original *x y z* triplets and then reprocess the data onto an equidistant
lattice via :doc:`greenspline`, :doc:`nearneighbor` or :doc:`surface`, for instance.

Time Coordinates
----------------

Time coordinates in netCDF grids, be it the x, y, or z coordinate, will
be recognized as such. The variable's **unit** attribute is parsed to
determine the unit and epoch of the time coordinate in the grid. Values
are then converted to the internal time system specified by
:term:`TIME_UNIT` and :term:`TIME_EPOCH` in the
:doc:`gmt.conf` file or on the
command line. The default output is relative time in that time system,
or absolute time when using the option **-f0T**, **-f1T**, or **-f2T**
for x, y, or z coordinate, respectively.

Row Order
---------

The **-Lr** option allows you to output a specific row in the grid. Note that while
a grid's y-coordinates are positive up, internal row numbers are scanline numbers
and hence positive down.  Therefore, the first row (0) coincides with the largest *y*-value.
This means that **-Lr**\ *0* and **-Ly**\ *ymax* (for the correct maximum y-value)
will yield the same result.  In contrast, both *x* and column numbers are positive to the right,
with **-Lc**\ *0* and **-Lx**\ *xmin* (for the correct minimum x-value) yielding the same output.

Examples
--------

To edit individual values in the 2' by 2' remote AFR.nc file, dump the .nc to ASCII:

::

  gmt grd2xyz @AFR.nc > AFR.xyz

To write a single precision binary file without the *x, y* positions from
the remote file @AFR.nc file, using scanline orientation, run:

::

  gmt grd2xyz @AFR.nc -ZTLf > AFR.b

To write out *lon, lat, topo, area* from the @AFR.nc file, selecting meter\ :sup:`2` as the area unit,
and where *area* reflects the size of each grid box, run:

::

  gmt grd2xyz @AFR.nc -Wa+ue > AFR.txt

See Also
--------

:doc:`gmt.conf`, :doc:`gmt`,
:doc:`grdedit`, :doc:`grdconvert`,
:doc:`xyz2grd`
