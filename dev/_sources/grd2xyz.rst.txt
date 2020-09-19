.. index:: ! grd2xyz
.. include:: module_core_purpose.rst_

*******
grd2xyz
*******

|grd2xyz_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grd2xyz** *grid*
[ |-C|\ [**f**\|\ **i**] ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |-W|\ [**a**\|\ *weight*] ] [ |-Z|\ [*flags*] ]
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
output z-values without the (x,y) coordinates; see **-Z** below.

Required Arguments
------------------

*grid*
    Names of 2-D binary grid files to be converted. (See GRID FILE
    FORMATS below.)

Optional Arguments
------------------

.. _-C:

**-C**\ [**f**\|\ **i**]
    Replace the x- and y-coordinates on output with the corresponding
    column and row numbers. These start at 0 (C-style counting); append
    **f** to start at 1 (Fortran-style counting). Alternatively, append
    **i** to write just the two columns *index* and *z*, where *index*
    is the 1-D indexing that GMT uses when referring to grid nodes.

.. _-R:

.. |Add_-R| replace:: Using the **-R** option
    will select a subsection of the grid. If this subsection exceeds the
    boundaries of the grid, only the common region will be output.
.. include:: explain_-R.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
..  include:: explain_-V.rst_

.. _-W:

**-W**\ [**a**\|\ *weight*]
    Write out *x,y,z,w*\ , where *w* is the supplied *weight* (or 1 if not
    supplied) [Default writes *x,y,z* only].  Choose **-Wa** to compute
    weights equal to the area each node represents.

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
    only applies to xyz output; see **-Z** for z table output.
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-f| replace:: See also **TIME
    COORDINATES** below. **-h** Output 1 header record based on
    information in the first grid file header. Ignored if binary output
    is selected. [Default is no header].
..  include:: explain_-f.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-qo.rst_

.. include:: explain_-s.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

.. include:: explain_grd_inout_short.rst_

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

Examples
--------

To edit individual values in the 2' by 2' remote AFR.nc file, dump the .nc to ASCII::

    gmt grd2xyz @AFR.nc > AFR.xyz

To write a single precision binary file without the x,y positions from
the remote file @AFR.nc file, using scanline orientation, run::

    gmt grd2xyz @AFR.nc -ZTLf > AFR.b

See Also
--------

:doc:`gmt.conf`, :doc:`gmt`,
:doc:`grdedit`, :doc:`grdconvert`,
:doc:`xyz2grd`
