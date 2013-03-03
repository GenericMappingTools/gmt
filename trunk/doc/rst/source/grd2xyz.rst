*******
grd2xyz
*******

grd2xyz - Convert grid file to data table

`Synopsis <#toc1>`_
-------------------

**grd2xyz** *grid* [ **-C**\ [**f**\ \|\ **i**] ] [ **-N**\ *nodata* ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-V**\ [*level*\ ]
] [ **-W**\ [*weight*\ ] ] [ **-Z**\ [*flags*\ ] ] [
**-bo**\ [*ncols*\ ][*type*\ ] ] [ **-f**\ *colinfo* ] [
**-ho**\ [*n*\ ] ] [ **-o**\ *cols*\ [,*...*] ] [
**-s**\ [*cols*\ ][\ **a**\ \|\ **r**] ]

`Description <#toc2>`_
----------------------

**grd2xyz** reads one or more binary 2-D grid files and writes out
xyz-triplets in ASCII [or binary] format to standard output. Modify the
precision of the ASCII output format by editing the
**FORMAT\_FLOAT\_OUT** parameter in your **gmt.conf** file or use
**--D\_FORMAT**\ =\ *format* on the command line, or choose binary
output using single or double precision storage. As an option you may
output z-values without the (x,y) coordinates; see **-Z** below. 

..  include:: explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

*grid*
    Names of 2-D binary grid files to be converted. (See GRID FILE
    FORMATS below.)

`Optional Arguments <#toc5>`_
-----------------------------

**-C**\ [**f**\ \|\ **i**]
    Replace the x- and y-coordinates on output with the corresponding
    column and row numbers. These start at 0 (C-style counting); append
    **f** to start at 1 (Fortran-style counting). Alternatively, append
    **i** to write just the two columns *index* and *z*, where *index*
    is the 1-D indexing that GMT users when referring to grid nodes.
**-N**
    Output this z-value where the latter equals NaN [Default writes NaN]. 

.. |Add_-R| replace:: Using the **-R** option
    will select a subsection of the grid. If this subsection exceeds the
    boundaries of the grid, only the common region will be output. 
.. include:: explain_-R.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
..  include:: explain_-V.rst_

**-W**\ [*weight*\ ]
    Write out x,y,z,w, where w is the supplied *weight* (or 1 if not
    supplied) [Default writes x,y,z only].
**-Z**\ [*flags*\ ]
    Write a 1-column ASCII [or binary] table. Output will be organized
    according to the specified ordering convention contained in *flags*.
    If data should be written by rows, make *flags* start with
    `**T**\ (op) <T.op.html>`_ if first row is y = ymax or
    `**B**\ (ottom) <B.ottom.html>`_ if first row is y = ymin. Then,
    append **L** or **R** to indicate that first element should start at
    left or right end of row. Likewise for column formats: start with
    **L** or **R** to position first column, and then append **T** or
    **B** to position first element in a row. For gridline registered
    grids: If grid is periodic in x but the outcoming data should not
    contain the (redundant) column at x = xmax, append **x**. For grid
    periodic in y, skip writing the redundant row at y = ymax by
    appending **y**. If the byte-order needs to be swapped, append
    **w**. Select one of several data types (all binary except **a**):

    **a** ASCII representation of a single item per record

    **c** int8\_t, signed 1-byte character

    **u** uint8\_t, unsigned 1-byte character

    **h** int16\_t, short 2-byte integer

    **H** uint16\_t, unsigned short 2-byte integer

    **i** int32\_t, 4-byte integer

    **I** uint32\_t, unsigned 4-byte integer

    **l** int64\_t, long (8-byte) integer

    **L** uint64\_t, unsigned long (8-byte) integer

    **f** 4-byte floating point single precision

    **d** 8-byte floating point double precision

    Default format is scanline orientation of ASCII numbers: **-ZTLa**.
    Note that **-Z** only applies to 1-column output. 

.. |Add_-bo| replace:: [Default is 3]. This option
    only applies to xyz output; see **-Z** for z table output. 
.. include:: explain_-bo.rst_

.. |Add_-f| replace:: See also **TIME
    COORDINATES** below. **-h** Output 1 header record based on
    information in the first grid file header. Ignored if binary output
    is selected. [Default is no header]. 
..  include:: explain_-f.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-s.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

.. include:: explain_grd_input.rst_

`Time Coordinates <#toc8>`_
---------------------------

Time coordinates in netCDF grids, be it the x, y, or z coordinate, will
be recognized as such. The variable’s BD(unit) attribute is parsed to
determine the unit and epoch of the time coordinate in the grid. Values
are then converted to the internal time system specified by
**TIME\_UNIT** and **TIME\_EPOCH** in the **gmt.conf** file or on the
command line. The default output is relative time in that time system,
or absolute time when using the option **-f0T**, **-f1T**, or **-f2T**
for x, y, or z coordinate, respectively.

`Examples <#toc9>`_
-------------------

To edit individual values in the 5’ by 5’ hawaii\_grv.nc file, dump the
.nc to ASCII:

grd2xyz hawaii\_grv.nc > hawaii\_grv.xyz

To write a single precision binary file without the x,y positions from
the file raw\_data.nc file, using scanline orientation, run

grd2xyz raw\_data.nc -ZTLf > hawaii\_grv.b

`See Also <#toc10>`_
--------------------

`gmt.conf <gmt.conf.html>`_ , `gmt <gmt.html>`_ ,
`grdedit <grdedit.html>`_ ,
`grdreformat <grdreformat.html>`_ ,
`xyz2grd <xyz2grd.html>`_
